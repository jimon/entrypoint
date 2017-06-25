
// based on https://github.com/jimon/osx_app_in_plain_c

#define ENTRYPOINT_CTX
#include "entrypoint.h"

#if defined(__APPLE__) && TARGET_OS_OSX
#if !__has_feature(objc_arc)
	#error hey, we need ARC here
#endif

#import <Cocoa/Cocoa.h>

static entrypoint_ctx_t ctx = {0};
entrypoint_ctx_t * ep_ctx() {return &ctx;}

// -----------------------------------------------------------------------------

@interface AppDelegate : NSObject<NSApplicationDelegate>
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
@end

@implementation AppDelegate
-(NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	ctx.terminated = true;
	return NSTerminateCancel;
}
@end

@interface WindowDelegate : NSObject<NSWindowDelegate>
-(void)windowWillClose:(NSNotification*)notification;
@end

@implementation WindowDelegate
-(void)windowWillClose:(NSNotification*)notification
{
	(void)notification;
	assert(ctx.window_count);
	if(--ctx.window_count == 0)
		ctx.terminated = true;
}
@end

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_INPUT
static uint8_t _macoskey(uint16_t key)
{
	// from Carbon HIToolbox/Events.h
	enum
	{
		kVK_ANSI_A              = 0x00,
		kVK_ANSI_S              = 0x01,
		kVK_ANSI_D              = 0x02,
		kVK_ANSI_F              = 0x03,
		kVK_ANSI_H              = 0x04,
		kVK_ANSI_G              = 0x05,
		kVK_ANSI_Z              = 0x06,
		kVK_ANSI_X              = 0x07,
		kVK_ANSI_C              = 0x08,
		kVK_ANSI_V              = 0x09,
		kVK_ANSI_B              = 0x0B,
		kVK_ANSI_Q              = 0x0C,
		kVK_ANSI_W              = 0x0D,
		kVK_ANSI_E              = 0x0E,
		kVK_ANSI_R              = 0x0F,
		kVK_ANSI_Y              = 0x10,
		kVK_ANSI_T              = 0x11,
		kVK_ANSI_1              = 0x12,
		kVK_ANSI_2              = 0x13,
		kVK_ANSI_3              = 0x14,
		kVK_ANSI_4              = 0x15,
		kVK_ANSI_6              = 0x16,
		kVK_ANSI_5              = 0x17,
		kVK_ANSI_Equal          = 0x18,
		kVK_ANSI_9              = 0x19,
		kVK_ANSI_7              = 0x1A,
		kVK_ANSI_Minus          = 0x1B,
		kVK_ANSI_8              = 0x1C,
		kVK_ANSI_0              = 0x1D,
		kVK_ANSI_RightBracket   = 0x1E,
		kVK_ANSI_O              = 0x1F,
		kVK_ANSI_U              = 0x20,
		kVK_ANSI_LeftBracket    = 0x21,
		kVK_ANSI_I              = 0x22,
		kVK_ANSI_P              = 0x23,
		kVK_ANSI_L              = 0x25,
		kVK_ANSI_J              = 0x26,
		kVK_ANSI_Quote          = 0x27,
		kVK_ANSI_K              = 0x28,
		kVK_ANSI_Semicolon      = 0x29,
		kVK_ANSI_Backslash      = 0x2A,
		kVK_ANSI_Comma          = 0x2B,
		kVK_ANSI_Slash          = 0x2C,
		kVK_ANSI_N              = 0x2D,
		kVK_ANSI_M              = 0x2E,
		kVK_ANSI_Period         = 0x2F,
		kVK_ANSI_Grave          = 0x32,
		kVK_ANSI_KeypadDecimal  = 0x41,
		kVK_ANSI_KeypadMultiply = 0x43,
		kVK_ANSI_KeypadPlus     = 0x45,
		kVK_ANSI_KeypadClear    = 0x47,
		kVK_ANSI_KeypadDivide   = 0x4B,
		kVK_ANSI_KeypadEnter    = 0x4C,
		kVK_ANSI_KeypadMinus    = 0x4E,
		kVK_ANSI_KeypadEquals   = 0x51,
		kVK_ANSI_Keypad0        = 0x52,
		kVK_ANSI_Keypad1        = 0x53,
		kVK_ANSI_Keypad2        = 0x54,
		kVK_ANSI_Keypad3        = 0x55,
		kVK_ANSI_Keypad4        = 0x56,
		kVK_ANSI_Keypad5        = 0x57,
		kVK_ANSI_Keypad6        = 0x58,
		kVK_ANSI_Keypad7        = 0x59,
		kVK_ANSI_Keypad8        = 0x5B,
		kVK_ANSI_Keypad9        = 0x5C,
		kVK_Return              = 0x24,
		kVK_Tab                 = 0x30,
		kVK_Space               = 0x31,
		kVK_Delete              = 0x33,
		kVK_Escape              = 0x35,
		kVK_Command             = 0x37,
		kVK_Shift               = 0x38,
		kVK_CapsLock            = 0x39,
		kVK_Option              = 0x3A,
		kVK_Control             = 0x3B,
		kVK_RightShift          = 0x3C,
		kVK_RightOption         = 0x3D,
		kVK_RightControl        = 0x3E,
		kVK_Function            = 0x3F,
		kVK_F17                 = 0x40,
		kVK_VolumeUp            = 0x48,
		kVK_VolumeDown          = 0x49,
		kVK_Mute                = 0x4A,
		kVK_F18                 = 0x4F,
		kVK_F19                 = 0x50,
		kVK_F20                 = 0x5A,
		kVK_F5                  = 0x60,
		kVK_F6                  = 0x61,
		kVK_F7                  = 0x62,
		kVK_F3                  = 0x63,
		kVK_F8                  = 0x64,
		kVK_F9                  = 0x65,
		kVK_F11                 = 0x67,
		kVK_F13                 = 0x69,
		kVK_F16                 = 0x6A,
		kVK_F14                 = 0x6B,
		kVK_F10                 = 0x6D,
		kVK_F12                 = 0x6F,
		kVK_F15                 = 0x71,
		kVK_Help                = 0x72,
		kVK_Home                = 0x73,
		kVK_PageUp              = 0x74,
		kVK_ForwardDelete       = 0x75,
		kVK_F4                  = 0x76,
		kVK_End                 = 0x77,
		kVK_F2                  = 0x78,
		kVK_PageDown            = 0x79,
		kVK_F1                  = 0x7A,
		kVK_LeftArrow           = 0x7B,
		kVK_RightArrow          = 0x7C,
		kVK_DownArrow           = 0x7D,
		kVK_UpArrow             = 0x7E
	};

	switch(key)
	{
	case kVK_ANSI_Q:              return 'Q';
	case kVK_ANSI_W:              return 'W';
	case kVK_ANSI_E:              return 'E';
	case kVK_ANSI_R:              return 'R';
	case kVK_ANSI_T:              return 'T';
	case kVK_ANSI_Y:              return 'Y';
	case kVK_ANSI_U:              return 'U';
	case kVK_ANSI_I:              return 'I';
	case kVK_ANSI_O:              return 'O';
	case kVK_ANSI_P:              return 'P';
	case kVK_ANSI_A:              return 'A';
	case kVK_ANSI_S:              return 'S';
	case kVK_ANSI_D:              return 'D';
	case kVK_ANSI_F:              return 'F';
	case kVK_ANSI_G:              return 'G';
	case kVK_ANSI_H:              return 'H';
	case kVK_ANSI_J:              return 'J';
	case kVK_ANSI_K:              return 'K';
	case kVK_ANSI_L:              return 'L';
	case kVK_ANSI_Z:              return 'Z';
	case kVK_ANSI_X:              return 'X';
	case kVK_ANSI_C:              return 'C';
	case kVK_ANSI_V:              return 'V';
	case kVK_ANSI_B:              return 'B';
	case kVK_ANSI_N:              return 'N';
	case kVK_ANSI_M:              return 'M';
	case kVK_ANSI_0:              return '0';
	case kVK_ANSI_1:              return '1';
	case kVK_ANSI_2:              return '2';
	case kVK_ANSI_3:              return '3';
	case kVK_ANSI_4:              return '4';
	case kVK_ANSI_5:              return '5';
	case kVK_ANSI_6:              return '6';
	case kVK_ANSI_7:              return '7';
	case kVK_ANSI_8:              return '8';
	case kVK_ANSI_9:              return '9';
	case kVK_ANSI_Keypad0:        return EK_PAD0;
	case kVK_ANSI_Keypad1:        return EK_PAD1;
	case kVK_ANSI_Keypad2:        return EK_PAD2;
	case kVK_ANSI_Keypad3:        return EK_PAD3;
	case kVK_ANSI_Keypad4:        return EK_PAD4;
	case kVK_ANSI_Keypad5:        return EK_PAD5;
	case kVK_ANSI_Keypad6:        return EK_PAD6;
	case kVK_ANSI_Keypad7:        return EK_PAD7;
	case kVK_ANSI_Keypad8:        return EK_PAD8;
	case kVK_ANSI_Keypad9:        return EK_PAD9;
	case kVK_ANSI_KeypadMultiply: return EK_PADMUL;
	case kVK_ANSI_KeypadPlus:     return EK_PADADD;
	case kVK_ANSI_KeypadEnter:    return EK_PADENTER;
	case kVK_ANSI_KeypadMinus:    return EK_PADSUB;
	case kVK_ANSI_KeypadDecimal:  return EK_PADDOT;
	case kVK_ANSI_KeypadDivide:   return EK_PADDIV;
	case kVK_F1:                  return EK_F1;
	case kVK_F2:                  return EK_F2;
	case kVK_F3:                  return EK_F3;
	case kVK_F4:                  return EK_F4;
	case kVK_F5:                  return EK_F5;
	case kVK_F6:                  return EK_F6;
	case kVK_F7:                  return EK_F7;
	case kVK_F8:                  return EK_F8;
	case kVK_F9:                  return EK_F9;
	case kVK_F10:                 return EK_F10;
	case kVK_F11:                 return EK_F11;
	case kVK_F12:                 return EK_F12;
	case kVK_Shift:               return EK_LSHIFT;
	case kVK_Control:             return EK_LCONTROL;
	case kVK_Option:              return EK_LALT;
	case kVK_CapsLock:            return EK_CAPSLOCK;
	case kVK_Command:             return EK_LWIN;
	case kVK_Command - 1:         return EK_RWIN;
	case kVK_RightShift:          return EK_RSHIFT;
	case kVK_RightControl:        return EK_RCONTROL;
	case kVK_RightOption:         return EK_RALT;
	case kVK_Delete:              return EK_BACKSPACE;
	case kVK_Tab:                 return EK_TAB;
	case kVK_Return:              return EK_RETURN;
	case kVK_Escape:              return EK_ESCAPE;
	case kVK_Space:               return EK_SPACE;
	case kVK_PageUp:              return EK_PAGEUP;
	case kVK_PageDown:            return EK_PAGEDN;
	case kVK_End:                 return EK_END;
	case kVK_Home:                return EK_HOME;
	case kVK_LeftArrow:           return EK_LEFT;
	case kVK_UpArrow:             return EK_UP;
	case kVK_RightArrow:          return EK_RIGHT;
	case kVK_DownArrow:           return EK_DOWN;
	case kVK_Help:                return EK_INSERT;
	case kVK_ForwardDelete:       return EK_DELETE;
	case kVK_F14:                 return EK_SCROLL;
	case kVK_F15:                 return EK_PAUSE;
	case kVK_ANSI_KeypadClear:    return EK_NUMLOCK;
	case kVK_ANSI_Semicolon:      return EK_SEMICOLON;
	case kVK_ANSI_Equal:          return EK_EQUALS;
	case kVK_ANSI_Comma:          return EK_COMMA;
	case kVK_ANSI_Minus:          return EK_MINUS;
	case kVK_ANSI_Slash:          return EK_SLASH;
	case kVK_ANSI_Backslash:      return EK_BACKSLASH;
	case kVK_ANSI_Grave:          return EK_BACKTICK;
	case kVK_ANSI_Quote:          return EK_TICK;
	case kVK_ANSI_LeftBracket:    return EK_LSQUARE;
	case kVK_ANSI_RightBracket:   return EK_RSQUARE;
	case kVK_ANSI_Period:         return EK_DOT;
	default:                      return 0;
	}
}
#endif

static void _onevent(NSEvent * event)
{
	if(!event)
		return;

	NSEventType eventType = [event type];
	switch(eventType)
	{
	#ifdef ENTRYPOINT_PROVIDE_INPUT
	case NSMouseMoved:
	case NSLeftMouseDragged:
	case NSRightMouseDragged:
	case NSOtherMouseDragged:
	{
		NSWindow * current_window = [NSApp keyWindow];
		NSRect adjust_frame = [[current_window contentView] frame];
		NSPoint p = [current_window mouseLocationOutsideOfEventStream];

		// map input to content view rect
		if(p.x < 0)
			p.x = 0;
		else if(p.x > adjust_frame.size.width)
			p.x = adjust_frame.size.width;
		if(p.y < 0)
			p.y = 0;
		else if(p.y > adjust_frame.size.height)
			p.y = adjust_frame.size.height;

			p.y = adjust_frame.size.height - p.y;

		// map input to pixels
		NSRect r = {p.x, p.y, 0, 0};
		#ifdef ENTRYPOINT_MACOS_RETINA
		r = [[current_window contentView] convertRectToBacking:r];
		#endif
		p = r.origin;

		ctx.touch.x = p.x;
		ctx.touch.y = p.y;
		ctx.touch.multitouch[0].x = p.x;
		ctx.touch.multitouch[0].y = p.y;
		break;
	}
	case NSLeftMouseDown:	ctx.touch.left = 1; ctx.touch.multitouch[0].touched = true; break;
	case NSLeftMouseUp:		ctx.touch.left = 0; ctx.touch.multitouch[0].touched = false; break;
	case NSRightMouseDown:	ctx.touch.right = 1; break;
	case NSRightMouseUp:	ctx.touch.right = 0; break;
	case NSOtherMouseDown:
		if([event buttonNumber] == 2) // number == 2 is a middle button
			ctx.touch.middle = 1;
		break;
	case NSOtherMouseUp:
		if([event buttonNumber] == 2) // number == 2 is a middle button
			ctx.touch.middle = 0;
		break;
//	case NSScrollWheel:
//	{
//		CGFloat deltaX = [event scrollingDeltaX];
//		CGFloat deltaY = [event scrollingDeltaY];
//
//		BOOL precisionScrolling = [event hasPreciseScrollingDeltas];
//		if(precisionScrolling)
//		{
//			deltaX *= 0.1f; // similar to glfw
//			deltaY *= 0.1f;
//		}
//
//		if(fabs(deltaX) > 0.0f || fabs(deltaY) > 0.0f)
//			printf("mouse scroll wheel delta %f %f\n", deltaX, deltaY);
//		break;
//	}
	case NSFlagsChanged:
	{
		NSEventModifierFlags modifiers = [event modifierFlags];

		// based on NSEventModifierFlags
		struct
		{
			union
			{
				struct
				{
					uint8_t alpha_shift:1;
					uint8_t shift:1;
					uint8_t control:1;
					uint8_t alternate:1;
					uint8_t command:1;
					uint8_t numeric_pad:1;
					uint8_t help:1;
					uint8_t function:1;
				};
				uint8_t mask;
			};
		} keys;

		keys.mask = (modifiers & NSDeviceIndependentModifierFlagsMask) >> 16;

		// TODO L,R variation of keys?
		ctx.keys[EK_LCONTROL] = keys.alpha_shift;
		ctx.keys[EK_LSHIFT]   = keys.shift;
		ctx.keys[EK_LCONTROL] = keys.control;
		ctx.keys[EK_RCONTROL] = keys.alpha_shift;
		ctx.keys[EK_RSHIFT]   = keys.shift;
		ctx.keys[EK_RCONTROL] = keys.control;
		ctx.keys[EK_ALT]      = keys.alternate;
		ctx.keys[EK_LWIN]     = keys.command;
		ctx.keys[EK_RWIN]     = keys.command;
		break;
	}
	case NSKeyDown:
	{
		NSString * characters = [event characters];
		NSData * utf32 = [characters dataUsingEncoding:NSUTF32StringEncoding];
		uint32_t * text = (uint32_t*)[utf32 bytes];
		if(text)
			ctx.last_char = text[text[0] == 0xfeff ? 1 : 0];
		ctx.keys[_macoskey([event keyCode])] = 1;
		break;
	}
	case NSKeyUp:
		ctx.keys[_macoskey([event keyCode])] = 0;
		break;
	#endif
	default:
		break;
	}

	[NSApp sendEvent:event];

	// if user closes the window we might need to terminate asap
	if(!ctx.terminated)
		[NSApp updateWindows];
}

ep_size_t ep_size()
{
	ep_size_t r;

	NSWindow * current_window = [NSApp keyWindow];
	NSRect rect = [[current_window contentView] frame];
	#ifdef ENTRYPOINT_MACOS_RETINA
	rect = [[current_window contentView] convertRectToBacking:rect];
	#endif
	r.w = rect.size.width;
	r.h = rect.size.height;

	return r;
}

bool ep_retina()
{
	#ifdef ENTRYPOINT_MACOS_RETINA
		return true;
	#else
		return false;
	#endif
}

// -----------------------------------------------------------------------------

static bool _infocus() {return [NSApp keyWindow] == ctx.window;}

int main(int argc, char * argv[])
{
	@autoreleasepool
	{
	ctx.argc = argc;
	ctx.argv = argv;

	ENTRYPOINT_MACOS_PREPARE_PARAMS;

	// poke it first to get things going
	[NSApplication sharedApplication];
	[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
	AppDelegate * dg =[[AppDelegate alloc] init]; // needs to be on a separate line
	[NSApp setDelegate:dg];
	[NSApp finishLaunching]; // only needed if we don't use [NSApp run]

	// create main menu
	NSMenu * menubar = [[NSMenu alloc] init];
	NSMenuItem * app_menu_item = [[NSMenuItem alloc] init];
	[menubar addItem:app_menu_item];
	[NSApp setMainMenu:menubar];
	NSMenu * app_menu = [[NSMenu alloc] init];
	NSString * app_name = [[NSProcessInfo processInfo] processName];
	NSString * quit_title = [@"Quit " stringByAppendingString:app_name];
	NSMenuItem * menuitem = [[NSMenuItem alloc] initWithTitle:quit_title action:@selector(terminate:) keyEquivalent:@"q"];
	[app_menu addItem:menuitem];
	[app_menu_item setSubmenu:app_menu];

	// create the window
	NSWindow * window = [[NSWindow alloc]
		initWithContentRect:NSMakeRect(0, 0, ENTRYPOINT_MACOS_WIDTH, ENTRYPOINT_MACOS_HEIGHT)
		styleMask:NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
		backing:NSBackingStoreBuffered
		defer:NO];
	ctx.window = (__bridge void *)(window); // we still own it here, but put a reference for everyone else
	[window setReleasedWhenClosed:NO];
	ctx.window_count = 1;
	WindowDelegate * wdg = [[WindowDelegate alloc] init];
	[window setDelegate:wdg];

	// configure the view
	NSView * content_view = [window contentView];
	#ifdef ENTRYPOINT_MACOS_RETINA
	[content_view setWantsBestResolutionOpenGLSurface:YES];
	#endif

	[window cascadeTopLeftFromPoint:NSMakePoint(ENTRYPOINT_MACOS_START_X, ENTRYPOINT_MACOS_START_Y)];
	[window setTitle:ENTRYPOINT_MACOS_TITLE];
	[window makeKeyAndOrderFront:window];
	[window setAcceptsMouseMovedEvents:YES];
	[window setBackgroundColor:[NSColor blackColor]];

	[NSApp activateIgnoringOtherApps:YES]; // TODO do we really need this?

	int32_t result_code = 0;
	if((result_code = entrypoint_init(ctx.argc, ctx.argv)) != 0)
		return result_code;

	while(entrypoint_loop() == 0 && ctx.terminated == false)
	{
		#ifdef ENTRYPOINT_PROVIDE_INPUT
		if(_infocus())
			memcpy(ctx.prev, ctx.keys, sizeof(ctx.prev));
		#endif

		NSEvent * event = nil;
		while((event = [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES]) && !ctx.terminated)
			_onevent(event);
	}

	if((result_code = entrypoint_might_unload()) != 0)
		return result_code;

	if((result_code = entrypoint_deinit()) != 0)
		return result_code;

	return 0;
	}
}


// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_TIME

double ep_delta_time()
{
	if(ctx.timebase_info.denom == 0)
	{
		mach_timebase_info(&ctx.timebase_info);
		ctx.prev_time = mach_absolute_time();
		return 0.0f;
	}

	uint64_t current_time = mach_absolute_time();
	double elapsed = (double)(current_time - ctx.prev_time) * ctx.timebase_info.numer / (ctx.timebase_info.denom * 1000000000.0);
	ctx.prev_time = current_time;
	return elapsed;
}

void ep_sleep(double seconds)
{
	usleep((useconds_t)(seconds * 1000000.0));
}

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_LOG

void ep_log(const char * message, ...)
{
	va_list args;
	va_start(args, message);
	vprintf(message, args);
	fflush(stdout);
	va_end(args);
}

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_INPUT

void ep_touch(ep_touch_t * touch)
{
	if(touch)
		*touch = ctx.touch;
}

bool ep_khit(int32_t key)
{
	if(!_infocus())
		return 0;
	assert(key < sizeof(ctx.keys));
	if (key >= 'a' && key <= 'z') key = key - 'a' + 'A';
	return ctx.keys[key] && !ctx.prev[key];
}

bool ep_kdown(int32_t key)
{
	if(!_infocus())
		return 0;
	assert(key < sizeof(ctx.keys));
	if (key >= 'a' && key <= 'z') key = key - 'a' + 'A';
	return ctx.keys[key];
}

uint32_t ep_kchar()
{
	if(!_infocus())
		return 0;
	uint32_t k = ctx.last_char;
	ctx.last_char = 0;
	return k;
}

#endif

// -----------------------------------------------------------------------------

#endif
