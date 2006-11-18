#ifndef _LOCATION_H_
#define _LOCATION_H_

#include "AReVi/Shapes/shapePart3D.h"
using namespace AReVi;

class Location : public ShapePart3D {
public :
  AR_CLASS(Location)
  AR_CONSTRUCTOR(Location)

protected :
  virtual
  void
  _rebuildIfNeeded(void);

protected :
};

#endif // _LOCATION_H_
