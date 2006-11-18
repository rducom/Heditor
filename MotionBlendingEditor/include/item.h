#ifndef _ITEM_H_
#define _ITEM_H_

/* This class provide an abstraction for the graphical & structural link object

**/

#include <gtk/gtk.h>
#include <glib.h>

#include <map>
#include <libgnomecanvas/libgnomecanvas.h>

//#include "link.h"

#include "AReVi/arObject.h"
#include "hLib/hLib.h"

using namespace AReVi;

using namespace std;



class Link;
class CanvasController;


class Item : public ArObject {

public:
  AR_CLASS(Item)
  AR_CONSTRUCTOR_5(Item, string, name, double, x, double, y, GnomeCanvas*, c, GdkPixbuf*, image)


	// Event handler for all events happening over an item (over his group).
  static int item_event(GnomeCanvasGroup* iGroup, GdkEvent* event, gpointer data);
  

	// Item Graphical attributes methodes.
  void set_width(double w);
  void set_height(double h);
	double get_width(void) const {return _width;}
  double get_height(void) const {return _height;}

	GnomeCanvasRect*  				accessItemRect()	{ return _module_box;}
	GnomeCanvasText*  				accessItemText()  { return _canvas_title;}
	GnomeCanvasPixbuf*  			accessItemPixbuf()  { return _pixbuf;}
	GnomeCanvasGroup*    			accessItemGroup()	{ return _itemGroup;}

	// Item Status methodes.
  bool get_isAblend(void)           {return _isAblend;}
  void set_isAblend(bool isAblend);
  
  bool get_item_selected(void)      {return _isSelected;}
  void set_item_selected(bool selected);
  
	// Item associated animation, animation attributes & name
	ArRef<hLib::AnimationCycle> getAnimation()	{ return _anim;}
	void                        setAnimation(ArRef<hLib::AnimationCycle> anim);  
	
	double 	get_weight(void)           {return _weight;}
  void 		set_weight(double weight);
	
  string 											getName() 					{ return _name;}
  void                        setName(string name);
  
	/* Item dependances methodes */
  LinksList*								accessItemParent() { return &_parents;}
	LinksList*								accessItemChildren() { return &_childrens;}
	
	
	// Abritrary Item positionnement
  void move_to(double x, double y);

	// Compute link's connection coord's for the current item
  GnomeCanvasPoints* connection_coords(ArRef<Link> link);
	
	// Full Item disconnecting
	// All links to current Item are removed
	void disconnect(void);
	
	// Disconnect the Item connection to "link"
	// Internaly used by disconnect(void)
  void disconnect(ArRef<Link> link);
  
protected:

	// Graphical Item attributes
  double 	_width;  
  double 	_height;
	int			_color;

	// Item data's
	string _name;

  ArRef<hLib::AnimationCycle>       _anim;


  bool _isSelected; // specifies if the item is selected
  bool _isAblend; 	// specifies if this item is a blending
	double _weight;   // if the item is a blend, his blend's weight
  
	GnomeCanvasGroup*    _itemGroup;   // group of the Item
  GnomeCanvasGroup*    _itemParent;  // parent group of the Item's group

	// Graphical objects associated to the Item
  GnomeCanvasRect*  _module_box;
  GnomeCanvasText*  _canvas_title;
  GnomeCanvasPixbuf*  _pixbuf;

	// Links to dependant Items.
  LinksList _parents; 				// parent are the sources Items in the bend process
	LinksList _childrens;       // childrens are the destination Items in the bend process
 };
 
 // Type definitions of List and Links Container
typedef multimap< string, ArRef<Item> > ItemMap;  // MultiMap is a multi-associated container
typedef list< ArRef<Item> >             ItemList; // Standart STL list
 
#endif
