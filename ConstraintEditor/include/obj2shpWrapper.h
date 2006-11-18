#ifndef _OBJ2SHP_WRAPPER_H_
#define _OBJ2SHP_WRAPPER_H_

#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Shapes/shape3D.h"
#include "AReVi/Shapes/shapePart3D.h"
using namespace AReVi;

class Obj2shpWrapper : public Object3D {
public :
  AR_CLASS(Obj2shpWrapper)
  AR_CONSTRUCTOR(Obj2shpWrapper)

  virtual
  void
  connect(ArRef<Object3D> obj, 
	  ArRef<ShapePart3D> sh);

  virtual
  void
  disconnect(void);

  virtual
  ArConstRef<Object3D>
  getObject3D(void) const;

  virtual
  ArRef<Object3D>
  accessObject3D(void);

  virtual
  ArConstRef<ShapePart3D>
  getShapePart3D(void) const;

  virtual
  ArRef<ShapePart3D>
  accessShapePart3D(void);

protected :

  // Callbacks

  virtual
  void
  _shapeChangedCB(const Object3D::ShapeChangeEvent& evt);

  virtual
  void
  _motionCB(const Object3D::MotionEvent& evt);

  virtual
  void
  _shapeStructureChangeCB(const Shape3D::StructureChangeEvent& evt);

  virtual
  void
  _shapeTransformationCB(const Shape3D::TransformationEvent& evt);

  virtual
  void
  _shapePartStructureChangeCB(const ShapePart3D::StructureChangeEvent& evt);

  virtual
  void
  _shapePartTransformationCB(const ShapePart3D::TransformationEvent& evt);

  // Utilities

  virtual
  bool
  _checkShape(ArRef<Object3D> obj, ArRef<ShapePart3D> shapePart);

  virtual
  void
  _connect(ArRef<Object3D> obj,
	   ArRef<ShapePart3D> shapePart);

  virtual
  void
  _disconnect(void);

  virtual
  void
  _updateLocation(void);

protected :
  ArRef<Object3D> _obj;
  ArRef<ShapePart3D> _shapePart; 

  StlVector<ArPtr<ShapePart3D> > _shapePartVec;
  ArPtr<Shape3D> _shape;

  ArRef<Transform3D> _flatShapePartTransform;
  ArRef<Transform3D> _shapeTransform;
};

#endif

