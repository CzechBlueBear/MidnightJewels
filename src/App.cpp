#include "App.h"

#include <locale>
#include <iostream>

//---

App::App(const char* staticAppName)
{
	// ensure we have a locale that uses Unicode for wide characters
	// (exact language is not important at this point)
	std::locale::global(std::locale("en_US.UTF-8"));

	libSDL = std::make_unique<SDL::Library>();
	if (!libSDL->Ok()) return;

	vulkan = std::make_unique<Vulkan::Instance>(staticAppName);

	window = SDL_CreateWindow(
		staticAppName,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		startupWindowWidth, startupWindowHeight,
		SDL_WINDOW_VULKAN|SDL_WINDOW_ALLOW_HIGHDPI);
	if (!window) {
		SDL_Log("SDL_CreateWindow() failed: %s\n", SDL_GetError());
		return;
	}
}

//---

App::~App()
{
}

//---

/*

bool App::initVulkan()
{
	vk::ApplicationInfo applicationInfo(staticAppName, 1, staticAppName, 1, VK_API_VERSION_1_1);
	vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);
	vk::UniqueInstance vulkanInstance = vk::createInstanceUnique(instanceCreateInfo);

	std::vector<vk::PhysicalDevice> physicalDevices = instance->enumeratePhysicalDevices();

	return true;
}

*/