
#include "entrypoint.h"
#include <stdio.h>

int32_t entrypoint_init(int32_t argc, char * argv[])
{
	printf("why, hello there!\n");
	return 0;
}

int32_t entrypoint_deinit()
{
	printf("see you later!\n");
	return 0;
}

int32_t entrypoint_might_unload()
{
	return 0;
}

int32_t entrypoint_loop()
{
	#ifdef ENTRYPOINT_PROVIDE_INPUT
	ep_touch_t t;
	ep_touch(&t);
	printf("touch %f %f %u %u %u\n", t.x, t.y, t.left, t.middle, t.right);

	uint32_t k = ep_kchar();
	if(k)
		printf("key %u\n", k);
		
	if(ep_kdown(EK_ESCAPE))
		return 1;
	#endif

	#ifdef ENTRYPOINT_PROVIDE_TIME
	//printf("dt %f\n", ep_time());
	ep_sleep(0.016f);
	#endif
	
	return 0;
}
