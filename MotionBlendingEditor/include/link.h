#ifndef _LINK_H_
#define _LINK_H_

#include "AReVi/arObject.h"
using namespace AReVi;

#include <libgnomecanvas/libgnomecanvas.h>
#include <list>

using namespace std;

class Item;


// A Link links two Items
class Link : public ArObject {

public:	
  
  AR_CLASS(Link)
  AR_CONSTRUCTOR_3(Link, GnomeCanvasGroup*, parent, ArRef<Item>, itemParent, ArRef<Item>, itemChildren)

	// Handle the Link coords position, from the parent Item to the children Item.
  void update_location();

	// Call the disconnect procedure to the current Link
	// for a specific Item
  void disconnect(ArRef<Item> item);

	// Item's reference assessor methods
  ArRef<Item> parent_item() const { return _parent; }
  ArRef<Item> children_item() const { return _children; }
  
	
	// Graphical Object of the Item
  GnomeCanvasItem* accessLineItem() const {return _line;}

private:

  GnomeCanvasItem* 	_line;

  ArRef<Item> 			_parent; 
  ArRef<Item>			 	_children;
	
 };
	
 // STL container of Link's
  typedef list< ArRef<Link> > LinksList;


#endif
