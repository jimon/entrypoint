#pragma once

// iOS is a bit different here
//
// indeed we could go SDL way and create everything for you
// but this is exactly why SDL is so bad on iOS - there is more to an app than just OpenGL view
//
// instead what we provide here is a UIView that you can use inside your storyboard/xib/code
// this should allow more easy integration of your code with iOS specific parts like ads/popups/etc
//
// also please note, only one view instance should be used (which is a most common scenario anyway)

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>

#if defined(ENTRYPOINT_IOS_CM_ACCELEROMETER)
	#define ENTRYPOINT_IOS_CM
#endif

@interface EntryPointView : UIView

#ifdef ENTRYPOINT_IOS_CM
@property(nonatomic, strong) CMMotionManager * motionManager;
#endif

@end

#ifdef __cplusplus
extern "C" {
#endif

// call this functions from app delegate or whatever other place where you decide when your game should be running or not
void ep_anim_start();
void ep_anim_stop();

#ifdef __cplusplus
}
#endif
