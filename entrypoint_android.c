
#ifdef __ANDROID__

// partially based on bgfx entry_android.cpp

#define ENTRYPOINT_CTX
#include "entrypoint.h"

#include <android/log.h>
#include <android/input.h>
#include <android/window.h>
#include <android/native_window.h>

#include <android_native_app_glue.c> // just so we don't need to build it

static entrypoint_ctx_t ctx = {0};
static ANativeWindow * new_window = NULL;
entrypoint_ctx_t * ep_ctx() {return &ctx;}

// -----------------------------------------------------------------------------

ep_size_t ep_size()
{
	ep_size_t r = {ctx.view_w, ctx.view_h};
	return r;
}

bool ep_retina()
{
	return false; // TODO
}

// -----------------------------------------------------------------------------

static void on_app_cmd(struct android_app * app, int32_t cmd)
{
	new_window = app->window;

	switch(cmd)
	{
	case APP_CMD_INPUT_CHANGED:
		// Command from main thread: the AInputQueue has changed.  Upon processing
		// this command, android_app->inputQueue will be updated to the new queue
		// (or NULL).
		break;

	case APP_CMD_INIT_WINDOW:
		// Command from main thread: a new ANativeWindow is ready for use.  Upon
		// receiving this command, android_app->window will contain the new window
		// surface.
		break;

	case APP_CMD_TERM_WINDOW:
		// Command from main thread: the existing ANativeWindow needs to be
		// terminated.  Upon receiving this command, android_app->window still
		// contains the existing window; after calling android_app_exec_cmd
		// it will be set to NULL.
		new_window = NULL;
		break;

	case APP_CMD_WINDOW_RESIZED:
		// Command from main thread: the current ANativeWindow has been resized.
		// Please redraw with its new size.
		break;

	case APP_CMD_WINDOW_REDRAW_NEEDED:
		// Command from main thread: the system needs that the current ANativeWindow
		// be redrawn.  You should redraw the window before handing this to
		// android_app_exec_cmd() in order to avoid transient drawing glitches.
		break;

	case APP_CMD_CONTENT_RECT_CHANGED:
		// Command from main thread: the content area of the window has changed,
		// such as from the soft input window being shown or hidden.  You can
		// find the new content rect in android_app::contentRect.
		break;

	case APP_CMD_GAINED_FOCUS:
		// Command from main thread: the app's activity window has gained
		// input focus.
		break;

	case APP_CMD_LOST_FOCUS:
		// Command from main thread: the app's activity window has lost
		// input focus.
		pthread_mutex_lock(&ctx.mutex);
		entrypoint_might_unload();
		pthread_mutex_unlock(&ctx.mutex);
		break;

	case APP_CMD_CONFIG_CHANGED:
		// Command from main thread: the current device configuration has changed.
		break;

	case APP_CMD_LOW_MEMORY:
		// Command from main thread: the system is running low on memory.
		// Try to reduce your memory use.
		pthread_mutex_lock(&ctx.mutex);
		entrypoint_might_unload();
		pthread_mutex_unlock(&ctx.mutex);
		break;

	case APP_CMD_START:
		// Command from main thread: the app's activity has been started.
		break;

	case APP_CMD_RESUME:
		// Command from main thread: the app's activity has been resumed.
		break;

	case APP_CMD_SAVE_STATE:
		// Command from main thread: the app should generate a new saved state
		// for itself, to restore from later if needed.  If you have saved state,
		// allocate it with malloc and place it in android_app.savedState with
		// the size in android_app.savedStateSize.  The will be freed for you
		// later.
		break;

	case APP_CMD_PAUSE:
		// Command from main thread: the app's activity has been paused.
		pthread_mutex_lock(&ctx.mutex);
		entrypoint_might_unload();
		pthread_mutex_unlock(&ctx.mutex);
		break;

	case APP_CMD_STOP:
		// Command from main thread: the app's activity has been stopped.
		break;

	case APP_CMD_DESTROY:
		// Command from main thread: the app's activity is being destroyed,
		// and waiting for the app thread to clean up and exit before proceeding.
		break;
	}
}

#ifdef ENTRYPOINT_PROVIDE_INPUT
int32_t on_input_event(struct android_app *_app, AInputEvent *_event)
{
	const int32_t type       = AInputEvent_getType(_event);
	const int32_t source     = AInputEvent_getSource(_event);
	const int32_t actionBits = AMotionEvent_getAction(_event);

	switch (type)
	{
	case AINPUT_EVENT_TYPE_MOTION:
		{
			//pthread_mutex_lock(&ctx.mutex);

			int32_t touch_count = AMotionEvent_getPointerCount(_event);
			if(touch_count > ENTRYPOINT_MAX_MULTITOUCH)
				touch_count = ENTRYPOINT_MAX_MULTITOUCH;

			for(int32_t touch = 0; touch < touch_count; ++touch)
			{
				ctx.touch.multitouch[touch].x = AMotionEvent_getX(_event, touch);
				ctx.touch.multitouch[touch].y = AMotionEvent_getY(_event, touch);
			}

			int32_t action = (actionBits & AMOTION_EVENT_ACTION_MASK);
			int32_t index  = (actionBits & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

			if(index < ENTRYPOINT_MAX_MULTITOUCH)
			{
				if(action == AMOTION_EVENT_ACTION_DOWN || action == AMOTION_EVENT_ACTION_POINTER_DOWN)
					ctx.touch.multitouch[index].touched = 1;
				else if(action == AMOTION_EVENT_ACTION_UP || action == AMOTION_EVENT_ACTION_POINTER_UP)
					ctx.touch.multitouch[index].touched = 0;
			}

			ctx.touch.x = ctx.touch.multitouch[0].x;
			ctx.touch.y = ctx.touch.multitouch[0].y;
			ctx.touch.left = ctx.touch.multitouch[0].touched;

			//pthread_mutex_unlock(&ctx.mutex);
		}
		break;

	default:
		break;
	}

	return 0;
}
#endif

static void * entrypoint_thread(void * param)
{
	// locking to main jvm thread is needed for fmod and other things
	JNIEnv * jni_env = NULL;
	(*ctx.app->activity->vm)->AttachCurrentThread(ctx.app->activity->vm, &jni_env, NULL);

	if(entrypoint_init(ctx.argc, ctx.argv))
	{
		(*ctx.app->activity->vm)->DetachCurrentThread(ctx.app->activity->vm);
		return NULL;
	}

	while(!ctx.flag_want_to_close)
	{
		pthread_mutex_lock(&ctx.mutex);
		if(entrypoint_loop())
		{
			pthread_mutex_unlock(&ctx.mutex);
			break;
		}
		pthread_mutex_unlock(&ctx.mutex);
	}

	if(entrypoint_might_unload()) // TODO is this needed ?
	{
		(*ctx.app->activity->vm)->DetachCurrentThread(ctx.app->activity->vm);
		return NULL;
	}

	if(entrypoint_deinit())
	{
		(*ctx.app->activity->vm)->DetachCurrentThread(ctx.app->activity->vm);
		return NULL;
	}

	(*ctx.app->activity->vm)->DetachCurrentThread(ctx.app->activity->vm);
	return NULL;
}

void android_main(struct android_app * app)
{
	const char * argv[1] = { "android.so" };
	ctx.argc = 1;
	ctx.argv = argv;
	ctx.app = app;

	app->userData = &ctx;
	app->onAppCmd = on_app_cmd;
#ifdef ENTRYPOINT_PROVIDE_INPUT
	app->onInputEvent = on_input_event;
#endif
	ANativeActivity_setWindowFlags(app->activity, AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

	ENTRYPOINT_ANDROID_PREPARE_PARAMS;

	pthread_mutex_init(&ctx.mutex, NULL);

	bool had_window = false;
	int poll_events = 0;
	struct android_poll_source * poll_source = NULL;
	while(!app->destroyRequested)
	{
		// TODO do we want one or all events?
		ALooper_pollAll(0, NULL, &poll_events, (void**)&poll_source);

		if(poll_source)
			poll_source->process(app, poll_source);

		//pthread_mutex_lock(&ctx.mutex);
		ctx.window = new_window;
		if(ctx.window)
		{
			ctx.view_w = ANativeWindow_getWidth(ctx.window);
			ctx.view_h = ANativeWindow_getHeight(ctx.window);
		}
		//pthread_mutex_unlock(&ctx.mutex);

		if(!had_window && ctx.window)
		{
			pthread_create(&ctx.thread, NULL, entrypoint_thread, NULL);
			had_window = true;
		}
	}

	ctx.flag_want_to_close = 1;
	pthread_join(ctx.thread, NULL);

	pthread_mutex_destroy(&ctx.mutex);

	return;
}

// -----------------------------------------------------------------------------
#ifdef ENTRYPOINT_PROVIDE_TIME

double ep_delta_time()
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
	usleep((useconds_t)(seconds * 1000000.0));
}

#endif
// -----------------------------------------------------------------------------
#ifdef ENTRYPOINT_PROVIDE_LOG

void ep_log(const char * message, ...)
{
	va_list args;
	va_start(args, message);
	__android_log_vprint(ANDROID_LOG_INFO, ENTRYPOINT_ANDROID_LOG_TAG, message, args);
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

bool ep_khit(int32_t key) {return false;}
bool ep_kdown(int32_t key) {return false;}
uint32_t ep_kchar() {return 0;}

#endif

#endif
