#include "Application.hpp"

std::vector<const char*> g_deviceExtensions = {
	vk::KHRSwapchainExtensionName,
	vk::KHRSpirv14ExtensionName,
	vk::KHRSynchronization2ExtensionName,
	vk::KHRCreateRenderpass2ExtensionName
};

static std::vector<char> readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file.");

	std::vector<char> buffer(file.tellg());
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
	file.close();

	return buffer;
}

void Application::run(void)
{
	_initWindow();
	_initVulkan();
	_mainLoop();
	_cleanup();
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Application::debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *)
{
	if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		std::cerr << "validation layer: type " << to_string(type) << " msg: "
			<< pCallbackData->pMessage << std::endl;
	return vk::False;
}

void Application::_initWindow(void)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
		throw (std::runtime_error("Unable to initialize SDL"));

	_sdl_window = SDL_CreateWindow("ShaderTool", 1920, 1080, SDL_WINDOW_VULKAN);
	if (_sdl_window == nullptr)
		throw (std::runtime_error("Failed to create SDL Window (Vulkan)"));
}

void Application::_initVulkan(void)
{
	_createVKInstance();
	_setupDebugMessenger();
	_createSurface();
	_pickPhysicalDevice();
	_createLogicalDevice();
	_createSwapChain();
	_createImageViews();
	_createGraphicsPipeline();
	_createCommandPool();
	_createCommandBuffer();
}

void Application::_setupDebugMessenger(void)
{
	if (!enableValidationLayers)
		return ;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
			.messageSeverity = severityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &debugCallback
	};
	_debugMessenger = _vkInstance.createDebugUtilsMessengerEXT(
			debugUtilsMessengerCreateInfoEXT);
}

void Application::_createSurface(void)
{
	VkSurfaceKHR raw_surface;
	if (!SDL_Vulkan_CreateSurface(_sdl_window, *_vkInstance, nullptr, &raw_surface))
		throw (std::runtime_error("Failed to create window surface"));
	_surface = vk::raii::SurfaceKHR(_vkInstance, raw_surface);
	std::cout << "Backend: " << SDL_GetCurrentVideoDriver() << std::endl;
}

void Application::_pickPhysicalDevice(void)
{
	auto devices = _vkInstance.enumeratePhysicalDevices();
	if (devices.empty())
	{
		throw (std::runtime_error("Failed to find GPUs with Vulkan support"));
	}

	std::multimap<int, vk::raii::PhysicalDevice> candidates;

	for (const auto &device : devices)
	{
		auto deviceProperties = device.getProperties();
		auto deviceFeatures = device.getFeatures();
		auto queueFamilies = device.getQueueFamilyProperties();

		bool isSuitable = deviceProperties.apiVersion >= VK_API_VERSION_1_3;
		const auto qfpIter = std::ranges::find_if(queueFamilies,
		[]( vk::QueueFamilyProperties const & qfp)
		{
			return ((qfp.queueFlags & vk::QueueFlagBits::eGraphics)
					!= static_cast<vk::QueueFlags>(0));
		});
		isSuitable = isSuitable && (qfpIter != queueFamilies.end());

		auto extensions = device.enumerateDeviceExtensionProperties();
		bool found = true;
		for (auto const &extension : g_deviceExtensions)
		{
			auto extensionIter = std::ranges::find_if(extensions, [extension](auto const &ext)
					{return strcmp(ext.extensionName, extension) == 0;});
			found = found && extensionIter != extensions.end();
		}
		isSuitable = isSuitable && found;

		Uint32 score = 0;
		if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 1000;
		score += deviceProperties.limits.maxImageDimension2D;
		if (!deviceFeatures.geometryShader || !isSuitable)
			continue ;
		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0)
	{
		_physicalDevice = candidates.rbegin()->second;
		auto properties = _physicalDevice.getProperties();
		std::cout << "Chosen GPU: " << properties.deviceName << std::endl;
	}
	else
		throw (std::runtime_error("Failed to find a suitable GPU"));
}

void Application::_createLogicalDevice(void)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
		_physicalDevice.getQueueFamilyProperties();

	auto graphicsQueueFamilyProperty = std::ranges::find_if(
			queueFamilyProperties,
			[](auto const &qfp)
			{
				return (qfp.queueFlags & vk::QueueFlagBits::eGraphics)
					!= static_cast<vk::QueueFlags>(0); 
			}
	);
	auto graphicsIndex = static_cast<uint32_t>(
			std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));

	auto presentIndex = _physicalDevice.getSurfaceSupportKHR(graphicsIndex, *_surface)
		? graphicsIndex : static_cast<uint32_t>(queueFamilyProperties.size());
	if (presentIndex == queueFamilyProperties.size())
	{
		for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
					&& _physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), *_surface))
			{
				graphicsIndex = static_cast<uint32_t>(i);
				presentIndex = graphicsIndex;
				break ;
			}
		}
		if (presentIndex == queueFamilyProperties.size())
		{
			for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
			{
				presentIndex = static_cast<uint32_t>(i);
				break ;
			}
		}
	}
	if ((graphicsIndex == queueFamilyProperties.size())
				|| (presentIndex == queueFamilyProperties.size()))
		throw (std::runtime_error("Could not find a queue for graphics or present"));

	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = graphicsIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	vk::PhysicalDeviceFeatures2 deviceFeatures2;
	vk::PhysicalDeviceVulkan13Features vk13;
	vk13.dynamicRendering = true;
	vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT ext;
	ext.extendedDynamicState = true;
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain(
				deviceFeatures2, vk13, ext);

	vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(g_deviceExtensions.size()),
		.ppEnabledExtensionNames = g_deviceExtensions.data()
	};

	_device = vk::raii::Device(_physicalDevice, deviceCreateInfo);
	_graphicsQueue = vk::raii::Queue(_device, graphicsIndex, 0);
	_presentQueue = vk::raii::Queue(_device, presentIndex, 0);
}

void Application::_createSwapChain(void)
{
	auto surfaceCapabilities = _physicalDevice.getSurfaceCapabilitiesKHR(_surface);
	std::vector<vk::SurfaceFormatKHR> availableFormats = _physicalDevice.getSurfaceFormatsKHR(_surface);
	std::vector<vk::PresentModeKHR> availablePresentModes = _physicalDevice.getSurfacePresentModesKHR(_surface);
	_swapChainSurfaceFormat = _chooseSwapSurfaceFormat(availableFormats);
	_swapChainExtent = _chooseSwapExtent(surfaceCapabilities);

	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
	minImageCount = (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) ? surfaceCapabilities.maxImageCount : minImageCount;
	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
		imageCount = surfaceCapabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.flags = vk::SwapchainCreateFlagsKHR(),
		.surface = _surface,
		.minImageCount = minImageCount,
		.imageFormat = _swapChainSurfaceFormat.format,
		.imageColorSpace = _swapChainSurfaceFormat.colorSpace,
		.imageExtent = _swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = _chooseSwapPresentMode(_physicalDevice.getSurfacePresentModesKHR(_surface)),
		.clipped = true,
		.oldSwapchain = nullptr
	};
	
	_swapChain = vk::raii::SwapchainKHR(_device, swapChainCreateInfo);
	_swapChainImages = _swapChain.getImages();
}

void Application::_createImageViews(void)
{
	_swapChainImageViews.clear();

	vk::ImageViewCreateInfo createInfo{
		.viewType = vk::ImageViewType::e2D,
		.format = _swapChainSurfaceFormat.format,
		.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};
	for (auto image : _swapChainImages)
	{
		createInfo.image = image;
		_swapChainImageViews.emplace_back(_device, createInfo);
	}
}

void Application::_createGraphicsPipeline(void)
{
	vk::raii::ShaderModule shaderModule = _createShaderModule(readFile("shaders/slang.spv"));

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shaderModule,
		.pName = "vertMain"
	};
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shaderModule,
		.pName = "fragMAin"
	};
	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
	vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f
	};

	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};
	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False,
		.logicOp = vk::LogicOp::eCopy,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	std::vector dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
	vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	_pipelineLayout = vk::raii::PipelineLayout(_device, pipelineLayoutInfo);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain =
	{
		{
			.stageCount = 2,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = _pipelineLayout,
			.renderPass = nullptr
		},
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &_swapChainSurfaceFormat.format
		}
	};

	_graphicsPipeline = vk::raii::Pipeline(_device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

vk::raii::ShaderModule Application::_createShaderModule(const std::vector<char> &code) const
{
	vk::ShaderModuleCreateInfo createInfo{
			.codeSize = code.size() * sizeof(uint32_t),
			.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};
	vk::raii::ShaderModule shaderModule{_device, createInfo};
	return shaderModule;
}

void Application::_createCommandPool(void)
{
	vk::CommandPoolCreateInfo poolInfo{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = graphicsIndex
	};
	_commandPool = vk::raii::CommandPool(_device, poolInfo);
}

void Application::_createCommandBuffer(void)
{
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = _commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};
	_commandBuffer = std::move(vk::raii::CommandBuffers(_device, allocInfo).front());
}

vk::SurfaceFormatKHR Application::_chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == vk::Format::eR8G8B8A8Srgb
				&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			return (availableFormat);
	}
	return (availableFormats[0]);
}

vk::PresentModeKHR Application::_chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
			return (availablePresentMode);
	}
	return (vk::PresentModeKHR::eFifo);
}

vk::Extent2D Application::_chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return (capabilities.currentExtent);
	int width, height;
	SDL_GetWindowSizeInPixels(_sdl_window, &width, &height);
	return
	{
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

void Application::_createVKInstance(void)
{
	constexpr vk::ApplicationInfo appInfo{
			.pApplicationName = "ShaderTool",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0)
	};
	
	auto available = _vkContext.enumerateInstanceExtensionProperties();
	std::cout << "Available extensions:" << std::endl;
	for (const auto &extension : available)
		std::cout << "- " << extension.extensionName << std::endl;

	std::vector<char const *> requiredLayers;
	if (enableValidationLayers)
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	auto layerProperties = _vkContext.enumerateInstanceLayerProperties();
	if (std::ranges::any_of(requiredLayers,
				[&layerProperties](auto const &requiredLayer) {
				return std::ranges::none_of(layerProperties,
						[requiredLayer](auto const &layerProperty)
						{ return strcmp(layerProperty.layerName, requiredLayer) == 0;});
				}))
		throw (std::runtime_error("One or more required layers are not supported"));

	Uint32 extension_count;
	const char * const *instance_extensions = SDL_Vulkan_GetInstanceExtensions(
			&extension_count);
	if (instance_extensions == nullptr)
		throw (std::runtime_error("Failed to get Vulkan instance extensions"));
	std::vector<const char*> extensions;
	extensions.reserve(extension_count + 1);
	for (Uint32 i = 0; i < extension_count; ++i)
		extensions.push_back(instance_extensions[i]);
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	vk::InstanceCreateInfo create_info{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = extension_count + 1,
		.ppEnabledExtensionNames = extensions.data()
	};
	_vkInstance = vk::raii::Instance(_vkContext, create_info);
}

void Application::_mainLoop(void)
{
	SDL_Event event;

	std::cout << "Main loop" << std::endl;
	while (_running)
	{
		while (SDL_PollEvent(&event))
		{
			if ((event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
					|| event.type == SDL_EVENT_QUIT)
				_running = false;
		}
	}
}

void Application::_cleanup(void)
{
	SDL_DestroyWindow(_sdl_window);
	SDL_Quit();
}
