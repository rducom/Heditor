#ifndef _C3DOF_H_
#define _C3DOF_H_

#include "AReVi/Lib3D/object3D.h"
using namespace AReVi;

#include "hLib/core/jointConstraint.h"
using namespace hLib;

class C3DOFSh;
class KeyOrientation;

class C3DOF : public Object3D {
public :
  AR_CLASS(C3DOF)
  AR_CONSTRUCTOR(C3DOF)
  AR_CONSTRUCTOR_1(C3DOF, ArRef<JointConstraint3DOF>, constraint)

  virtual
  void
  setRadius(double radius);

  virtual
  double
  getRadius(void) const;

  // Create new key orientation between selected one and next one
  virtual
  void
  addKeyOrientation(void);

  // Remove selected key orientation
  virtual
  void
  removeKeyOrientation(void);

  virtual
  StlList<Util3D::Dbl3>&
  getKeyOrientation(void);

  virtual
  StlList<ArRef<KeyOrientation> >
  accessKeyOrientation(void);

  AR_CALLBACK(C3DOF, Change, _changeCB, StlVector<Util3D::Dbl3> keyOrientation;)
    
  virtual
  void
  select(ArRef<KeyOrientation> keyOrientation);

  virtual
  void
  unselect(void);

  virtual
  ArConstRef<KeyOrientation>
  getSelected(void) const;

  virtual
  ArRef<KeyOrientation>
  accessSelected(void);

  virtual
  void
  updateShape(void);

protected :

  virtual
  void
  _keyOrientationMotionCB(const Base3D::MotionEvent& evt);

  virtual
  void
  _updateShape(void);

protected :
  double _radius;
  int _selected;
  StlList<ArRef<KeyOrientation> > _keyOrientation;
  StlList<Util3D::Dbl3> _localKO;
  CallbackManager<C3DOF, ChangeEvent> _changeCB;

  ArRef<C3DOFSh> _sh;
};

#endif // _C3DOF_H_
