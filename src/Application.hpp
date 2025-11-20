#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>

class Application
{
	public:
		void run(void);
	private:
		void initWindow(void);
		void initVulkan(void);
		void mainLoop(void);
		void cleanup(void);
};
