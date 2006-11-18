#include "AReVi/arClass.h"

#include "container.h"

AR_CLASS_DEF(Container, ShapePart3D)

Container::Container(ArCW& arCW)
: ShapePart3D(arCW) {
}

Container::~Container(void) {
}

void
Container::_setAppearance(void) const {
}

void
Container::_resetAppearance(void) const {
}

void
Container::_drawGeometry(void) const {
}

void
Container::_rebuildIfNeeded(void) {
}
