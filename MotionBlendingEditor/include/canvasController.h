#ifndef _CANVAS_CONTROLLER_H_
#define _CANVAS_CONTROLLER_H_

#include <gdk/gdkkeysyms.h>
#include <libgnomecanvas/libgnomecanvas.h>
#include <string.h>
#include "link.h"
#include "item.h"
#include "AReVi/arObject.h"
#include "hLib/animation/animationCycle.h"

using namespace AReVi;
using namespace hLib;



class CanvasController : public ArObject {
  
public :

  AR_CLASS(CanvasController)
  AR_CONSTRUCTOR_1(CanvasController, GtkWidget*, canvasWidget)

	// Creation / Destruction Item methods.
  void addItem(ArRef<Item> i);
  void removeItem(ArRef<Item> i);

	// Link / Unlink, Blending methods.
  ArRef<Item> makeBlend( ArRef<Item> item1, ArRef<Item> item2);
	ArRef<Item> areLinked( ArRef<Item> item1, ArRef<Item> item2);
	virtual bool removeBlend( ArRef<Item> item1, ArRef<Item> item2);


	// Canvas event handler, used for selection handling.
  static bool canvas_event(GtkWidget *widget, GdkEvent* event, gpointer data);

	// Class attributes assessors.
  ItemMap&					getitems() 					{ return _items; }
  double						width() const				{ return _width; }
  double						height() const			{ return _height;}
	GnomeCanvasGroup*	accessGroup() const	{ return _group; }
	ArRef<Item>	      accessDraggedItem() { return _draggedItem;}
  GtkWidget*        accessCanvas()      { return _canvas;}
  ItemList*         accessSelected()    { return &_selected;}
  
	// Access the Item beyond the canvas reprensatation
  ArRef<Item> find_item(GnomeCanvasItem* item);
	
	// Selection handling methods.
	void selectSingleItem(ArRef<Item> item);
  void unselectAllItems(void);
	bool selectItem(ArRef<Item> item);
	bool unselectItem(ArRef<Item> item);
	bool is_selectedItem(ArRef<Item> item);
	
	// Drag'N Drop facilities methods, so as to represent an Item
	// at the first time he's beeing dragged over the canvas.
	void DraggedItem_New(string s);
	void DraggedItem_Finalize();
	void DraggedItem_End();
	
	// Drag'N Drop event handlers.
	static gboolean DNDCanvasDragMotion(GtkWidget *widget,GdkDragContext *context,int x, int y,guint32 time,gpointer user_data);
	static gboolean DNDCanvasDragDrop(GtkWidget *widget,GdkDragContext *context,int x,int y,guint32 time,gpointer data);
	static void DNDCanvasDragLeave(GtkWidget *widget,GdkDragContext *context,guint32 time,gpointer data);
	
	
	// Current selection refresh callback 
  AR_CALLBACK(CanvasController, Setanim, _setanimCB, ArRef<Item> item;)
  
protected :

	// Current selection refresh callback signal definition
  CallbackManager<CanvasController, SetanimEvent> _setanimCB;
	// and his current signal caller.
  virtual void _onSetanim(void);


private:
	
  GtkWidget*           _canvas; // The Gnome Canvas Widget.
  GnomeCanvasGroup*    _group;  // The canvas group.

  ItemMap              _items; // Multimap of all Items, used to reference them.

	ArRef<Item>					 _draggedItem; // Temporary Item reference used for Drag&Drop.

  GnomeCanvasRect*      _base_rect; // Background of the canvas
  GnomeCanvasGroup*     _selectorGroup;	// Selection group. Not yet implemented.(For further multi-Item move)
  ItemList              _selected; 			// List of selected Items

  GdkPixbuf*            _blendpixmap; // icon for blended animations

  bool                 _connection_dragging;
  bool                 _scroll_dragging;
  ArPtr<Link>          _drag_connection;
  double               _width;
  double               _height;
};

#endif
