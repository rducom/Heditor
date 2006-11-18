#ifndef _CAMERA_INTERACTOR_H_
#define _CAMERA_INTERACTOR_H_

#include "AReVi/Lib3D/abstractInteractor.h"
#include "AReVi/Lib3D/viewer3D.h"
#include "AReVi/Lib3D/boundingBox3D.h"

#include "AReVi/Contrib/arMath.h"

using namespace AReVi;

#include "hLib/core/body.h"
#include "hLib/core/skeleton.h"
#include "hLib/animation/goal.h"
using namespace hLib;

#define NAVIGATION_DT 20
#define ROTATION_DT 20
#define ROTATION_TRESHOLD 100

class CameraInteractor : public AbstractInteractor {
public :
  AR_CLASS(CameraInteractor)
  AR_CONSTRUCTOR(CameraInteractor)

  virtual
  void
  setLinearSpeed(double linearSpeed);

  virtual
  double // half-screen linear speed in 1/s
  getLinearSpeed(void) const;

  virtual
  void
  setAngularSpeed(double angularSpeed);

  virtual
  double // half-screen angular speed in rad/s
  getAngularSpeed(void) const;

//-----

  virtual
  void
  setWindow(ArRef<Window3D> window);

  virtual
  void
  setBody(ArRef<Body> body);

  virtual
  ArConstRef<Body>
  getBody(void) const;

  virtual
  ArRef<Body>
  accessBody(void);

protected :

  AR_UNSTUBBED virtual
  Util3D::Base & // current location
  _setupLocation(ArRef<Base3D> base);

  AR_UNSTUBBED virtual
  void
  _clearTimeOut(void);

  AR_UNSTUBBED virtual
  void
  _onMouseButton(const Window3D::MouseButtonEvent& evt);

  AR_UNSTUBBED virtual
  void
  _onMouseMotion(const Window3D::MouseMotionEvent& evt);

  AR_UNSTUBBED virtual
  void
  _onKeyboard(const Window3D::KeyboardEvent & evt);

  AR_UNSTUBBED virtual
  void
  _onTimeOut(const Window3D::TimeOutEvent & evt);

protected :

  bool _init;

  int _mx, _my;
  bool _btn1Pressed;

  ArRef<Object3D> _selected;

  ArRef<Body> _body;

  bool _useNavigate;
  bool _useRotate;
  bool _useCentered;

  bool _remember_cursor;
  int _mouseX,_mouseY,_mouseBtn; // current mouse status
  int _oldX, _oldY;              // old mouse status for relative mouvement
  int _clickX,_clickY;           // old click point, for relative displacement
  
  Util3D::Dbl3 _localPoint;

  unsigned int _timeOut;
  unsigned int _lastTime;
  ArRef<Base3D> _newLocation;
  ArRef<Window3D> _window;
  int _newX, _newY;
  
  double _linearSpeed;
  double _angularSpeed;
};

#endif // _IK_INTERACTOR_H_
