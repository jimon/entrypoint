
#ifdef __ANDROID__

// partially based on bgfx entry_android.cpp

#define ENTRYPOINT_CTX
#include "entrypoint.h"
#include "entrypoint_android.h"

#include <android/log.h>
#include <android/input.h>
#include <android/window.h>
#include <android/native_window.h>

#include <jni.h>

#include <android_native_app_glue.c> // just so we don't need to build it

static entrypoint_ctx_t ctx = {0};
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

ep_ui_margins_t ep_ui_margins()
{
	// TODO some phones actually have curved screen, how do we detect that?
	ep_ui_margins_t r = {0, 0, 0, 0};
	return r;
}

// -----------------------------------------------------------------------------

static void * entrypoint_thread(void * param);

static void on_app_cmd(struct android_app * app, int32_t cmd)
{
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
		if(ctx.window != app->window)
		{
			ctx.window = app->window;
			pthread_mutex_lock(&ctx.mutex);
			ctx.view_w = ANativeWindow_getWidth(ctx.window);
			ctx.view_h = ANativeWindow_getHeight(ctx.window);
			pthread_mutex_unlock(&ctx.mutex);

			if(!ctx.thread_running)
			{
				ctx.thread_running = true;
				pthread_create(&ctx.thread, NULL, entrypoint_thread, NULL);
			}
		}
		break;

	case APP_CMD_TERM_WINDOW:
		// Command from main thread: the existing ANativeWindow needs to be
		// terminated.  Upon receiving this command, android_app->window still
		// contains the existing window; after calling android_app_exec_cmd
		// it will be set to NULL.
		pthread_mutex_lock(&ctx.mutex);
		ctx.window = NULL;
		pthread_mutex_unlock(&ctx.mutex);
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

		pthread_mutex_lock(&ctx.mutex);
		entrypoint_might_unload();
		pthread_mutex_unlock(&ctx.mutex);
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

void * entrypoint_thread(void * param)
{
	JNIEnv * jni_env_entrypoint_thread = NULL;
	(*ctx.app->activity->vm)->AttachCurrentThread(ctx.app->activity->vm, &jni_env_entrypoint_thread, NULL);
	ctx.jni_env_entrypoint_thread = jni_env_entrypoint_thread;

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
	ctx.argv = (char**)argv;
	ctx.app = app;

	app->userData = &ctx;
	ctx.app->onAppCmd = on_app_cmd;
	#ifdef ENTRYPOINT_PROVIDE_INPUT
	ctx.app->onInputEvent = on_input_event;
	#endif

	ANativeActivity_setWindowFlags(ctx.app->activity, AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON, 0);

	JNIEnv * jni_env_main_thread = NULL;
	(*ctx.app->activity->vm)->AttachCurrentThread(ctx.app->activity->vm, &jni_env_main_thread, NULL);
	ctx.jni_env_main_thread = jni_env_main_thread;

	{
		JNIEnv * env = ctx.jni_env_main_thread;

		jobject na_class = ctx.app->activity->clazz;
		jclass acl = (*env)->GetObjectClass(env, na_class);
		jmethodID get_class_loader = (*env)->GetMethodID(env, acl, "getClassLoader", "()Ljava/lang/ClassLoader;");
		jobject cl_obj = (*env)->CallObjectMethod(env, na_class, get_class_loader);
		ctx.j_class_loader = (void*)((*env)->NewWeakGlobalRef(env, cl_obj));

		jclass class_loader = (*env)->FindClass(env, "java/lang/ClassLoader");
		ctx.j_find_class_method_id = (void*)((*env)->GetMethodID(env, class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"));
	}

	ENTRYPOINT_ANDROID_PREPARE_PARAMS;

	pthread_mutex_init(&ctx.mutex, NULL);

	int poll_events = 0;
	struct android_poll_source * poll_source = NULL;
	while(!ctx.app->destroyRequested)
	{
		// TODO do we want one or all events?
		ALooper_pollAll(-1, NULL, &poll_events, (void**)&poll_source);

		if(poll_source)
			poll_source->process(ctx.app, poll_source);
	}

	ctx.flag_want_to_close = 1;
	pthread_join(ctx.thread, NULL);

	pthread_mutex_destroy(&ctx.mutex);

	// TODO do we need this?
	(*ctx.app->activity->vm)->DetachCurrentThread(ctx.app->activity->vm);

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
// -----------------------------------------------------------------------------
#ifdef ENTRYPOINT_PROVIDE_OPENURL

void ep_openurl(const char * url)
{
	ep_jni_method_t r = ep_get_static_java_method("com/beardsvibe/EntrypointNativeActivity", "openURL", "(Ljava/lang/String;)V");
	if(r.found)
	{
		JNIEnv * e = (JNIEnv*)r.env;

		jstring j_str = (*e)->NewStringUTF(e, url);
		(*e)->CallStaticVoidMethod(r.env, r.class_id, r.method_id, j_str);
		(*e)->DeleteLocalRef(e, j_str);
	}
	ep_get_static_java_method_clear(r);
}

#endif
// -----------------------------------------------------------------------------

// jni helper is loosely based on cocos2d-x
ep_jni_method_t ep_get_static_java_method(const char * class_name, const char * method_name, const char * param_code)
{
	ep_jni_method_t r = {0};
	JNIEnv * env = ctx.jni_env_entrypoint_thread;
	if(!class_name || !method_name || !param_code || !env)
		return r;

	jobject j_class_loader = (jobject)ctx.j_class_loader;
	jmethodID j_find_class_method_id = (jmethodID)ctx.j_find_class_method_id;

	jstring class_jstr = (*env)->NewStringUTF(env, class_name);
	jclass class_id = (jclass)((*env)->CallObjectMethod(env, j_class_loader, j_find_class_method_id, class_jstr));
	if(!class_id) {
		(*env)->ExceptionClear(env);
		(*env)->DeleteLocalRef(env, class_jstr);
		return r;
	}
	(*env)->DeleteLocalRef(env, class_jstr);

	jmethodID method_id = (*env)->GetStaticMethodID(env, class_id, method_name, param_code);
	if(!method_id) {
		(*env)->ExceptionClear(env);
		return r;
	}

	r.found = true;
	r.env = (void*)env;
	r.class_id = (void*)class_id;
	r.method_id = (void*)method_id;
	return r;
}

void ep_get_static_java_method_clear(ep_jni_method_t method)
{
	if(!method.found)
		return;

	(*((JNIEnv*)method.env))->DeleteLocalRef(method.env, method.class_id);
}

#endif
