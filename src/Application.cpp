#include "Application.hpp"

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
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT(
			{},
			severityFlags,
			messageTypeFlags,
			&debugCallback
	);
	_debugMessenger = _vkInstance.createDebugUtilsMessengerEXT(
			debugUtilsMessengerCreateInfoEXT);
}

void Application::_createVKInstance(void)
{
	constexpr VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = "ShaderTool",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "No Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_API_VERSION(0, 1, 4, 0)};
	
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

	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = extension_count + 1,
		.ppEnabledExtensionNames = extensions.data()};
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
				return ;
		}
	}
}

void Application::_cleanup(void)
{
	SDL_DestroyWindow(_sdl_window);
	SDL_Quit();
}
