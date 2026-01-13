#pragma once
#ifndef VK_EXT_DEBUG_REPORT_EXTENSION_NAME
# define VK_EXT_DEBUG_REPORT_EXTENSION_NAME "VK_EXT_debug_report"
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <vector>
#include <iostream>
#include <map>
#include <fstream>

const std::vector<char const*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

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
		void _createImageViews(void);
		void _createGraphicsPipeline(void);
		[[nodiscard]]vk::raii::ShaderModule _createShaderModule(const std::vector<char> &code) const;
		vk::SurfaceFormatKHR _chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const &availableFormats);
		vk::PresentModeKHR _chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
		vk::Extent2D _chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
		void _createVKInstance(void);
		void _mainLoop(void);
		void _cleanup(void);

		SDL_Window *_sdl_window = nullptr;
		vk::raii::Context _vkContext;
		vk::raii::Instance _vkInstance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
		vk::raii::SurfaceKHR _surface = nullptr;
		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;
		vk::raii::Queue _graphicsQueue = nullptr;
		vk::raii::Queue _presentQueue = nullptr;
		vk::raii::SwapchainKHR _swapChain = nullptr;
		std::vector<vk::Image> _swapChainImages;
		vk::SurfaceFormatKHR _swapChainSurfaceFormat;
		vk::Extent2D _swapChainExtent;
		std::vector<vk::raii::ImageView> _swapChainImageViews;
		bool _running = true;
};
