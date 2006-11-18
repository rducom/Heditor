//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkToolsDialogs.h
// description : Tools for dialogs box (Msg/File/Color)...
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------
#ifndef GTKTOOLS_DIALOGS_H
#define GTKTOOLS_DIALOGS_H 1

#include <gtk/gtk.h>
#include <string>
#include <vector>

//----------------------------------------------------------------------------
// GtkTools : Dialogs
//----------------------------------------------------------------------------
namespace GtkTools {

//----------------------------  Widgets  -------------------------------------
//! This returns the toplevel window/dialog/menu that contains the given
//!  widget. It even walks up menus, which gtk_widget_get_toplevel() does not.
GtkWidget*
getToplevel(GtkWidget *widget);

//--------------------------- Modals Dialogs ----------------------------------
//! Suspend AReVi Scheduler / run modal box / unsuspend
//! return result of gtk_dialog_run(dialog);
int
runModalBox(GtkWidget *dialog);

//----------------------------- Messages -------------------------------------
//! This shows a simple dialog box with a label and an 'OK' button.
//! Example usage:
//!   showMessageBox("Error saving file");
//!   NB : Message must be UTF-8 encoded
//!   type must be GTK_MESSAGE_INFO, GTK_MESSAGE_WARNING,
//!                GTK_MESSAGE_QUESTION, GTK_MESSAGE_ERROR
void
showMessageBox(const std::string &message,
                GtkMessageType type=GTK_MESSAGE_ERROR,
                GtkWidget *transient_widget=NULL);

//! This shows a question dialog box response can be (YES/NO/CANCEL)
//!    => -1  GTK_RESPONSE_NONE
//!    => GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, GTK_RESPONSE_CLOSE,
//!    => GTK_RESPONSE_YES, GTK_RESPONSE_NO ...
//!   NB : Message must be UTF-8 encoded
//!
//!   Example usage:
//!    response= showAskingBox("Really Quit", GTK_BUTTONS_YES_NO);
gint
showAskingBox(const std::string& message,
	      GtkMessageType type = GTK_MESSAGE_ERROR,
	      GtkButtonsType buttonsType = GTK_BUTTONS_OK,
	      GtkWidget* transientWidget = NULL);

//! This shows a cutomizable message dialog box
//!   NB : Message and title must both be UTF-8 encoded
//!   NB : titlePixbuf & messagePixbuf must be managed by user.
//!
//!   Example usage:
//!    showCustomBox("Saving file advice", "Can't allocate enough disk space");
void showCustomBox(const std::string &title,
                   const std::string &message,
                   GdkPixbuf         *titlePixbuf=NULL,
                   GdkPixbuf         *messagePixbuf=NULL,
                   GtkWidget         *transient_widget=NULL);

//------------------------- Color Selection -----------------------------------
//! Open Dlg for color selection return true, if color changed
//!   NB : title must be UTF-8 encoded
bool
showColorBox(const std::string &title,
              GdkColor &colorInOut,
              GtkWidget *transient_widget=NULL);

//-------------------------- File Selection -----------------------------------
//! Open Dlg for file selection
//! Title and selectedFile are UTF8 encoded
//! selectedFile is an in/out attribut (start select position)
//! if fileOption is true, this allow user to rename/delete files...
bool
showSelectFileBox(const std::string &title,
                  std::string &selectedFile,
                  const bool &fileOptions=true);

/*
gint
showOpenFileChooser(const std::string& title,
		    const std::vector<std::pair<std::string, std::string> >& filters,
		    std::vector<std::string>& selectedFiles,
		    bool selectMultiple = false);

gint
showSaveFileChooser(const std::string& title,
   		    const std::vector<std::pair<std::string, std::string> >& filters,
		    std::string& selectedFile);
*/
} // end of namespace GtkTools
#endif // end of GTKTOOLS_DIALOGS_H
//---End-Of-File---------------------------------------------------------------
