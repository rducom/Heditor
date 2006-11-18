//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkToolsImages.h
// description : Tools for load pixmap/pixbuf...
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------
#ifndef GTKTOOLS_IMAGES_H
#define GTKTOOLS_IMAGES_H 1

#include <gtk/gtk.h>
#include <string>

//----------------------------------------------------------------------------
// GtkTools : Images
//----------------------------------------------------------------------------
namespace GtkTools {

//! Use this function to set the directory containing installed images.
void
addPixmapDirectory(const std::string &directory);

//------------------------------ Pixmaps -------------------------------------
//! This is used to create the pixmaps used in the interface.
GtkWidget*
createPixmap(const std::string &filename);

//! This is used to create the pixmaps used in the interface.
GtkWidget*
createPixmapFromData(const char **data);

//! This is used to create the pixmaps used in the interface.
GtkWidget*
createPixmapFromPixbuf(GdkPixbuf* pixbuf);

//------------------------------ Pixbufs -------------------------------------
//! This is used to create the pixbufs used in the interface.
GdkPixbuf*
createPixbuf(const std::string &filename);

//! This is used to create the pixbuf used in the interface.
GdkPixbuf*
createPixbufFromData(const char **data);

//------------------------------ Colors -------------------------------------
//! Convert a string eg : #00FF00 to GdkColor (RGB = 0, 255, 0).
bool
strToGdkColor(const std::string &strColor, GdkColor &color);

//! Convert a GtkColor to a string  eg : GdkColor (RGB = 0, 255, 0) to #00FF00.
std::string
gdkColorToStr(GdkColor &color);




} // end of namespace GtkTools
#endif // end of GTKTOOLS_IMAGES_H
//---End-Of-File---------------------------------------------------------------
