#include "SDLWrapper.h"

namespace SDL {

//---

Library::Library(uint32_t initFlags)
{
	if (SDL_Init(initFlags) != 0) {
		throw SDL::Error("SDL::Library::Library(): SDL_Init() failed: " + Library::getError());
	}
}

//---

Library::~Library()
{
	SDL_Quit();
}

//---

EventLoop::EventLoop(Library &libSDL_)
	: libSDL(libSDL_)
{
}

//---

EventLoop::~EventLoop()
{
}

//---

void EventLoop::Run()
{
	bool redrawNeeded = false;
	bool haveEvent = false;
	std::vector<SDL_Window*> windowsToRedraw;
	SDL_Event event;
	while (1) {
		windowsToRedraw.clear();

		// wait for any incoming events, then handle the whole batch
		SDL_WaitEvent(&event);
		do {
			if (event.type == SDL_QUIT) {	// closing button pressed
				quitRequested = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (OnKey)
					OnKey(event.key);
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (OnMouseButton)
					OnMouseButton(event.button);
			}
			else if (event.type == SDL_MOUSEMOTION) {
				if (OnMouseMotion)
					OnMouseMotion(event.motion);
			}
			else if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
					redrawNeeded = true;
					windowsToRedraw.push_back(SDL_GetWindowFromID(event.window.windowID));
				}
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					if (OnWindowResized)
						OnWindowResized(int(event.window.data1), int(event.window.data2));
				}
			}
			else if (event.type == SDL_USEREVENT) {
				if (OnUserEvent)
					OnUserEvent(event.user);
			}
		} while (SDL_PollEvent(&event));

		if (quitRequested) break;

		if (redrawNeeded) {
			for (auto window : windowsToRedraw) {
				if (OnRedraw) {
					OnRedraw();
				}
				SDL_GL_SwapWindow(window);
			}
			windowsToRedraw.clear();
		}
	}
}

//---

void EventLoop::PushUserEvent(int code, void* data1, void* data2)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	SDL_UserEvent& userevent = event.user;
	userevent.type = SDL_USEREVENT;
	userevent.code = code;
	userevent.data1 = data1;
	userevent.data2 = data2;
	SDL_PushEvent(&event);
}

//---

Surface::Surface(int width, int height, int depth, uint32_t format)
{
	if (width < 0 || height < 0 || depth < 0) {
		throw Error("SDL::Surface::Surface(): Surface dimensions must be >= 0");
	}
	wrapped = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
	if (!wrapped) {
		throw Error("SDL::Surface::Surface(): SDL_CreateRGBSurfaceWithFormat() failed: " + Library::getError());
	}
}

//---

Surface::~Surface()
{
	if (wrapped)
		SDL_FreeSurface(wrapped);
}

//---

void Surface::blit(const SDL::Rect& rect, SDL::Surface& dest, SDL::Rect& destRect) const
{
	if (SDL_BlitSurface(wrapped, rect, dest.GetWrapped(), destRect) != 0)
		throw Error("SDL::Surface::Blit(): SDL_BlitSurface() failed: " + Library::getError());
}

//---

Texture::Texture(SDL_Renderer* renderer, Surface& src)
{
	wrapped = SDL_CreateTextureFromSurface(renderer, src.GetWrapped());
}

//---

Texture::~Texture()
{
	if (wrapped) {
		SDL_DestroyTexture(wrapped);
		wrapped = nullptr;
	}
}

//---

Renderer::Renderer(SDL_Window* window, int index, uint32_t flags)
{
	wrapped = SDL_CreateRenderer(window, index, flags);
}

//---

Renderer::~Renderer()
{
	if (wrapped) {
		SDL_DestroyRenderer(wrapped);
	}
}

//---

Timer::Timer(Type type_, uint32_t interval_, std::function<void(void)> payload_)
	: type(type_), interval(interval_), payload(payload_)
{
	timerId = SDL_AddTimer(interval, CallPayload, this);
}

//---

Timer::~Timer()
{
	SDL_RemoveTimer(timerId);
}

//---

uint32_t Timer::CallPayload(uint32_t timePassed, void* indirectThis)
{
	Timer* theThis = reinterpret_cast<Timer*>(indirectThis);

	theThis->payload();

	if (theThis->type == Timer::Type::kOneShot) {

		// do not call multiple times
		return 0;
	}
	else {

		// how much we are delayed in comparison with the interval
		// (positive - we are delayed, negative - we are earlier)
		int32_t delay = timePassed - theThis->interval;

		// schedule next call, trying to compensate for delays
		return uint32_t(int32_t(theThis->interval) - delay);
	}
}

//---

Window::Window(const std::string& title, int width, int height)
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	wnd = SDL_CreateWindow(
		title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL|SDL_WINDOW_ALLOW_HIGHDPI);
	if (!wnd) {
		throw Error("SDL::Window::Window(): SDL_CreateWindow() failed: " + Library::getError());
	}

	ctx = SDL_GL_CreateContext(wnd);
	if (!ctx) {
		SDL_DestroyWindow(wnd);
		throw Error("SDL::Window::Window(): SDL_GL_CreateContext() failed: " + Library::getError());
	}
}

Window::~Window()
{
	if (ctx) {
		SDL_GL_DeleteContext(ctx);
	}
	if (wnd) {
		SDL_DestroyWindow(wnd);
	}
}

} // namespace SDL
