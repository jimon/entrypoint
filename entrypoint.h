// lightweight entry point for games
//
// how to use:
// - instead of main(...) use int entrypoint(int argc, char * argv[]) in your app
// - when you will get control, expect a window to be created and valid
// - call ep_poll from your main loop, please use emscripten_set_main_loop for emscripten
//
// so yes:
// - this lib does overrides main :( this is needed because of platforms like Andriod and iOS
// - on platforms like emscripten there is no system window to begin with

#pragma once

#include "entrypoint_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------

// !!! please define this function in your code !!!
int32_t entrypoint(int32_t argc, char * argv[]);

// call this function in your main loop
// > 0 means everything is ok
// = 0 means we want to exit or there is an error
uint32_t ep_poll();

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_TIME

// returns the amount of time elapsed since ep_time was last called in seconds, or 0 first time
double ep_time();

void ep_sleep(double seconds);

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_INPUT

// mouse/touch stuff
typedef struct
{
	float x, y;
	union
	{
		uint8_t flags;
		struct
		{
			uint8_t left: 1;
			uint8_t middle: 1;
			uint8_t right: 1;
		};
	};
	// TODO mouse wheel, additional buttons
} ep_touch_t;
void ep_touch(ep_touch_t * touch);

// keys stuff
// key scancodes, for letters/numbers, use ASCII ('A'-'Z' and '0'-'9').
typedef enum {
	EK_PAD0=128,EK_PAD1,EK_PAD2,EK_PAD3,EK_PAD4,EK_PAD5,EK_PAD6,EK_PAD7,EK_PAD8,EK_PAD9,
	EK_PADMUL,EK_PADADD,EK_PADENTER,EK_PADSUB,EK_PADDOT,EK_PADDIV,
	EK_F1,EK_F2,EK_F3,EK_F4,EK_F5,EK_F6,EK_F7,EK_F8,EK_F9,EK_F10,EK_F11,EK_F12,
	EK_BACKSPACE,EK_TAB,EK_RETURN,EK_SHIFT,EK_CONTROL,EK_ALT,EK_PAUSE,EK_CAPSLOCK,
	EK_ESCAPE,EK_SPACE,EK_PAGEUP,EK_PAGEDN,EK_END,EK_HOME,EK_LEFT,EK_UP,EK_RIGHT,EK_DOWN,
	EK_INSERT,EK_DELETE,EK_LWIN,EK_RWIN,EK_NUMLOCK,EK_SCROLL,EK_LSHIFT,EK_RSHIFT,
	EK_LCONTROL,EK_RCONTROL,EK_LALT,EK_RALT,EK_SEMICOLON,EK_EQUALS,EK_COMMA,EK_MINUS,
	EK_DOT,EK_SLASH,EK_BACKTICK,EK_LSQUARE,EK_BACKSLASH,EK_RSQUARE,EK_TICK
} ep_key_t;
// reads the keyboard from a window, returns non-zero if a key is pressed / held
// key down tests for the initial press, key held repeats each frame
bool ep_kdown(int32_t key);
bool ep_kheld(int32_t key);
// reads character input for a window, returns the UTF-32 value of the last key pressed, or 0 if none
uint32_t ep_kchar();

// TODO iOS style keyboard input

// TODO gamepad

#endif

// -----------------------------------------------------------------------------
// whole ctx structure is usually not needed and it's quite bulky because of system headers
// so if you need it - define ENTRYPOINT_CTX before including this file
#ifdef ENTRYPOINT_CTX

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

typedef struct entrypoint_ctx_t
{
	// -------------------------------------------------------------------------
	// arguments

	int argc;
	char ** argv;

	// -------------------------------------------------------------------------
	// platform stuff

	#ifdef _WIN32
	// entry point data:
	// - hInstance is GetModuleHandle(0)
	// - hPrevInstance is always NULL
	// - lpCmdLine can be get as GetCommandLineA()
	// - nCmdShow is wShowWindow in GetStartupInfoA(STARTUPINFOA*)

	HWND hwnd; // window handle
	RECT rect; // current window rect
	RECT rect_saved; // saved window rect (for fullscreen<->windowed transitions)
	DWORD dwStyle; // current window style

	#ifdef ENTRYPOINT_PROVIDE_TIME
	LARGE_INTEGER prev_qpc_time;
	#endif

	union
	{
		uint8_t flags;
		struct
		{
			uint8_t flag_want_to_close: 1;
			uint8_t flag_borderless: 1;
			#ifdef ENTRYPOINT_PROVIDE_TIME
			uint8_t flag_time_set: 1;
			#endif
		};
	};

	#endif


	// -------------------------------------------------------------------------
	// input

	#ifdef ENTRYPOINT_PROVIDE_INPUT

	uint32_t last_char;
	char keys[256], prev[256];
	//#ifdef __APPLE__
	//int mouseButtons;
	//#endif

	#endif

} entrypoint_ctx_t;

// you can get all platform handles from ctx
entrypoint_ctx_t * ep_ctx();

#endif

// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
