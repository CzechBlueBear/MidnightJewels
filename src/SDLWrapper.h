#pragma once

// C++ wrapper for SDL.
// This is really just for fun and to make things look more object-y,
// but there is no deeper reason at all.

#include "SDL.h"
#include <cstdint>
#include <stdexcept>
#include <optional>
#include <functional>

namespace SDL {

/// Wraps SDL errors that are not reasonable to return in-band (panic-grade).
class Error : public std::runtime_error
{
public:

	Error(const std::string &reason_) : std::runtime_error(reason_) {}
};

//---

class OkAble
{
public:

	virtual bool Ok() const = 0;
};

//---

/// Wraps SDL initialization; SDL_Init() is called on construction,
/// and SDL_Quit() upon destruction.
class Library
{
public:

	/// Calls SDL_Init(). Throws SDL::Error if this fails.
	Library(uint32_t initFlags = SDL_INIT_EVERYTHING);
	Library(const Library& src) = delete;
	~Library();
	static std::string getError() { return std::string(SDL_GetError()); }
};

//---

class EventLoop
{
public:

	EventLoop(Library &libSDL_);
	~EventLoop();
	void Run();

	/// Pushes a user event (with user-defined meaning) to the event stream.
	void PushUserEvent(int code, void* data1 = nullptr, void* data2 = nullptr);

	/// Flag to set to true to leave Run().
	bool quitRequested = false;

	std::function<void(void)> OnRedraw;
	std::function<void(const SDL_KeyboardEvent&)> OnKey;
	std::function<void(const SDL_MouseMotionEvent&)> OnMouseMotion;
	std::function<void(const SDL_MouseButtonEvent&)> OnMouseButton;
	std::function<void(const SDL_UserEvent&)> OnUserEvent;
	std::function<void(int, int)> OnWindowResized;

protected:

	Library &libSDL;
};

//---

class Rect : public SDL_Rect
{
public:

	/**
	 * Constructor, initializes everything to 0.
	 */
	Rect() { x = 0; y = 0; w = 0; h = 0; }

	/**
	 * Constructor, sets explicitly both position (X, Y) and width and height (W, H).
	 */
	Rect(int x_, int y_, int w_, int h_)
	{
		x = x_; y = y_; w = w_; h = h_;
	}

	/**
	 * Sets the rectangle position (X, Y) and its width and height (W, H).
	 */
	Rect &SetXYWH(int x_, int y_, int w_, int h_)
	{
		x = x_; y = y_; w = w_; h = h_;
		return *this;
	}

	operator SDL_Rect*() { return this; }
	operator SDL_Rect const*() const { return this; }
};

//---

/// Base class for objects that wrap another object using its pointer.
template<class T>
class PtrWrapper : public virtual OkAble
{
public:

	/// Returns a pointer to the wrapped object (null if the wrapper is invalid).
	T* GetWrapped() { return wrapped; }

	/// Const-variant of GetWrapped().
	const T* GetWrapped() const { return wrapped; }

	/// For automatical unwrapping when passing to a function that expects the pointer to the underlying object.
	operator T*() { return wrapped; }

	/// Const-variant of operator T*().
	operator const T*() { return wrapped; }

	/// Basic implementation of Ok(), checks whether the wrapped object exists.
	bool Ok() const { return (wrapped != nullptr); }

protected:

	/// Pointer to the wrapped object (plain pointer as it usually needs special allocation/freeing calls anyway).
	T* wrapped = nullptr;
};

//---

class Surface : public PtrWrapper<SDL_Surface>
{
public:

	/// Constructor, equivalent to SDL_CreateRGBSurfaceWithFormat().
	Surface(int width, int height, int depth, uint32_t format);

	Surface(const Surface& surface) = delete;

	/// Destructor, calls Discard().
	~Surface();

	/// Frees the wrapped object (with SDL_FreeSurface()), leaving the wrapper invalid.
	void Discard();

	SDL_PixelFormat* GetFormat() const { return wrapped ? wrapped->format : nullptr; }
	void* GetPixels() { return wrapped ? wrapped->pixels : 0; }
	int GetWidth() const { return wrapped ? wrapped->w : 0; }
	int GetHeight() const { return wrapped ? wrapped->h : 0; }
	int GetPitch() const { return wrapped ? wrapped->pitch : 0; }

	/// Blits a rectangle of pixels from this surface to the target surface.
	void blit(const SDL::Rect& srcRect, SDL::Surface& dest, SDL::Rect& destRect) const;
};

//---

class Texture : public PtrWrapper<SDL_Texture>
{
public:

	Texture(SDL_Renderer* renderer, Surface& src);
	~Texture();
};

//---

class Renderer : public PtrWrapper<SDL_Renderer>
{
public:

	Renderer(SDL_Window* window, int index, uint32_t flags);
	~Renderer();
};

//---

/// Wraps SDL_Timer, allows to use a C++ lambda as the payload function.
class Timer
{
public:

	enum class Type {
		kOneShot = 0,	///< Triggered only once.
		kRepeated = 1	///< Triggered repeatedly in specified intervals.
	};

	Timer(Timer::Type type, uint32_t interval, std::function<void(void)> payload);
	Timer(const Timer &src) = delete;
	~Timer();

protected:

	/// ID of the SDL timer.
	SDL_TimerID timerId = 0;

	/// SDL callback that ensures calling our payload function.
	static uint32_t CallPayload(uint32_t interval, void* indirectThis);

	/// The interval set in the constructor.
	uint32_t interval = 0;

	Timer::Type type;

	/// The payload, called when the timer elapses (probably in a different thread).
	std::function<void(void)> payload;
};

//---

class Window
{
public:

	Window(const std::string& title, int width, int height);
	Window(const Window&) = delete;
	~Window();
	uint32_t getID() { return SDL_GetWindowID(wnd); }

private:

	SDL_Window* wnd = nullptr;
	SDL_GLContext ctx = nullptr;
};

} // namespace SDL
