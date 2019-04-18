#pragma once

#include "SDLWrapper.h"
#include "VulkanWrapper2.hpp"

#include <memory>

//---

class App
{
public:

	App(const char* staticAppName_);
	~App();

public:

	const char* staticAppName = nullptr;
	int startupWindowWidth = 1024;
	int startupWindowHeight = 1024;

protected:

	std::unique_ptr<SDL::Library> libSDL;
	std::unique_ptr<Vulkan::Instance> vulkan;

	SDL_Window* window = nullptr;
};
