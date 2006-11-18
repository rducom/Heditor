//----------------------------------------------------------------------------
// @(#) ENIB/LI2 (c) - VET : Virtual Environement Training
//----------------------------------------------------------------------------
// package     : GtkTools
// file        : gtkToolsDialogs.cpp
// description : Tools for dialogs box (Msg/File/Color)...
// author(s)   : CAZEAUX Eric
// mail        : cazeaux@enib.fr
//----------------------------------------------------------------------------
// last update : 28/01/2004
//----------------------------------------------------------------------------

#include "AReVi/scheduler.h"
#include "gtkToolsDialogs.h"
#include "gtkToolsImages.h"
using namespace std;

//----------------------------------------------------------------------------
// GtkTools : Dialogs
//----------------------------------------------------------------------------
namespace GtkTools {

// This returns the toplevel window/dialog/menu that contains the given
//  widget. It even walks up menus, which gtk_widget_get_toplevel() does not.
GtkWidget*
getToplevel(GtkWidget *widget)
{
  GtkWidget *parent;

  if (widget == NULL)
    return NULL;

  for (;;)
  {
    if (GTK_IS_MENU (widget))
      parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
    else
      parent = widget->parent;
    if (parent == NULL)
      break;
    widget = parent;
  }

  return widget;
}

// Suspend AReVi Scheduler / run modal box / unsuspend and return result
int
runModalBox(GtkWidget *dialog)
{
  AReVi::Scheduler::access()->setSuspended(true);
  int result=gtk_dialog_run (GTK_DIALOG (dialog));
  AReVi::Scheduler::access()->setSuspended(false);
  return result;
}
//----------------------------- Messages -------------------------------------
// This shows a simple dialog box with a label and an 'OK' button.
void
showMessageBox(const std::string &message,
                GtkMessageType type/*=GTK_MESSAGE_ERROR*/,
                GtkWidget *transient_widget/*=NULL*/)
{
  GtkWidget* dialog;
  GtkWindow* transientParent = NULL;

  // Make transient if necessary
  if(transient_widget) {
    GtkWidget* toplevel = getToplevel(transient_widget);
    if(toplevel                   && 
       GTK_IS_WINDOW(toplevel)    && 
       GTK_WIDGET_MAPPED(toplevel))
      transientParent = GTK_WINDOW(toplevel);
  }

  dialog = gtk_message_dialog_new(transientParent,
				  GTK_DIALOG_MODAL,
				  type,
				  GTK_BUTTONS_OK,
				  "%s", message.c_str());

  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  runModalBox(dialog);
  gtk_widget_destroy(dialog);
}

// This shows a question dialog box response can be (YES/NO/CANCEL)
gint
showAskingBox(const std::string& message,
	      GtkMessageType type,
              const GtkButtonsType buttonsType,
              GtkWidget* transient_widget)
{
  GtkWidget *dialog;
  GtkWindow *transient_parent(NULL);

  // Make transient if necessary
  if (transient_widget)
  {
    GtkWidget *toplevel = getToplevel(transient_widget);
    if (toplevel && GTK_IS_WINDOW (toplevel) && GTK_WIDGET_MAPPED (toplevel))
        transient_parent=GTK_WINDOW(toplevel);
  }

  dialog = gtk_message_dialog_new (transient_parent,
				   GTK_DIALOG_MODAL,
				   type,
				   buttonsType,
				   "%s", message.c_str());
  //gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_position(GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  // Suspend AReVi Scheduler
  gint result=runModalBox(dialog);
  gtk_widget_destroy (dialog);

  return result;
}

void showCustomBox(const std::string &title,
                   const std::string &message,
                   GdkPixbuf         *titlePixbuf/*=NULL*/,
                   GdkPixbuf         *messagePixbuf/*=NULL*/,
                   GtkWidget         *transient_widget/*=NULL*/)
{
  GtkWidget *dialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *okbutton1;

  dialog = gtk_dialog_new ();
  gtk_widget_set_size_request (dialog, -1, 120);
  gtk_window_set_title (GTK_WINDOW (dialog), title.c_str());
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

  // Make transient if necessary
  if (transient_widget)
  {
    GtkWidget *toplevel = getToplevel(transient_widget);
    if (toplevel && GTK_IS_WINDOW (toplevel) && GTK_WIDGET_MAPPED (toplevel))
        gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                      GTK_WINDOW (toplevel));
  }


  if (titlePixbuf)
    gtk_window_set_icon (GTK_WINDOW (dialog), titlePixbuf);

  dialog_vbox1 = GTK_DIALOG (dialog)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  image1 = createPixmapFromPixbuf (messagePixbuf);
  gtk_widget_set_size_request (image1 , 70, 64);
  gtk_widget_show (image1);
  gtk_box_pack_start (GTK_BOX (hbox1), image1, FALSE, TRUE, 0);

  label1 = gtk_label_new (message.c_str());
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label1, -1, 23);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);

  dialog_action_area1 = GTK_DIALOG (dialog)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  // Suspend AReVi Scheduler
  runModalBox(dialog);
  gtk_widget_destroy (dialog);
}


//------------------------- Color Selection -----------------------------------
// Open Dlg for color selection return true, if color changed
bool
showColorBox(const std::string &title,
             GdkColor &colorInOut,
             GtkWidget *transient_widget/*=NULL*/)
{
  GtkWidget *dialog;
  GtkColorSelection *colorsel;
  gint response;

  dialog = gtk_color_selection_dialog_new (title.c_str());

  // Make transient if necessary
  if (transient_widget)
  {
    GtkWidget *toplevel = getToplevel(transient_widget);
    if (toplevel && GTK_IS_WINDOW (toplevel) && GTK_WIDGET_MAPPED (toplevel))
        gtk_window_set_transient_for (GTK_WINDOW (dialog),
                                      GTK_WINDOW (toplevel));
  }

  colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);

  gtk_color_selection_set_previous_color (colorsel, &colorInOut);
  gtk_color_selection_set_current_color (colorsel, &colorInOut);
  gtk_color_selection_set_has_palette (colorsel, TRUE);

  response = runModalBox(dialog);

  if (response == GTK_RESPONSE_OK)
  {
    gtk_color_selection_get_current_color (colorsel,&colorInOut);
  }
  gtk_widget_destroy (dialog);
  return (response == GTK_RESPONSE_OK);
}

//-------------------------- File Selection -----------------------------------
/* Converts a filename from UTF-8 to on-disk encoding, and sets it in a
   GtkFileSelection. */
void
util_set_file_selection_filename (GtkWidget *filesel,
					const gchar *filename_utf8)
{
  gchar *on_disk_filename;

  on_disk_filename = g_filename_from_utf8 (filename_utf8, -1, NULL, NULL,
					   NULL);
  gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel),
				   on_disk_filename);
  g_free (on_disk_filename);
}


// Open Dlg for file selection
bool
showSelectFileBox(const std::string &title,
                  std::string &selectedFile,
                  const bool &fileOptions/*=true*/)
{
  // Create the selector
  GtkWidget *file_selector = gtk_file_selection_new(title.c_str());

  // Set current filename
  if (selectedFile!="")
  {
    // Converts a filename from UTF-8 to on-disk encoding, and sets it in the
    // GtkFileSelection.
    gchar *on_disk_filename;

    on_disk_filename = g_filename_from_utf8(selectedFile.c_str(), -1, NULL, NULL,
              NULL);

    gtk_file_selection_set_filename (GTK_FILE_SELECTION (file_selector),
            on_disk_filename);
    g_free(on_disk_filename);
  }

  if (fileOptions)
    gtk_file_selection_show_fileop_buttons(GTK_FILE_SELECTION(file_selector));
  else
    gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(file_selector));

  // Display that dialog
  gtk_widget_show (file_selector);

  gint result=runModalBox(file_selector);

  if (result==GTK_RESPONSE_OK)
  {
  // Retrieve selected file
    const gchar *on_disk_filename;
    gchar *filename;

    on_disk_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
    filename = g_filename_to_utf8 (on_disk_filename, -1, NULL, NULL, NULL);

    if (filename)
    {
      selectedFile=filename;
      g_free(filename);
    }

    gtk_widget_destroy(file_selector);
    return true;
  }
  else
  {
    gtk_widget_destroy(file_selector);
    return false;
  }
}
/*
gint
showOpenFileChooser(const std::string& title,
		    const std::vector<std::pair<std::string, std::string> >& filters,
		    std::vector<std::string>& selectedFiles,
		    bool selectMultiple) {
  // Create the file chooser
  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new(title.c_str(),
				       NULL,
				       GTK_FILE_CHOOSER_ACTION_OPEN,
				       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       NULL);
  // Multiple selection
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), 
				       selectMultiple);

  // Set the filters
  std::vector<GtkFileFilter*> f(filters.size() + 1);

  f[filters.size()] = gtk_file_filter_new();
  gtk_file_filter_set_name(f[filters.size()], "Tous types");
  gtk_file_filter_add_pattern(f[filters.size()], "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), 
			      f[filters.size()]);

  for(size_t i = 0; i < filters.size(); i++) {
    f[i] = gtk_file_filter_new();
    gtk_file_filter_set_name(f[i], filters[i].first.c_str());
    gtk_file_filter_add_pattern(f[i], filters[i].second.c_str());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f[i]);
    if(i == 0) {
      gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), f[i]);
    }
  }

  // Set the current filename
  if(!selectedFiles.empty()) {
    if(selectedFiles[0] != "") {
      gchar* on_disk_filename;
      on_disk_filename = g_filename_from_utf8(selectedFiles[0].c_str(), 
					      -1, 
					      NULL, 
					      NULL,
					      NULL);
      gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), on_disk_filename);
      g_free(on_disk_filename);
    }
  }

  // Run the dialog box
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  
  // Clean
  selectedFiles.clear();
  GSList* sel = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
  for(size_t i = 0; i < g_slist_length(sel); i++) {
    gchar* file = (gchar*)g_slist_nth_data(sel, i);
    selectedFiles.push_back(std::string(file));
    g_free(file);
  }
  g_slist_free(sel);
  gtk_widget_destroy(dialog);

  return result;
}

gint
showSaveFileChooser(const std::string& title,
   		    const std::vector<std::pair<std::string, std::string> >& filters,
		    std::string& selectedFile) {
  // Create the file chooser
  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new(title.c_str(),
				       NULL,
				       GTK_FILE_CHOOSER_ACTION_SAVE,
				       GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				       NULL);

  // Set the filters
  std::vector<GtkFileFilter*> f(filters.size() + 1);

  f[filters.size()] = gtk_file_filter_new();
  gtk_file_filter_set_name(f[filters.size()], "Tous types");
  gtk_file_filter_add_pattern(f[filters.size()], "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), 
			      f[filters.size()]);
  
  for(size_t i = 0; i < filters.size(); i++) {
    gtk_file_filter_set_name(f[i], filters[i].first.c_str());
    gtk_file_filter_add_pattern(f[i], filters[i].second.c_str());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f[i]);
    if(i == 0) {
      gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), f[i]);
    }
  }

  // Set the current filename
  if(selectedFile != "") {
    gchar* on_disk_filename;
    on_disk_filename = g_filename_from_utf8(selectedFile.c_str(), 
					    -1, 
					    NULL, 
					    NULL,
					    NULL);
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), on_disk_filename);
    g_free(on_disk_filename);
  }

  // Run the dialog box
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));

  // Clean
  gchar* file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
  selectedFile = std::string(file);
  g_free(file);
  for(size_t i = f.size(); i--;) {
    g_free(f[i]);
  }
  gtk_widget_destroy(dialog);

  return result;
}
*/
} // end of namespace GtkTools
//---End-Of-File---------------------------------------------------------------
