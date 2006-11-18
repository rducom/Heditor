/*
*
*Representation des contraintes a trois degres de liberte
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/material3D.h"
#include "AReVi/Contrib/arMath.h"

#include "c3dofSh.h"

AR_CLASS_DEF(C3DOFSh, Shape3D)

C3DOFSh::C3DOFSh(ArCW& arCW) : Shape3D(arCW) {
  //  _lineSet = new_LineSet3D();
  _lineSet = LineSet3D::NEW();
  _lineSet->setThickness(2);
  addRootPart(_lineSet);


  //  ArRef<Material3D> mat = new_Material3D();
  ArRef<Material3D> mat = Material3D::NEW();
  mat->setTransparency(0.5);
  mat->setDiffuseColor(0.8, 0.8, 0.8);

  //  _sphere = new_Sphere3D();
  _sphere = Sphere3D::NEW();
  _sphere->writeMaterial(mat);
  addRootPart(_sphere);
}

C3DOFSh::~C3DOFSh(void) {
}

/*
*Initialisation de la representaion des KO
*/
void
C3DOFSh::setKeyOrientation(StlList<Util3D::Dbl3>& keyOrientation) {
  Util3D::Dbl3& front = keyOrientation.front();
  double radius = Vector3d(front.x, front.y, front.z).length();
  _sphere->setRadius(radius);
  
  StlVector<StlVector<Util3D::Dbl3> >& v = _lineSet->accessVertices();
  StlVector<StlVector<Util3D::Dbl3> >& c = _lineSet->accessColors();
  v.clear();
  c.clear();

  for(StlList<Util3D::Dbl3>::iterator it = keyOrientation.begin(); it != keyOrientation.end(); it++) {
    StlList<Util3D::Dbl3>::iterator itOut = it;
    itOut++;
    if(itOut == keyOrientation.end()) {
      itOut = keyOrientation.begin();
    }

    Vector3d in(it->x, it->y, it->z);
    Vector3d out(itOut->x, itOut->y, itOut->z);

    StlVector<Util3D::Dbl3> vertices;
    StlVector<Util3D::Dbl3> colors;

    for(size_t j = 0; j < 20; j++) {
      double p = (double)j / 20.0;
      Vector3d tmp = lerp(in, out, p);
      tmp.normalize();
      tmp *= radius;

      vertices.push_back(Util3D::Dbl3(tmp.x(), tmp.y(), tmp.z()));
      colors.push_back(Util3D::Dbl3(0.0, 0.2, 0.8));
    }

    v.push_back(vertices);
    c.push_back(colors);
  }

  _lineSet->applyChanges(false);
}
