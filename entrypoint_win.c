
// TODO:
// - would be nice to have proper fullscreen mode

#ifdef _WIN32

#define ENTRYPOINT_CTX
#include "entrypoint.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#if defined(ENTRYPOINT_WINDOWS_NEED_ARGS) && defined(ENTRYPOINT_WINDOWS_WINMAIN)
#include <shellapi.h>
#endif

static entrypoint_ctx_t ctx = {0};
entrypoint_ctx_t * ep_ctx() {return &ctx;}

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_WINDOWS_LINKER_OPT
	#pragma comment(lib, "User32.lib")
	#ifdef ENTRYPOINT_WINDOWS_SUBSYSTEM
		#ifdef _MSC_VER
			#pragma comment(linker, "/subsystem:windows")
			#ifdef ENTRYPOINT_WINDOWS_WINMAIN
				#ifdef ENTRYPOINT_WINDOWS_NEED_ARGS
					#pragma comment(lib, "Shell32.lib")
				#endif
			#else
				// use main directly
				#pragma comment(linker, "/ENTRY:mainCRTStartup")
			#endif
		#endif
	#endif
#endif

// -----------------------------------------------------------------------------

static void _fail(const char * msg)
{
	fprintf(stderr, "%s - failed, error code %u\n", msg, (uint32_t)GetLastError());
}

// -----------------------------------------------------------------------------

static void _enter_borderless()
{
	if(ctx.flag_borderless)
		return;

	if(!GetWindowRect(ctx.hwnd, &ctx.rect_saved))
	{
		_fail("get window rect");
		return;
	}

	MONITORINFO mi = {sizeof(mi)};
	if(!GetMonitorInfo(MonitorFromWindow(ctx.hwnd, MONITOR_DEFAULTTONEAREST), &mi))
	{
		_fail("get monitor info");
		return;
	}

	DWORD old_style = ctx.dwStyle;
	ctx.dwStyle = WS_VISIBLE | WS_POPUP;
	if(!SetWindowLong(ctx.hwnd, GWL_STYLE, ctx.dwStyle))
	{
		ctx.dwStyle = old_style;
		_fail("set window style");
		return;
	}

	if(!SetWindowPos(ctx.hwnd, HWND_TOP,
		mi.rcMonitor.left,
		mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,
		0))
	{
		_fail("set window pos");
		return;
	}

	ctx.flag_borderless = 1;
}

static void _leave_borderless()
{
	if(!ctx.flag_borderless)
		return;

	DWORD old_style = ctx.dwStyle;
	ctx.dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	if(!SetWindowLong(ctx.hwnd, GWL_STYLE, ctx.dwStyle))
	{
		ctx.dwStyle = old_style;
		_fail("set window style");
		return;
	}

	if(!SetWindowPos(ctx.hwnd, NULL,
		ctx.rect_saved.left,
		ctx.rect_saved.top,
		ctx.rect_saved.right - ctx.rect_saved.left,
		ctx.rect_saved.bottom - ctx.rect_saved.top,
		0))
	{
		_fail("set window pos");
		return;
	}

	ctx.flag_borderless = 0;
}

// -----------------------------------------------------------------------------

LRESULT CALLBACK tigrWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_PAINT:
	{
		#ifdef ENTRYPOINT_WINDOWS_DO_WMPAINT
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(ctx.hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(ctx.hwnd, &ps);
		#endif
		ValidateRect(hWnd, NULL);
		return 0;
	}
	case WM_CLOSE:
		ctx.flag_want_to_close = 1;
		return 0;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO * info = (MINMAXINFO*)lParam;
		info->ptMinTrackSize.x = ENTRYPOINT_WINDOWS_MIN_WIDTH;
		info->ptMinTrackSize.y = ENTRYPOINT_WINDOWS_MIN_HEIGHT;
		return 0;
	}
	case WM_SIZING:
		return TRUE;
	case WM_SIZE:
		// if someone tried to maximize us (e.g. via shortcut launch options), prefer instead to be borderless
		if (wParam == SIZE_MAXIMIZED)
		{
			ShowWindow(ctx.hwnd, SW_NORMAL);
			_enter_borderless();
		}
		return 0;
#ifdef ENTRYPOINT_PROVIDE_INPUT
	case WM_ACTIVATE:
		memset(ctx.keys, 0, sizeof(ctx.keys));
		memset(ctx.prev, 0, sizeof(ctx.prev));
		ctx.last_char = 0;
		return 0;
	case WM_CHAR:
		if(wParam == '\r')
			wParam = '\n';
		ctx.last_char = wParam;
		break;
	case WM_MENUCHAR:
		if(LOWORD(wParam) == VK_RETURN) // disable beep on Alt+Enter
			return MNC_CLOSE << 16;
		break;
	case WM_SYSKEYDOWN:
		if(wParam == VK_RETURN)
		{
			// Alt+Enter
			if(ctx.dwStyle & WS_POPUP)
				_leave_borderless();
			else
				_enter_borderless();
			return 0;
		} // fall through
	case WM_KEYDOWN:
		ctx.keys[wParam] = 1;
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		ctx.keys[wParam] = 0;
		break;
#endif
	default:
		break;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}

// -----------------------------------------------------------------------------

ep_size_t ep_size()
{
	ep_size_t r;
	GetClientRect(ctx.hwnd, &ctx.rect);
	r.w = ctx.rect.right - ctx.rect.left;
	r.h = ctx.rect.bottom - ctx.rect.top;
	return r;
}

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_WINDOWS_NEED_ARGS
static void _getargs()
{
	LPWSTR * wargv = CommandLineToArgvW(GetCommandLineW(), &ctx.argc);
	if(wargv)
	{
		ctx.argv = (char**)calloc(ctx.argc + 1, sizeof(char*));
		for(int i = 0; i < ctx.argc; ++i)
		{
			int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, 0, 0, NULL, NULL);
			if(len)
			{
				ctx.argv[i] = (char*)calloc(len, sizeof(char));
				WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, ctx.argv[i], len, NULL, NULL);
			}
			else
				ctx.argv[i] = (char*)calloc(1, sizeof(char));
		}
		LocalFree(wargv);
	}
}
static void _freeargs()
{
	for(int i = 0; i < ctx.argc; ++i)
		free(ctx.argv[i]);
	if(ctx.argv)
		free(ctx.argv);
	ctx.argc = 0;
	ctx.argv = NULL;
}
#else
static void _getargs(){} // noop
static void _freeargs(){}
#endif

// -----------------------------------------------------------------------------

static int32_t _freewindow(int32_t errorcode)
{
	if(!DestroyWindow(ctx.hwnd))
	{
		_fail("destroy window");
		_freeargs();
		return 1;
	}

	ctx.hwnd = 0;

	if(!UnregisterClassW(ENTRYPOINT_WINDOWS_CLASS, GetModuleHandle(NULL)))
	{
		_fail("unregister class");
		_freeargs();
		return 1;
	}

	_freeargs();
	return errorcode;
}

#ifdef ENTRYPOINT_WINDOWS_WINMAIN
#ifdef UNICODE
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#endif
{
	ctx.argc = 0;
	ctx.argv = NULL;
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;
	_getargs();

#else
int main(int argc, char * argv[])
{
	ctx.argc = argc;
	ctx.argv = argv;
#endif

	ENTRYPOINT_WINDOWS_PREPARE_PARAMS;

	WNDCLASSEXW wcex = {0};
	wcex.cbSize			= sizeof(WNDCLASSEXW);
	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= tigrWndProc;
	wcex.hInstance		= GetModuleHandle(NULL);
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName	= ENTRYPOINT_WINDOWS_CLASS;
	if(!RegisterClassExW(&wcex))
	{
		_fail("register class");
		_freeargs();
		return 1;
	}

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	RECT rc = {0, 0, ENTRYPOINT_WINDOWS_WIDTH, ENTRYPOINT_WINDOWS_HEIGHT};
	AdjustWindowRect(&rc, dwStyle, FALSE);

	ctx.hwnd = CreateWindowW(ENTRYPOINT_WINDOWS_CLASS, ENTRYPOINT_WINDOWS_TITLE, dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, wcex.hInstance, NULL);

	if(!ctx.hwnd)
	{
		_fail("create window");
		_freeargs();
		return 1;
	}

	ShowWindow(ctx.hwnd, SW_SHOW);
	SetForegroundWindow(ctx.hwnd);
	SetFocus(ctx.hwnd);

	int32_t result_code = 0;
	if((result_code = entrypoint_init(ctx.argc, ctx.argv)) != 0)
	{
		_fail("entrypoint init");
		return _freewindow(result_code);
	}

	while(entrypoint_loop() == 0 && ctx.flag_want_to_close == false)
	{
		#ifdef ENTRYPOINT_PROVIDE_INPUT
		memcpy(ctx.prev, ctx.keys, sizeof(ctx.prev));
		#endif

		MSG msg;
		while(PeekMessage(&msg, ctx.hwnd, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				ctx.flag_want_to_close = 1;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		GetClientRect(ctx.hwnd, &ctx.rect);
	}

	if((result_code = entrypoint_might_unload()) != 0)
	{
		_fail("entrypoint might unload");
		return _freewindow(result_code);
	}

	if((result_code = entrypoint_deinit()) != 0)
	{
		_fail("entrypoint deinit");
		return _freewindow(result_code);
	}

	return 0;
}

#endif

// -----------------------------------------------------------------------------
#ifdef ENTRYPOINT_PROVIDE_TIME

double ep_time()
{
	LARGE_INTEGER qpc, freq;
	QueryPerformanceCounter(&qpc);
	QueryPerformanceFrequency(&freq);

	if(ctx.flag_time_set)
	{
		ULONGLONG diff = qpc.QuadPart - ctx.prev_qpc_time.QuadPart;
		ctx.prev_qpc_time = qpc;
		return diff / (double)freq.QuadPart;
	}
	else
	{
		ctx.flag_time_set = 1;
		ctx.prev_qpc_time = qpc;
		return 0;
	}
}

void ep_sleep(double seconds)
{
	if(seconds > 0.0f)
		Sleep((uint32_t)(seconds * 1000));
}

#endif
// -----------------------------------------------------------------------------
#ifdef ENTRYPOINT_PROVIDE_INPUT

void ep_touch(ep_touch_t * touch)
{
	if(!touch)
		return;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(ctx.hwnd, &pt);
	GetClientRect(ctx.hwnd, &ctx.rect);
	touch->x = pt.x - ctx.rect.left;
	touch->y = pt.y - ctx.rect.top;

	touch->flags = 0;
	if(GetFocus() == ctx.hwnd)
	{
		if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) touch->left = 1;
		if(GetAsyncKeyState(VK_MBUTTON) & 0x8000) touch->middle = 1;
		if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) touch->right = 1;
	}

	touch->multitouch[0].x = touch->x;
	touch->multitouch[0].y = touch->y;
	touch->multitouch[0].touched = touch->left;
}

int32_t _vkkey(int32_t key)
{
	if (key >= 'A' && key <= 'Z') return key;
	if (key >= 'a' && key <= 'z') return key - 'a' + 'A';
	if (key >= '0' && key <= '9') return key;
	switch (key) {
	case EK_BACKSPACE: return VK_BACK;
	case EK_TAB: return VK_TAB;
	case EK_RETURN: return VK_RETURN;
	case EK_ALT: return VK_MENU;
	case EK_PAUSE: return VK_PAUSE;
	case EK_CAPSLOCK: return VK_CAPITAL;
	case EK_ESCAPE: return VK_ESCAPE;
	case EK_SPACE: return VK_SPACE;
	case EK_PAGEUP: return VK_PRIOR;
	case EK_PAGEDN: return VK_NEXT;
	case EK_END: return VK_END;
	case EK_HOME: return VK_HOME;
	case EK_LEFT: return VK_LEFT;
	case EK_UP: return VK_UP;
	case EK_RIGHT: return VK_RIGHT;
	case EK_DOWN: return VK_DOWN;
	case EK_INSERT: return VK_INSERT;
	case EK_DELETE: return VK_DELETE;
	case EK_LWIN: return VK_LWIN;
	case EK_RWIN: return VK_RWIN;
	//casEKTK_APPS: return VK_APPS; // this key doesn't exist on OS X
	case EK_PAD0: return VK_NUMPAD0;
	case EK_PAD1: return VK_NUMPAD1;
	case EK_PAD2: return VK_NUMPAD2;
	case EK_PAD3: return VK_NUMPAD3;
	case EK_PAD4: return VK_NUMPAD4;
	case EK_PAD5: return VK_NUMPAD5;
	case EK_PAD6: return VK_NUMPAD6;
	case EK_PAD7: return VK_NUMPAD7;
	case EK_PAD8: return VK_NUMPAD8;
	case EK_PAD9: return VK_NUMPAD9;
	case EK_PADMUL: return VK_MULTIPLY;
	case EK_PADADD: return VK_ADD;
	case EK_PADENTER: return VK_SEPARATOR;
	case EK_PADSUB: return VK_SUBTRACT;
	case EK_PADDOT: return VK_DECIMAL;
	case EK_PADDIV: return VK_DIVIDE;
	case EK_F1: return VK_F1;
	case EK_F2: return VK_F2;
	case EK_F3: return VK_F3;
	case EK_F4: return VK_F4;
	case EK_F5: return VK_F5;
	case EK_F6: return VK_F6;
	case EK_F7: return VK_F7;
	case EK_F8: return VK_F8;
	case EK_F9: return VK_F9;
	case EK_F10: return VK_F10;
	case EK_F11: return VK_F11;
	case EK_F12: return VK_F12;
	case EK_NUMLOCK: return VK_NUMLOCK;
	case EK_SCROLL: return VK_SCROLL;
	case EK_LSHIFT: return VK_LSHIFT;
	case EK_RSHIFT: return VK_RSHIFT;
	case EK_LCONTROL: return VK_LCONTROL;
	case EK_RCONTROL: return VK_RCONTROL;
	case EK_LALT: return VK_LMENU;
	case EK_RALT: return VK_RMENU;
	case EK_SEMICOLON: return VK_OEM_1;
	case EK_EQUALS: return VK_OEM_PLUS;
	case EK_COMMA: return VK_OEM_COMMA;
	case EK_MINUS: return VK_OEM_MINUS;
	case EK_DOT: return VK_OEM_PERIOD;
	case EK_SLASH: return VK_OEM_2;
	case EK_BACKTICK: return VK_OEM_3;
	case EK_LSQUARE: return VK_OEM_4;
	case EK_BACKSLASH: return VK_OEM_5;
	case EK_RSQUARE: return VK_OEM_6;
	case EK_TICK: return VK_OEM_7;
	}
	return 0;
}

bool ep_khit(int32_t key)
{
	if(GetFocus() != ctx.hwnd)
		return 0;
	int k = _vkkey(key);
	return ctx.keys[k] && !ctx.prev[k];
}

bool ep_kdown(int32_t key)
{
	if(GetFocus() != ctx.hwnd)
		return 0;
	int k = _vkkey(key);
	return ctx.keys[k];
}

uint32_t ep_kchar()
{
	if(GetFocus() != ctx.hwnd)
		return 0;
	uint32_t k = ctx.last_char;
	ctx.last_char = 0;
	return k;
}
#endif
