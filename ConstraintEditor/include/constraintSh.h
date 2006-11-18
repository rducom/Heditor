#ifndef _CONSTRAINT_1D_H_
#define _CONSTRAINT_1D_H_

#include "AReVi/Shapes/cone3D.h"
#include "AReVi/Shapes/lineSet3D.h"
using namespace AReVi;

class ConstraintSh : public LineSet3D {
public :
  AR_CLASS(ConstraintSh)
  AR_CONSTRUCTOR(ConstraintSh)

  virtual
  void
  setColor(float r, float g, float b);

  virtual
  void
  getColor(float& r, float& g, float& b) const;

  virtual
  void
  setRadius(double radius);

  virtual
  double
  getRadius(void) const;

  virtual
  void
  setValues(double in, double out);

  virtual
  void
  getValues(double& in, double& out) const;

protected :

  virtual
  void
  _rebuild(void);

protected :
  Util3D::Dbl3 _color;
  double _radius;
  double _in, _out;

  ArRef<Cone3D> _inSh, _outSh;
};

#endif // _CONSTRAINT_1D_H_
