/*
*
*Gestion des articulations et de leurs contraintes
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Shapes/shapePart3D.h"
#include "AReVi/Lib3D/boundingBox3D.h"
#include "AReVi/VRML/vrmlShape3D.h"

#include "hLib/core/joint.h"
#include "hLib/core/anchor.h"
#include "hLib/animation/goal.h"

#include "jointController.h"
#include "keyOrientation.h"

AR_CLASS_DEF(JointController, ArObject) 

/*constructor*/
JointController::JointController(ArCW& arCW) 
: ArObject(arCW),
  _highLighted(),
  _sphere(),
  _material(),
  _selected(),
  _in(Util3D::Dbl3(0.0, 0.0, 0.0)),
  _out(Util3D::Dbl3(360.0, 360.0, 360.0)),
  _wrapper(),
  _obj(),
  _highLightCB(thisRef()),
  _selectCB(thisRef()),
  _constraintCB(thisRef()),
  _valueCB(thisRef()),
  _changeMotionCB(thisRef()) {

  ArRef<Transform3D> tr;
  
  //  tr = new_Transform3D();
  tr = Transform3D::NEW();
  tr->preScale(6.0, 6.0, 6.0);
  //  _container = new_Container();
  _container = Container::NEW();
  _container->writeTransformation(tr);
 
  //  ArRef<VrmlShape3D> tmpSh = new_VrmlShape3D();
  ArRef<VrmlShape3D> tmpSh = VrmlShape3D::NEW();
  tmpSh->parseURL("./data/location.wrl");
  while(tmpSh->getNbRootParts()) {
    _container->addSubPart(tmpSh->accessRootPart(0));
  }

  for(size_t i = 0; i < 3; i++) {
    //    _constraintSh[i] = new_ConstraintSh();
    _constraintSh[i] = ConstraintSh::NEW();
    _constraintSh[i]->setRadius(0.5);
    _constraintSh[i]->setValues(0, M_2PI);
  }

  //  _constraintTr[0] = new_Transform3D();
  _constraintTr[0] = Transform3D::NEW();
  _constraintTr[0]->postTranslate(0.66, 0.0, 0.0);
  _constraintSh[0]->writeTransformation(_constraintTr[0]);
  _constraintSh[0]->setColor(1.0, 0.0, 0.0);

  //  _constraintTr[1] = new_Transform3D();
  _constraintTr[1] = Transform3D::NEW();
  _constraintTr[1]->postYaw(M_PI_2);
  _constraintTr[1]->postTranslate(0.66, 0.0, 0.0);
  _constraintSh[1]->writeTransformation(_constraintTr[1]);
  _constraintSh[1]->setColor(0.0, 1.0, 0.0);

  //  _constraintTr[2] = new_Transform3D();
  _constraintTr[2] = Transform3D::NEW();
  _constraintTr[2]->postPitch(- M_PI_2);
  _constraintTr[2]->postTranslate(0.66, 0.0, 0.0);
  _constraintSh[2]->writeTransformation(_constraintTr[2]);  
  _constraintSh[2]->setColor(0.0, 0.0, 1.0);

}

/*destructor*/
JointController::~JointController(void) {
}

/*highlights a joint*/
void
JointController::highLightJoint(ArRef<Joint> joint) {
  if(_highLighted.valid()) {
    _sphere->writeMaterial(_material);
  }
  
  _highLighted = joint;

  if(!_highLighted.valid()) {
    return;
  }

  //  ArRef<Material3D> mhl = new_Material3D();
  ArRef<Material3D> mhl = Material3D::NEW();
  mhl->setTransparency(0.666);
  mhl->setDiffuseColor(0.8,0.0,0.0);
  mhl->setEmissiveColor(0.25,0.0,0.0);
  mhl->setAmbientIntensity(0.4);

  for(size_t i = 0; i < _highLighted->getNbSubParts(); i++) {
    if(_highLighted->getSubPart(i)->getClass()->isA(Sphere3D::CLASS())) {
      _sphere = ar_down_cast<Sphere3D>(_highLighted->accessSubPart(i));
      //      _material = new_Material3D();
      _material = Material3D::NEW();
      _sphere->readMaterial(_material);
      _sphere->writeMaterial(mhl);
    }
  }

  _onHighLight();
}

/*return a constant reference of the joint which is highlighted*/
ArConstRef<Joint>
JointController::getHighLightedJoint(void) const {
  return _highLighted;
}

/*return the joint which is highlighted*/
ArRef<Joint>
JointController::accessHighLightedJoint(void) {
  return _highLighted;
}

/*selects a joint*/
void
JointController::selectJoint(ArRef<Joint> joint) {

  if(_highLighted) {
    highLightJoint(Joint::nullRef());
  }

  if(_selected) {
    _selected->removeSubPart(_container);
    _selected->accessSkeleton()->removeRebuildCB(thisRef(), &JointController::_onSkeletonRebuild);
  }

  _selected = joint;

  if(_selected) {
    _selected->addSubPart(_container);
    setJointConstraint(_selected->accessConstraint());
    _selected->accessSkeleton()->addRebuildCB(thisRef(), &JointController::_onSkeletonRebuild);
  }

  _onSelect();
}

/*return the constant joint which is selected*/
ArConstRef<Joint>
JointController::getSelectedJoint(void) const {
  return _selected;
}

/*return the joint which is selected*/
ArRef<Joint>
JointController::accessSelectedJoint(void) {
  return _selected;
}

/*Set jointConstraint type and jointConstraint axis*/
void
JointController::setJointConstraint(ArRef<JointConstraint> jointConstraint) {

  for(size_t i = 0; i < 3; i++) {
    _container->removeSubPart(_constraintSh[i]);
  }
  _c3dof = C3DOF::nullRef();
  _wrapper = Obj2shpWrapper::nullRef();

  _selected->setConstraint(jointConstraint);
  
  if(jointConstraint != JointConstraint::nullRef()){
    
    if(jointConstraint->getClass()->isA(JointConstraint1DOF::CLASS())) {
      //cas d'une contrainte 1DOF
      switch(ar_down_cast<JointConstraint1DOF>(jointConstraint)->getAxisType()) {
      case JointConstraint1DOF::AXIS_X :
	_container->addSubPart(_constraintSh[AXIS_X]);
	break;
      case JointConstraint1DOF::AXIS_Y :
	_container->addSubPart(_constraintSh[AXIS_Y]);
	break;
      case JointConstraint1DOF::AXIS_Z :
	_container->addSubPart(_constraintSh[AXIS_Z]);
	break;
      default :
	break;
      }
    }
    
    if(jointConstraint->getClass()->isA(JointConstraint2DOF::CLASS())) {
      //cas d'une contrainte 2DOF
      switch(ar_down_cast<JointConstraint2DOF>(jointConstraint)->getPlaneType()) {
      case JointConstraint2DOF::PLANE_XY :
	_container->addSubPart(_constraintSh[AXIS_X]);
	_container->addSubPart(_constraintSh[AXIS_Y]);
	break;
      case JointConstraint2DOF::PLANE_XZ :
	_container->addSubPart(_constraintSh[AXIS_X]);
	_container->addSubPart(_constraintSh[AXIS_Z]);
	break;
      case JointConstraint2DOF::PLANE_YZ :
	_container->addSubPart(_constraintSh[AXIS_Y]);
	_container->addSubPart(_constraintSh[AXIS_Z]);
	break;
      default :
	break;
      }
    }
    
    if(jointConstraint->getClass()->isA(JointConstraint3DOF::CLASS())) {
      //cas d'une contrainte 3DOF
      if( _c3dof != C3DOF::nullRef() ) { _c3dof->removeChangeCB(thisRef(), &JointController::_constraintChangeCB); }
      //initialisation de la representation
      //      _c3dof = new_C3DOF( ar_down_cast<JointConstraint3DOF>(jointConstraint) );
      _c3dof = C3DOF::NEW( ar_down_cast<JointConstraint3DOF>(jointConstraint) );
      _c3dof->setMouseButtonInteraction(false);
      _c3dof->setMotionInteraction(false);
      _c3dof->addChangeCB(thisRef(), &JointController::_constraintChangeCB);
      
      //      ArRef<BoundingBox3D> bb = new_BoundingBox3D();
      ArRef<BoundingBox3D> bb = BoundingBox3D::NEW();
      _selected->accessSkeleton()->readBoundingBox(bb);

      double r = bb->getMaxSize() / 400;
      _c3dof->setRadius(r);

      //initialisation des KeyOrientation
      StlVector<ArRef<Anchor> > anchors;
      StlVector<ArRef<Goal> > goals;
      if(_selected->getNbChildren() == 0) {
	// Create goal
	//	ArRef<Transform3D> tr = new_Transform3D();
	ArRef<Transform3D> tr = Transform3D::NEW();
	const Util3D::Transform& t3d = tr->getTransformation();
	Point3d anchorPos(t3d.matrix[Util3D::TX], t3d.matrix[Util3D::TY], t3d.matrix[Util3D::TZ]);
	ArConstRef<Joint> link = _selected;
	const Matrix4x4d linkMatrix = link->getAbsoluteMatrix();
	anchorPos = linkMatrix * anchorPos;
	_selected->accessSkeleton()->readTransformation(tr);
	tr->apply(anchorPos.x, anchorPos.y, anchorPos.z, anchorPos.x, anchorPos.y, anchorPos.z);
	//	ArRef<Goal> g = new_Goal();
	ArRef<Goal> g = Goal::NEW();
	g->setLocationType(Goal::GLOBAL);
	g->setConstraints(true, false);
	g->setPosition(anchorPos);
	goals.push_back(g);
      }
      else {
	for(int i = 0 ; i < _selected->getNbChildren() ; i++) {
	  // Create goal
	  //	  ArRef<Transform3D> tr = new_Transform3D();
	  ArRef<Transform3D> tr = Transform3D::NEW();
	  const Util3D::Transform& t3d = tr->getTransformation();
	  Point3d anchorPos(t3d.matrix[Util3D::TX], t3d.matrix[Util3D::TY], t3d.matrix[Util3D::TZ]);
	  ArConstRef<Joint> link = _selected->getChild(i);
	  const Matrix4x4d linkMatrix = link->getAbsoluteMatrix();
	  anchorPos = linkMatrix * anchorPos;
	  _selected->accessSkeleton()->readTransformation(tr);
	  tr->apply(anchorPos.x, anchorPos.y, anchorPos.z, anchorPos.x, anchorPos.y, anchorPos.z);
	  //	  ArRef<Goal> g = new_Goal();
	  ArRef<Goal> g = Goal::NEW();
	  g->setLocationType(Goal::GLOBAL);
	  g->setConstraints(true, false);
	  g->setPosition(anchorPos);
	  goals.push_back(g);
	}
      }
      
      //      ArRef<Transform3D> trSelected = new_Transform3D();
      ArRef<Transform3D> trSelected = Transform3D::NEW();
      const Util3D::Transform& t3ds = trSelected->getTransformation();
      Point3d selectedPos(t3ds.matrix[Util3D::TX], t3ds.matrix[Util3D::TY], t3ds.matrix[Util3D::TZ]);
      const Matrix4x4d jointMatrix = _selected->getAbsoluteMatrix();
      selectedPos = jointMatrix * selectedPos;
      _selected->accessSkeleton()->readTransformation(trSelected);
      trSelected->apply(selectedPos.x, selectedPos.y, selectedPos.z, selectedPos.x, selectedPos.y, selectedPos.z);
      
      _c3dof->setPosition(selectedPos.x, selectedPos.y, selectedPos.z);
      
      //accrochage  de la representation a l'articulation
      //      _wrapper = new_Obj2shpWrapper();
      _wrapper = Obj2shpWrapper::NEW();
      //      _obj = new_Object3D();
      _obj = Object3D::NEW();
      _obj->setShape(_selected->accessSkeleton());
      _wrapper->connect(_obj, _selected);
      _c3dof->attachTo(_wrapper);
      
    }
    
    if(jointConstraint->getClass()->isA(JointConstraintLocked::CLASS())) {
      
    }
    
  }

  else {

  }

  _onConstraint(jointConstraint);
}

/*return the selected joint's constraint*/
ArRef<JointConstraint>
JointController::getJointConstraint(void) const {
  return _selected->accessConstraint();
}

/*set a value for an axis*/
void
JointController::setAxisValues(AxisType axis, double in, double out) {

  if(_selected->getConstraint() != JointConstraint::nullRef()) {
    
    if(_selected->getConstraint()->getClass()->isA(JointConstraint1DOF::CLASS())) {
      switch(axis) {
      case AXIS_X :
	_in.x = in;
	_out.x = out;
	_constraintSh[AXIS_X]->setValues(in * M_PI_180, out * M_PI_180);
	ar_down_cast<JointConstraint1DOF>(_selected->accessConstraint())->setLimitValues(in*M_PI_180, out*M_PI_180);
	break;
      case AXIS_Y :
	_in.y = in;
	_out.y = out;
	_constraintSh[AXIS_Y]->setValues(in * M_PI_180, out * M_PI_180);
	ar_down_cast<JointConstraint1DOF>(_selected->accessConstraint())->setLimitValues(in*M_PI_180, out*M_PI_180);
	break;
      case AXIS_Z :
	_in.z = in;
	_out.z = out;
	_constraintSh[AXIS_Z]->setValues(in * M_PI_180, out * M_PI_180);
	ar_down_cast<JointConstraint1DOF>(_selected->accessConstraint())->setLimitValues(in*M_PI_180, out*M_PI_180);
	break;
      default :
	break;
      }
    }
    
    if(_selected->getConstraint()->getClass()->isA(JointConstraint2DOF::CLASS())) {
      double* planeIn[2];
      planeIn[0] = new double();
      planeIn[1] = new double();
      double* planeOut[2];
      planeOut[0] = new double();
      planeOut[1] = new double();
      ar_down_cast<JointConstraint2DOF>(_selected->getConstraint())->getLimitValues(planeIn, planeOut);
      switch(ar_down_cast<JointConstraint2DOF>(_selected->getConstraint())->getPlaneType()) {
      case JointConstraint2DOF::PLANE_XY :
	switch(axis) {
	case AXIS_X :
	  _in.x = in;
	  _out.x = out;
	  _constraintSh[AXIS_X]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[0] = in * M_PI_180;
	  *planeOut[0] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	case AXIS_Y :
	  _in.y = in;
	  _out.y = out;
	  _constraintSh[AXIS_Y]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[1] = in * M_PI_180;
	  *planeOut[1] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	default :
	  break;
	}
	break;

      case JointConstraint2DOF::PLANE_XZ :
	switch(axis) {
	case AXIS_X :
	  _in.x = in;
	  _out.x = out;
	  _constraintSh[AXIS_X]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[0] = in * M_PI_180;
	  *planeOut[0] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	case AXIS_Z :
	  _in.z = in;
	  _out.z = out;
	  _constraintSh[AXIS_Z]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[1] = in * M_PI_180;
	  *planeOut[1] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	default :
	  break;
	}
	break;

      case JointConstraint2DOF::PLANE_YZ :
	switch(axis) {
	case AXIS_Y :
	  _in.y = in;
	  _out.y = out;
	  _constraintSh[AXIS_Y]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[0] = in * M_PI_180;
	  *planeOut[0] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	case AXIS_Z :
	  _in.z = in;
	  _out.z = out;
	  _constraintSh[AXIS_Z]->setValues(in * M_PI_180, out * M_PI_180);
	  *planeIn[1] = in * M_PI_180;
	  *planeOut[1] = out * M_PI_180;
	  ar_down_cast<JointConstraint2DOF>(_selected->accessConstraint())->setLimitValues(*planeIn, *planeOut);
	  break;
	default :
	  break;
	}
	break;

      default :
	break;
      }
      delete planeIn[0];
      delete planeIn[1];
      delete planeOut[0];
      delete planeOut[1];
      
    }
  }
  _onValue(axis);
}

/*return the value of an axis*/
void
JointController::getAxisValues(AxisType axis, double& in, double& out) {
  switch(axis) {
  case AXIS_X :
    in = _in.x;
    out = _out.x;
    break;
  case AXIS_Y :
    in = _in.y;
    out = _out.y;
    break;
  case AXIS_Z :
    in = _in.z;
    out = _out.z;
    break;
  default :
    break;
  }
}

/*return the C3DOF*/
ArRef<C3DOF>
JointController::accessC3DOF(void){
  return _c3dof;
}

/*set KeyOrientation values*/
void
JointController::setC3DOFKO(StlList<Util3D::Dbl3 > keyOrientation) {
  StlVector<Util3D::Dbl3 > KO;
  for(StlList<Util3D::Dbl3 >::iterator itDbl = keyOrientation.begin() ; itDbl != keyOrientation.end() ; itDbl++) {
    KO.push_back(*itDbl);
  }
  ArRef<JointConstraint3DOF> constraint = ar_down_cast<JointConstraint3DOF>(_selected->accessConstraint());
  constraint->accessKeyOrientation() = KO;
}

/*calls the callback function for the highlighted joints*/
void
JointController::_onHighLight(void) {
  if(!_highLightCB.empty()) {
    HighLightEvent evt;
    evt.joint = _highLighted;
    _highLightCB.fire(evt);
  }
}

/*calls the callback function for the selected joints*/
void
JointController::_onSelect(void) {
  if(!_selectCB.empty()) {
    SelectEvent evt;
    evt.joint = _selected;
    _selectCB.fire(evt);
  }
}

/*calls the callback function for the jointConstraints*/
void
JointController::_onConstraint(ArRef<JointConstraint> jointConstraint) {
  if(!_constraintCB.empty()) {
    ConstraintEvent evt;
    evt.jointConstraint = jointConstraint;
    _constraintCB.fire(evt);
  }
}

/*calls the callback function for the axis' values*/
void
JointController::_onValue(AxisType axis) {
  if(!_valueCB.empty()) {
    ValueEvent evt;
    evt.axis = axis;
    switch(axis) {
    case AXIS_X :
      evt.in = _in.x;
      evt.out = _out.x;
      break;
    case AXIS_Y : 
      evt.in = _in.y;
      evt.out = _out.y;
      break;
    case AXIS_Z :
      evt.in = _in.z;
      evt.out = _out.z;
      break;
    default :
      break;
    }
    _valueCB.fire(evt);
  }
}

/*if the 3DOF contraint is changed by the interactor*/
void 
JointController::_onChangeMotion(void) {
  if( !_changeMotionCB.empty() ) {
    ChangeMotionEvent evt;
    if( _c3dof.valid() ) {
      evt.key = _c3dof->accessSelected();
    }
    else {
      evt.key = KeyOrientation::nullRef();
    }
    _changeMotionCB.fire(evt);
  }
}

/*calls the callback function for the skeleton rebuilds*/
void
JointController::_onSkeletonRebuild(const Skeleton::RebuildEvent& /* evt */) {
  if(_selected) {
    Vector3d t = _selected->getDeltaTranslation();
    Quaterniond q = _selected->getDeltaRotation();
    
    Util3D::Dbl3 axis[3];
    axis[0] = Util3D::Dbl3(1.0, 0.0, 0.0);
    axis[1] = Util3D::Dbl3(0.0, 1.0, 0.0);
    axis[2] = Util3D::Dbl3(0.0, 0.0, 1.0);

    double r[3];
    q.getEulerAngles(r[0], r[1], r[2]);

    ArRef<Transform3D> tr[3];

    for(size_t i = 0; i < 3; i++) {
      //      tr[i] = new_Transform3D();
      tr[i] = Transform3D::NEW();
      tr[i]->preTransform(_constraintTr[i]);
      tr[i]->postRoll(- r[i]);
      _constraintSh[i]->writeTransformation(tr[i]);
    } 
  }
}

/*reflex function on change 3DOF constraint*/
 void
JointController::_constraintChangeCB(const C3DOF::ChangeEvent& evt) {
   ArRef<JointConstraint3DOF> constraint = ar_down_cast<JointConstraint3DOF>(_selected->accessConstraint());
   constraint->accessKeyOrientation() = evt.keyOrientation;
   constraint->applyChanges();
   _onChangeMotion();
}
