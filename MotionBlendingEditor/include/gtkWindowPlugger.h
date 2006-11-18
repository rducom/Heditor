#ifndef GTK_WINDOW_PLUGGER_H
#define GTK_WINDOW_PLUGGER_H

#include <gtk/gtk.h>

#include "AReVi/arSystem.h"
#include "AReVi/arObject.h"
#include "AReVi/Lib3D/embeddedWindow3D.h"
using namespace AReVi;

namespace GtkTools {

class GtkWindowPlugger : public ArObject {
public:
  AR_CLASS(GtkWindowPlugger)
  AR_CONSTRUCTOR(GtkWindowPlugger)

  virtual 
  GtkWidget* 
  getWidget(void);

  virtual 
  long 
  getWidgetID() const;
  
  virtual 
  void 
  setWindow(ArRef<EmbeddedWindow3D> window);
  
  virtual 
  ArRef<EmbeddedWindow3D> 
  getWindow(void);

protected:
  virtual 
  void 
  _buttonCB(const Window3D::MouseButtonEvent& evt);
  
  static 
  gboolean 
  _configureCB(GtkWidget* w,
	       GdkEventConfigure* evt,
	       void* data);

protected :
  GtkWidget* _widget;
  ArPtr<EmbeddedWindow3D> _window;
};

} // namespace GtkTools

#endif // GTK_WINDOW_PLUGGER_H
