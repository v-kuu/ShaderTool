#include "Application.hpp"

void Application::run(void)
{
	initWindow();
}

void Application::initWindow(void)
{
	if (!SDL_Init(SDL_INIT_VIDEO))
		throw (std::runtime_error("Unable to initialize SDL"));

	SDL_DisplayID id = SDL_GetPrimaryDisplay();
	if (id == 0)
		throw (std::runtime_error("Failed to get display"));

	SDL_Rect bounds;
	if (!SDL_GetDisplayBounds(id, &bounds))
		throw (std::runtime_error("Failed to get dislay bounds"));


}

void Application::initVulkan(void)
{

}

void Application::mainLoop(void)
{

}

void Application::cleanup(void)
{

}
