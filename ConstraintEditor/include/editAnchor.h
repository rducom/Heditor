#ifndef _EDIT_ANCHOR_H_
#define _EDIT_ANCHOR_H_

#include "hLib/core/anchor.h"
#include "hLib/core/body.h"
using namespace hLib;

#include "gtkAReViWidget.h"
using namespace GtkTools;

#include "undoRedo.h"

enum LinkColumnID {
  COLUMN_LINK_NAME = 0,
  COLUMN_LINK_DATA = 1
};

class EditAnchor : public GtkAReViWidget {
public :
  AR_CLASS(EditAnchor)
  AR_CONSTRUCTOR(EditAnchor)

  virtual
  void
  fill(ArRef<Anchor> anchor, ArRef<Body> body);

protected :
};

#endif // _EDIT_ANCHOR_H_
