#include "AReVi/activity.h"
#include "AReVi/Lib3D/object3D.h"
using namespace AReVi;

#include "hLib/animation/animationCycle.h"
#include "hLib/animation/animationBlendCycle.h"
using namespace hLib;


class AnimBody : public Object3D {
	// depends on Object3D for Activity facilities

 public :
  AR_CLASS(AnimBody)
  AR_CONSTRUCTOR(AnimBody)

	// Animation reference blending handler
  virtual void setAnimation(ArRef<AnimationCycle> animation);
  virtual void setAnimation(ArRef<AnimationBlendCycle> animation);
 
	// Play utilities
  virtual void suspendAnimation(void);
  virtual void playAnimation(void);
  virtual void stopAnimation(void);
  virtual void restartAnimation(void);

	// Keyframe access, by time.
	virtual void setTimePosition(double timePosition);

	// Weight methods.
	virtual double getBlendWeight(void);
  virtual void setBlendWeight(double weight);

	// TimeLine refresh callback
  AR_CALLBACK(AnimBody, Time, _timeCB, double timeLine;)

 protected :
	 
	//Activity method for keyframe by keyframe playing.
  virtual bool _bodyActivity( ArRef<Activity> act, double dt );

	// Signale emitter for the TimeLine callback
  virtual void _onTime(void);
  CallbackManager<AnimBody, TimeEvent> _timeCB;

 private :
	 
  bool _paused;						//	Playing Status (for activity)
  double _timePosition;   //  Current time position in TimeLine
  double _duration;       //  Total duration of current animation
 
  ArRef<AnimationCycle> _anim;  // Reference on the current AnimationCycle
  ArRef<AnimationBlendCycle> _blendAnim; // or AnimationBlendCycle
 
  ArRef<Activity> _act;   // Activity instance, connected 
													// to _bodyActivity() behavior in constructor

};
 
