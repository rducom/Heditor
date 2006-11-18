#include "AReVi/arClass.h"
#include <libgnomecanvas/libgnomecanvas.h>
#include "canvasController.h"
#include "item.h"



AR_CLASS_NOVOID_DEF(Item, ArObject)

Item::Item(ArCW& arCW, string name, double x, double y,GnomeCanvas *c, GdkPixbuf* image) 
:	ArObject(arCW),
	_width(),
  _height(),
  _name(name),
	_anim(),
	_isSelected(false),
	_isAblend(false),
	_itemGroup(),
	_itemParent(),
	_module_box(),
	_canvas_title(),
	_pixbuf(NULL)	{

	_itemParent = gnome_canvas_root(c);
		
	set_width(48.0);
  set_height(48.0);	
		
	//_color = 0x8899AAFF;
	_color = 0x1F2A3CFF;
		
		// Create the group of the Item
	GnomeCanvasItem *group;
	group = gnome_canvas_item_new(_itemParent,	
																gnome_canvas_group_get_type(),
																"x", x, "y", y, NULL);
	_itemGroup = 	GNOME_CANVAS_GROUP(group);
		
		// Create the Item rectangle, & attach it to the item group
	GnomeCanvasItem* rect = gnome_canvas_item_new(_itemGroup,
																								gnome_canvas_rect_get_type(),
																								"x1",0.0,"x2",get_width(),"y1",0.0,"y2",get_height(),
																								"width-units", 1.0,
																								"outline-color-rgba",0x8899AAFF,
																								"join-style",GDK_JOIN_ROUND,
																								"fill-color-rgba",_color, NULL);
	_module_box = GNOME_CANVAS_RECT(rect);

		// Create the Item text, & attach it to the item group
	GnomeCanvasItem* text = gnome_canvas_item_new(_itemGroup,
																								gnome_canvas_text_get_type(),
																								"size-set", true,
																								"size",10000,
																								"weight-set",true,
																								"weight",400,
																								"text",(gchar*)name.c_str(),
																								"fill-color-rgba",0xDDEEFFFF, NULL);
	_canvas_title = GNOME_CANVAS_TEXT(text);
	
	// If there's a pixbuf, attach it to the Item. (currently used for blend's)
	if(image) {
		GnomeCanvasItem* logo = gnome_canvas_item_new(_itemGroup,
																								gnome_canvas_pixbuf_get_type(),
																								"height",(double)gdk_pixbuf_get_height(image),
																								"height-in-pixels",true,
																								"height-set",false,
																								"pixbuf",image,
																								"width",(double)gdk_pixbuf_get_width(image),
																								"width-in-pixels",true,
																								"width-set",false,
																								"x",0.0,"x-in-pixels",false,
																								"y",0.0,"y-in-pixels",false, NULL);
	_pixbuf = GNOME_CANVAS_PIXBUF(logo);
	}
	
	
	// We raise graphicals objects to be sure they are on the foreground.
	gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(rect));
	if(_pixbuf){gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(_pixbuf));}
	gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(text));
	
	// We connect the "event" signal to our Item event's handler, passing the Item himself as argument.
	gtk_signal_connect(GTK_OBJECT(_itemGroup),"event",GTK_SIGNAL_FUNC(Item::item_event),this);

}

Item::~Item() {
	// GTK object are explicitely destroyed. 
	// They arent referenced by the AReVi Garbage collector.
	gtk_object_destroy(GTK_OBJECT(_module_box));
	gtk_object_destroy(GTK_OBJECT(_canvas_title));
	gtk_object_destroy(GTK_OBJECT(_itemGroup));
	if(_pixbuf){gtk_object_destroy(GTK_OBJECT(_pixbuf));}
	
	_parents.clear();
	_childrens.clear();
}



int
Item::item_event(GnomeCanvasGroup* iGroup, GdkEvent* event, gpointer data){
	Item* item = (Item*)data;
	
static double x, y;
double module_x, module_y;
static bool dragging = false;
bool handled = true;
	
module_x = event->button.x;
module_y = event->button.y;
	
switch (event->type) {	
	
	case GDK_BUTTON_PRESS:
		
		if (event->button.button == 1) {
			x = module_x;
			y = module_y;
			// we start grab on button press.
			gnome_canvas_item_grab(GNOME_CANVAS_ITEM(iGroup),
			GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK| GDK_BUTTON_PRESS_MASK,
														 NULL,
														 event->button.time);
			dragging = true;
		} else {
			handled = false;
		}
		break;

	case GDK_MOTION_NOTIFY:
		// when any movement happends.
		if (dragging && (event->motion.state & GDK_BUTTON1_MASK)) {
			// if we are dragging an item,
			double new_x = module_x;
			double new_y = module_y;
			// we move it on the canvas.
			gnome_canvas_item_move(GNOME_CANVAS_ITEM(iGroup), new_x - x, new_y - y);
			x = new_x;
			y = new_y;
			
			// And we update all his links, for them to connect to the right place
			// during the move.
			for (LinksList::iterator i = item->_parents.begin(); i != item->_parents.end(); ++i)
				(*i)->update_location();
			for (LinksList::iterator i = item->_childrens.begin(); i != item->_childrens.end(); ++i)
				(*i)->update_location();
			
		} else {
			// If we arent dragging, do nothing.
			handled = false;
		}
		break;

	case GDK_BUTTON_RELEASE:
		if (dragging) {
			// If we are draggin, we ungrab the Item on the canvas
			gnome_canvas_item_ungrab(GNOME_CANVAS_ITEM(iGroup),event->button.time);
			dragging = false;
		} else {
			handled = false;}
		break;

	case GDK_ENTER_NOTIFY:
		// If mouse over Item, we raise Item to the foreground.
		gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(iGroup));
	
		for (LinksList::iterator i = item->_parents.begin(); i != item->_parents.end(); ++i)
				gnome_canvas_item_raise_to_top((*i)->accessLineItem());
		for (LinksList::iterator i = item->_childrens.begin(); i != item->_childrens.end(); ++i)
				gnome_canvas_item_raise_to_top((*i)->accessLineItem());
		
		break;
		
	case GDK_NOTHING:
		break;
	
	default:
		handled = false;
	}

	return false;
}
  
void
Item::set_width(double w){
	_width = w;
}
  
void
Item::set_height(double h){
	_height = h;
}
  

void 
Item::move_to(double x, double y){
	gnome_canvas_item_set(GNOME_CANVAS_ITEM(_itemGroup),
												"x", x, "y", y, NULL);
}

void 
Item::set_isAblend(bool isAblend){
	_isAblend = isAblend;
	_color = 0x8822AA55;
	gnome_canvas_item_set(GNOME_CANVAS_ITEM(_module_box),
                        "fill-color-rgba",_color, NULL);

}

void
Item::set_item_selected(bool selected){
	if(selected) {
		gnome_canvas_item_set(GNOME_CANVAS_ITEM(_module_box),"fill-color-rgba",0xB41A2CFF, NULL);
		_isSelected = selected;
	}
	else {
		gnome_canvas_item_set(GNOME_CANVAS_ITEM(_module_box),"fill-color-rgba",_color, NULL);
		_isSelected = selected;
	}
}

void
Item::setName(string name){
	_name = name;
}

void
Item::setAnimation(ArRef<hLib::AnimationCycle> anim){
	_anim = anim;
}

void 		
Item::set_weight(double weight){
	_weight = weight;
}
  

GnomeCanvasPoints* Item::connection_coords(ArRef<Link> link) {
	GnomeCanvasPoints* point = gnome_canvas_points_new(2);
	
	double x = _height / 2;
	LinksList::iterator i;
	
	for( i = _parents.begin(); i!= _parents.end(); ++i){
		if( (*i) == link )
		{ x = _height; }
	}
	for( i = _childrens.begin(); i!= _childrens.end(); ++i){
		if( (*i) == link )
		{ x = 0.0; }
	}
	
	double y = _height / 2;
	
	gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(_itemGroup),&x, &y); 

	point->coords[0] = x;
	point->coords[1] = y;
	return point;
}
  
void 
Item::disconnect(ArRef<Link> link){
	LinksList::iterator i;
	
	// First we handle parent's disconnect
	if( _parents.size() != 0){
		if( _parents.size() == 2){
			
			// If the Link links to the father (the front)
			if( _parents.front() == link){
				_parents.pop_front(); // ureference the father (so as him to be destructed)
				// Disconnect the mother Item.
				_parents.back()->parent_item()->disconnect(_parents.back());
				// Clear all parents references
				_parents.clear();
				
			} // If the Link links to the mother (the back)
			else if(_parents.back() == link){
				_parents.pop_back(); // ureference the mother (so as her to be destructed)
				// disconnect the father Item
				_parents.front()->parent_item()->disconnect(_parents.front());
				// Clear all parents references
				_parents.clear();				
				
			}
		}
		
	}
	
	// Then we remove all childrens
	if(_childrens.size() != 0){
		for ( i = _childrens.begin(); i != _childrens.end(); ++i) {
			if( (*i) == link){
				i = _childrens.erase(i);
				return;
			}
		}
	}
}

void
Item::disconnect(void){
	
	LinksList::iterator i;
	ArRef<Item> tmp;
	
	if( _parents.size() != 0){
		while( _parents.size() > 0 ){
			// while they're still parents connected
				i = _parents.begin();
				tmp = (*i)->parent_item();
			// we say to each parent to disconnect the current Item from his list
			// via the link disconnect method.
				(*i)->disconnect(tmp);
			// and we remove the Link connection to the current Item
				i = _parents.erase(i);
		}
	}
	
	if(_childrens.size() != 0){	
		while( _childrens.size() > 0 ){
				i = _childrens.begin();
				tmp = (*i)->children_item();
				(*i)->disconnect(tmp);
				i = _childrens.erase(i);
		}
	}
}














