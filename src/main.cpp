#include "Application.hpp"
#include <cstdlib>

int main(void)
{

	try
	{
		Application app;
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
