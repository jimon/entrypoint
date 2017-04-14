#pragma once

// configuration for entrypoint lib

// -----------------------------------------------------------------------------
// general

#define ENTRYPOINT_PROVIDE_TIME
#define ENTRYPOINT_PROVIDE_INPUT

// -----------------------------------------------------------------------------
// detailed configs
#define ENTRYPOINT_MAX_MULTITOUCH 16

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
// macOS

#define ENTRYPOINT_MACOS_PREPARE_PARAMS {}
#define ENTRYPOINT_MACOS_TITLE		@"test window"
#define ENTRYPOINT_MACOS_WIDTH		1024
#define ENTRYPOINT_MACOS_HEIGHT		768
#define ENTRYPOINT_MACOS_START_X	20
#define ENTRYPOINT_MACOS_START_Y	20

// disable this if you don't want retina support
//#define ENTRYPOINT_MACOS_RETINA

// -----------------------------------------------------------------------------
// iOS

// if you wish we can provide an app delegate for you, it will have UIWindow->UIViewController->UIView with your game
// and the name of app delegate class will be EntryPointAppDelegate
// but please remember that you will need to provide either launch image or launch storyboard to get proper size viewport
// otherwise iOS will think like this is iPhone4 app and will limit your screensize
#define ENTRYPOINT_IOS_APPDELEGATE

// if you would like we can define a main for you, just please provide us with your app delegate class name as a string
#define ENTRYPOINT_IOS_MAIN
#define ENTRYPOINT_IOS_MAIN_APPDELEGATE_CLASSNAME @"EntryPointAppDelegate"

// disable this if you don't want retina support
#define ENTRYPOINT_IOS_RETINA

// disable this if you don't need accelerometer
// WARNING: Remember to fill NSMotionUsageDescription in your Info.plist!
#define ENTRYPOINT_IOS_CM_ACCELEROMETER
#define ENTRYPOINT_IOS_CM_ACCELEROMETER_FREQ 60 // in Hertz

// -----------------------------------------------------------------------------
