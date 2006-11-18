#ifndef _KEY_ORIENTATION_H_
#define _KEY_ORIENTATION_H_

#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Shapes/sphere3D.h"
using namespace AReVi;

class C3DOF;

class KeyOrientation : public Object3D {
public :
  AR_CLASS(KeyOrientation)
  AR_CONSTRUCTOR_1(KeyOrientation, ArRef<C3DOF>, c3dof)

  virtual
  void
  onMouseButtonInteraction(ArRef<AbstractInteractor> source, int button, bool pressed);

  virtual
  void
  onMotionInteraction(ArRef<AbstractInteractor> source, ArRef<Base3D> newLocation, bool begin, bool end);

  virtual
  void
  setRadius(double r);

protected :
  ArPtr<C3DOF> _c3dof;
  ArRef<Sphere3D> _sphere;
};

#endif // _KEY_ORIENTATION_H_
