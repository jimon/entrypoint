
#define ENTRYPOINT_CTX
#include "entrypoint.h"

#if defined(__APPLE__) && TARGET_OS_IOS
#if !__has_feature(objc_arc)
  #error hey, we need ARC here
#endif

#import "entrypoint_ios.h"

static entrypoint_ctx_t ctx = {0};
entrypoint_ctx_t * ep_ctx() {return &ctx;}

// -----------------------------------------------------------------------------

@interface EntryPointView ()
{
	CADisplayLink* m_displayLink;
}

- (void)start;
- (void)stop;
- (void)tick;
- (ep_ui_margins_t)ui_margins;

#ifdef ENTRYPOINT_PROVIDE_INPUT
- (void)onTouchAddOrMove:(UITouch *)touch;
- (void)onTouchRemove:(UITouch *)touch;
#endif
@end

@implementation EntryPointView

+ (Class)layerClass
{
	return [CAEAGLLayer class];
}

- (void)commonInit
{
	ctx.view = (__bridge void *)(self);
	ctx.caeagllayer = (__bridge void *)(self.layer);
	
	if(entrypoint_init(ctx.argc, ctx.argv) != 0)
	{
		NSLog(@"failed to init entrypoint");
		ctx.flag_failed_to_init = 1;
	}
	else
		ctx.flag_failed_to_init = 0;

	#ifdef ENTRYPOINT_IOS_RETINA
	self.contentScaleFactor = [UIScreen mainScreen].scale;
	#endif
	self.multipleTouchEnabled = true;

	#ifdef ENTRYPOINT_IOS_CM
	self.motionManager = [[CMMotionManager alloc] init];
	#endif
	#ifdef ENTRYPOINT_IOS_CM_ACCELEROMETER
	self.motionManager.accelerometerUpdateInterval = 1.0f / ENTRYPOINT_IOS_CM_ACCELEROMETER_FREQ;
	#endif
}

- (id)initWithFrame:(CGRect)rect
{
	if ((self = [super initWithFrame:rect]))
		[self commonInit];
	return self;
}

- (id)initWithCoder:(NSCoder*)coder
{
	if ((self = [super initWithCoder:coder]))
		[self commonInit];
	return self;
}

- (void)dealloc
{
	if(entrypoint_deinit() != 0)
	{
		NSLog(@"failed to deinit entrypoint"); // not much we can do here
	}
}

- (void)layoutSubviews
{
	ctx.view_w = (uint16_t)(self.contentScaleFactor * self.frame.size.width);
	ctx.view_h = (uint16_t)(self.contentScaleFactor * self.frame.size.height);
}

- (void)start
{
	UIGestureRecognizer * gr0 = self.window.gestureRecognizers[0];
	UIGestureRecognizer * gr1 = self.window.gestureRecognizers[1];
	gr0.delaysTouchesBegan = false;
	gr1.delaysTouchesBegan = false;

	if(m_displayLink == nil)
	{
		ctx.flag_anim = 1;
		m_displayLink = [self.window.screen displayLinkWithTarget:self selector:@selector(tick)];
		[m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
	}

	#ifdef ENTRYPOINT_IOS_CM_ACCELEROMETER
	if([self.motionManager isAccelerometerAvailable])
		[self.motionManager startAccelerometerUpdates];
	#endif
}

- (void)stop
{
	if(m_displayLink != nil)
	{
		ctx.flag_anim = 0;
		[m_displayLink invalidate];
		m_displayLink = nil;
		entrypoint_might_unload();
	}

	#ifdef ENTRYPOINT_IOS_CM_ACCELEROMETER
	if([self.motionManager isAccelerometerAvailable])
		[self.motionManager stopAccelerometerUpdates];
	#endif
}

- (void)tick
{
	if(!ctx.flag_failed_to_init && ctx.flag_anim)
	{
		#if defined(ENTRYPOINT_PROVIDE_INPUT) && defined(ENTRYPOINT_IOS_CM_ACCELEROMETER)
		if([self.motionManager isAccelerometerAvailable] && self.motionManager.accelerometerData)
		{
			ctx.touch.acc_x = self.motionManager.accelerometerData.acceleration.x;
			ctx.touch.acc_y = self.motionManager.accelerometerData.acceleration.y;
			ctx.touch.acc_z = self.motionManager.accelerometerData.acceleration.z;
		}
		#endif

		if(entrypoint_loop() != 0)
			[self stop]; // an iOS app cannot force itself to close under normal UX flows
	}
}

- (ep_ui_margins_t)ui_margins
{
	ep_ui_margins_t r = {0, 0, 0, 0};

	if ([self respondsToSelector: @selector(safeAreaInsets)])
	{
		UIEdgeInsets insets = [self safeAreaInsets];

		#define CLAMP0(_x) ( (_x) > 0.0f ? (_x) : 0.0f )

		r.l = self.contentScaleFactor * CLAMP0(insets.left);
		r.t = self.contentScaleFactor * CLAMP0(insets.top);
		r.r = self.contentScaleFactor * CLAMP0(insets.right);
		r.b = self.contentScaleFactor * CLAMP0(insets.bottom);

		#undef CLAMP0
	}

	return r;
}

#ifdef ENTRYPOINT_PROVIDE_INPUT
- (void)onTouchAddOrMove:(UITouch *)touch
{
	CGPoint touchLocation = [touch locationInView:self];
	ctx.touch.x = self.contentScaleFactor * touchLocation.x;
	ctx.touch.y = self.contentScaleFactor * touchLocation.y;
	
	void * context = (__bridge void *)(touch);
	uint8_t first_free = ENTRYPOINT_MAX_MULTITOUCH;
	uint8_t update_index = ENTRYPOINT_MAX_MULTITOUCH;
	for(uint8_t i = 0; i < ENTRYPOINT_MAX_MULTITOUCH; ++i)
	{
		if(ctx.touch.multitouch[i].context == context && ctx.touch.multitouch[i].touched)
		{
			update_index = i;
			break;
		}
		else if(first_free >= ENTRYPOINT_MAX_MULTITOUCH && ctx.touch.multitouch[i].touched == 0)
			first_free = i;
	}
	
	uint8_t index = update_index < ENTRYPOINT_MAX_MULTITOUCH ? update_index : first_free;
	
	if(index < ENTRYPOINT_MAX_MULTITOUCH)
	{
		ctx.touch.multitouch[index].context = context;
		ctx.touch.multitouch[index].x = ctx.touch.x;
		ctx.touch.multitouch[index].y = ctx.touch.y;
		ctx.touch.multitouch[index].touched = 1;
	}
}

- (void)onTouchRemove:(UITouch *)touch
{
	CGPoint touchLocation = [touch locationInView:self];
	ctx.touch.x = self.contentScaleFactor * touchLocation.x;
	ctx.touch.y = self.contentScaleFactor * touchLocation.y;
	
	void * context = (__bridge void *)(touch);
	for(uint8_t i = 0; i < ENTRYPOINT_MAX_MULTITOUCH; ++i)
	{
		if(ctx.touch.multitouch[i].context == context)
		{
			ctx.touch.multitouch[i].x = ctx.touch.x;
			ctx.touch.multitouch[i].y = ctx.touch.y;
			ctx.touch.multitouch[i].touched = 0;
			break;
		}
	}
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	for(UITouch * touch in touches)
		[self onTouchAddOrMove:touch];
	ctx.touch.left = 1;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	for(UITouch * touch in touches)
		[self onTouchRemove:touch];
	ctx.touch.left = 0;
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	for(UITouch * touch in touches)
		[self onTouchAddOrMove:touch];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	for(UITouch * touch in touches)
		[self onTouchRemove:touch];
	ctx.touch.left = 0;
}
#endif

@end

// -----------------------------------------------------------------------------

void ep_anim_start()
{
	if(ctx.view)
		[(__bridge EntryPointView*)ctx.view start];
}

void ep_anim_stop()
{
	if(ctx.view)
		[(__bridge EntryPointView*)ctx.view stop];
}

// -----------------------------------------------------------------------------

ep_size_t ep_size()
{
	ep_size_t r;
	r.w = ctx.view_w;
	r.h = ctx.view_h;
	return r;
}

bool ep_retina()
{
	#ifdef ENTRYPOINT_IOS_RETINA
		return true;
	#else
		return false;
	#endif
}

ep_ui_margins_t ep_ui_margins()
{
	if(ctx.view)
		return [(__bridge EntryPointView*)ctx.view ui_margins];
	else
	{
		ep_ui_margins_t r = {0, 0, 0, 0};
		return r;
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

bool ep_khit(int32_t key) {return false;}
bool ep_kdown(int32_t key) {return false;}
uint32_t ep_kchar() {return 0;}

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_PROVIDE_OPENURL

void ep_openurl(const char * url)
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]];
}

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_IOS_APPDELEGATE

@implementation EntryPointViewController
@dynamic view;
- (void)loadView { self.view = [[EntryPointView alloc] initWithFrame:UIScreen.mainScreen.bounds]; }
@end

// TODO sometimes it complains on "[App] if we're in the real pre-commit handler we can't actually add any new fences due to CA restriction"
@implementation EntryPointAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
 	self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
	[self.window makeKeyAndVisible];
	self.window.rootViewController = [[EntryPointViewController alloc] init];
	return YES;
}
- (void)applicationWillResignActive:(UIApplication *)application { ep_anim_stop(); }
- (void)applicationDidEnterBackground:(UIApplication *)application { ep_anim_stop(); }
- (void)applicationWillEnterForeground:(UIApplication *)application { ep_anim_start(); }
- (void)applicationDidBecomeActive:(UIApplication *)application { ep_anim_start(); }
- (void)applicationWillTerminate:(UIApplication *)application { ep_anim_stop(); }
@end

#endif

// -----------------------------------------------------------------------------

#ifdef ENTRYPOINT_IOS_MAIN

int main(int argc, char * argv[])
{
	ctx.argc = argc;
	ctx.argv = argv;
	@autoreleasepool
	{
		return UIApplicationMain(argc, argv, nil, ENTRYPOINT_IOS_MAIN_APPDELEGATE_CLASSNAME);
	}
}

#endif

#endif
