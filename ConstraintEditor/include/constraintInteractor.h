#ifndef _CONSTRAINT_INTERACTOR_H_
#define _CONSTRAINT_INTERACTOR_H_

#include "AReVi/Lib3D/abstractInteractor.h"
#include "AReVi/Lib3D/viewer3D.h"
#include "AReVi/Lib3D/boundingBox3D.h"

#include "AReVi/Contrib/arMath.h"

using namespace AReVi;

#include "hLib/core/body.h"
#include "hLib/animation/goal.h"


using namespace hLib;

#include "anchorController.h"
#include "bodyController.h"
#include "editorIkSolver.h"
#include "jointController.h"
#include "osdSelector.h"
#include "osdSelectorItem.h"
#include "keyOrientation.h"


#define NAVIGATION_DT 20
#define ROTATION_DT 20
#define ROTATION_TRESHOLD 100

class ConstraintInteractor : public AbstractInteractor {
public :
  AR_CLASS(ConstraintInteractor)
  AR_CONSTRUCTOR_3(ConstraintInteractor, ArRef<BodyController>, bodyController, ArRef<JointController>, jointController, ArRef<AnchorController>, anchorController)

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

  virtual
  void
  setGoals(StlVector<ArRef<Goal> >& goals);

  virtual
  const StlVector<ArRef<Goal> >& getGoals(void);

  virtual
  StlVector<ArRef<Goal> >& accessGoals(void);

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

  virtual
  void
  _onAnchorCB(const AnchorController::AnchorEvent& evt);

  virtual
  Point3d
  _getAnchorGlobalPosition(ArConstRef<Anchor> anchor);

  virtual
  void
  _resetGoals(void);
 
  AR_UNSTUBBED virtual
  void
  _onTimeOut(const Window3D::TimeOutEvent & evt);

protected :

  bool init;

  ArRef<BodyController> _bodyController;
  ArRef<JointController> _jointController;
  ArRef<AnchorController> _anchorController;
  
  int _mx, _my;
  bool _btn1Pressed;

  ArConstRef<Joint> _selected;
  Vector3d _selPosition;

  ArRef<Body> _body;
/*   ArRef<IkCcd> _solver; */
  ArRef<EditorIkSolver> _solver;
  StlVector<ArRef<Goal> > _goals;
  ArRef<Goal> _interacted;
  ArRef<Window3D> _window;

  bool _useIK;
  bool _useNavigate;
  bool _useRotate;
  bool _useCentered;
  bool _isKO;

  bool _remember_cursor;
  int _mouseX,_mouseY,_mouseBtn; // current mouse status
  int _oldX, _oldY;              // old mouse status for relative mouvement
  int _clickX,_clickY;           // old click point, for relative displacement
  
  Util3D::Dbl3 _localPoint;

  unsigned int _timeOut;
  unsigned int _lastTime;
  ArRef<Base3D> _newLocation;
  int _newX, _newY;
  
  double _linearSpeed;
  double _angularSpeed;
};

#endif // _CONSTRAINT_INTERACTOR_H_
