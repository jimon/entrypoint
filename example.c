
#include "entrypoint.h"

int32_t entrypoint(int32_t argc, char * argv[])
{
	printf("hello there\n");

	while(ep_poll())
	{
		#ifdef ENTRYPOINT_PROVIDE_INPUT
		ep_touch_t t;
		ep_touch(&t);

		printf("touch %f %f %u %u %u\n", t.x, t.y, t.left, t.middle, t.right);

		uint32_t k = ep_kchar();
		if(k)
			printf("key %u\n", k);
		#endif

		#ifdef ENTRYPOINT_PROVIDE_TIME
		printf("dt %f\n", ep_time());
		ep_sleep(0.05f);
		#endif
	}

	printf("bye there =)\n");

	return 0;
}
