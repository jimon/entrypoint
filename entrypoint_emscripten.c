
// this is a bit rough implementation
// please take care if you plan to use it in production

#ifdef EMSCRIPTEN

#define ENTRYPOINT_CTX
#include "entrypoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <emscripten.h>
#ifdef ENTRYPOINT_PROVIDE_INPUT
#include <html5.h>
#endif

static entrypoint_ctx_t ctx = {0};
entrypoint_ctx_t * ep_ctx() {return &ctx;}

// -----------------------------------------------------------------------------

ep_size_t ep_size()
{
	double w = 0.0, h = 0.0;
	emscripten_get_element_css_size(NULL, &w, &h);
	ep_size_t r;
	r.w = w;
	r.h = h;
	return r;
}

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_INPUT

// ported from SDL2 emscripten fork
// .keyCode to scancode
// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent
// https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode
static const uint8_t emscripten_scancode_table[] = {
	/*  0 */	0,
	/*  1 */	0,
	/*  2 */	0,
	/*  3 */	0,
	/*  4 */	0,
	/*  5 */	0,
	/*  6 */	0,
	/*  7 */	0,
	/*  8 */	EK_BACKSPACE,
	/*  9 */	EK_TAB,
	/*  10 */	0,
	/*  11 */	0,
	/*  12 */	0,
	/*  13 */	EK_RETURN,
	/*  14 */	0,
	/*  15 */	0,
	/*  16 */	EK_LSHIFT,
	/*  17 */	EK_LCONTROL,
	/*  18 */	EK_LALT,
	/*  19 */	EK_PAUSE,
	/*  20 */	EK_CAPSLOCK,
	/*  21 */	0,
	/*  22 */	0,
	/*  23 */	0,
	/*  24 */	0,
	/*  25 */	0,
	/*  26 */	0,
	/*  27 */	EK_ESCAPE,
	/*  28 */	0,
	/*  29 */	0,
	/*  30 */	0,
	/*  31 */	0,
	/*  32 */	EK_SPACE,
	/*  33 */	EK_PAGEUP,
	/*  34 */	EK_PAGEDN,
	/*  35 */	EK_END,
	/*  36 */	EK_HOME,
	/*  37 */	EK_LEFT,
	/*  38 */	EK_UP,
	/*  39 */	EK_RIGHT,
	/*  40 */	EK_DOWN,
	/*  41 */	0,
	/*  42 */	0,
	/*  43 */	0,
	/*  44 */	0,
	/*  45 */	EK_INSERT,
	/*  46 */	EK_DELETE,
	/*  47 */	0,
	/*  48 */	'0',
	/*  49 */	'1',
	/*  50 */	'2',
	/*  51 */	'3',
	/*  52 */	'4',
	/*  53 */	'5',
	/*  54 */	'6',
	/*  55 */	'7',
	/*  56 */	'8',
	/*  57 */	'9',
	/*  58 */	0,
	/*  59 */	EK_SEMICOLON,
	/*  60 */	0,
	/*  61 */	EK_EQUALS,
	/*  62 */	0,
	/*  63 */	0,
	/*  64 */	0,
	/*  65 */	'A',
	/*  66 */	'B',
	/*  67 */	'C',
	/*  68 */	'D',
	/*  69 */	'E',
	/*  70 */	'F',
	/*  71 */	'G',
	/*  72 */	'H',
	/*  73 */	'I',
	/*  74 */	'J',
	/*  75 */	'K',
	/*  76 */	'L',
	/*  77 */	'M',
	/*  78 */	'N',
	/*  79 */	'O',
	/*  80 */	'P',
	/*  81 */	'Q',
	/*  82 */	'R',
	/*  83 */	'S',
	/*  84 */	'T',
	/*  85 */	'U',
	/*  86 */	'V',
	/*  87 */	'W',
	/*  88 */	'X',
	/*  89 */	'Y',
	/*  90 */	'Z',
	/*  91 */	EK_LWIN,
	/*  92 */	0,
	/*  93 */	0, // EK_APPLICATION,
	/*  94 */	0,
	/*  95 */	0,
	/*  96 */	EK_PAD0,
	/*  97 */	EK_PAD1,
	/*  98 */	EK_PAD2,
	/*  99 */	EK_PAD3,
	/* 100 */	EK_PAD4,
	/* 101 */	EK_PAD5,
	/* 102 */	EK_PAD6,
	/* 103 */	EK_PAD7,
	/* 104 */	EK_PAD8,
	/* 105 */	EK_PAD9,
	/* 106 */	EK_PADMUL,
	/* 107 */	EK_PADADD,
	/* 108 */	0, // isn't it EK_PADENTER ?
	/* 109 */	EK_PADSUB,
	/* 110 */	EK_PADDOT,
	/* 111 */	EK_PADDIV,
	/* 112 */	EK_F1,
	/* 113 */	EK_F2,
	/* 114 */	EK_F3,
	/* 115 */	EK_F4,
	/* 116 */	EK_F5,
	/* 117 */	EK_F6,
	/* 118 */	EK_F7,
	/* 119 */	EK_F8,
	/* 120 */	EK_F9,
	/* 121 */	EK_F10,
	/* 122 */	EK_F11,
	/* 123 */	EK_F12,
	/* 124 */	0, // EK_F13
	/* 125 */	0, // EK_F14
	/* 126 */	0, // EK_F15
	/* 127 */	0, // EK_F16
	/* 128 */	0, // EK_F17
	/* 129 */	0, // EK_F18
	/* 130 */	0, // EK_F19
	/* 131 */	0, // EK_F20
	/* 132 */	0, // EK_F21
	/* 133 */	0, // EK_F22
	/* 134 */	0, // EK_F23
	/* 135 */	0, // EK_F24
	/* 136 */	0,
	/* 137 */	0,
	/* 138 */	0,
	/* 139 */	0,
	/* 140 */	0,
	/* 141 */	0,
	/* 142 */	0,
	/* 143 */	0,
	/* 144 */	EK_NUMLOCK,
	/* 145 */	EK_SCROLL,
	/* 146 */	0,
	/* 147 */	0,
	/* 148 */	0,
	/* 149 */	0,
	/* 150 */	0,
	/* 151 */	0,
	/* 152 */	0,
	/* 153 */	0,
	/* 154 */	0,
	/* 155 */	0,
	/* 156 */	0,
	/* 157 */	0,
	/* 158 */	0,
	/* 159 */	0,
	/* 160 */	0,
	/* 161 */	0,
	/* 162 */	0,
	/* 163 */	0,
	/* 164 */	0,
	/* 165 */	0,
	/* 166 */	0,
	/* 167 */	0,
	/* 168 */	0,
	/* 169 */	0,
	/* 170 */	0,
	/* 171 */	0,
	/* 172 */	0,
	/* 173 */	EK_MINUS, /*FX*/
	/* 174 */	0, // EK_VOLUMEDOWN, /*IE, Chrome*/
	/* 175 */	0, // EK_VOLUMEUP, /*IE, Chrome*/
	/* 176 */	0, // EK_AUDIONEXT, /*IE, Chrome*/
	/* 177 */	0, // EK_AUDIOPREV, /*IE, Chrome*/
	/* 178 */	0,
	/* 179 */	0, // EK_AUDIOPLAY, /*IE, Chrome*/
	/* 180 */	0,
	/* 181 */	0, // EK_AUDIOMUTE, /*FX*/
	/* 182 */	0, // EK_VOLUMEDOWN, /*FX*/
	/* 183 */	0, // EK_VOLUMEUP, /*FX*/
	/* 184 */	0,
	/* 185 */	0,
	/* 186 */	EK_SEMICOLON, /*IE, Chrome, D3E legacy*/
	/* 187 */	EK_EQUALS, /*IE, Chrome, D3E legacy*/
	/* 188 */	EK_COMMA,
	/* 189 */	EK_MINUS, /*IE, Chrome, D3E legacy*/
	/* 190 */	EK_DOT,
	/* 191 */	EK_SLASH,
	/* 192 */	EK_BACKTICK, /*FX, D3E legacy (EK_APOSTROPHE in IE/Chrome)*/
	/* 193 */	0,
	/* 194 */	0,
	/* 195 */	0,
	/* 196 */	0,
	/* 197 */	0,
	/* 198 */	0,
	/* 199 */	0,
	/* 200 */	0,
	/* 201 */	0,
	/* 202 */	0,
	/* 203 */	0,
	/* 204 */	0,
	/* 205 */	0,
	/* 206 */	0,
	/* 207 */	0,
	/* 208 */	0,
	/* 209 */	0,
	/* 210 */	0,
	/* 211 */	0,
	/* 212 */	0,
	/* 213 */	0,
	/* 214 */	0,
	/* 215 */	0,
	/* 216 */	0,
	/* 217 */	0,
	/* 218 */	0,
	/* 219 */	EK_LSQUARE,
	/* 220 */	EK_BACKSLASH,
	/* 221 */	EK_RSQUARE,
	/* 222 */	EK_TICK, /*FX, D3E legacy*/
};

static EM_BOOL _key_cb(int event_type, const EmscriptenKeyboardEvent * key_event, void * notused)
{
	if(key_event->keyCode < sizeof(emscripten_scancode_table) / sizeof(emscripten_scancode_table[0]))
	{
		uint8_t c = emscripten_scancode_table[key_event->keyCode];
		if(key_event->location == DOM_KEY_LOCATION_RIGHT)
		{
			switch(c)
			{
			case EK_LSHIFT: c = EK_RSHIFT; break;
			case EK_LCONTROL: c = EK_RCONTROL; break;
			case EK_LALT: c = EK_RALT; break;
			case EK_LWIN: c = EK_RWIN; break;
			default: break;
			}
		}
		ctx.keys[c] = event_type == EMSCRIPTEN_EVENT_KEYDOWN ? 1 : 0;
	}

	ctx.last_char = key_event->charCode;

	// TODO fix this
	// if TEXTINPUT events are enabled we can't prevent keydown or we won't get keypress
	// we need to ALWAYS prevent backspace and tab otherwise chrome takes action and does bad navigation UX
	//if (event_type == EMSCRIPTEN_EVENT_KEYDOWN && SDL_GetEventState(SDL_TEXTINPUT) == SDL_ENABLE && key_event->keyCode != 8 /* backspace */ && key_event->keyCode != 9 /* tab */)
	//	return false;
	return true;
}

static EM_BOOL _mouse_move_cb(int event_type, const EmscriptenMouseEvent * mouse_event, void * notused)
{
	ctx.touch.x = mouse_event->canvasX;
	ctx.touch.y = mouse_event->canvasY;
	return false;
}

static EM_BOOL _mouse_key_cb(int event_type, const EmscriptenMouseEvent * mouse_event, void * notused)
{
	bool v = event_type == EMSCRIPTEN_EVENT_MOUSEDOWN;
	if(mouse_event->button == 0) ctx.touch.left = v;
	else if(mouse_event->button == 1) ctx.touch.middle = v;
	else if(mouse_event->button == 2) ctx.touch.right = v;
	return true;
}

static EM_BOOL _touch_cb(int event_type, const EmscriptenTouchEvent * touch_event, void * notused)
{
	if(touch_event->numTouches > 0)
	{
		ctx.touch.x = touch_event->touches[0].canvasX;
		ctx.touch.y = touch_event->touches[0].canvasY;
	}
	ctx.touch.left = event_type == EMSCRIPTEN_EVENT_TOUCHSTART || event_type == EMSCRIPTEN_EVENT_TOUCHMOVE;
	return true;
}
#endif

// -----------------------------------------------------------------------------

static void _tick()
{
	if(ctx.flag_want_to_close)
		return;

	if(entrypoint_loop() != 0)
		ctx.flag_want_to_close = true;

	#ifdef ENTRYPOINT_PROVIDE_INPUT
	memcpy(ctx.prev, ctx.keys, sizeof(ctx.prev));
	#endif
}

int main(int argc, char * argv[])
{
	ctx.argc = argc;
	ctx.argv = argv;

	// TODO emscripten_set_element_css_size ???

	int32_t result_code = 0;
	if((result_code = entrypoint_init(ctx.argc, ctx.argv)) != 0)
		return result_code;

	#ifdef ENTRYPOINT_PROVIDE_INPUT
	emscripten_set_keypress_callback("#window", NULL, false, _key_cb);
	emscripten_set_keydown_callback("#window", NULL, false, _key_cb);
	emscripten_set_keyup_callback("#window", NULL, false, _key_cb);

	emscripten_set_mousemove_callback("#canvas", NULL, 0, _mouse_move_cb);
	emscripten_set_mousedown_callback("#canvas", NULL, 0, _mouse_key_cb);
	emscripten_set_mouseup_callback("#document", NULL, 0, _mouse_key_cb);

	emscripten_set_touchstart_callback("#canvas", NULL, 0, _touch_cb);
	emscripten_set_touchend_callback("#canvas", NULL, 0, _touch_cb);
	emscripten_set_touchmove_callback("#canvas", NULL, 0, _touch_cb);
	emscripten_set_touchcancel_callback("#canvas", NULL, 0, _touch_cb);
	#endif

	emscripten_set_main_loop(_tick, -1, 1);

	entrypoint_might_unload(); // TODO add this to onclose
	entrypoint_deinit();

	return 0;
}

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_TIME

double ep_time()
{
	if(!ctx.flag_time_set)
	{
		gettimeofday(&ctx.prev_time, NULL);
		ctx.flag_time_set = true;
		return 0.0;
	}

	struct timeval now;
	gettimeofday(&now, NULL);
	double elapsed = (now.tv_sec - ctx.prev_time.tv_sec) + ((now.tv_usec - ctx.prev_time.tv_usec) / 1000000.0);
	ctx.prev_time = now;
	return elapsed;
}

void ep_sleep(double seconds)
{
	//emscripten_sleep(seconds); // TODO requires EMTERPRETIFY, probably not the wisest idea anyway
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
	assert(key < sizeof(ctx.keys));
	if (key >= 'a' && key <= 'z') key = key - 'a' + 'A';
	return ctx.keys[key] && !ctx.prev[key];
}

bool ep_kdown(int32_t key)
{
	assert(key < sizeof(ctx.keys));
	if (key >= 'a' && key <= 'z') key = key - 'a' + 'A';
	return ctx.keys[key];
}

uint32_t ep_kchar()
{
	uint32_t k = ctx.last_char;
	ctx.last_char = 0;
	return k;
}

#endif

// -----------------------------------------------------------------------------

#endif

