#include "AReVi/arClass.h"
#include "AReVi/VRML/vrmlShape3D.h"

#include "location.h"

AR_CLASS_DEF(Location, ShapePart3D)

Location::Location(ArCW& arCW) 
: ShapePart3D(arCW) {
  ArRef<VrmlShape3D> sh = new_VrmlShape3D();
  sh->parseURL("./data/location.wrl");

  while(sh->getNbRootParts()) {
    addSubPart(sh->accessRootPart(0));
  }
}

Location::~Location(void) {
}

void
Location::_rebuildIfNeeded(void) {
}
