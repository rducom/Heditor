#ifndef MULTI_VIEW_H
#define MULTI_VIEW_H

#include <gtk/gtk.h>
#include "gtkWindowPlugger.h"
#include "AReVi/arObject.h"
#include "AReVi/Lib3D/boundingBox3D.h"
#include "AReVi/Lib3D/embeddedWindow3D.h"
#include "AReVi/Lib3D/simpleInteractor.h"
#include "AReVi/Lib3D/osdText.h"

using namespace GtkTools;
using namespace AReVi;

class MultiView : public ArObject {
public :
  AR_CLASS(MultiView)
  AR_CONSTRUCTOR(MultiView)

  enum Views {
    FRONT_VIEW = 0,
    RIGHT_VIEW,
    TOP_VIEW,
    PERSPECTIVE_VIEW_MULTI,
    PERSPECTIVE_VIEW
  };

  virtual void initViews(void);

  virtual GtkWidget* getMultiViewWidget(void);

  virtual GtkWidget* getSingleViewWidget(void);

  virtual ArRef<EmbeddedWindow3D> getWindow(Views v);

  virtual void adjustViews(ArRef<BoundingBox3D> bbox);

protected :
  ArRef<EmbeddedWindow3D> _views[5];
  ArRef<GtkWindowPlugger> _pluggers[5];

  GtkWidget* _multiView;
  GtkWidget* _singleView;

  ArRef<OSDText> _osd[5];
};

#endif // MULTI_VIEW_H
