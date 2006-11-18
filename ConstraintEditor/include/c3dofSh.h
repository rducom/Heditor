#ifndef _C3DOF_SH_H_
#define _C3DOF_SH_H_

#include "AReVi/Shapes/lineSet3D.h"
#include "AReVi/Shapes/shape3D.h"
#include "AReVi/Shapes/sphere3D.h"
using namespace AReVi;

class C3DOFSh : public Shape3D {
public :
  AR_CLASS(C3DOFSh)
  AR_CONSTRUCTOR(C3DOFSh)

  virtual
  void
  setKeyOrientation(StlList<Util3D::Dbl3>& v);

protected :
  ArRef<LineSet3D> _lineSet;
  ArRef<Sphere3D> _sphere;
};

#endif // _C3DOF_SH_H_
