//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkAReViWidget.h
// description : AReVi GtkWidget container
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------
#ifndef GTKTOOLS_GTKAREVIWIDGET_H
#define GTKTOOLS_GTKAREVIWIDGET_H 1

#include <AReVi/arObject.h>
#include <gtk/gtk.h>
#include <glade/glade.h>


namespace AReVi
{
  class Activity;
}
using namespace AReVi;

//----------------------------------------------------------------------------
// GtkTools : Class GtkAReViWidget
//----------------------------------------------------------------------------
namespace GtkTools {

class GtkAReViWidget : public ArObject {
public:
  AR_CLASS(GtkAReViWidget)
//-------- Construction / Destruction ----------------------------------------
  AR_CONSTRUCTOR_2(GtkAReViWidget,
                  const StlString &,fileName,
                  const StlString &,mainWndName)

//---------------- Widget Controle -------------------------------------------
  //! Show widget
  virtual 
  void
  show(void);

  //! hide widget
  virtual 
  void
  hide(void);

  //! Bring widget on top
  virtual 
  void
  raise(void);

  //! Verify widget visibility
  virtual 
  bool
  isVisible(void) const;

  //! Access to main widget
  virtual 
  GtkWidget *
  accessMainWidget(void);

  //! Access to any widget pointed by his name
  virtual 
  GtkWidget *
  accessWidget(const StlString &wName);

//---------------- Widget Attributes -----------------------------------------
  bool
  getHideOnDestroy(void) const;

  void
  setHideOnDestroy(const bool &val);

//------- Dockable Widget Attributes -----------------------------------------
  bool
  isDockable(void) const;

  bool
  isDocked(void) const;

  //! Active/Desactive dock skill.
  //! WARNING : If dockDst is destroy until this widget is docked, then
  //!           docked part will be lost!!!!
  bool
  setDock(const StlString &dockingPart,
          ArPtr<GtkAReViWidget> dockDst,
          const StlString &dockWidgetName);

  void
  dock(void);

  void
  undock(void);

//-------- Widget data tools -------------------------------------------------
  //! Get Widget data as a string (entry, textview, toggleButton)
  bool
  getWidgetString(const StlString &propName, StlString& propValueOut);

  //! Set Widget data as a string (entry, textview, toggleButton)
  bool
  setWidgetString(const StlString &propName,const StlString& propValue);

  //! Get Widget data as an bool (entry, textview)
  bool
  getWidgetBool(const StlString &propName,bool &propValueOut);

  //! Set Widget data as an bool (entry, textview)
  bool
  setWidgetBool(const StlString &propName,bool propValue);

  //! Get Widget data as an int (entry, textview)
  bool
  getWidgetInt(const StlString &propName,int &propValueOut);

  //! Set Widget data as an int (entry, textview)
  bool
  setWidgetInt(const StlString &propName,int propValue);

  //! Get Widget data as an float (entry, textview)
  bool
  getWidgetFloat(const StlString &propName,float &propValueOut);

  //! Set Widget data as an float (entry, textview)
  bool
  setWidgetFloat(const StlString &propName,float propValue);

  //! Get Widget data as an double (entry, textview)
  bool
  getWidgetDouble(const StlString &propName,double &propValueOut);

  //! Set Widget data as an double (entry, textview)
  bool
  setWidgetDouble(const StlString &propName,double propValue);

//-------- Forbidden operations ----------------------------------------------
protected:
  //! Callback on delete_event.
  static bool
  _onWidgetCloseCB(GtkWidget * w,GdkEvent *, void *data);

//-------- Member attributes -------------------------------------------------
protected:
  GladeXML* _xmlObject;
  GtkWidget* _wndMain;

  //! Destroy widget on close demand
  bool _hideOnDestroy;

  //! Dockable skill related attributes
  ArPtr<GtkAReViWidget> _dockDst;       // Dock destination Container
  StlString _dockDstWidget; // Dock destination Widget Name

  GtkWidget* _wndDockSrc; // Dockable part.
  bool _docked;
};

} // end of namespace GtkTools

#endif // end of GTKTOOLS_GTKAREVIWIDGET_H
