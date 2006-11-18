#ifndef _JOINT_CONTROLLER_H_
#define _JOINT_CONTROLLER_H_

#include "AReVi/arObject.h"
#include "AReVi/Lib3D/material3D.h"
#include "AReVi/Shapes/sphere3D.h"
#include "AReVi/Shapes/surface3D.h"
using namespace AReVi;

#include "hLib/core/joint.h"
#include "hLib/core/jointConstraint.h"
#include "hLib/core/skeleton.h"
using namespace hLib;

#include "container.h"
#include "constraintSh.h"
#include "types.h"
#include "undoRedo.h"
#include "c3dof.h"
#include "obj2shpWrapper.h"

class JointController : public ArObject {
public :
  AR_CLASS(JointController)
  AR_CONSTRUCTOR(JointController)

// ----- Highlight management ----------------------------------------

  virtual
  void
  highLightJoint(ArRef<Joint> joint);

  virtual
  ArConstRef<Joint>
  getHighLightedJoint(void) const;

  virtual
  ArRef<Joint>
  accessHighLightedJoint(void);

  AR_CALLBACK(JointController, HighLight, _highLightCB, ArRef<Joint> joint; )

// ----- Selected join management ------------------------------------

  virtual
  void
  selectJoint(ArRef<Joint> joint);

  virtual
  ArConstRef<Joint>
  getSelectedJoint(void) const;

  virtual
  ArRef<Joint>
  accessSelectedJoint(void);

  AR_CALLBACK(JointController, Select, _selectCB, ArRef<Joint> joint; )

  virtual
  void
  setJointConstraint(ArRef<JointConstraint> jointConstraint);

  virtual
  ArRef<JointConstraint>
  getJointConstraint(void) const;

  AR_CALLBACK(JointController, Constraint, _constraintCB, ArRef<JointConstraint> jointConstraint;)

  virtual
  void
  setAxisValues(AxisType axis, double in, double out);

  virtual
  void
  getAxisValues(AxisType axis, double& in, double& out);

  virtual
  ArRef<C3DOF>
  accessC3DOF(void);

  virtual
  void
  setC3DOFKO(StlList<Util3D::Dbl3 > keyOrientation);

  AR_CALLBACK(JointController, Value, _valueCB, AxisType axis; double in; double out; )

  AR_CALLBACK(JointController, ChangeMotion, _changeMotionCB, ArRef<KeyOrientation> key;)

protected :

  virtual
  void
  _onHighLight(void);

  virtual
  void
  _onSelect(void);

  virtual
  void
  _onConstraint(ArRef<JointConstraint> jointConstraint);

  virtual
  void
  _onValue(AxisType axis);

  virtual void _onChangeMotion(void);

  virtual
  void
  _onSkeletonRebuild(const Skeleton::RebuildEvent& evt);

  virtual void _constraintChangeCB(const C3DOF::ChangeEvent& evt);

protected :
  ArRef<Joint> _highLighted;
  ArRef<Sphere3D> _sphere;
  ArRef<Material3D> _material;

  ArRef<Joint> _selected;
  Util3D::Dbl3 _in;
  Util3D::Dbl3 _out;

  ArRef<Obj2shpWrapper> _wrapper;
  ArRef<Object3D> _obj;

  ArRef<Container> _container;
  ArRef<ConstraintSh> _constraintSh[3];
  ArRef<C3DOF> _c3dof;
  ArRef<Transform3D> _constraintTr[3];

  CallbackManager<JointController, HighLightEvent> _highLightCB;
  CallbackManager<JointController, SelectEvent> _selectCB;
  CallbackManager<JointController, ConstraintEvent> _constraintCB;
  CallbackManager<JointController, ValueEvent> _valueCB;
  CallbackManager<JointController, ChangeMotionEvent> _changeMotionCB;

};

#endif // _JOINT_CONTROLLER_H_
