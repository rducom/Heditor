/*
*
*Gestion des KeyOrientation des contraintes a trois degre de liberte
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/material3D.h"
#include "AReVi/Shapes/shape3D.h"
#include "AReVi/Contrib/arMath.h"

#include "c3dof.h"
#include "keyOrientation.h"

AR_CLASS_NOVOID_DEF(KeyOrientation, Object3D)

KeyOrientation::KeyOrientation(ArCW& arCW, ArRef<C3DOF> c3dof) : Object3D(arCW), _c3dof(c3dof) {
  attachTo(_c3dof);

  //  ArRef<Material3D> mat = new_Material3D();
  ArRef<Material3D> mat = Material3D::NEW();
  mat->setDiffuseColor(0.0, 0.2, 0.8);

  //  _sphere = new_Sphere3D();
  _sphere = Sphere3D::NEW();
  _sphere->setRadius(0.2);
  _sphere->writeMaterial(mat);

  //  ArRef<Shape3D> sh = new_Shape3D();
  ArRef<Shape3D> sh = Shape3D::NEW();
  sh->addRootPart(_sphere);

  setShape(sh);
}

KeyOrientation::~KeyOrientation(void) {
}

/*
*Interaction sur click de souris -> selection
*/
void
KeyOrientation::onMouseButtonInteraction(ArRef<AbstractInteractor> /*source*/, int button, bool pressed) {
  if(button == 1 && pressed) {
    _c3dof->unselect();
    _c3dof->select(thisRef());
  }

  //  Object3D::onMouseButtonInteraction(source, button, pressed); -> ne fait rien a priori
}

/*
*interaction sur mouvement de souris -> bouge la KO
*/
void
KeyOrientation::onMotionInteraction(ArRef<AbstractInteractor> source, ArRef<Base3D> newLocation, bool begin, bool end) {
  _c3dof->globalToLocalLocation(newLocation);
  
  double x, y, z;
  newLocation->getPosition(x, y, z);
  Vector3d v(x, y, z);
  v.normalize();
  v *= _c3dof->getRadius();
  newLocation->setPosition(v.x(),v.y(), v.z());
  _c3dof->localToGlobalLocation(newLocation);
  
  Object3D::onMotionInteraction(source, ShapePart3D::nullRef(), 0, newLocation, begin, end);
}

/*
*Initialise le rayon de la KO
*/
void
KeyOrientation::setRadius(double r) {
  _sphere->setRadius(r);
}
