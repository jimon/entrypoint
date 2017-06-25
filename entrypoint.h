// lightweight entry point for games
//
// how to use:
// - instead of main(...) use int entrypoint_init(int argc, char * argv[]) in your app
// - when you will get control, expect a window to be created and valid
// - don't create run loop on your own (it's simply not possible on some platforms)
// - instead provide entrypoint_loop for loop tick (do updating and rendering there)
// - add entrypoint_deinit for loop finish, but don't expect it to be called on all platforms
// - to save critical data implement entrypoint_might_unload
//
// so yes:
// - this lib does overrides main :(
// - this lib does take control over runloop
// - but hey, on platforms like iOS, Android and Emscripten you don't normally have it anyway

#pragma once

#include "entrypoint_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// please implement this functions in your code

// initialize your application, returns error code, 0 is ok, != 0 if failure
int32_t entrypoint_init(int32_t argc, char * argv[]);

// deinitialize your application, returns error code, 0 is ok, != 0 if failure
// don't rely on this to be called because it's not feasible on some platforms
int32_t entrypoint_deinit();

// will be called if we application goes into background and might be terminated there
// aka "applicationWillResignActive" and "beforeunload"
// you should save you critical data here as fast as possible, and please don't block
int32_t entrypoint_might_unload();

// frame tick for your application, 0 is ok, != 0 if want to close the application
// depending on a platform might be called as fast as possible, or connected to VSYNC
int32_t entrypoint_loop();

// -----------------------------------------------------------------------------

// you can get all platform handles from ctx
// see entrypoint_ctx_t definition at the bottom of this file
typedef struct entrypoint_ctx_t entrypoint_ctx_t;
entrypoint_ctx_t * ep_ctx();

// get size of the window
typedef struct ep_size_t {uint16_t w, h;} ep_size_t;
ep_size_t ep_size();

// is current platform built with retina support
bool ep_retina();

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_TIME

// returns the amount of time elapsed since ep_delta_time was last called in seconds, or 0 first time
double ep_delta_time();

// sleep for provided amount of seconds, not available on emscripten
void ep_sleep(double seconds);

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_LOG

// on all platforms except android you can simply and safely use printf
// use this function if you want consistent logging on all platforms
void ep_log(const char * message, ...);

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_INPUT

// mouse/touch stuff
typedef struct
{
	// normal mouse and one finger touch
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

	// multitouch feature
	struct
	{
		void * context;
		float x, y;
		uint8_t touched;
	} multitouch[ENTRYPOINT_MAX_MULTITOUCH];

	// accelerometer data, if available
	float acc_x, acc_y, acc_z;

	// TODO mouse wheel, additional buttons
} ep_touch_t;
void ep_touch(ep_touch_t * touch);

// keys stuff
// key scancodes, for letters/numbers, use ASCII ('A'-'Z' and '0'-'9').
typedef enum {
	EK_PAD0=128,EK_PAD1,EK_PAD2,EK_PAD3,EK_PAD4,EK_PAD5,EK_PAD6,EK_PAD7,EK_PAD8,EK_PAD9,
	EK_PADMUL,EK_PADADD,EK_PADENTER,EK_PADSUB,EK_PADDOT,EK_PADDIV,
	EK_F1,EK_F2,EK_F3,EK_F4,EK_F5,EK_F6,EK_F7,EK_F8,EK_F9,EK_F10,EK_F11,EK_F12,
	EK_BACKSPACE,EK_TAB,EK_RETURN,EK_ALT,EK_PAUSE,EK_CAPSLOCK,
	EK_ESCAPE,EK_SPACE,EK_PAGEUP,EK_PAGEDN,EK_END,EK_HOME,EK_LEFT,EK_UP,EK_RIGHT,EK_DOWN,
	EK_INSERT,EK_DELETE,EK_LWIN,EK_RWIN,EK_NUMLOCK,EK_SCROLL,EK_LSHIFT,EK_RSHIFT,
	EK_LCONTROL,EK_RCONTROL,EK_LALT,EK_RALT,EK_SEMICOLON,EK_EQUALS,EK_COMMA,EK_MINUS,
	EK_DOT,EK_SLASH,EK_BACKTICK,EK_LSQUARE,EK_BACKSLASH,EK_RSQUARE,EK_TICK
} ep_key_t;

// reads the keyboard from a window, returns non-zero if a key is hit or pressed
// key hit tests for the initial press, key down repeats each frame
bool ep_khit(int32_t key);
bool ep_kdown(int32_t key);
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
#elif defined(__APPLE__)
	#include <TargetConditionals.h>
	#include <mach/mach_time.h>

	// use the following:
	// TARGET_OS_IPHONE
	// TARGET_OS_OSX

	#ifndef TARGET_OS_OSX
		//#warning TARGET_OS_OSX is not available, simulating it
		#define TARGET_OS_OSX (!(TARGET_OS_IOS || TARGET_OS_TV || TARGET_OS_WATCH))
	#endif
#elif defined(EMSCRIPTEN)
	#include <sys/time.h>
#elif defined(__ANDROID__)
	#include <android/native_window.h>
	#include <android_native_app_glue.h>
	#include <pthread.h>
#endif

struct entrypoint_ctx_t
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
	#elif defined(__APPLE__)
		#if TARGET_OS_OSX
			void * window; // NSWindow
			uint32_t window_count;
			bool terminated;
		#elif TARGET_OS_IOS
			void * view; // EntryPointView: UIView
			void * caeagllayer; // EntryPointView.layer
			uint16_t view_w, view_h;
			union
			{
				uint8_t flags;
				struct
				{
					uint8_t flag_failed_to_init: 1;
					uint8_t flag_anim: 1;
				};
			};
		#endif

		#ifdef ENTRYPOINT_PROVIDE_TIME
			uint64_t prev_time;
			mach_timebase_info_data_t timebase_info;
		#endif
	#elif defined(EMSCRIPTEN)
		union
		{
			uint8_t flags;
			struct
			{
				uint8_t flag_want_to_close: 1;
				#ifdef ENTRYPOINT_PROVIDE_TIME
				uint8_t flag_time_set: 1;
				#endif
			};
		};
		#ifdef ENTRYPOINT_PROVIDE_TIME
			struct timeval prev_time;
		#endif
	#elif defined(__ANDROID__)
		struct android_app * app;
		ANativeWindow * window;
		uint16_t view_w, view_h;
		pthread_mutex_t mutex;
		pthread_t thread;
		union
		{
			uint8_t flags;
			struct
			{
				uint8_t flag_want_to_close: 1;
				#ifdef ENTRYPOINT_PROVIDE_TIME
				uint8_t flag_time_set: 1;
				#endif
			};
		};
		#ifdef ENTRYPOINT_PROVIDE_TIME
			struct timeval prev_time;
		#endif
	#endif

	// -------------------------------------------------------------------------
	// input

	#ifdef ENTRYPOINT_PROVIDE_INPUT
		uint32_t last_char;
		char keys[256], prev[256];
		#if defined(__APPLE__) || defined(EMSCRIPTEN) || defined(__ANDROID__)
			ep_touch_t touch;
		#endif
	#endif

};

#endif

// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
