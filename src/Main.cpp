#include "MapFile.h"
#include "LoadFont.h"
#include "ToUnicode.h"
#include "SDL.h"
#include "SDLWrapper.h"
#include <memory>
#include <array>
#include <iostream>
#include <string.h>
#include <locale>
#include <sstream>

const char* kDefWindowTitle = "Midnight Jewels";
const int kDefWindowWidth = 1280;
const int kDefWindowHeight = 1024;

std::array<const char*, 2> kFontFileCandidates = {
	"/usr/share/fonts/TTF/DejaVuSans.ttf",		// Arch-ism
	"/usr/share/fonts/dejavu/DejaVuSans.ttf"	// Fedora
};

//---

int main(int argc, const char** argv)
{
	SDL::Library libSDL;
	if (!libSDL.Ok()) {
		std::cerr << "error: could not initialize SDL: " << SDL_GetError() << std::endl;
		return 127;
	}

	// set locale (important otherwise the default is C and we don't have Unicode!)
	std::locale::global(std::locale("en_US.UTF-8"));

/*
	SDL_Rect displayUsableBounds;
	SDL_GetDisplayUsableBounds(DISPLAY_NUMBER, &displayUsableBounds);
*/

	SDL::Window window(kDefWindowTitle, kDefWindowWidth, kDefWindowHeight);
	if (!window.Ok()) {
		std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
		return 127;		
	}

/*
	const int windowWidth = (options.explicitWidth > 0) ? options.explicitWidth : displayUsableBounds.w/2;
	const int windowHeight = (options.explicitHeight > 0) ? options.explicitHeight : displayUsableBounds.h/8;

	SDL_Window* window = SDL_CreateWindow(
		kDefWindowTitle,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		kDefWindowWidth, kDefWindowHeight,
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL
	);
	if (!window) {
		std::cerr << "Could not create window: " << SDL_GetError() << std::endl;
		return 127;
	}
*/

/*
	SDL::Renderer renderer(window, -1, 0);
	if (!renderer.Ok()) {
		std::cerr << "Could not create renderer: " << SDL_GetError() << std::endl;
		return 127;
	}

	// load the font; if no font is given explicitly, try multiple usual locations
	std::unique_ptr<MappedFile> fontFile;
	if (!options.explicitFont.empty()) {
		fontFile.reset(new MappedFile(options.explicitFont.c_str()));
	}
	else {
		for (auto candidateFile : FONT_FILE_CANDIDATES) {
			fontFile.reset(new MappedFile(candidateFile));
			if (fontFile->Ok()) break;		// candidate successful
		}
	}
	if (!fontFile->Ok()) {
		std::cerr << "Could not open font file: " << SDL_GetError() << std::endl;
		return 127;
	}

	Font font(*fontFile, 32.0f);
	if (!font.Ok()) {
		std::cerr << "Could not load font: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL::Surface messageSurface(windowWidth, windowHeight, 32, SDL_PIXELFORMAT_RGBA32);
	if (!messageSurface.Ok()) {
		std::cerr << "Could not create surface: " << SDL_GetError() << std::endl;
		return 127;
	}

	SDL_Rect textRect = font.ComputeTextSize(messageText);
	int startX = windowWidth/2 - textRect.w/2;
	int startY = windowHeight/2 - textRect.h/2;

	int x = startX;
	for (int i = 0; i < messageText.size(); i++) {
		stbtt_packedchar glyphGeometry;
		if (font.GetGlyphGeometry(int(messageText[i]), glyphGeometry)) {
			SDL::Rect glyphRect(
				glyphGeometry.x0,
				glyphGeometry.y0,
				glyphGeometry.x1 - glyphGeometry.x0,
				glyphGeometry.y1 - glyphGeometry.y0
			);
			SDL::Rect destRect(
				x + glyphGeometry.xoff,
				startY + glyphGeometry.yoff,
				glyphGeometry.x1 - glyphGeometry.x0,
				glyphGeometry.y1 - glyphGeometry.y0
			);
			if (!font.GetSurface().Blit(glyphRect, messageSurface, destRect)) {
				std::cerr << "Could not blit glyph: " << SDL_GetError() << std::endl;
				break;
			}
			x += glyphGeometry.xadvance;
		}
	}

	SDL::Texture messageTexture(renderer, messageSurface);
	if (!messageTexture.Ok()) {
		std::cerr << "Could not create message texture: " << SDL_GetError() << std::endl;
		return 127;
	}

	messageSurface.Discard();
*/

	SDL::EventLoop eventLoop(libSDL);

/*
	// install timer for closing after specified time; it sends a UserEvent we then catch in the event loop
	std::unique_ptr<SDL::Timer> closingTimer;
	if (options.closingDelay > 0) {
		closingTimer.reset(new SDL::Timer(SDL::Timer::Type::kOneShot, options.closingDelay, [&eventLoop]{
			eventLoop.PushUserEvent(0);
		}));
	};

	eventLoop.OnRedraw = [&renderer, &messageTexture](){
		SDL_SetRenderDrawColor(renderer, 0x0f, 0x0f, 0x0f, 0x00);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, messageTexture, NULL, NULL);
		SDL_RenderPresent(renderer);
	};
*/
	eventLoop.OnKey = [&eventLoop](const SDL_KeyboardEvent &event) {
		if (event.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			eventLoop.quitRequested = true;
		}
	};
/*
	eventLoop.OnMouseButton = [&eventLoop, options](const SDL_MouseButtonEvent &event) {
		if (options.closeOnClick) {
			eventLoop.quitRequested = true;
		}
	};
	eventLoop.OnUserEvent = [&eventLoop](const SDL_UserEvent& event) {

		// quit when we receive the user event sent by the timer
		eventLoop.quitRequested = true;
	};
*/
	eventLoop.Run();

	return 0;
}
