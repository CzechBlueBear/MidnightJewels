#include "MapFile.h"
#include "LoadFont.h"
#include "ToUnicode.h"
#include "SDL.h"
#include "SDLWrapper.h"

#include "GL/gl.h"

#include <memory>
#include <array>
#include <iostream>
#include <string.h>
#include <locale>
#include <sstream>

const char* kDefWindowTitle = "Midnight Jewels";
const int kDefWindowWidth = 1280;
const int kDefWindowHeight = 1024;

//---

int main(int argc, const char** argv)
{
	SDL::Library libSDL;

	// set locale (important otherwise the default is C and we don't have Unicode!)
	std::locale::global(std::locale("en_US.UTF-8"));

	SDL::Window window(kDefWindowTitle, kDefWindowWidth, kDefWindowHeight);

	SDL::EventLoop eventLoop(libSDL);
	eventLoop.OnKey = [&eventLoop](const SDL_KeyboardEvent &event) {
		if (event.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			eventLoop.quitRequested = true;
		}
	};
	eventLoop.OnRedraw = []() {
		glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	};
	eventLoop.Run();

	return 0;
}
