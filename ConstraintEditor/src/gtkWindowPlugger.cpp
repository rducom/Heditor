#include "gtkWindowPlugger.h"

#include "AReVi/Lib3D/embeddedWindow3D.h"

namespace GtkTools {

AR_CLASS_DEF(GtkWindowPlugger, ArObject)

GtkWindowPlugger::GtkWindowPlugger(ArCW & arCW)
: ArObject(arCW),
  _widget(gtk_socket_new()),
  _window() {
}

GtkWindowPlugger::~GtkWindowPlugger(void) {
  if(_window) {
    _window.destroy();
  }

  if(GTK_IS_WIDGET(_widget)) {
    gtk_widget_destroy(_widget);
  }
}

GtkWidget* 
GtkWindowPlugger::getWidget(void) {
  return _widget;
}

long 
GtkWindowPlugger::getWidgetID(void) const {
  gtk_widget_realize(_widget);
  return(gtk_socket_get_id(GTK_SOCKET(_widget)));
}

void 
GtkWindowPlugger::setWindow(ArRef<EmbeddedWindow3D> window) {
  if(_window) {
    _window.destroy();
  }

  _window = window;

  if(_window) {
    g_signal_connect(_widget,
		     "configure-event",
                     G_CALLBACK(&GtkWindowPlugger::_configureCB), 
		     this);
    _window->addMouseButtonCB(thisRef(),
			      &GtkWindowPlugger::_buttonCB);
  }
}

ArRef<EmbeddedWindow3D> 
GtkWindowPlugger::getWindow(void) {
  return _window->thisRef();
}

void 
GtkWindowPlugger::_buttonCB(const Window3D::MouseButtonEvent & evt) {
  if(evt.pressed) {
    GTK_WIDGET_SET_FLAGS(_widget, GTK_CAN_FOCUS);
    gtk_widget_grab_focus(_widget);
  }
}

gboolean 
GtkWindowPlugger::_configureCB(GtkWidget*,
			       GdkEventConfigure* evt,
			       void* data) {
  GtkWindowPlugger* obj = reinterpret_cast<GtkWindowPlugger*>(data);
  if(obj->_window) {
    obj->_window->setWindowGeometry(0,0,evt->width,evt->height);
  }

  return false; // allow default behavior
}

} // namespace GtkTools;
