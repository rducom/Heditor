#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include "AReVi/Shapes/shapePart3D.h"
using namespace AReVi;

class Container : public ShapePart3D {
public :
  AR_CLASS(Container)
  AR_CONSTRUCTOR(Container)

protected :
  
  virtual
  void
  _setAppearance(void) const;

  virtual
  void
  _resetAppearance(void) const;

  virtual
  void
  _drawGeometry(void) const;

  virtual
  void
  _rebuildIfNeeded(void);

protected :
};

#endif // _CONTAINER_H_
