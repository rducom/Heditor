//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkAReViWidget.cpp
// description : AReVi GtkWidget container
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------

#include "gtkAReViWidget.h"
#include "AReVi/arClass.h"

//----------------------------------------------------------------------------
// GtkTools : Class GtkAReViWidget
//----------------------------------------------------------------------------
namespace GtkTools {
AR_CLASS_NOVOID_DEF(GtkAReViWidget,ArObject)

//-------- Construction / Destruction ----------------------------------------
GtkAReViWidget::GtkAReViWidget(ArCW & arCW,
                               const StlString &fileName,
                               const StlString &mainWidget) // Protected
: ArObject(arCW),
  _xmlObject(NULL),
  _wndMain(NULL),
  _hideOnDestroy(false),
  _dockDst(),
  _dockDstWidget(""),
  _wndDockSrc(NULL),
  _docked(false) {    
  _xmlObject = glade_xml_new(fileName.c_str(),  mainWidget.c_str(), NULL); 
  if(_xmlObject) {
    _wndMain=GTK_WIDGET(glade_xml_get_widget(_xmlObject,mainWidget.c_str()));
    glade_xml_signal_autoconnect(_xmlObject);
    if (!_wndMain)
      setErrorMessage("Can't find main widget " + mainWidget);
    else
      g_signal_connect((gpointer)_wndMain,"delete_event",G_CALLBACK(_onWidgetCloseCB),this);
  }
  else
    thisRef().destroy();
}

GtkAReViWidget::~GtkAReViWidget(void) // Protected
{
// Destroy widget if any
  if (_wndMain && GTK_IS_WIDGET(_wndMain))
    gtk_widget_destroy(_wndMain);

  _wndMain=NULL;

// Unref xmlObject
  if (_xmlObject)
    g_object_unref(G_OBJECT(_xmlObject));

  _xmlObject=NULL;
}

void
GtkAReViWidget::show(void)
{
  if (_wndMain)
    gtk_widget_show(_wndMain);
}

void
GtkAReViWidget::hide(void)
{
  if (_wndMain)
    gtk_widget_hide(_wndMain);
}

bool
GtkAReViWidget::isVisible(void) const
{
  if (_wndMain)
    return GTK_WIDGET_VISIBLE(_wndMain);
  else
    return false;
}

void
GtkAReViWidget::raise(void)
{
  if (_wndMain)
    gdk_window_raise(_wndMain->window);
}

GtkWidget *
GtkAReViWidget::accessMainWidget(void)
{
  return _wndMain;
}

GtkWidget *
GtkAReViWidget::accessWidget(const StlString &wName)
{
  return GTK_WIDGET(glade_xml_get_widget(_xmlObject,wName.c_str()));
}

bool
GtkAReViWidget::getHideOnDestroy(void) const
{
  return _hideOnDestroy;
}

void
GtkAReViWidget::setHideOnDestroy(const bool &val)
{
  _hideOnDestroy=val;
}

//------- Dockable Widget Attributes -----------------------------------------
bool
GtkAReViWidget::isDockable(void) const
{
  return _dockDst;
}

bool
GtkAReViWidget::isDocked(void) const
{
  return _docked;
}

// Active/Desactive dock skill.
// WARNING : If dockDst is destroy until this widget is docked, then
//           docked part will be lost!!!!
bool
GtkAReViWidget::setDock(const StlString &dockingPart,
                        ArPtr<GtkAReViWidget> dockDst,
                        const StlString &dockWidgetName)
{
  if (!_xmlObject || !_wndMain)
    return false;

  _wndDockSrc=accessWidget(dockingPart);
  //cerr << "Getting Dockable part=" << _wndDockSrc << endl;
  if (!_wndDockSrc)
    return false;

  //cerr << "Getting Dock destination  part" << endl;
  if (dockDst->accessWidget(dockWidgetName))
  {
    _dockDst=dockDst;
    _dockDstWidget=dockWidgetName;

    setHideOnDestroy(true);
  }
  else
    return false;

  return true;
}

void
GtkAReViWidget::dock(void)
{
  if (isDocked())
    return;

  if (isDockable())
  {
    gtk_widget_reparent(_wndDockSrc, _dockDst->accessWidget(_dockDstWidget));
    hide();
    _docked=true;
  }
  else if (!GTK_IS_WIDGET(_wndDockSrc))
    _wndDockSrc=NULL;
}

void
GtkAReViWidget::undock(void)
{
  if (!isDocked())
    return;

  if (isDockable())
  {
    gtk_widget_reparent(_wndDockSrc, _wndMain);
    _docked=false;
  }
  else if (!GTK_IS_WIDGET(_wndDockSrc))
    _wndDockSrc=NULL;

  show();
}


bool
GtkAReViWidget::getWidgetString(const StlString &propName,StlString &propValueOut)
{
  GtkWidget *widget=glade_xml_get_widget(_xmlObject,propName.c_str());
  if (!widget)
  {
    cerr << "[getPropertyString] property '" << propName << "' no exist" << endl;
    return false;
  }

  if (GTK_IS_ENTRY(widget))
    propValueOut=/*strFromUTF8*/(gtk_entry_get_text(GTK_ENTRY(widget)));
  else if (GTK_IS_TOGGLE_BUTTON(widget))
  {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) propValueOut="true";
    else propValueOut="false";
  }
  else if (GTK_IS_TEXT_VIEW(widget))
  {
    GtkTextBuffer* buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer,&start,&end);
    propValueOut=/*strFromUTF8*/(gtk_text_buffer_get_text(buffer,&start,&end,true));
  }
  else return false;

  return true;
}

bool
GtkAReViWidget::setWidgetString(const StlString &propName,const StlString &propValue)
{
  GtkWidget *widget=glade_xml_get_widget(_xmlObject,propName.c_str());
  if (!widget)
  {
    cerr << "[setPropertyString] property '" << propName << "' no exist" << endl;
    return false;
  }
  if (GTK_IS_ENTRY(widget))
    gtk_entry_set_text(GTK_ENTRY(widget),/*strToUTF8*/(propValue).c_str());
  else if (GTK_IS_TOGGLE_BUTTON(widget))
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),propValue=="true");
  }
  else if (GTK_IS_TEXT_VIEW(widget))
  {
    GtkTextBuffer* buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(widget));
    gtk_text_buffer_set_text(buffer,/*strToUTF8*/(propValue).c_str(),-1);
  }
  else return false;

  return true;
}

// Get Widget data as an bool (entry, textview)
bool
GtkAReViWidget::getWidgetBool(const StlString &propName,bool &propValueOut)
{
  StlString strValue;
  if (!getWidgetString(propName,strValue))
    return false;

  if (strValue=="true" || strValue=="TRUE" || strValue=="True")
    propValueOut=true;
  else if (strValue=="false" || strValue=="FALSE" || strValue=="False")
    propValueOut=false;
  else
    return false;

  return true;
}


// Set Widget data as an bool (entry, textview)
bool
GtkAReViWidget::setWidgetBool(const StlString &propName,bool propValue)
{
  if (propValue)
    return setWidgetString(propName,"true");
  else
    return setWidgetString(propName,"false");
}

// Get Widget data as an int (entry, textview)
bool
GtkAReViWidget::getWidgetInt(const StlString &propName,int &propValueOut)
{
  StlString strValue;
  if (!getWidgetString(propName,strValue))
    return false;
  return strToInt(propValueOut,strValue);
}

// Set Widget data as an int (entry, textview)
bool
GtkAReViWidget::setWidgetInt(const StlString &propName,int propValue)
{
  return setWidgetString(propName,intToStr(propValue));
}

// Get Widget data as an float (entry, textview)
bool
GtkAReViWidget::getWidgetFloat(const StlString &propName,float &propValueOut)
{
  StlString strValue;
  if (!getWidgetString(propName,strValue))
    return false;
  return strToFloat(propValueOut,strValue);
}

// Set Widget data as an float (entry, textview)
bool
GtkAReViWidget::setWidgetFloat(const StlString &propName,float propValue)
{
  return setWidgetString(propName,floatToStr(propValue));
}

// Get Widget data as an double (entry, textview)
bool
GtkAReViWidget::getWidgetDouble(const StlString &propName,double &propValueOut)
{
  StlString strValue;
  if (!getWidgetString(propName,strValue))
    return false;
  return strToDouble(propValueOut,strValue);
}

// Set Widget data as an double (entry, textview)
bool
GtkAReViWidget::setWidgetDouble(const StlString &propName,double propValue)
{
  return setWidgetString(propName,doubleToStr(propValue));
}

//-------- Forbidden operations ----------------------------------------------
bool //static protected
GtkAReViWidget::_onWidgetCloseCB(GtkWidget *,GdkEvent *, void *data)
{
  GtkAReViWidget * obj=reinterpret_cast<GtkAReViWidget *>(data);
  if (obj->_hideOnDestroy)
  {
    if (obj->isDockable())
      obj->dock();
    else
      obj->hide();
    return true;
  }
  else
  {
    if (obj->thisRef().valid())
      obj->thisRef().destroy();
    return true;
  }
}

} // end of namespace GtkTools
//---End-Of-File--------------------------------------------------------------
