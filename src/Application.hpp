#pragma once
#ifndef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
# define VK_EXT_DEBUG_REPORT_EXTENSION_NAME "VK_EXT_debug_report"
#endif

#include <stb_image.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <iostream>
#include <map>
#include <fstream>
#include <chrono>

#include "Vertex.hpp"
#include "UniformBufferObject.hpp"

const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif
constexpr int MAX_FRAMES_IN_FLIGHT = 2;
const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

class Application
{
	public:
		void run(void);
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
				vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
				vk::DebugUtilsMessageTypeFlagsEXT type,
				const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
				void *);
	private:
		void _initWindow(void);
		void _initVulkan(void);
		void _setupDebugMessenger(void);
		void _createSurface(void);
		void _pickPhysicalDevice(void);
		void _createLogicalDevice(void);

		void _createSwapChain(void);
		void _recreateSwapChain(void);
		void _cleanupSwapChain(void);

		void _createImageViews(void);
		void _createDescriptorSetLayout(void);
		void _createGraphicsPipeline(void);
		[[nodiscard]]vk::raii::ShaderModule _createShaderModule(const std::vector<char> &code) const;
		void _createCommandPool(void);
		void _createBuffer(
				vk::DeviceSize size,
				vk::BufferUsageFlags usage,
				vk::MemoryPropertyFlags properties,
				vk::raii::Buffer& buffer,
				vk::raii::DeviceMemory& bufferMemory
				);
		void _createTextureImage(void);
		void _createTextureImageView(void);
		void _createTextureSampler(void);
		vk::raii::ImageView _createImageView(vk::raii::Image& image, vk::Format format);
		void _createImage(
				uint32_t width,
				uint32_t height,
				vk::Format format,
				vk::ImageTiling tiling,
				vk::ImageUsageFlags usage,
				vk::MemoryPropertyFlagBits properties,
				vk::raii::Image &image,
				vk::raii::DeviceMemory &imageMemory
				);
		void _createVertexBuffer(void);
		void _createIndexBuffer(void);
		void _createUniformBuffers(void);
		void _createDescriptorPool(void);
		void _createDescriptorSets(void);
		void _updateUniformBuffer(uint32_t currentImage);
		void _copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size);
		void _copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);
		vk::raii::CommandBuffer _beginSingleTimeCommands(void);
		void _endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer);
		uint32_t _findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		void _createCommandBuffers(void);
		void _recordCommandBuffer(uint32_t imageIndex);
		void _transitionImageLayout(
				uint32_t imageIndex,
				vk::ImageLayout oldLayout,
				vk::ImageLayout newLayout,
				vk::AccessFlags2 srcAccessMask,
				vk::AccessFlags2 dstAccessMask,
				vk::PipelineStageFlags2 srcStageMask,
				vk::PipelineStageFlags2 dstStageMask
				);
		void _transition_image_layout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void _createSyncObjects(void);
		vk::SurfaceFormatKHR _chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
		vk::PresentModeKHR _chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
		vk::Extent2D _chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
		void _createVKInstance(void);
		void _mainLoop(void);
		void _drawFrame(void);
		void _cleanup(void);

		SDL_Window *_sdl_window = nullptr;
		vk::raii::Context _vkContext;
		vk::raii::Instance _vkInstance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
		vk::raii::SurfaceKHR _surface = nullptr;

		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;
		vk::raii::Queue _queue = nullptr;
		uint32_t _queueIndex = 0;

		vk::raii::DescriptorSetLayout _descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout _pipelineLayout = nullptr;
		vk::raii::Pipeline _graphicsPipeline = nullptr;

		vk::raii::Buffer _vertexBuffer = nullptr;
		vk::raii::DeviceMemory _vertexBufferMemory = nullptr;
		vk::raii::Buffer _indexBuffer = nullptr;
		vk::raii::DeviceMemory _indexBufferMemory = nullptr;

		std::vector<vk::raii::Buffer> _uniformBuffers;
		std::vector<vk::raii::DeviceMemory> _uniformBuffersMemory;
		std::vector<void*> _uniformBuffersMapped;

		vk::raii::DescriptorPool _descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> _descriptorSets;

		vk::raii::CommandPool _commandPool = nullptr;
		std::vector<vk::raii::CommandBuffer> _commandBuffers;

		std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
		std::vector<vk::raii::Fence> _inFlightFences;

		vk::raii::SwapchainKHR _swapChain = nullptr;
		std::vector<vk::Image> _swapChainImages;
		vk::SurfaceFormatKHR _swapChainSurfaceFormat;
		vk::Extent2D _swapChainExtent;
		std::vector<vk::raii::ImageView> _swapChainImageViews;

		vk::raii::Image _textureImage = nullptr;
		vk::raii::DeviceMemory _textureImageMemory = nullptr;
		vk::raii::ImageView _textureImageView = nullptr;
		vk::raii::Sampler _textureSampler = nullptr;

		bool _running = true;
		bool _frameBufferResized = false;
		uint32_t _frameIndex = 0;
};
