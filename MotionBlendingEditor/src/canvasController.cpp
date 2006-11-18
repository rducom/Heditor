#include "canvasController.h"

#include "AReVi/arClass.h"
#include <gtk/gtk.h>
#include <stdlib.h>

//#include <map>

AR_CLASS_NOVOID_DEF(CanvasController, ArObject)

  CanvasController::CanvasController(ArCW& arCW, GtkWidget* canvasWidget) 
    : ArObject(arCW),
      _setanimCB(thisRef()),
      _canvas(canvasWidget),
      _items(),
      //_links(),
      _draggedItem(),
      _base_rect(),
      _selectorGroup(),
      _selected(),
      _blendpixmap(),
      _connection_dragging(false),
      _scroll_dragging(false),
      _drag_connection(NULL) {
		
      _blendpixmap = gdk_pixbuf_new_from_file("data/animation.png",NULL);
		
      GnomeCanvas* c = GNOME_CANVAS(_canvas);
		
      _group = gnome_canvas_root(c);
			
      gnome_canvas_set_dither(c, GDK_RGB_DITHER_NORMAL);	
      gnome_canvas_set_scroll_region(c, 0.0, 0.0, 800.0, 800.0);
      gnome_canvas_set_center_scroll_region(c, true);
	
      _base_rect = GNOME_CANVAS_RECT( gnome_canvas_item_new(_group,
							    gnome_canvas_rect_get_type(),
							    "x1", 0.0, "x2", 800.0, "y1", 0.0, "y2", 800.0, 
							    "fill-color-rgba",0x222222FF, NULL) );
      gnome_canvas_item_show(GNOME_CANVAS_ITEM(_base_rect));
	
      //g_signal_connect(_canvas, "activate", G_CALLBACK(&Editor::_onFileQuitMI), this);
		
		
      //  Setup Drag and Drop Destination canvas
      enum {
	TARGET_MBEanim,
	TARGET_STRING
      };
		
      GtkTargetEntry target_entry[3] = { {"canvas",		GTK_TARGET_SAME_APP, TARGET_MBEanim},
					 {"text/plain", GTK_TARGET_SAME_APP, TARGET_STRING },
					 {"text/uri-list", 		GTK_TARGET_SAME_APP, TARGET_STRING } };
      gtk_drag_dest_set(canvasWidget,GTK_DEST_DEFAULT_ALL,
			target_entry,
			sizeof(target_entry) / sizeof(GtkTargetEntry),
			GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK | GDK_ACTION_ASK));
																			 
      /* Messages for incoming drag. */
      g_signal_connect(canvasWidget, "drag_motion",  		  G_CALLBACK (CanvasController::DNDCanvasDragMotion), this);
      g_signal_connect(canvasWidget, "drag_drop",				  G_CALLBACK (CanvasController::DNDCanvasDragDrop), this);
      g_signal_connect(canvasWidget, "drag_leave",				  G_CALLBACK (CanvasController::DNDCanvasDragLeave), this);

      // Setup canvas event signal
      g_signal_connect(canvasWidget, "event",  G_CALLBACK (CanvasController::canvas_event), this);
																			 
    }

CanvasController::~CanvasController(void){
  _items.clear();
	
  gtk_object_destroy(GTK_OBJECT(_group));
  gtk_object_destroy(GTK_OBJECT(_base_rect));
	
  gtk_widget_destroy(_canvas);
}

void CanvasController::addItem(ArRef<Item> i){
	
  std::pair<string, ArRef<Item> >  p(i->getName(), i);
  _items.insert(p);
	
}

void
CanvasController::removeItem(ArRef<Item> item) {
	
  if ( item != Item:: nullRef() ) {
		
    item->disconnect();
		
    gnome_canvas_item_hide( GNOME_CANVAS_ITEM(item->accessItemGroup()) );
		
    for ( ItemMap::iterator i = _items.begin(); i != _items.end(); i++) {
      if( (*i).second == item){
	_items.erase(i);
	i = _items.end();
	return;
      }
    }
		
  } else {
    std::cerr << "[CanvasController::remove_module] Unable to find Item!" << std::endl;
  }
	
}


ArRef<Item>
CanvasController::makeBlend( ArRef<Item> item1, ArRef<Item> item2){
	
  if( (item1 != Item::nullRef()) && (item2 != Item::nullRef()) )
    {
      // Create new coords, and new name of the new animation
      double x1 =0 , y1 = 0; // Coords of the first item
      double x2 = 0, y2 = 0; // Coords of the second item
      string blendname = item1->getName() + item2->getName();
	
		
      if(item1->accessItemGroup()){
	gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(item1->accessItemGroup()),&x1,&y1);
	gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(item2->accessItemGroup()),&x2,&y2);
      }
			
      double nx, ny; // Position of the new blend
      if( x1 <= x2 ) {
	nx = x1 - ( item1->get_width() + item2->get_width() ) ;
	if(nx<=0) { nx = x1 + 2*( item1->get_width() + item2->get_width() ) ;}
      } else {
	nx = x2 - ( item1->get_width() + item2->get_width() ) ;
	if(nx<=0) { nx = x2 + 2*( item1->get_width() + item2->get_width() ) ;}
      }
			
      ny = (y1 + y2) /2;
			
      // Create new canvas item and link it to his parents
      //      ArRef<Item> newblend = new_Item(blendname,nx,ny, GNOME_CANVAS(_canvas), _blendpixmap);
      ArRef<Item> newblend = Item::NEW(blendname,nx,ny, GNOME_CANVAS(_canvas), _blendpixmap);
      newblend->set_isAblend(true);
      addItem( newblend );
			
      //      ArRef<Link> link1 = new_Link(_group,item1,newblend);
      ArRef<Link> link1 = Link::NEW(_group,item1,newblend);
      //      ArRef<Link> link2 = new_Link(_group,item2,newblend);
      ArRef<Link> link2 = Link::NEW(_group,item2,newblend);
			
      item1->accessItemChildren()->push_back(link1);
      item2->accessItemChildren()->push_back(link2);
			
      newblend->accessItemParent()->push_back(link1);
      newblend->accessItemParent()->push_back(link2);
			
      link1->update_location();
      link2->update_location();
			
      // Create new AnimationBlendCycle with blend's parents AnimationCycle
      //      ArRef<AnimationBlendCycle> animblend = new_AnimationBlendCycle();
      ArRef<AnimationBlendCycle> animblend = AnimationBlendCycle::NEW();
			
      ArRef<AnimationCycle> anim1 = item1->getAnimation();
      ArRef<AnimationCycle> anim2 = item2->getAnimation();
    
      if( (anim1 != AnimationCycle::nullRef() ) && (anim2 != AnimationCycle::nullRef() ) ) {
				
	animblend->addAnimationCycle(anim1);
	animblend->addAnimationCycle(anim2);
	newblend->setAnimation( ar_down_cast<AnimationCycle>(animblend) );
      }
      return newblend;
    }
  else {
    return Item::nullRef();
  }
}


ArRef<Item> // Return ref on the blend Item of the link of item1 & item2
CanvasController::areLinked( ArRef<Item> item1, ArRef<Item> item2){
	
  ArRef<Item> found = Item::nullRef();	
  if( !(item1->accessItemChildren()->empty() || item2->accessItemChildren()->empty()) ) {
    LinksList::iterator i;
    LinksList::iterator c;	

    for (i = item1->accessItemChildren()->begin(); i != item1->accessItemChildren()->end(); ++i) {

      LinksList* potential = (*i)->children_item()->accessItemParent();
			
      for (c = potential->begin(); c != potential->end(); ++c) {
	if( (*c)->parent_item() == item2 )
	  { found = (*i)->children_item();}
      }
    }	
  }
  return found;
}

bool 
CanvasController::removeBlend( ArRef<Item> item1, ArRef<Item> item2){
  bool ok;
  ArRef<Item> found = areLinked(item1,item2);
	
  if( found != Item::nullRef() ){
    LinksList::iterator i;
    ArRef<Item> tmp;
		
    if( found->accessItemParent()->size() != 0){
      while( found->accessItemParent()->size() > 0 ){
	i = found->accessItemParent()->begin();
	tmp = (*i)->parent_item();
	(*i)->disconnect(tmp);
	i = found->accessItemParent()->erase(i);
      }
    }
    ok = 1;
  } else {
    ok =0;
  }
	
  return ok;
}


bool
CanvasController::canvas_event(GtkWidget *widget, GdkEvent* event, gpointer data){
	
  CanvasController* cc = (CanvasController*)data;
  double module_x, module_y;
	
  static bool will_select;
	
  module_x = event->button.x;
  module_y = event->button.y;

  gtk_widget_grab_focus(cc->accessCanvas());
	
  if(cc->accessGroup()) {
    gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(cc->accessGroup()),&module_x,&module_y);
  }
	
  switch (event->type) {	
	
  case GDK_BUTTON_PRESS:
    if ( event->button.button == 1) {
			
      GnomeCanvasItem* aimed = gnome_canvas_get_item_at(GNOME_CANVAS(widget),module_x, module_y);
      ArRef<Item> found = cc->find_item(aimed);
			
      if( (found != Item::nullRef() ) && will_select)	{
				
	if( cc->is_selectedItem(found)) {cc->unselectItem(found);}
	else 														{cc->selectItem(found);}
      }
      else if ( (found != Item::nullRef() ) && !will_select) {
	cc->selectSingleItem(found);
      } else { // If nothing found, we unselect all
	cc->unselectAllItems();
      }
    }	
    break;
	
  case GDK_BUTTON_RELEASE:
    break;
	
  default:
    break;
  }

	
  switch (event->key.type) {
	
  case GDK_KEY_PRESS:
		
    if(	(event->key.keyval == GDK_Control_L) || // Activate multi-selection
	(event->key.keyval == GDK_Control_R) ||
	(event->key.keyval == GDK_Shift_L)   ||
	(event->key.keyval == GDK_Shift_R) 	) {
      will_select = true;
    }
		
    if( (event->key.keyval == GDK_Delete) || (event->key.keyval == GDK_BackSpace) ){
      // Suppress selected Items
      if( cc->accessSelected()->size() != 0){
	ItemList::iterator i;
	while( cc->accessSelected()->size() > 0 ){
	  i = cc->accessSelected()->begin();
	  (*i)->set_item_selected(false);
	  cc->removeItem(*i);
	  i = cc->accessSelected()->erase(i);
	}
      }
			
    }
		
		
    break;

  case GDK_KEY_RELEASE: // Desactivate multi-selection
    if(	(event->key.keyval == GDK_Control_L) ||
	(event->key.keyval == GDK_Control_R) ||
	(event->key.keyval == GDK_Shift_L)   ||
	(event->key.keyval == GDK_Shift_R) 	) {
      will_select = false;
    }
    break;	
  default:
    break;
  }
  return false;
}

/*
  ArRef<Item> CanvasController::find_item(const string& name){
  ItemMap::iterator m = _items.find(name);
  if (m != _items.end())
  return (*m).second;
  else
  return Item::nullPtr();
  }*/

ArRef<Item> CanvasController::find_item(GnomeCanvasItem* item){
  ItemMap::iterator c;
  for (c = _items.begin(); c != _items.end(); ++c) {
    if(GNOME_CANVAS_ITEM( (*c).second->accessItemText() ) == item) {
      return (*c).second;
    }
    else if(GNOME_CANVAS_ITEM( (*c).second->accessItemRect() ) == item)	{
      return (*c).second;
    }
    else if(GNOME_CANVAS_ITEM( (*c).second->accessItemPixbuf() ) == item)	{
      return (*c).second;
    }
  }
  return Item::nullRef();
}

void
CanvasController::_onSetanim(void){
  if(!_setanimCB.empty()) {
    SetanimEvent evt;
    evt.item = _selected.back();
    _setanimCB.fire(evt);
  }
}

void
CanvasController::selectSingleItem(ArRef<Item> item){
  unselectAllItems();
  selectItem(item);
  _onSetanim();
}

void
CanvasController::unselectAllItems(void){
  ItemList::iterator c;
  for (c = _selected.begin(); c != _selected.end(); ++c) {
    (*c)->set_item_selected(false);
  }
  _selected.clear();
}

bool
CanvasController::selectItem(ArRef<Item> item){
  bool ok;
  if(item != Item::nullRef() ) {
    item->set_item_selected(true);
    _selected.push_back(item);
    _onSetanim();
    ok = true;
  } else {
    ok = false;
  }
  return ok;
}

bool
CanvasController::unselectItem(ArRef<Item> item){
  bool ok = false;
  if(!_selected.empty())
    { 
      ItemList::iterator i = find(_selected.begin(), _selected.end(), item);
      if( item == (*i) )	{
	if( _selected.size() > 1 ) {
	  for (i = _selected.begin(); i != _selected.end(); ++i) {
	    if( (*i) == item ) {
	      (*i)->set_item_selected(false);
						
	      if( i == _selected.begin() ){
		_selected.remove(*i); 
		i = _selected.begin();
	      } else if( i == _selected.end() ){
		_selected.remove(*i); 
		i = _selected.end();
	      }
	      ok = 1;
	      _onSetanim();
	    }
	  }
	} else { //  _selected.size() = 1
	  (*i)->set_item_selected(false);
	  _selected.clear();
	  ok = 1;
	}
      } else { 
	ok = 0;
      }
    } else {
    ok = 0;
  }
  return ok;
}

bool
CanvasController::is_selectedItem(ArRef<Item> item){ 
  bool ok;
  if(!_selected.empty())
    { 
      ItemList::iterator i = find(_selected.begin(), _selected.end(), item);
		
      if( item == (*i) )	{ ok = 1;}
      else { ok=0;}
		
    } else { ok=0;}
  return ok;
}


void
CanvasController::DraggedItem_New(string s){
	
  //  ArRef <Item> newItem = new_Item(s,0,0, GNOME_CANVAS(_canvas) , NULL);
  ArRef <Item> newItem = Item::NEW(s,0,0, GNOME_CANVAS(_canvas) , NULL);
	
  _draggedItem = newItem;
	
  gnome_canvas_item_hide(GNOME_CANVAS_ITEM(_draggedItem->accessItemGroup()));
  /* Set transparency on the dragged object*/
  gnome_canvas_item_set(GNOME_CANVAS_ITEM(_draggedItem->accessItemRect()),
			"fill-color-rgba",0x0F1A2C11, NULL);
	
  GdkCursor* cursor = gdk_cursor_new(GDK_STAR);
  gnome_canvas_item_grab(GNOME_CANVAS_ITEM(_draggedItem->accessItemGroup()),
			 GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK,
			 cursor,GDK_CURRENT_TIME);
  addItem( newItem );
}

void
CanvasController::DraggedItem_Finalize(){
  if( _draggedItem != Item::nullRef() )
    {
      gnome_canvas_item_set(GNOME_CANVAS_ITEM(_draggedItem->accessItemRect()),
			    "fill-color-rgba",0x1F2A3CFF, NULL);
    }
}

void
CanvasController::DraggedItem_End(){
  selectSingleItem(_draggedItem);
  _draggedItem = Item::nullRef();
}


gboolean 
CanvasController::DNDCanvasDragMotion(GtkWidget* /*widget*/,GdkDragContext* /*context*/ ,int x, int y,guint32 /*time*/,gpointer user_data){
  CanvasController* cc = (CanvasController*)user_data;
  gboolean end = true;
  double local_x = (double)x;
  double local_y = (double)y;
	
	
  if(cc->_draggedItem != Item::nullRef() ) {
    gnome_canvas_item_w2i(GNOME_CANVAS_ITEM(cc->_draggedItem->accessItemGroup()),&local_x,&local_y);
    gnome_canvas_item_move(GNOME_CANVAS_ITEM(cc->_draggedItem->accessItemGroup()), local_x, local_y);
    gnome_canvas_item_show(GNOME_CANVAS_ITEM(cc->_draggedItem->accessItemGroup()));
    end = false;
  }
  else { end = true;}
	
  return end;
}

gboolean 
CanvasController::DNDCanvasDragDrop(GtkWidget* /*widget*/,GdkDragContext* /*context*/,int /*x*/,int /*y*/,guint32 /*time*/,gpointer data){
  CanvasController* cc = (CanvasController*)data;
  if(cc->_draggedItem != Item::nullRef() ) {
    gnome_canvas_item_ungrab(GNOME_CANVAS_ITEM(cc->_draggedItem->accessItemGroup()),GDK_CURRENT_TIME);
  }
  return false;
}

void 
CanvasController::DNDCanvasDragLeave(GtkWidget* /*widget*/,GdkDragContext* /*context*/ ,guint32 /*time*/ ,gpointer data){
  CanvasController* cc = (CanvasController*)data;
  cc->DraggedItem_Finalize();
}
