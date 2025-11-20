#include "Application.hpp"
#include <iostream>
#include <cstdlib>

int main(void)
{
	Application app;

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "ShaderTool error: " << e.what() << std::endl;
		std::cerr << SDL_GetError() << std::endl;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}
