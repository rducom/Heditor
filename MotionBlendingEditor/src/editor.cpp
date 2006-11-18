#include <set>

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/boundingBox3D.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Lib3D/simpleInteractor.h"
#include "AReVi/Utils/fileStream.h"

#include "hLib/core/joint.h"
//#include "hLib/core/anchor.h"

#include "gtkToolsDialogs.h"
#include "editor.h"

AR_CLASS_DEF(Editor, GtkAReViWidget)

Editor::Editor(ArCW& arCW) : 
  GtkAReViWidget(arCW, "./data/ihm/main.glade", "main"),
  _view(),
  _plugger(),
  _canvasCtrl(),
  _interactor(),
  //  _bodyObj(new_AnimBody()),
  _bodyObj(AnimBody::NEW()),
  _current(),
  _anims(),
  _bodys(),
  _blendCycles() {
	
    // Setup Canvas Widget
    GtkWidget* canvas = accessWidget("canvas1");
    gtk_widget_realize(canvas);
    gtk_widget_show(canvas);
    //    _canvasCtrl = new_CanvasController(canvas);
    _canvasCtrl = CanvasController::NEW(canvas);
		
    // Setup AReVi viewport Widget
    GtkWidget* RenderBox = accessWidget("windowBox");
    
    //  _plugger = new_GtkWindowPlugger();
    _plugger = GtkWindowPlugger::NEW();
  gtk_widget_show(_plugger->getWidget());
  gtk_box_pack_start(GTK_BOX(RenderBox),_plugger->getWidget(),true,true, 1);
  gtk_box_reorder_child(GTK_BOX(RenderBox), _plugger->getWidget(), 0);
  
  //  _view = new_EmbeddedWindow3D(_plugger->getWidgetID());
  _view = EmbeddedWindow3D::NEW(_plugger->getWidgetID());
  _view->setOriginVisible(true);
  _plugger->setWindow(_view);

  gtk_widget_show(_plugger->getWidget());
		
  //  _interactor = new_CameraInteractor();
  _interactor = CameraInteractor::NEW();
  _interactor->setWindow(_view);


  // GTK signals connect
  _connectSignals();
	

  // Setup animation-list treeview
  GtkCellRenderer* animCelRenderer;
  GtkTreeViewColumn* animCol;
  GtkTreeView* animTreeView = GTK_TREE_VIEW(accessWidget("anim_treeview"));
  GtkListStore* animModel = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
  gtk_tree_view_set_model(animTreeView, GTK_TREE_MODEL(animModel));
  
  animCelRenderer = gtk_cell_renderer_text_new();
  animCol = gtk_tree_view_column_new_with_attributes("Animation", animCelRenderer, "text", COLUMN_ANIM_NAME, NULL);
  gtk_tree_view_column_set_sort_column_id(animCol, COLUMN_ANIM_NAME);
  gtk_tree_view_append_column(animTreeView, animCol);
  GtkTreeSelection* animSelection = gtk_tree_view_get_selection(animTreeView);
	
  g_signal_connect(animSelection, "changed", G_CALLBACK(&Editor::_onSelectAnim), this);
  
	
  //Setup Drag and drop for treeview
  gtk_widget_realize(GTK_WIDGET(animTreeView));
  gtk_widget_realize(canvas);
	
  if(!GTK_WIDGET_NO_WINDOW(GTK_WIDGET(animTreeView)) && !GTK_WIDGET_NO_WINDOW(canvas)) {	
    enum {
      TARGET_MBEanim,
      TARGET_STRING
    };
		
    GtkTargetEntry target_entry[3] = { {"canvas",  GTK_TARGET_SAME_APP, TARGET_MBEanim},
				       {"text/plain", GTK_TARGET_SAME_APP, TARGET_STRING },
				       {"text/uri-list", GTK_TARGET_SAME_APP, TARGET_STRING } };

    gtk_drag_source_set(GTK_WIDGET(animTreeView),GDK_BUTTON1_MASK,
			target_entry,
			sizeof(target_entry) / sizeof(GtkTargetEntry),
			GDK_ACTION_COPY);

    g_signal_connect(GTK_OBJECT(animTreeView), "drag_begin",G_CALLBACK(Editor::DNDBeginCB), this);
    g_signal_connect(GTK_OBJECT(animTreeView), "drag_end",G_CALLBACK (Editor::DNDEndCB), this);
  }

	
	
  //  _undoRedo = new_UndoRedo();   ===>>> TO IMPLEMENT

  _bodyObj->setAlias("body");

  // Extract supported extensions
  ArRef<HLibLoader> loader = HLibLoader::access();
  loader->addDirectory("./plugins");
  loader->refresh();
  _extensions = loader->getExtensions();


  _setBody(Body::nullRef(), "none");

  gtk_widget_set_sensitive(accessWidget("button_begin"), false);
  gtk_widget_set_sensitive(accessWidget("button_play"), false);
  gtk_widget_set_sensitive(accessWidget("button_pause"), false);
  gtk_widget_set_sensitive(accessWidget("button_stop"), false);
  gtk_widget_set_sensitive(accessWidget("button_end"), false);
  gtk_widget_set_sensitive(accessWidget("button_begin"), false);
  gtk_widget_set_sensitive(accessWidget("time_line"), false);
  gtk_widget_set_sensitive(accessWidget("scale_line"), false);
  gtk_widget_set_sensitive(accessWidget("button_undo"), false);
  gtk_widget_set_sensitive(accessWidget("button_redo"), false);

  GtkRange* sRange = GTK_RANGE(accessWidget("scale_line"));
  gtk_range_set_range(sRange, 0.0, 100.0);

  _bodyObj->addTimeCB(thisRef(), &Editor::_onTimeCB);
  _canvasCtrl->addSetanimCB(thisRef(), &Editor::_onSetanimCB );

}

Editor::~Editor(void) {
}


//-------------------------------------------------------------------
// CONNECT SIGNALS
//-------------------------------------------------------------------
void
Editor::_connectSignals(void) {

  //Connect signals for menubar
  g_signal_connect(accessWidget("file_quitMI"), "activate", G_CALLBACK(&Editor::_onFileQuitMI), this);
  g_signal_connect(accessWidget("file_cutMI"), "activate", G_CALLBACK(&Editor::_onFileCutMI), this);
  g_signal_connect(accessWidget("file_copyMI"), "activate", G_CALLBACK(&Editor::_onFileCopyMI), this);
  g_signal_connect(accessWidget("file_pasteMI"), "activate", G_CALLBACK(&Editor::_onFilePasteMI), this);
  g_signal_connect(accessWidget("file_helpMI"), "activate", G_CALLBACK(&Editor::_onFileHelpMI), this);

 // Toolbar
  g_signal_connect(accessWidget("file_new"), "clicked", G_CALLBACK(&Editor::_onFileNew), this);
  g_signal_connect(accessWidget("file_open"), "clicked", G_CALLBACK(&Editor::_onFileOpen), this);
  g_signal_connect(accessWidget("file_export"), "clicked", G_CALLBACK(&Editor::_onFileExport), this);
  g_signal_connect(accessWidget("file_save"), "clicked", G_CALLBACK(&Editor::_onFileSave), this);
  g_signal_connect(accessWidget("file_saveas"), "clicked", G_CALLBACK(&Editor::_onFileSaveAs), this);
  g_signal_connect(accessWidget("button_undo"), "clicked", G_CALLBACK(&Editor::_onButtonUndo), this);
  g_signal_connect(accessWidget("button_redo"), "clicked", G_CALLBACK(&Editor::_onButtonRedo), this);
  g_signal_connect(accessWidget("button_link"), "clicked", G_CALLBACK(&Editor::_onButtonLink), this);
  g_signal_connect(accessWidget("button_unlink"), "clicked", G_CALLBACK(&Editor::_onButtonUnlink), this);  
  g_signal_connect(accessWidget("button_delete"), "clicked", G_CALLBACK(&Editor::_onButtonDelete), this);  
  g_signal_connect(accessWidget("file_help"), "clicked", G_CALLBACK(&Editor::_onFileHelp), this);
  
  // Treeview bar
  g_signal_connect(accessWidget("file_import"), "clicked", G_CALLBACK(&Editor::_onFileImport), this);  
  g_signal_connect(accessWidget("file_importFolder"), "clicked", G_CALLBACK(&Editor::_onFileImportFolder), this);
  g_signal_connect(accessWidget("_selectBody"), "clicked", G_CALLBACK(&Editor::_onFileSelectBody), this);
  
  // TimeLine bar
  g_signal_connect(accessWidget("button_begin"), "clicked", G_CALLBACK(&Editor::_onButtonBegin), this);  
  g_signal_connect(accessWidget("button_play"), "clicked", G_CALLBACK(&Editor::_onButtonPlay), this);    
  g_signal_connect(accessWidget("button_pause"), "clicked", G_CALLBACK(&Editor::_onButtonPause), this);  
  g_signal_connect(accessWidget("button_stop"), "clicked", G_CALLBACK(&Editor::_onButtonStop), this);
  g_signal_connect(accessWidget("button_end"), "clicked", G_CALLBACK(&Editor::_onButtonEnd), this);
  g_signal_connect(accessWidget("time_line"), "value_changed", G_CALLBACK(&Editor::_onTimeLine), this);
  g_signal_connect(accessWidget("scale_line"), "value_changed", G_CALLBACK(&Editor::_onScaleLine), this);
  
  
  // Options d'affichages toolbar
  /*
  g_signal_connect(accessWidget("display_wireframe"), "toggled", G_CALLBACK(&Editor::_onDisplayWireframe), this);
  g_signal_connect(accessWidget("display_mesh"), "toggled", G_CALLBACK(&Editor::_onDisplayMesh), this);
  */
}


//-------------------------------------------------------------------
// CALLBACKS
//-------------------------------------------------------------------

/*
  Menubar :  (MI : Menu Item)
*/

void
Editor::_onFileQuitMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  if(!e->_blendCycles.empty()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  ArSystem::leaveSimulation();
}

void
Editor::_onFileCutMI(GtkImageMenuItem* /*item*/, gpointer /*data*/) {
	
	
  //A implementer
}

void
Editor::_onFileCopyMI(GtkImageMenuItem* /*item*/, gpointer /*data*/) {
  //A implementer
}

void
Editor::_onFilePasteMI(GtkImageMenuItem* /*item*/, gpointer /*data*/) {
  //A implementer
}

void
Editor::_onFileHelpMI(GtkImageMenuItem* /*item*/, gpointer /*data*/) {
    showMessageBox("En developpement", GTK_MESSAGE_ERROR, NULL);
}

/*
  Toolbar Buttons
*/

void
Editor::_onFileNew(GtkToolButton* /*tb*/, gpointer data) {
  
  Editor* e = (Editor*)data;

  if(!e->_blendCycles.empty()) {
    if(!e->_askBeforeGoOn()) {
      return;
    } 
  }

  e->_setFilename(StlString());
  e->_blendCycles.clear();
  
}

void
Editor::_onFileOpen(GtkToolButton* /*tb*/, gpointer /*data*/) {
  
//   Editor* e = (Editor*)data;

//   if(!e->_blendCycles.empty()) {
//     if(!e->_askBeforeGoOn()) {
//       return;
//     }
//   }

//   GtkWidget* dialog;
//   dialog = gtk_file_chooser_dialog_new("Ouvrir un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, 
// 									GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

//   for(size_t i = e->_extensions.size(); i--;) {
//     GtkFileFilter* f = gtk_file_filter_new();
//     gtk_file_filter_set_name(f, e->_extensions[i].description.c_str());

//     for(size_t j = e->_extensions[i].extension.size(); j--;) {
//       StlString tmpPattern = "*" + e->_extensions[i].extension[j];
//       gtk_file_filter_add_pattern(f, tmpPattern.c_str());
//     }

//     gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
//   }

//   if(!e->_filename.empty()) {
//     gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), e->_filename.c_str());
//   }
  
//   if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
//     char *filename;
//     filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    
//     LoaderData d = HLibLoader::access()->loadFile(filename, false);
		
//     if(d.body) {
//       e->_setBody(d.body);
//     }

//     if( d.animations[0] ) {
			
//       GtkWidget* animDialog;
//       animDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, 
// 					  GTK_BUTTONS_YES_NO, "Ce modele contient une animation. La charger ?");
//       if(gtk_dialog_run(GTK_DIALOG (animDialog)) == GTK_RESPONSE_YES) {
				
// 	ArRef<AnimationCycle> anim = new_AnimationCycle();
// 	anim->setAnimationData(d.animations[0]);
// 	int n = strlen(filename);
// 	int nameIndex = 0;
				
// 	for( int i = 0 ; i < n ; i++ ) {
// 	  if( filename[i] == '/' ) { nameIndex = i+1; }
// 	}
// 	char* name = new char[n-nameIndex];
				
// 	for( int i = 0 ; i < n-nameIndex ; i++ ) {
// 	  name[i] = filename[nameIndex+i];
// 	}
// 	pair<StlString, ArRef<AnimationCycle> > p;
// 	p.first = name;
// 	p.second = anim;
// 	e->_anims.insert(p);
// 	e->_fillAnimTree();
//       }
//       gtk_widget_destroy(animDialog);
//     }
    
//     g_free(filename);
//   }

//   gtk_widget_destroy(dialog);
  
}

void
Editor::_onFileExport(GtkToolButton* /*tb*/, gpointer /*data*/) { // To implement
}

void
Editor::_onFileSave(GtkToolButton* item, gpointer data) { // To implement
  
  Editor* e = (Editor*)data;

  if(!e->_filename.empty()) {
    e->_saveAnimation(e->_filename);
  } else {
    _onFileSaveAs(item, data);
  }
  //e->_undoRedo->emptyStacks();
}

void
Editor::_onFileSaveAs(GtkToolButton* /*tb*/, gpointer data) { // To implement
  
  Editor* e = (Editor*)data;

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Enregistrer sous", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  GtkFileFilter* f = gtk_file_filter_new();
  gtk_file_filter_set_name(f, "Animation Cycle");
  gtk_file_filter_add_pattern(f, "*.acy");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_filename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), e->_filename.c_str());
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    StlString filenameWithExtension(filename);

    cerr << strExtension(filenameWithExtension) << endl;

    if(strExtension(filenameWithExtension) == "") {
      filenameWithExtension += ".hlh";
    }

    e->_saveAnimation(filenameWithExtension);   
    e->_setFilename(filenameWithExtension);
    g_free(filename);
  }

  gtk_widget_destroy (dialog);
  //e->_undoRedo->emptyStacks();  
}

void
Editor::_onButtonUndo(GtkToolButton* /*tb*/, gpointer /*data*/) { 
	// To implement
}

void
Editor::_onButtonRedo(GtkToolButton* /*tb*/, gpointer /*data*/) { 
	// To implement
}

void
Editor::_onButtonLink(GtkToolButton* /*tb*/, gpointer data) { 
  Editor* e = (Editor*)data;
  
  ItemList* list = e->_canvasCtrl->accessSelected();
  
  cout << "Link :::  size of selected=" << list->size() << endl;
  
  if( list->size() == 2 )
  {
    ArRef<Item> item1 = list->front();
    ArRef<Item> item2 = list->back();
    ArRef<Item> blend = e->_canvasCtrl->makeBlend( item1, item2);
  }
  else
  {
    cout << "Attention, dialogue va s'ouvrir !!!" << endl;
      showMessageBox("Vous ne pouvez Linker que 2 animations à la fois !!! \n Faites vos selections avec Ctrl ou Alt", GTK_MESSAGE_WARNING, NULL);
  }
}

void
Editor::_onButtonUnlink(GtkToolButton* /*tb*/, gpointer data) {
  Editor* e = (Editor*)data;
  
  ItemList* list = e->_canvasCtrl->accessSelected();
	
  if( list->size() == 2 ) {
    e->_canvasCtrl->removeBlend( list->front(), list->back() );
  } else if( list->size() == 1 ) {
    list->front()->disconnect();
  } else if( list->size() != 0 ) {
    for( ItemList::iterator i = list->begin(); i != list->end(); ++i){
      (*i)->disconnect();
    }
  }
}

void
Editor::_onButtonDelete(GtkToolButton* /*tb*/, gpointer data) {
	
  Editor* e = (Editor*)data;
  ItemList* list = e->_canvasCtrl->accessSelected();

  if( list->size() != 0){
    ItemList::iterator i;
    while( list->size() > 0 ){
      i = list->begin();
      (*i)->set_item_selected(false);
      e->_canvasCtrl->removeItem(*i);
      i = list->erase(i);
    }
  }
}

void
Editor::_onFileHelp(GtkToolButton* /*tb*/, gpointer /*data*/) {
  showMessageBox("Chargez des animations avec Import, et faites les glisser sur le canvas, \n Selections multiples avec Ctrl ou Maj\n Pour Créer un Blend d'animation, faites Link sur deux animations. ", GTK_MESSAGE_ERROR, NULL);
}


/*
  Treeview buttons
*/

void
Editor::_onFileImportFolder(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Importer les fichiers d'un dossier", NULL, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

//   GtkFileFilter* f = gtk_file_filter_new();
//   gtk_file_filter_set_name(f, "Biovision Motion Capture");
//   gtk_file_filter_add_pattern(f, "*.bvh");
//   gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_importFilename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), e->_importFilename.c_str());
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *foldername;
    foldername = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    GtkFileChooser * filechooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_folder(filechooser, foldername);
    char* tmp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
    cerr << "dossier courant : " << tmp << endl;
    gtk_file_chooser_set_action(filechooser, GTK_FILE_CHOOSER_ACTION_OPEN);
    cerr << "0" << endl;
    gtk_file_chooser_set_select_multiple(filechooser, true);
    cerr << "1" << endl;
    gtk_file_chooser_unselect_all(filechooser);
    cerr << "2" << endl;
    gtk_file_chooser_select_all(filechooser);
    cerr << "3" << endl;
    GSList * animList = gtk_file_chooser_get_filenames(filechooser);

    cerr << foldername << "->" <<  g_slist_length(animList) << " elements" << endl;

    for(unsigned int i = 0 ; i < g_slist_length(animList) ; i++) {
//     for(unsigned int i = 0 ; g_slist_nth(animList,i) != NULL ; i++) {
      char* filename;
//       (gpointer)filename = g_slist_nth_data (animList, i);
      filename = (char*)g_slist_nth_data (animList, i);
      cerr << filename << endl;
      LoaderData d = HLibLoader::access()->loadFile(filename, true);
      if(d.body) {
	//	ArRef<AnimationCycle> anim = new_AnimationCycle();
	ArRef<AnimationCycle> anim = AnimationCycle::NEW();
	anim->setAnimationData(d.animations[0]);
	int n = strlen(filename);
	int nameIndex = 0;
	for( int i = 0 ; i < n ; i++ ) {
	  if( filename[i] == '/' ) { nameIndex = i+1; }
	}
	char* name = new char[n-nameIndex];
	for( int i = 0 ; i < n-nameIndex ; i++ ) {
	  name[i] = filename[nameIndex+i];
	}
	pair<StlString, ArRef<AnimationCycle> > p;
	p.first = name;
	p.second = anim;
	e->_anims.insert(p);
      }
      g_free(filename);
    }

    g_slist_free(animList);
    g_free(foldername);
    e->_fillAnimTree();

  }

  gtk_widget_destroy (dialog);

}

void
Editor::_onFileImport(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Importer un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN,
                                       GTK_RESPONSE_ACCEPT, NULL);
  
  for(size_t i = e->_extensions.size(); i--;) {
    
    GtkFileFilter* f = gtk_file_filter_new();
    gtk_file_filter_set_name(f, e->_extensions[i].description.c_str());
    
    for(size_t j = e->_extensions[i].extension.size(); j--;) {
      StlString tmpPattern = "*" + e->_extensions[i].extension[j];
      gtk_file_filter_add_pattern(f, tmpPattern.c_str());
    }
    
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);
  }

  if(!e->_importFilename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), e->_importFilename.c_str());
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    LoaderData d = HLibLoader::access()->loadFile(filename, true);
    if(!d.animations.empty()) {
      
      //      ArRef<AnimationCycle> anim = new_AnimationCycle();
      ArRef<AnimationCycle> anim = AnimationCycle::NEW();
      anim->setAnimationData(d.animations[0]);
      int n = strlen(filename);
      int nameIndex = 0;
      for( int i = 0 ; i < n ; i++ ) {
        if( filename[i] == '/' ) { nameIndex = i+1;	}
      }
      char* name = new char[n-nameIndex];
      for( int i = 0 ; i < n-nameIndex ; i++ ) {
        name[i] = filename[nameIndex+i];
      }
      
      gchar* name_non_utf8 =  gdk_utf8_to_string_target(name);
      string label(name_non_utf8);
      
      pair<string, ArRef<AnimationCycle> > p;
      p.first = label;
      p.second = anim;
      e->_anims.insert(p);
      
      if(d.body) {
	ArRef<Body> body = d.body;
	pair<StlString, ArRef<Body> > pb;
	pb.first = label;
	pb.second = body;
	if(e->_bodys.empty()) {
	  e->_setBody(body, name_non_utf8);
	}
	e->_bodys.insert(pb);
      }
      
    }
    g_free(filename);
    e->_fillAnimTree();
  }

  gtk_widget_destroy (dialog);

}


void
Editor::_onFileSelectBody(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;

  GtkTreeView* animTreeView = GTK_TREE_VIEW(e->accessWidget("anim_treeview"));
  GtkTreeSelection* selection = gtk_tree_view_get_selection(animTreeView);
  GtkTreeModel* model;
  GtkTreeIter iter;

  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    Body* body;
    gtk_tree_model_get(model, &iter, COLUMN_ANIM_BODY, &body, -1);
    char* name;
    gtk_tree_model_get(model, &iter, COLUMN_ANIM_NAME, &name, -1);

    if( body != NULL ) {
      e->_setBody( ArRef<Body>(body), name );
    }
  }
}


/*
  Timeline buttons
*/

void
Editor::_onButtonBegin(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;
  e->_bodyObj->restartAnimation();

}

void
Editor::_onButtonPlay(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;
  e->_bodyObj->playAnimation();
  gtk_widget_set_sensitive(e->accessWidget("button_begin"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_play"), false);
  gtk_widget_set_sensitive(e->accessWidget("button_pause"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_stop"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_end"), true);

}

void
Editor::_onButtonPause(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;
  e->_bodyObj->suspendAnimation();
  gtk_widget_set_sensitive(e->accessWidget("button_begin"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_play"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_pause"), false);
  gtk_widget_set_sensitive(e->accessWidget("button_stop"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_end"), true);

}

void
Editor::_onButtonStop(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;
  e->_bodyObj->stopAnimation();
  gtk_widget_set_sensitive(e->accessWidget("button_begin"), false);
  gtk_widget_set_sensitive(e->accessWidget("button_play"), true);
  gtk_widget_set_sensitive(e->accessWidget("button_pause"), false);
  gtk_widget_set_sensitive(e->accessWidget("button_stop"), false);
  gtk_widget_set_sensitive(e->accessWidget("button_end"), false);

}

void
Editor::_onButtonEnd(GtkToolButton* /*tb*/, gpointer data) {

  Editor* e = (Editor*)data;
  e->_bodyObj->restartAnimation();

}

void
Editor::_onTimeLine(GtkHScale* item, gpointer data) {

  Editor* e = (Editor*)data;
  GtkRange* tRange = GTK_RANGE(item);
  e->_bodyObj->setTimePosition(gtk_range_get_value(tRange));

}

void
Editor::_onScaleLine(GtkHScale* item, gpointer data) {

  Editor* e = (Editor*)data;
  GtkRange* tRange = GTK_RANGE(item);
  e->_bodyObj->setBlendWeight(gtk_range_get_value(tRange));
  e->accessCurrentItem()->set_weight(gtk_range_get_value(tRange));
}

// Anim tree view callbacks

#if 0
void
Editor::_onActivateAnim(GtkTreeView* /* view */, GtkTreePath* /* path */, GtkTreeViewColumn* /* arg2 */, gpointer /*data*/) {

}
#endif

void        
Editor::_onSelectAnim(GtkTreeSelection* selection, gpointer data) {
  
  Editor* e = (Editor*)data;
  
  GtkTreeModel* model;
  GtkTreeIter iter;
  double playDuration;
  
  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    AnimationCycle* aPtr;
    gtk_tree_model_get(model, &iter, COLUMN_ANIM_DATA, &aPtr, -1);
    e->_bodyObj->setAnimation(ArRef<AnimationCycle>(aPtr));
    
    playDuration = aPtr->getDuration();
    GtkRange* tRange = GTK_RANGE(e->accessWidget("time_line"));
    gtk_range_set_range(tRange, 0.0, playDuration);
    
    gtk_widget_set_sensitive(e->accessWidget("button_begin"), false);
    gtk_widget_set_sensitive(e->accessWidget("button_play"), true);
    gtk_widget_set_sensitive(e->accessWidget("button_pause"), false);
    gtk_widget_set_sensitive(e->accessWidget("button_stop"), false);
    gtk_widget_set_sensitive(e->accessWidget("button_end"), false);
    gtk_widget_set_sensitive(e->accessWidget("time_line"), true);
    gtk_widget_set_sensitive(e->accessWidget("scale_line"), false);

  }
}

void
Editor::setCurrentItem(ArRef<Item> item){
	_current = item;}

//-------------------------------------------------------------------
// Drag & Drop functions
//-------------------------------------------------------------------

void // Called to initialize dragging from treeview to canvas
Editor::DNDBeginCB(GtkWidget *widget, GdkDragContext* /*dc*/, gpointer data){
	
  Editor* e = (Editor*)data;
  GtkTreeModel* model;
  GtkTreeIter iter;
  
  gchar* txt= NULL;
  ArRef<AnimationCycle> anim = AnimationCycle::nullRef();
  GtkTreeSelection* selection = NULL;
  
  selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
  
  if( selection != NULL)
    {
      // We retreive file information and associated animation
      if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
	gtk_tree_model_get(model, &iter, COLUMN_ANIM_NAME, &txt, -1);
	gtk_tree_model_get(model, &iter, COLUMN_ANIM_DATA, &anim, -1);
      }
    
      if( (txt != NULL) && (anim != AnimationCycle::nullRef() ) )
	{
	  string label(txt);
	  // supplies animation name
	  e->_canvasCtrl->DraggedItem_New(label);
	  //supplies animation AnimationCycle
	  e->_canvasCtrl->accessDraggedItem()->setAnimation(anim);
	}
    }
}

void 
Editor::DNDEndCB(	GtkWidget* /*widget*/, GdkDragContext* /*dc*/, gpointer data){
  // Finalize Dragging by ungrabbing Item on the canvas 
  Editor* e = (Editor*)data;
  e->_canvasCtrl->DraggedItem_End();
}


//-------------------------------------------------------------------
// Utilities functions
//-------------------------------------------------------------------

bool 
Editor::_saveAnimation(StlString filename) {
  
  //  ArRef<FileOStream> output = new_FileOStream(filename, false);
  ArRef<FileOStream> output = FileOStream::NEW(filename, false);
  if(output->fail()) {
    showMessageBox("Erreur lors de la sauvegarde\n" + output->getErrorMessage(), GTK_MESSAGE_ERROR, NULL);
    return false;
  }

  ArRef<AnimationBlendCycle> anim;

  SerializationDependencies dep;
  anim->extractDependencies(dep, true);
  dep.addReference(anim);
  dep.writeToStream(output);

  const StlVector<ArRef<ArObject> >& refsOut = dep.getReferences();
  for(size_t i = 0; i < refsOut.size(); i++) {
    refsOut[i]->serialize(dep, output);
  }

  return true;
}

void
Editor::_setFilename(StlString filename) {
  
  _filename = filename;

}

bool
Editor::_askBeforeGoOn(void) {
  GtkWidget* dialog;
  dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "Les modifications seront perdues ! Continuer ?");
  bool result;
  if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_YES) {
    result = true;
  } else {
    result = false;
  }
  gtk_widget_destroy(dialog);
  return result;
}

void // Fill the animation gtk treeview with filenames
Editor::_fillAnimTree(void) {
  
  GtkTreeIter iter;
  GtkTreeView* animTreeView = GTK_TREE_VIEW(accessWidget("anim_treeview"));
  GtkListStore* animModel = GTK_LIST_STORE(gtk_tree_view_get_model(animTreeView));

  gtk_list_store_clear(animModel);

  for(StlMap<StlString, ArRef<AnimationCycle> >::iterator it = _anims.begin() ; it != _anims.end() ; it++) {
    StlMap<StlString, ArRef<Body> >::iterator tmp = _bodys.find(it->first);
    Body* body = NULL;
    if( tmp != _bodys.end() ) {
      body = tmp->second.c_ptr();
    }
    gtk_list_store_append(animModel, &iter);
    gtk_list_store_set(animModel, &iter, COLUMN_ANIM_NAME, it->first.c_str(), COLUMN_ANIM_DATA, it->second.c_ptr(), COLUMN_ANIM_BODY, body, -1);
  }

}

void // Set body as the structure on wich apply animation
Editor::_setBody(ArRef<Body> body, char* name) {

  _bodyObj->setShape(body);

  if(body.valid()) {

    //    ArRef<BoundingBox3D> bbox = new_BoundingBox3D();
    ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
    body->readBoundingBox(bbox);

    double x, y, z, sx, sy, sz;
    bbox->getCenter(x, y, z);
    bbox->getSize(sx, sy, sz);
		// Preset view position
    _view->setFarDistance(3.0 * bbox->getMaxSize());
    _view->setNearDistance(0.001 * _view->getFarDistance());

    _view->setPosition(x - 1.2*sy, y, z);

    body->setVisible(true);
		// Affec Body to the BodyAnim controller
    _interactor->setBody(body);

		// Update the Body information widget
    GtkTextView* text = GTK_TEXT_VIEW(accessWidget("_selectedBody"));
    GtkTextTagTable* table = gtk_text_tag_table_new();
    GtkTextBuffer* buffer = gtk_text_buffer_new(table);
    gtk_text_buffer_set_text( buffer, name, strlen(name) );
    gtk_text_view_set_buffer(text, buffer);

  }

}

void
Editor::_onTimeCB(const AnimBody::TimeEvent& evt) {

  GtkRange* tRange = GTK_RANGE(accessWidget("time_line"));
  gtk_range_set_value(tRange, evt.timeLine);

}

void // Selects an Item for it's viewing in AReVi window
Editor::_onSetanimCB(const CanvasController::SetanimEvent& evt){

  double playDuration;
	
  ArRef<Item> item = evt.item;
  _current = evt.item;
	
  if( item != Item::nullRef() ){
		
    gtk_widget_set_sensitive(accessWidget("button_begin"), false);
    gtk_widget_set_sensitive(accessWidget("button_play"), true);
    gtk_widget_set_sensitive(accessWidget("button_pause"), false);
    gtk_widget_set_sensitive(accessWidget("button_stop"), false);
    gtk_widget_set_sensitive(accessWidget("button_end"), false);
    gtk_widget_set_sensitive(accessWidget("time_line"), true);
	
    GtkRange* sRange = GTK_RANGE(accessWidget("scale_line"));
    GtkRange* tRange = GTK_RANGE(accessWidget("time_line"));   

    if( item->get_isAblend() ){
      // If selected Item is a blend, we refresh parents animations
			
      if( item->getAnimation()->getClass()->isA( AnimationBlendCycle::CLASS() ) ) {
	LinksList* list = item->accessItemParent();
	if( list->size() == 2 )
	  {
	    ArRef<Item> item1 = list->front()->parent_item();
	    ArRef<Item> item2 = list->back()->parent_item();
								
	    if(item1!=Item::nullRef() && item2!=Item::nullRef() ){
							
	      // We extract parent's blend's an we recompute them
	      ArRef<AnimationCycle> anim1 = item1->getAnimation();
	      ArRef<AnimationCycle> anim2 = item2->getAnimation();
	      //	      ArRef<AnimationBlendCycle> animblend = new_AnimationBlendCycle();
	      ArRef<AnimationBlendCycle> animblend = AnimationBlendCycle::NEW();
	      ArRef<AnimationCycle> computed1 = AnimationCycle::nullRef();
	      ArRef<AnimationCycle> computed2 = AnimationCycle::nullRef();
									
	      // 0.04 is for 25 image/s.
	      if( item1->get_isAblend() &&  anim1->getClass()->isA(AnimationBlendCycle::CLASS()) ){
		ArRef<AnimationBlendCycle> tmp = ar_down_cast<AnimationBlendCycle>(anim1);
		int nb = (int)(tmp->getDuration() / 0.04);
		computed1 = tmp->computeAnimation(nb);
	      }
									
	      if( item2->get_isAblend() &&  anim2->getClass()->isA(AnimationBlendCycle::CLASS()) ){
		ArRef<AnimationBlendCycle> tmp = ar_down_cast<AnimationBlendCycle>(anim2);
		int nb = (int)(tmp->getDuration() / 0.04);
		computed2 = tmp->computeAnimation(nb);
	      }			
							
	      // Merge parent's anims into the current item	
	      if( (anim1 != AnimationCycle::nullRef() ) && (anim2 != AnimationCycle::nullRef() ) ) {
								
		if( computed1 != AnimationCycle::nullRef() ){
		  animblend->addAnimationCycle(computed1);
		} else {
		  animblend->addAnimationCycle(anim1); }
							
		if( computed2 != AnimationCycle::nullRef() ){
		  animblend->addAnimationCycle(computed2);
		} else {
		  animblend->addAnimationCycle(anim2); }
													
		item->setAnimation( ar_down_cast<AnimationCycle>(animblend) );
	      }
	    }
	  }
		
	_bodyObj->setAnimation( ar_down_cast<AnimationBlendCycle>(item->getAnimation()) );
	gtk_widget_set_sensitive(GTK_WIDGET(sRange), true);
	playDuration = ar_down_cast<AnimationBlendCycle>(item->getAnimation())->getDuration();
					
	double weight = /*_bodyObj->getBlendWeight();*/accessCurrentItem()->get_weight();
	_bodyObj->setBlendWeight(weight);
	gtk_range_set_range(tRange, 0.0, playDuration);
	gtk_range_set_value(sRange, weight );
      } 

    } else {  
      _bodyObj->setAnimation( item->getAnimation() );
      gtk_widget_set_sensitive(GTK_WIDGET(sRange), false);
      playDuration = item->getAnimation()->getDuration();
      gtk_range_set_range(tRange, 0.0, playDuration);
    }
	
  }
}
