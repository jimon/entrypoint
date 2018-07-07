#pragma once

#ifdef __ANDROID__

#include "entrypoint_config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	bool found;
	void * env;
	void * class_id;
	void * method_id;
} ep_jni_method_t;

ep_jni_method_t ep_get_static_java_method(const char * class_name, const char * method_name, const char * param_code);
void ep_get_static_java_method_clear(ep_jni_method_t method);

#ifdef __cplusplus
}
#endif

#endif