#pragma once

// configuration for entrypoint lib

// -----------------------------------------------------------------------------
// general

#define ENTRYPOINT_PROVIDE_TIME
#define ENTRYPOINT_PROVIDE_INPUT

// -----------------------------------------------------------------------------
// Windows

// adds an additional linker flags to use windows subsystem, aka don't create console window
//#define ENTRYPOINT_WINDOWS_SUBSYSTEM

// forces WinMain
//#define ENTRYPOINT_WINDOWS_WINMAIN

// if we use WinMain we need Shell32.lib and calloc to get argc/argv, disable this if you don't need them
//#define ENTRYPOINT_WINDOWS_NEED_ARGS

// add stuff to the linker options, disable this if you plan to provide all flags by yourself
#define ENTRYPOINT_WINDOWS_LINKER_OPT

// this will be called before following parameters are used
// you might want to use this to read some config file or something
#define ENTRYPOINT_WINDOWS_PREPARE_PARAMS {}
#define ENTRYPOINT_WINDOWS_CLASS		L"entrypoint"
#define ENTRYPOINT_WINDOWS_TITLE		L"test window"
#define ENTRYPOINT_WINDOWS_WIDTH		1024
#define ENTRYPOINT_WINDOWS_HEIGHT		768

#define ENTRYPOINT_WINDOWS_MIN_WIDTH	128
#define ENTRYPOINT_WINDOWS_MIN_HEIGHT	128

// usually not needed for games, but good option for non games to fill empty areas during resize
//#define ENTRYPOINT_WINDOWS_DO_WMPAINT

// -----------------------------------------------------------------------------

#define ENTRYPOINT_CTX // TODO remove
