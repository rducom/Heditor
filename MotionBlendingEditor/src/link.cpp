#include "AReVi/arClass.h"

#include "link.h"
#include "item.h"

#include <gtk/gtk.h>

AR_CLASS_NOVOID_DEF(Link, ArObject)
	
Link::Link(ArCW& arCW, GnomeCanvasGroup* parent,ArRef<Item> itemParent, ArRef<Item> itemChildren)
: ArObject(arCW),
	_parent(itemParent),
	_children(itemChildren) {

	_line = gnome_canvas_item_new(parent, gnome_canvas_line_get_type()
																			,"width_units",1.0
																			,"fill-color-rgba",0x888888FF
																			,"cap-style",GDK_CAP_ROUND
																			,"arrow-shape-a",7.5
																			,"arrow-shape-b",10.0
																			,"arrow-shape-c",2.5
																			,"smooth",true ,   NULL);
	update_location();		
}
	
Link::~Link() {
	gtk_object_destroy(GTK_OBJECT(_line));
}

void Link::update_location(){
	GnomeCanvasPoints* points = gnome_canvas_points_new(2);
	
	GnomeCanvasPoints* src = _parent->connection_coords(Link::thisRef());
	GnomeCanvasPoints* dst = _children->connection_coords(Link::thisRef());
	
	points->coords[0] = src->coords[0];
	points->coords[1] = src->coords[1];
	points->coords[2] = dst->coords[0];
	points->coords[3] = dst->coords[1];
	
	gnome_canvas_item_set(_line,"points",points, NULL);
}

void Link::disconnect(ArRef<Item> item){
	if( _parent == item ){
		_parent->disconnect(Link::thisRef() );
	}
	if( _children == item ){
		_children->disconnect(Link::thisRef() );
	}
	_parent = Item::nullRef();
	_children = Item::nullRef();
}

