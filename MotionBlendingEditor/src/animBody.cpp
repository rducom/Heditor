#include "AReVi/arSystem.h"
#include "AReVi/arClass.h"
#include "hLib/core/body.h"
#include "animBody.h"

AR_CLASS_DEF(AnimBody, Object3D)

AnimBody::AnimBody(ArCW& arCW)
: Object3D(arCW), 
  _timeCB(thisRef()), 
  _paused(false),
  _timePosition(0.0),
  _duration(0.0),
  //  _anim(new_AnimationCycle()),
  _anim(AnimationCycle::NEW()),
  //  _blendAnim(new_AnimationBlendCycle()) {
  _blendAnim(AnimationBlendCycle::NEW()) {

  //  _act = new_Activity(0.05);
  _act = Activity::NEW(0.05);
  _act->setBehavior( thisRef(), &AnimBody::_bodyActivity );

}

AnimBody::~AnimBody() {

}

void
AnimBody::setAnimation(ArRef<AnimationCycle> animation) {
  
  if( animation.valid() ) {
    _paused = true;
    _timePosition = 0.0;
    _blendAnim = AnimationBlendCycle::nullRef();
    _anim = animation;
    _duration = animation->getDuration();
  }

}

void
AnimBody::setAnimation(ArRef<AnimationBlendCycle> animation) {

  if( animation.valid() ) {
    _paused = true;
    _timePosition = 0.0;
    _anim = AnimationCycle::nullRef();
    _blendAnim = animation;
    _duration = animation->getDuration();
  }

}

void
AnimBody::suspendAnimation(void) {

  _paused = true;

}

void
AnimBody::playAnimation(void) {

  _paused = false;

}

void
AnimBody::stopAnimation(void) {

  _timePosition = 0.0;
  _paused = true;

}

void
AnimBody::restartAnimation(void) {

  _timePosition = 0.0;

}

void
AnimBody::setTimePosition(double timePosition) {

  _timePosition = timePosition;

}

double
AnimBody::getBlendWeight(void) {

  double w;
  w = _blendAnim->getAnimationWeight(1);
  return 100*w;

}

void
AnimBody::setBlendWeight(double weight) {

  _blendAnim->setAnimationWeight(0, (100.0 - weight) / 100.0);
  _blendAnim->setAnimationWeight(1, weight / 100.0);

}

bool
AnimBody::_bodyActivity( ArRef<Activity> /*act*/, double dt ) {

  if( _paused ) {
    dt = 0.0;
  }
  
  _timePosition += dt;
  ArRef<Keyframe> kf;
  
  if( _anim != AnimationCycle::nullRef() ) {      
    kf = _anim->evaluate(_timePosition);
  }

  if( _blendAnim != AnimationBlendCycle::nullRef() ) {
    kf = _blendAnim->evaluate(_timePosition);
  }
    
  if(!kf) {
    return true;
  }
  
  ar_down_cast<Body>(accessShape())->applyPose(kf, true);
  _onTime();

  return true;

}

void
AnimBody::_onTime(void) {

  if(!_timeCB.empty()) {
    TimeEvent evt;
    evt.timeLine = _timePosition;
    while(evt.timeLine > _duration) {
      evt.timeLine -= _duration;
    }
    _timeCB.fire(evt);
  }

}
