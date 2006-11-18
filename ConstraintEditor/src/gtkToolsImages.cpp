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
#include "gtkToolsImages.h"
#include "AReVi/Utils/stlString_priv.h"

using namespace std;
using namespace AReVi;
//----------------------------------------------------------------------------
// GtkTools : Images
//----------------------------------------------------------------------------
namespace GtkTools {
static GList *pixmaps_directories = NULL;

//! Use this function to set the directory containing installed pixmaps.
void
addPixmapDirectory(const std::string &directory)
{
  pixmaps_directories = g_list_prepend (pixmaps_directories,
                                        g_strdup (directory.c_str()));
}

//! This is an internally used function to find pixmap files.
static gchar*
findPixmapFile(const gchar *filename)
{
  GList *elem;

  /* We step through each of the pixmaps directory to find it. */
  elem = pixmaps_directories;
  while (elem)
    {
      gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data,
                                         G_DIR_SEPARATOR_S, filename);
      if (g_file_test (pathname, G_FILE_TEST_EXISTS))
        return pathname;
      g_free (pathname);
      elem = elem->next;
    }
  return NULL;
}

//------------------------------ Pixmaps -------------------------------------
// This is used to create the pixmaps used in the interface.
GtkWidget*
createPixmap(const string &filename)
{
  gchar *pathname = NULL;
  GtkWidget *pixmap;

  if (filename=="")
      return gtk_image_new ();

  pathname = findPixmapFile(filename.c_str());

  if (!pathname)
  {
    g_warning ("Couldn't find pixmap file: %s", filename.c_str());
    return gtk_image_new ();
  }

  pixmap = gtk_image_new_from_file (pathname);
  g_free (pathname);
  return pixmap;
}

// This is used to create the pixmaps used in the interface from data.
GtkWidget*
createPixmapFromData(const char **data)
{
  GtkWidget *pixmap;
  GdkPixbuf*  pixbuf;

  if (!data) return gtk_image_new ();

  pixbuf=gdk_pixbuf_new_from_xpm_data(data);
  if (!pixbuf)
  {
    fprintf (stderr, "Failed to make pixbuf\n");
  }
  pixmap = gtk_image_new_from_pixbuf (pixbuf);
  return pixmap;
}


GtkWidget*
createPixmapFromPixbuf(GdkPixbuf* pixbuf)
{
  if (pixbuf)  return gtk_image_new_from_pixbuf (pixbuf);
  else return gtk_image_new();
}

//------------------------------ Pixbufs -------------------------------------
// This is used to create the pixbufs used in the interface.
GdkPixbuf*
createPixbuf(const string &filename)
{
  gchar *pathname = NULL;
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  if (filename=="")
      return NULL;

  pathname = findPixmapFile(filename.c_str());

  if (!pathname)
  {
    g_warning ("Couldn't find pixmap file: %s", filename.c_str());
    return NULL;
  }

  pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
  if (!pixbuf)
    {
      fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
               pathname, error->message);
      g_error_free (error);
    }
  g_free (pathname);
  return pixbuf;
}

// This is used to create the pixbufs used in the interface from data.
GdkPixbuf*
createPixbufFromData(const char **data)
{
  GdkPixbuf*  pixbuf;

  if (!data) return NULL;
  pixbuf=gdk_pixbuf_new_from_xpm_data(data);
  return pixbuf;
}

//------------------------------ Colors -------------------------------------
// Convert a string eg : #00FF00 to GdkColor (RGB = 0, 255, 0).
bool
strToGdkColor(const std::string &strColor, GdkColor &color)
{
  unsigned uColor;

  if (strColor.length()>1 && strColor[0]=='#') // Codage des couleurs type HTML
        uColor=strtoul(strColor.c_str()+1,NULL,16); // Converti une chaine type 0xRRGGBB en unsigned.
  else
        return false;

  color.red=(((unsigned)uColor)&0xFF0000)>>8;
  color.green=((unsigned)uColor)&0x00FF00;
  color.blue=(((unsigned)uColor)&0x0000FF)<<8;
  return true;
}

// Convert a GtkColor to a string  eg : GdkColor (RGB = 0, 255, 0) to #00FF00.
std::string
gdkColorToStr(GdkColor &color)
{
  return std::string("#")+
          ucharToHexaStr(color.red>>8)+
          ucharToHexaStr(color.green>>8)+
          ucharToHexaStr(color.blue>>8);
}

} // end of namespace GtkTools
//---End-Of-File---------------------------------------------------------------
