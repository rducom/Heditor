#include "AReVi/arClass.h"

#include "obj2shpWrapper.h"

AR_CLASS_DEF(Obj2shpWrapper, Object3D)

Obj2shpWrapper::Obj2shpWrapper(ArCW& arCW)
: Object3D(arCW),
  _obj(),
  _shapePart(),
  _shapePartVec(),
  _shape(),
  //  _flatShapePartTransform(new_Transform3D()),
  _flatShapePartTransform(Transform3D::NEW()),
  //  _shapeTransform(new_Transform3D()) {
  _shapeTransform(Transform3D::NEW()) {
}

Obj2shpWrapper::~Obj2shpWrapper(void) {
}

void
Obj2shpWrapper::connect(ArRef<Object3D> obj, 
			ArRef<ShapePart3D> sh) {
  _disconnect();
  _connect(obj, sh);
}

void
Obj2shpWrapper::disconnect(void) {
  _disconnect();
}

ArConstRef<Object3D>
Obj2shpWrapper::getObject3D(void) const {
  return _obj;
}

ArRef<Object3D>
Obj2shpWrapper::accessObject3D(void) {
  return _obj;
}

ArConstRef<ShapePart3D>
Obj2shpWrapper::getShapePart3D(void) const {
  return _shapePart;
}

ArRef<ShapePart3D>
Obj2shpWrapper::accessShapePart3D(void) {
  return _shapePart;
}

void
Obj2shpWrapper::_shapeChangedCB(const Object3D::ShapeChangeEvent& /* evt */) {
  if(!_checkShape(_obj, _shapePart)) {
    _disconnect();
  }
}

void
Obj2shpWrapper::_motionCB(const Object3D::MotionEvent& /* evt */) {
  _updateLocation();
}

void
Obj2shpWrapper::_shapeStructureChangeCB(const Shape3D::StructureChangeEvent& /* evt */) {
  ArRef<Object3D> obj = _obj;
  ArRef<ShapePart3D> shp = _shapePart;

  _disconnect();
  _connect(obj, shp);
}

void
Obj2shpWrapper::_shapeTransformationCB(const Shape3D::TransformationEvent& evt) {
  evt.source->readTransformation(_shapeTransform);
  _updateLocation();
}

void
Obj2shpWrapper::_shapePartStructureChangeCB(const ShapePart3D::StructureChangeEvent& /* evt */) {
  ArRef<Object3D> obj = _obj;
  ArRef<ShapePart3D> shp = _shapePart;

  _disconnect();
  _connect(obj, shp);
}

void
Obj2shpWrapper::_shapePartTransformationCB(const ShapePart3D::TransformationEvent& evt) {
  evt.source->readFlatTransformation(_flatShapePartTransform);
  _updateLocation();
}

bool
Obj2shpWrapper::_checkShape(ArRef<Object3D> obj, ArRef<ShapePart3D> shapePart) {
  if(!shapePart || !obj) {
    return false;
  }

  StlVector<double> distancesOut;
  StlVector<ArRef<Shape3D> > shapesOut;
  obj->queryShapes(distancesOut, shapesOut);
  
  for(size_t i = shapesOut.size(); i--;) {
    if(shapesOut[i] == shapePart->getShape()) {
      return true;
    }
  }

  return false;
}

void
Obj2shpWrapper::_connect(ArRef<Object3D> obj,
			 ArRef<ShapePart3D> shapePart) {
  if(!_checkShape(obj, shapePart)) {
    return;
  }

  if(obj) {
    _obj = obj;
    _obj->addShapeChangeCB(thisRef(),
			   &Obj2shpWrapper::_shapeChangedCB);
    _obj->addMotionCB(thisRef(),
		      &Obj2shpWrapper::_motionCB);
  }
  
  if(shapePart) {
    _shapePart = shapePart;
    _shapePart->readFlatTransformation(_flatShapePartTransform);

    ArPtr<ShapePart3D> tmp = _shapePart;
    do {
      tmp->addStructureChangeCB(thisRef(),
				&Obj2shpWrapper::_shapePartStructureChangeCB);
      tmp->addTransformationCB(thisRef(),
			       &Obj2shpWrapper::_shapePartTransformationCB);
      _shapePartVec.push_back(tmp);
      tmp = tmp->accessParentPart();
    } while(tmp != ShapePart3D::nullPtr());
    
    _shape = _shapePart->accessShape();
    
    if(_shape) {
      _shape->readTransformation(_shapeTransform);
      _shape->addStructureChangeCB(thisRef(), 
				   &Obj2shpWrapper::_shapeStructureChangeCB);
      _shape->addTransformationCB(thisRef(),
				  &Obj2shpWrapper::_shapeTransformationCB);
    }
  }
}

void
Obj2shpWrapper::_disconnect(void) {
  if(_obj) {
    _obj->removeShapeChangeCB(thisRef(), 
			      &Obj2shpWrapper::_shapeChangedCB);
    _obj->removeMotionCB(thisRef(),
			 &Obj2shpWrapper::_motionCB);
    _obj = Object3D::nullRef();
  }

  if(_shapePart) {
    for(size_t i = _shapePartVec.size(); i--;) {
      _shapePartVec[i]->removeStructureChangeCB(thisRef(),
						&Obj2shpWrapper::_shapePartStructureChangeCB);
      _shapePartVec[i]->removeTransformationCB(thisRef(),
					       &Obj2shpWrapper::_shapePartTransformationCB);
    }
    _shape->removeStructureChangeCB(thisRef(),
				    &Obj2shpWrapper::_shapeStructureChangeCB);
    _shape->removeTransformationCB(thisRef(),
				   &Obj2shpWrapper::_shapeTransformationCB);

    _shapePartVec.clear();
    _shape = Shape3D::nullPtr();
    _shapePart = ShapePart3D::nullRef();
  }
}

void
Obj2shpWrapper::_updateLocation(void) {
  if(!_obj) {
    return;
  }

  //  ArRef<Transform3D> tr = new_Transform3D();
  ArRef<Transform3D> tr = Transform3D::NEW();
  tr->preTransform(_flatShapePartTransform);
  tr->preTransform(_shapeTransform);
  tr->preMove(_obj);
  accessBase().setMatrix(tr->accessTransformation().matrix);
  _onMotion();
}

