/*
*
*Gestion des principales fonctionnalites de l'editeur
*
*/

#include <set>

#include "AReVi/Lib3D/boundingBox3D.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Utils/fileStream.h"

#include "hLib/core/joint.h"

#include "gtkToolsDialogs.h"
#include "editor.h"

AR_CLASS_DEF(Editor, GtkAReViWidget)

Editor::Editor(ArCW& arCW) 
: GtkAReViWidget(arCW, "./data/ihm/main.glade", "main"),
  _osdFilename(),
  _planeInteractors(),
  _constraintInteractors(),
  _editAnchor(), 
  _editConstraint(),
  _bodyObj(Object3D::NEW()),
  _editing(false) {
  // Create plugger and replace label by our plugger
  
  _multiView  = MultiView::NEW();
  
  GtkWidget* RenderBox = accessWidget("windowBox");

  gtk_box_pack_start(GTK_BOX(RenderBox),_multiView->getSingleViewWidget(),true,true, 1);

  gtk_box_pack_start(GTK_BOX(RenderBox), _multiView->getMultiViewWidget(),true, true, 4);

  gtk_box_reorder_child(GTK_BOX(RenderBox), _multiView->getSingleViewWidget(), 0);
  gtk_box_reorder_child(GTK_BOX(RenderBox), _multiView->getMultiViewWidget(), 0);

  _multiView->initViews();
  gtk_widget_show(_multiView->getSingleViewWidget());

  _connectSignals();


  // Setup joint tree view
  GtkCellRenderer* jointCelRenderer;
  GtkTreeViewColumn* jointCol;
  GtkTreeView* jointTreeView = GTK_TREE_VIEW(accessWidget("joint_tree"));
  GtkTreeStore* jointModel = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);

  gtk_tree_view_set_model(jointTreeView, GTK_TREE_MODEL(jointModel));

  jointCelRenderer = gtk_cell_renderer_text_new();
  jointCol = gtk_tree_view_column_new_with_attributes("Joint", jointCelRenderer, "text", COLUMN_JOINT_NAME, NULL);
  gtk_tree_view_column_set_sort_column_id(jointCol, COLUMN_JOINT_NAME);
  gtk_tree_view_append_column(jointTreeView, jointCol);
  
  GtkTreeSelection* jointSelection = gtk_tree_view_get_selection(jointTreeView);
  
  g_signal_connect(jointSelection, "changed", G_CALLBACK(&Editor::_onSelectJoint), this);


  // Setup anchor list view
  GtkCellRenderer* anchorCelRenderer;
  GtkTreeViewColumn* anchorCol;
  GtkTreeView* anchorTreeView = GTK_TREE_VIEW(accessWidget("anchor_list"));

  GtkListStore* anchorModel = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
  gtk_tree_view_set_model(anchorTreeView, GTK_TREE_MODEL(anchorModel));
  anchorCelRenderer = gtk_cell_renderer_text_new();
  anchorCol = gtk_tree_view_column_new_with_attributes("Ancre", anchorCelRenderer, "text", COLUMN_ANCHOR_NAME, NULL);
  gtk_tree_view_append_column(anchorTreeView, anchorCol);
  anchorCelRenderer = gtk_cell_renderer_text_new();
  anchorCol = gtk_tree_view_column_new_with_attributes("Lien", anchorCelRenderer, "text", COLUMN_ANCHOR_LINK, NULL);
  gtk_tree_view_column_set_sort_column_id(anchorCol, COLUMN_ANCHOR_NAME);
  gtk_tree_view_append_column(anchorTreeView, anchorCol);

  g_signal_connect(anchorTreeView, "row-activated", G_CALLBACK(&Editor::_onActivateAnchor),  this);
  GtkTreeSelection* anchorSelection = gtk_tree_view_get_selection(anchorTreeView);
  g_signal_connect(anchorSelection, "changed", G_CALLBACK(&Editor::_onSelectAnchor), this);
  g_signal_connect(accessWidget("add_anchor"), "clicked", G_CALLBACK(&Editor::_onAddAnchor), this);
  g_signal_connect(accessWidget("remove_anchor"), "clicked", G_CALLBACK(&Editor::_onRemoveAnchor), this);

  _undoRedo = UndoRedo::NEW();
  _undoRedo->addSizeCB(thisRef(), &Editor::_onSizeCB);

  _anchorCtrl = AnchorController::NEW();
  _anchorCtrl->addAnchorCB(thisRef(), &Editor::_onAnchorCB);

  _bodyCtrl = BodyController::NEW();
  _bodyCtrl->addBodyCB(thisRef(), &Editor::_onBodyCB);
  _bodyCtrl->setBody(Body::nullRef());

  _jointCtrl = JointController::NEW();

  // Create interactor for inverse kinematic manipulation
  for(int i = 0 ; i < 5 ; i++) {
    if(i<3) {
      _planeInteractors[i] = PlaneInteractor::NEW(_bodyCtrl, _jointCtrl, _anchorCtrl);
      _planeInteractors[i]->setWindow(_multiView->getWindow((MultiView::Views)i)); 
    }
    else {
      _constraintInteractors[i-3] = ConstraintInteractor::NEW(_bodyCtrl, _jointCtrl, _anchorCtrl);
      _constraintInteractors[i-3]->setWindow(_multiView->getWindow((MultiView::Views)i)); 
    }
    
  }


  _bodyObj->setAlias("body");

  // Extract supported extensions
  ArRef<HLibLoader> loader = HLibLoader::access();
  loader->addDirectory("./plugins");
  loader->refresh();
  _extensions = loader->getExtensions();
  
  // Create constraints representation
  for(size_t i = 0; i < 3; i++) {
    _osdAxis[i] = Axis::NEW((AxisType)i);
    _osdAxis[i]->setBounds(0.0, 360.0);
    _osdAxis[i]->setButtonValues(0.0, 360.0);
    _osdAxis[i]->addRenderer(_multiView->getWindow(MultiView::PERSPECTIVE_VIEW_MULTI));
    _osdAxis[i]->setVisible(false);
    _osdAxis[i]->addValueCB(thisRef(), &Editor::_onOSDValueCB);

    _jointCtrl->addSelectCB(thisRef(), &Editor::_onJointSelectCB);
    _jointCtrl->addConstraintCB(thisRef(), &Editor::_onJointConstraintCB);
    _jointCtrl->addValueCB(thisRef(), &Editor::_onJointValueCB);
  }

  // Create dialogs windows
  _editAnchor = EditAnchor::NEW();

  _editConstraint = EditConstraint::NEW(_undoRedo);
  gtk_widget_set_sensitive(_editConstraint->accessWidget("dockSrc"), false);
  _editConstraint->setDock("dockSrc", thisRef(), "dockDst");
  _editConstraint->dock();

}

Editor::~Editor(void) {
}


//-------------------------------------------------------------------
// CONNECT SIGNALS
//-------------------------------------------------------------------

void
Editor::_connectSignals(void) {

  //Connect signals for menubar
  //Fichier
  g_signal_connect(accessWidget("file_newMI"),"activate", G_CALLBACK(&Editor::_onFileNewMI), this);
  g_signal_connect(accessWidget("file_openMI"), "activate", G_CALLBACK(&Editor::_onFileOpenMI),this);
  g_signal_connect(accessWidget("file_importMI"), "activate", G_CALLBACK(&Editor::_onFileImportMI), this);
  g_signal_connect(accessWidget("file_saveMI"), "activate", G_CALLBACK(&Editor::_onFileSaveMI), this);
  g_signal_connect(accessWidget("file_save_asMI"), "activate", G_CALLBACK(&Editor::_onFileSaveAsMI), this);
  g_signal_connect(accessWidget("file_quitMI"), "activate", G_CALLBACK(&Editor::_onFileQuitMI), this);

  //Edition
  g_signal_connect(accessWidget("file_undoMI"), "activate", G_CALLBACK(&Editor::_onFileUndoMI), this);
  g_signal_connect(accessWidget("file_redoMI"), "activate", G_CALLBACK(&Editor::_onFileRedoMI), this);

  //Affichage
  g_signal_connect(accessWidget("display_adjust_bestMI"), "activate", G_CALLBACK(&Editor::_onDisplayAdjustBestMI), this);
  g_signal_connect(accessWidget("display_split_viewMI"), "toggled", G_CALLBACK(&Editor::_onDisplaySplitViewMI), this);

  //Affichage->Options d'affichage
  g_signal_connect(accessWidget("display_wireframeMI"), "toggled", G_CALLBACK(&Editor::_onDisplayWireframeMI), this);
  g_signal_connect(accessWidget("display_skeletonMI"), "toggled", G_CALLBACK(&Editor::_onDisplaySkeletonMI), this);
  g_signal_connect(accessWidget("display_anchorMI"), "toggled", G_CALLBACK(&Editor::_onDisplayAnchorMI), this);
  g_signal_connect(accessWidget("display_meshMI"), "toggled", G_CALLBACK(&Editor::_onDisplayMeshMI), this);

  //Aide
  g_signal_connect(accessWidget("file_helpMI"), "activate", G_CALLBACK(&Editor::_onFileHelpMI), this);

 // Toolbar
  g_signal_connect(accessWidget("file_new"), "clicked", G_CALLBACK(&Editor::_onFileNew), this);
  g_signal_connect(accessWidget("file_open"), "clicked", G_CALLBACK(&Editor::_onFileOpen), this);
  g_signal_connect(accessWidget("file_save"), "clicked", G_CALLBACK(&Editor::_onFileSave), this);
  g_signal_connect(accessWidget("file_save_as"), "clicked", G_CALLBACK(&Editor::_onFileSaveAs), this);
  g_signal_connect(accessWidget("file_undo"), "clicked", G_CALLBACK(&Editor::_onFileUndo), this);
  g_signal_connect(accessWidget("file_redo"), "clicked", G_CALLBACK(&Editor::_onFileRedo), this);
  g_signal_connect(accessWidget("file_import"), "clicked", G_CALLBACK(&Editor::_onFileImport), this);
  g_signal_connect(accessWidget("display_split_view"), "toggled", G_CALLBACK(&Editor::_onDisplaySplitView), this);
  g_signal_connect(accessWidget("display_adjust_best"), "clicked", G_CALLBACK(&Editor::_onDisplayAdjustBest), this);
  g_signal_connect(accessWidget("file_help"), "clicked", G_CALLBACK(&Editor::_onFileHelp), this);
  g_signal_connect(accessWidget("file_quit"), "clicked", G_CALLBACK(&Editor::_onFileQuit), this);

  // Options d'affichages toolbar
  g_signal_connect(accessWidget("display_wireframe"), "toggled", G_CALLBACK(&Editor::_onDisplayWireframe), this);
  g_signal_connect(accessWidget("display_skeleton"), "toggled", G_CALLBACK(&Editor::_onDisplaySkeleton), this);
  g_signal_connect(accessWidget("display_anchor"), "toggled", G_CALLBACK(&Editor::_onDisplayAnchor), this);
  g_signal_connect(accessWidget("display_mesh"), "toggled", G_CALLBACK(&Editor::_onDisplayMesh), this);

}


//-------------------------------------------------------------------
// CALLBACKS
//-------------------------------------------------------------------

/*
  Menubar : Fichier (MI : Menu Item)
*/


//fichier->nouveau
void
Editor::_onFileNewMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  e->_anchorCtrl->clearAnchors();
  e->_setFilename(StlString());
  e->_bodyCtrl->setBody(Body::nullRef());
}

//fichier->ouvrir
void
Editor::_onFileOpenMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  //s'il y a deja un humanoide charge
  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  //boite de dialogue d'ouverture de fichier
  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Ouvrir un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  //filtre pour les fichiers *.hlh
  GtkFileFilter* f = gtk_file_filter_new();
  gtk_file_filter_set_name(f, "Serialized humanoid");
  gtk_file_filter_add_pattern(f, "*.hlh");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_filename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),  e->_filename.c_str());
  }

  //si OK
  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    ArRef<FileIStream> input = FileIStream::NEW(filename);

    if(!input->fail()) {

      //deserialisation du fichier
      SerializationDependencies dep;
      dep.readFromStream(input);

      ArRef<Body> body;
      StlVector<ArRef<Anchor> > anchors;
      const StlVector<ArRef<ArObject> >& refs = dep.getReferences();

      for(size_t i = 0; i < refs.size(); i++) {
	if(!refs[i]->unserialize(dep, input)) {
	  g_free(filename);
	  gtk_widget_destroy (dialog);
	  return;
	}
	if(refs[i]->getClass()->isA(Anchor::CLASS())) {
	  anchors.push_back(ar_down_cast<Anchor>(refs[i]));
	}
      }

      body = ar_down_cast<Body>(refs.back());

      e->_jointCtrl->selectJoint(Joint::nullRef());
      e->_anchorCtrl->highLightAnchor(Anchor::nullRef());
      e->_bodyCtrl->setBody(body);
      e->_anchorCtrl->clearAnchors();
      //recuperation des ancres
      for(size_t i = anchors.size(); i--;) {
	e->_anchorCtrl->addAnchor(anchors[i]);
      }
      e->_undoRedo->emptyStacks();
      e->_undoRedo->saveState( e->_bodyCtrl );

      e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
      e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
      e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
      e->_updateAnchorListView();
      e->_updateJointTreeView();

      e->_bodyObj = Object3D::NEW();
      e->_bodyObj->setShape(body);
      //ajustement de la visualisation a l'humanoide
      ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
      e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
      e->_multiView->adjustViews( bbox );
      
    }

    e->_setFilename(filename);
    g_free(filename);
  }

  gtk_widget_destroy(dialog);

}

//fichier->importer (voir les comentaire de _onFileNewMI)
void
Editor::_onFileImportMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Importer un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
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

    LoaderData d = HLibLoader::access()->loadFile(filename, false);
    if(d.body) {
      e->_jointCtrl->selectJoint(Joint::nullRef());
      e->_anchorCtrl->highLightAnchor(Anchor::nullRef());
      e->_bodyCtrl->setBody(d.body);
      e->_anchorCtrl->clearAnchors();
      for(size_t i = d.anchors.size(); i--;) {
	e->_anchorCtrl->addAnchor(d.anchors[i]);
      }
      e->_undoRedo->emptyStacks();
      e->_undoRedo->saveState( e->_bodyCtrl );

      e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
      e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
      e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
      e->_updateAnchorListView();
      e->_updateJointTreeView();

      e->_bodyObj = Object3D::NEW();
      e->_bodyObj->setShape(d.body);
      ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
      e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
      e->_multiView->adjustViews( bbox );
      
    }

    g_free(filename);
  }

  gtk_widget_destroy (dialog);

  e->_jointCtrl->selectJoint(Joint::nullRef());
  e->_anchorCtrl->highLightAnchor(Anchor::nullRef());

}

//fichier->enregistrer
void
Editor::_onFileSaveMI(GtkImageMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(!e->_filename.empty()) {
    e->_saveBody(e->_filename);
    //e->_undoRedo->emptyStacks();
  } else {
    //si le fichier a deja ete enregistre
    _onFileSaveAsMI(item, data);
  }
}



//fichier->enregistrer sous
void
Editor::_onFileSaveAsMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  //boite de dialogue d'enregissstrement de fichier
  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Enregistrer sous", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  //filtre pour les fichiers *.hlh
  GtkFileFilter* f = gtk_file_filter_new();
  gtk_file_filter_set_name(f, "Serialized humanoid");
  gtk_file_filter_add_pattern(f, "*.hlh");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_filename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), e->_filename.c_str());
  }

  //si OK
  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    StlString filenameWithExtension(filename);

    if(strExtension(filenameWithExtension) == "") {
      filenameWithExtension += ".hlh";
    }

    e->_saveBody(filenameWithExtension);   
    e->_setFilename(filenameWithExtension);
    g_free(filename);
  }

  gtk_widget_destroy (dialog);
  //e->_undoRedo->emptyStacks();
}

//fichier->quitter
void
Editor::_onFileQuitMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  //si un fichier est charge
  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  ArSystem::leaveSimulation();
}

/*
  Menubar : Edition (MI : Menu Item)
*/

//edtion->defaire
void
Editor::_onFileUndoMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;

  e->_jointCtrl->selectJoint(Joint::nullRef());
  e->_anchorCtrl->highLightAnchor(Anchor::nullRef());

  e->_undoRedo->undo(e->_bodyCtrl, e->_anchorCtrl);

  e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
  e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
  e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
  e->_updateAnchorListView();
  e->_updateJointTreeView();
}

//edition->refaire
void
Editor::_onFileRedoMI(GtkImageMenuItem*, gpointer data) {
  Editor* e = (Editor*)data;
  e->_jointCtrl->selectJoint(Joint::nullRef());
  e->_anchorCtrl->highLightAnchor(Anchor::nullRef());

  e->_undoRedo->redo(e->_bodyCtrl, e->_anchorCtrl);

  e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
  e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
  e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
  e->_updateAnchorListView();
  e->_updateJointTreeView();
}


/*
  Menubar : Affichage (MI : Menu Item)
*/

//affichage->ajuster au mieux
void
Editor::_onDisplayAdjustBestMI(GtkImageMenuItem*, gpointer data){
  Editor* e = (Editor*)data;

  //  ArRef<BoundingBox3D> bbox = new_BoundingBox3D();
  ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
  e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
  e->_multiView->adjustViews( bbox );

}

//affichage->vues separees
void
Editor::_onDisplaySplitViewMI(GtkCheckMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;
  
  if(gtk_check_menu_item_get_active(item)) {
    gtk_widget_hide(e->_multiView->getSingleViewWidget());
    gtk_widget_show(e->_multiView->getMultiViewWidget());
  }
  else{
    gtk_widget_hide(e->_multiView->getMultiViewWidget());
    gtk_widget_show(e->_multiView->getSingleViewWidget());
  }
  
  if(gtk_check_menu_item_get_active(item)){
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(e->accessWidget("display_split_view")),true);
  }
  else{
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(e->accessWidget("display_split_view")),false);
  }
  
}

//affichage->fil de fer
void
Editor::_onDisplayWireframeMI(GtkCheckMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;
  
  e->_multiView->getWindow(MultiView::FRONT_VIEW)->setWireFrame(gtk_check_menu_item_get_active(item));
  e->_multiView->getWindow(MultiView::RIGHT_VIEW)->setWireFrame(gtk_check_menu_item_get_active(item));
  e->_multiView->getWindow(MultiView::TOP_VIEW)->setWireFrame(gtk_check_menu_item_get_active(item));
  e->_multiView->getWindow(MultiView::PERSPECTIVE_VIEW_MULTI)->setWireFrame(gtk_check_menu_item_get_active(item));
  e->_multiView->getWindow(MultiView::PERSPECTIVE_VIEW)->setWireFrame(gtk_check_menu_item_get_active(item));
  
  if(gtk_check_menu_item_get_active(item)){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_wireframe")),true);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_wireframe")),false);
  }
  
}

//affichage->squelette
void
Editor::_onDisplaySkeletonMI(GtkCheckMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    e->_bodyCtrl->accessBody()->setVisible(gtk_check_menu_item_get_active(item));
  }
  
  if(gtk_check_menu_item_get_active(item)){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_skeleton")),true);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_skeleton")),false);
  }
  
}

//affichage->ancres
void
Editor::_onDisplayAnchorMI(GtkCheckMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;

  for(size_t i = e->_anchorCtrl->getNbAnchors(); i--;) {
    e->_anchorCtrl->accessAnchor(i)->setVisible(gtk_check_menu_item_get_active(item));
  }
  
  if(gtk_check_menu_item_get_active(item)){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_anchor")),true);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_anchor")),false);
  }
  
}

//affichage->geometrie
void
Editor::_onDisplayMeshMI(GtkCheckMenuItem* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(gtk_check_menu_item_get_active(item)) {
    while(e->_parts.size()) {
      e->_bodyCtrl->accessBody()->addRootPart(e->_parts.back());
      e->_parts.pop_back();
    }
  } else {
    for(size_t i = e->_bodyCtrl->getBody()->getNbRootParts(); i--;) {
      ArRef<ShapePart3D> m = e->_bodyCtrl->accessBody()->accessRootPart(i);
      if(m->getClass()->isA(Skin::CLASS())) {
	e->_parts.push_back(ar_down_cast<Skin>(m));
      }
    }

    for(size_t i = e->_parts.size(); i--;) {
      e->_bodyCtrl->accessBody()->removeRootPart(e->_parts[i]);
    }
  }
  
  if(gtk_check_menu_item_get_active(item)){
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_mesh")),true);
  }
  else{
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(e->accessWidget("display_mesh")),false);
  }
  
}


/*
  Menubar : Aide (MI : Menu Item)
*/

//Aide
void
Editor::_onFileHelpMI(GtkImageMenuItem*, gpointer) {
  showMessageBox("Débrouilles toi !", GTK_MESSAGE_ERROR, NULL);
}

/*
  Toolbar Buttons
*/

//nouveau
void
Editor::_onFileNew(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  e->_anchorCtrl->clearAnchors();
  e->_setFilename(StlString());
  e->_bodyCtrl->setBody(Body::nullRef());
}

//ouvrir (cf commentaire _onFileOpenMI)
void
Editor::_onFileOpen(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Ouvrir un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  GtkFileFilter* f = gtk_file_filter_new();
  gtk_file_filter_set_name(f, "Serialized humanoid");
  gtk_file_filter_add_pattern(f, "*.hlh");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_filename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), 
				  e->_filename.c_str());
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    ArRef<FileIStream> input = FileIStream::NEW(filename);

    if(!input->fail()) {

      SerializationDependencies dep;
      dep.readFromStream(input);

      ArRef<Body> body;
      StlVector<ArRef<Anchor> > anchors;
      const StlVector<ArRef<ArObject> >& refs = dep.getReferences();

      for(size_t i = 0; i < refs.size(); i++) {
	if(!refs[i]->unserialize(dep, input)) {
	  g_free(filename);
	  gtk_widget_destroy (dialog);
	  return;
	}
	if(refs[i]->getClass()->isA(Anchor::CLASS())) {
	  anchors.push_back(ar_down_cast<Anchor>(refs[i]));
	}
      }

      body = ar_down_cast<Body>(refs.back());

      e->_jointCtrl->selectJoint(Joint::nullRef());
      e->_anchorCtrl->highLightAnchor(Anchor::nullRef());
      e->_bodyCtrl->setBody(body);
      e->_anchorCtrl->clearAnchors();
      for(size_t i = anchors.size(); i--;) {
	e->_anchorCtrl->addAnchor(anchors[i]);
      }
      e->_undoRedo->emptyStacks();
      e->_undoRedo->saveState( e->_bodyCtrl );

      e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
      e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
      e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
      e->_updateAnchorListView();
      e->_updateJointTreeView();

      e->_bodyObj = Object3D::NEW();
      e->_bodyObj->setShape(body);
      ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
      e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
      e->_multiView->adjustViews( bbox );
      
    }

    e->_setFilename(filename);
    g_free(filename);
  }

  gtk_widget_destroy(dialog);

}

//enregistrer
void
Editor::_onFileSave(GtkToolButton* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(!e->_filename.empty()) {
    e->_saveBody(e->_filename);
    e->_undoRedo->emptyStacks();
  } else {
    _onFileSaveAs(item, data);
  }
}

//enregistrer sous
void
Editor::_onFileSaveAs(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Enregistrer sous", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  GtkFileFilter* f = gtk_file_filter_new();
  gtk_file_filter_set_name(f, "Serialized humanoid");
  gtk_file_filter_add_pattern(f, "*.hlh");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), f);

  if(!e->_filename.empty()) {
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), 
				  e->_filename.c_str());
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *filename;
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

    StlString filenameWithExtension(filename);

    cerr << strExtension(filenameWithExtension) << endl;

    if(strExtension(filenameWithExtension) == "") {
      filenameWithExtension += ".hlh";
    }

    e->_saveBody(filenameWithExtension);   
    e->_setFilename(filenameWithExtension);
    g_free(filename);
  }

  gtk_widget_destroy (dialog);
  e->_undoRedo->emptyStacks();  
}

//defaire
void
Editor::_onFileUndo(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  e->_jointCtrl->selectJoint(Joint::nullRef());
  e->_anchorCtrl->highLightAnchor(Anchor::nullRef());

  e->_undoRedo->undo(e->_bodyCtrl, e->_anchorCtrl );

  e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
  e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
  e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
  e->_updateAnchorListView();
  e->_updateJointTreeView();
}

//refaire
void
Editor::_onFileRedo(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  e->_jointCtrl->selectJoint(Joint::nullRef());
  e->_anchorCtrl->highLightAnchor(Anchor::nullRef());

  e->_undoRedo->redo(e->_bodyCtrl, e->_anchorCtrl );

  e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
  e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
  e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
  
  e->_updateAnchorListView();
  e->_updateJointTreeView();
}

//importer (commentaire cf _onFileOpenMI)
void
Editor::_onFileImport(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  GtkWidget* dialog;
  dialog = gtk_file_chooser_dialog_new("Importer un fichier", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
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

    LoaderData d = HLibLoader::access()->loadFile(filename, false);
    if(d.body) {

      e->_jointCtrl->selectJoint(Joint::nullRef());
      e->_anchorCtrl->highLightAnchor(Anchor::nullRef());
      e->_bodyCtrl->setBody(d.body);
      e->_anchorCtrl->clearAnchors();
      for(size_t i = d.anchors.size(); i--;) {
	e->_anchorCtrl->addAnchor(d.anchors[i]);
      }
      e->_undoRedo->emptyStacks();
      e->_undoRedo->saveState( e->_bodyCtrl );

      e->_jointCtrl->addSelectCB(e->thisRef(), &Editor::_onJointSelectCB);
      e->_jointCtrl->addConstraintCB(e->thisRef(), &Editor::_onJointConstraintCB);
      e->_jointCtrl->addValueCB(e->thisRef(), &Editor::_onJointValueCB);
      e->_updateAnchorListView();
      e->_updateJointTreeView();

      e->_bodyObj = Object3D::NEW();
      e->_bodyObj->setShape(d.body);
      ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
      e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
      e->_multiView->adjustViews( bbox );
      
    }

    g_free(filename);
  }

  gtk_widget_destroy (dialog);

}

//vues separees
void
Editor::_onDisplaySplitView(GtkToggleToolButton* item, gpointer data) {
  Editor* e = (Editor*)data;
  
  if(gtk_toggle_tool_button_get_active(item)) {
    gtk_widget_hide(e->_multiView->getSingleViewWidget());
    gtk_widget_show(e->_multiView->getMultiViewWidget());
  }
  else{
    gtk_widget_hide(e->_multiView->getMultiViewWidget());
    gtk_widget_show(e->_multiView->getSingleViewWidget());
  }

  if(gtk_toggle_tool_button_get_active(item)){
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_split_viewMI")),true);
  }
  else{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_split_viewMI")),false);
  }

}

//ajuster au mieux
void
Editor::_onDisplayAdjustBest(GtkToolButton*, gpointer data){
  Editor* e = (Editor*)data;

  ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
  e->_bodyObj->computeBoundingBox(bbox,ArConstRef<Base3D>(e->_bodyObj) );
  e->_multiView->adjustViews( bbox );

}

//aide
void
Editor::_onFileHelp(GtkToolButton*, gpointer) {
  showMessageBox("Débrouilles toi !", GTK_MESSAGE_ERROR, NULL);
}

//quitter
void
Editor::_onFileQuit(GtkToolButton*, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    if(!e->_askBeforeGoOn()) {
      return;
    }
  }

  ArSystem::leaveSimulation();
}

/*
  Toolbar : Options d'affichage
*/

//afficher les fil de fer
void
Editor::_onDisplayWireframe(GtkToggleButton* item, gpointer data) {
  Editor* e = (Editor*)data;
  
  e->_multiView->getWindow(MultiView::FRONT_VIEW)->setWireFrame(gtk_toggle_button_get_active(item));
  e->_multiView->getWindow(MultiView::RIGHT_VIEW)->setWireFrame(gtk_toggle_button_get_active(item));
  e->_multiView->getWindow(MultiView::TOP_VIEW)->setWireFrame(gtk_toggle_button_get_active(item));
  e->_multiView->getWindow(MultiView::PERSPECTIVE_VIEW_MULTI)->setWireFrame(gtk_toggle_button_get_active(item));
  e->_multiView->getWindow(MultiView::PERSPECTIVE_VIEW)->setWireFrame(gtk_toggle_button_get_active(item));

  if(gtk_toggle_button_get_active(item)){
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_wireframeMI")),true);
  }
  else{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_wireframeMI")),false);
  }

}


//afficher le squelette
void
Editor::_onDisplaySkeleton(GtkToggleButton* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(e->_bodyCtrl->getBody().valid()) {
    e->_bodyCtrl->accessBody()->setVisible(gtk_toggle_button_get_active(item));
  }

  if(gtk_toggle_button_get_active(item)){
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_skeletonMI")),true);
  }
  else{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_skeletonMI")),false);
  }

}

//afficher les ancres
void
Editor::_onDisplayAnchor(GtkToggleButton* item, gpointer data) {
  Editor* e = (Editor*)data;

  for(size_t i = e->_anchorCtrl->getNbAnchors(); i--;) {
    e->_anchorCtrl->accessAnchor(i)->setVisible(gtk_toggle_button_get_active(item));
  }

  if(gtk_toggle_button_get_active(item)){
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_anchorMI")),true);
  }
  else{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_anchorMI")),false);
  }

}

//afficher la geometrie
void
Editor::_onDisplayMesh(GtkToggleButton* item, gpointer data) {
  Editor* e = (Editor*)data;

  if(gtk_toggle_button_get_active(item)) {
    while(e->_parts.size()) {
      e->_bodyCtrl->accessBody()->addRootPart(e->_parts.back());
      e->_parts.pop_back();
    }
  } else {
    for(size_t i = e->_bodyCtrl->getBody()->getNbRootParts(); i--;) {
      ArRef<ShapePart3D> m = e->_bodyCtrl->accessBody()->accessRootPart(i);
      if(m->getClass()->isA(Skin::CLASS())) {
	e->_parts.push_back(ar_down_cast<Skin>(m));
      }
    }

    for(size_t i = e->_parts.size(); i--;) {
      e->_bodyCtrl->accessBody()->removeRootPart(e->_parts[i]);
    }
  }

  if(gtk_toggle_button_get_active(item)){
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_meshMI")),true);
  }
  else{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(e->accessWidget("display_meshMI")),false);
  }

}



// Joint tree view callbacks
#if 0
void
Editor::_onActivateJoint(GtkTreeView* /* view */, GtkTreePath* /* path */, GtkTreeViewColumn* /* arg2 */, gpointer data) {
  Editor* e = (Editor*)data;
  e->_editConstraint->fill(e->_jointCtrl);
  e->_editConstraint->show();
}
#endif

//Gestion de la selection d'articulation dans l'arbre
void        
Editor::_onSelectJoint(GtkTreeSelection* selection, gpointer data) {
  Editor* e = (Editor*)data;

  GtkTreeModel* model;
  GtkTreeIter iter;

  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    Joint* jPtr;
    gtk_tree_model_get(model, &iter, COLUMN_JOINT_DATA, &jPtr, -1);
    e->_jointCtrl->selectJoint(ArRef<Joint>(jPtr));
    e->_editConstraint->fill(e->_jointCtrl);
  }
  
  
  
  
}

// Anchor list view callbacks

void
Editor::_onActivateAnchor(GtkTreeView* view, GtkTreePath* path, GtkTreeViewColumn* /* arg2 */, gpointer data) {
  Editor* e = (Editor*)data;
  
  GtkTreeIter iter;
  GtkTreeModel* model = gtk_tree_view_get_model(view);;
  gtk_tree_model_get_iter(model, &iter, path);

  Anchor* aPtr;
  gtk_tree_model_get(model, &iter, COLUMN_ANCHOR_DATA, &aPtr, -1);

  ArRef<Anchor> anchor(aPtr);
  e->_editAnchor->fill(anchor, e->_bodyCtrl->accessBody());
  
  if(gtk_dialog_run(GTK_DIALOG(e->_editAnchor->accessMainWidget())) == GTK_RESPONSE_OK) {
    GtkEntry* nameEntry = GTK_ENTRY(e->_editAnchor->accessWidget("name"));
    const gchar* name = gtk_entry_get_text(nameEntry);

    GtkComboBox* linkCombo = GTK_COMBO_BOX(e->_editAnchor->accessWidget("link"));
    GtkTreeModel* model = GTK_TREE_MODEL(gtk_combo_box_get_model(linkCombo));

    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(linkCombo, &iter);
    Joint* jPtr;
    gtk_tree_model_get(model, &iter, COLUMN_LINK_DATA, &jPtr, -1);

    anchor->setName(name);
    anchor->linkTo(ArRef<Joint>(jPtr));
    e->_anchorCtrl->postChanges();
  }

  e->_editAnchor->hide(); 
}

//Gestion de la selection d'une ancre dans la liste des ancres
void        
Editor::_onSelectAnchor(GtkTreeSelection* selection, gpointer data) {
  Editor* e = (Editor*)data;

  GtkTreeModel* model;
  GtkTreeIter iter;

  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    Anchor* aPtr;
    gtk_tree_model_get(model, &iter, COLUMN_ANCHOR_DATA, &aPtr, -1);
    e->_anchorCtrl->highLightAnchor(ArRef<Anchor>(aPtr));
  }
}

//Gestion de l'ajout d'une ancre
void
Editor::_onAddAnchor(GtkButton*, gpointer data) {
  Editor* e = (Editor*)data;

  e->_editAnchor->fill(Anchor::nullRef(), e->_bodyCtrl->accessBody());
  
  if(gtk_dialog_run(GTK_DIALOG(e->_editAnchor->accessMainWidget())) == GTK_RESPONSE_OK) {
    GtkEntry* nameEntry = GTK_ENTRY(e->_editAnchor->accessWidget("name"));
    const gchar* name = gtk_entry_get_text(nameEntry);

    GtkComboBox* linkCombo = GTK_COMBO_BOX(e->_editAnchor->accessWidget("link"));
    GtkTreeModel* model = GTK_TREE_MODEL(gtk_combo_box_get_model(linkCombo));

    GtkTreeIter iter;
    gtk_combo_box_get_active_iter(linkCombo, &iter);
    Joint* jPtr;
    gtk_tree_model_get(model, &iter, COLUMN_LINK_DATA, &jPtr, -1);
    ArRef<Anchor> a = Anchor::NEW();
    a->setName(name);
    a->linkTo(ArRef<Joint>(jPtr));
    
    ArRef<Transform3D> tr = Transform3D::NEW();
    tr->preTranslate(1.0, 1.0, 1.0);
    a->writeTransformation(tr);
    e->_anchorCtrl->addAnchor(a);
    e->_undoRedo->saveState( e->_bodyCtrl );
  }

  e->_editAnchor->hide();
}

//Gestion de la suppression d'une ancre
void
Editor::_onRemoveAnchor(GtkButton*, gpointer data) {
  Editor* e = (Editor*)data;
  GtkTreeView* view = GTK_TREE_VIEW(e->accessWidget("anchor_list"));
  GtkTreeSelection* sel = gtk_tree_view_get_selection(view);

  GtkTreeIter iter;
  GtkTreeModel* model;
  if(!gtk_tree_selection_get_selected(sel, &model, &iter)) {
    return;
  }
  
  Anchor* aPtr;
  gtk_tree_model_get(model, &iter, COLUMN_ANCHOR_DATA, &aPtr, -1);
  ArRef<Anchor> anchor(aPtr);
  if(showAskingBox("Supprimer l'ancre " + anchor->getName() + " ?", GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, NULL) == GTK_RESPONSE_YES) {
    e->_anchorCtrl->removeAnchor( anchor );
    e->_undoRedo->saveState( e->_bodyCtrl );
  }
}

//-------------------------------------------------------------------
// Utilities functions
//-------------------------------------------------------------------

//sauvegarde d'un humanoide
bool
Editor::_saveBody(StlString filename) {
  ArRef<FileOStream> output = FileOStream::NEW(filename, false);
  if(output->fail()) {
    showMessageBox("Erreur lors de la sauvegarde\n" + output->getErrorMessage(), GTK_MESSAGE_ERROR, NULL);
    return false;
  }

  ArRef<Joint> selection = _jointCtrl->accessSelectedJoint();
  _jointCtrl->selectJoint(Joint::nullRef());

  ArRef<Anchor> anchor = _anchorCtrl->accessHighLightedAnchor();
  _anchorCtrl->highLightAnchor(Anchor::nullRef());

  //serialisation de l'humanoide et redirection vers le fichier de sortie
  SerializationDependencies dep;
  _bodyCtrl->getBody()->extractDependencies(dep, true);
  dep.addReference(_bodyCtrl->accessBody());
  dep.writeToStream(output);

  const StlVector<ArRef<ArObject> >& refsOut = dep.getReferences();
  for(size_t i = 0; i < refsOut.size(); i++) {
    refsOut[i]->serialize(dep, output);
  }
  
  _jointCtrl->selectJoint(selection);
  _anchorCtrl->highLightAnchor(anchor);

  return true;
}

//mise a jour du nom de fichier
void
Editor::_setFilename(StlString filename) {
  _filename = filename;
//   if(_filename.empty()) {
//     _osdFilename->setText("no file");
//   } else {
//     _osdFilename->setText(_filename);
//   }
}

//confimer de continuer
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

//Regeneration de l'arbre des articulations
void
Editor::_updateJointTreeView(void) {
  GtkTreeView* jointTreeView = GTK_TREE_VIEW(accessWidget("joint_tree"));
  GtkTreeStore* jointModel = GTK_TREE_STORE(gtk_tree_view_get_model(jointTreeView));

  gtk_tree_store_clear(jointModel);
  
  if(_bodyCtrl->getBody().valid()) {
    ArConstRef<Joint> root = _bodyCtrl->getBody()->getRootJoint();
    if(root.valid()) {
      _fillJointTree(root, (GtkTreeIter*)0);
    }
  }

  gtk_tree_view_expand_all(jointTreeView);
}

//remplissage de l'arbre des articualtions
void
Editor::_fillJointTree(ArConstRef<Joint> node, GtkTreeIter* parentIter) {
  GtkTreeIter iter;
  GtkTreeView* jointTreeView = GTK_TREE_VIEW(accessWidget("joint_tree"));
  GtkTreeStore* jointModel = GTK_TREE_STORE(gtk_tree_view_get_model(jointTreeView));

  gtk_tree_store_append(jointModel, &iter, parentIter);
  gtk_tree_store_set(jointModel, &iter, COLUMN_JOINT_NAME, node->getName().c_str(), COLUMN_JOINT_DATA, node.c_ptr(), -1);

  for(int i = 0; i < node->getNbChildren(); i++) {
    _fillJointTree(node->getChild(i), &iter);
  }
}

//Regeneration de la liste des ancres
void
Editor::_updateAnchorListView(void) {
  GtkTreeIter iter;
  GtkTreeView* anchorTreeView = GTK_TREE_VIEW(accessWidget("anchor_list"));
  GtkListStore* anchorModel = GTK_LIST_STORE(gtk_tree_view_get_model(anchorTreeView));
  
  gtk_list_store_clear(anchorModel);

  for(size_t i = 0; i < _anchorCtrl->getNbAnchors(); i++) {
    gtk_list_store_append(anchorModel, &iter);
    gtk_list_store_set(anchorModel, &iter, COLUMN_ANCHOR_NAME, _anchorCtrl->getAnchor(i)->getName().c_str(), COLUMN_ANCHOR_LINK, _anchorCtrl->getAnchor(i)->getLinkedTo()->getName().c_str(), COLUMN_ANCHOR_DATA, _anchorCtrl->getAnchor(i).c_ptr(), -1);     
  }

  bool sensitive = false;
  if(_bodyCtrl->getBody().valid() &&
     _bodyCtrl->getBody()->getNbJoints() > 0) {
    sensitive = true;
  }

  gtk_widget_set_sensitive(accessWidget("add_anchor"), sensitive);
  gtk_widget_set_sensitive(accessWidget("remove_anchor"), sensitive);
}

//fonction reflexe au signaux de changement d'humanoide
void
Editor::_onBodyCB(const BodyController::BodyEvent& evt) {
  _parts.clear();

  _bodyObj->setShape(evt.body);

  bool sensitive = false;
  if(evt.body.valid()) {

    //mise a jour des interacteur
    for(int i = 0 ; i < 5 ; i++) {
      if(i<3){
	_planeInteractors[i]->setBody(evt.body);
      }
      else {
	_constraintInteractors[i-3]->setBody(evt.body);
      }
    }
    sensitive = true;

    ArRef<BoundingBox3D> bbox = BoundingBox3D::NEW();
    evt.body->readBoundingBox(bbox);
    double sf = 1.80 / bbox->getMaxSize();

    ArRef<Transform3D> tr = Transform3D::NEW();
    tr->preScale(sf, sf, sf);
    tr->preRoll(M_PI_2);
    tr->preYaw(M_PI_2);
    evt.body->writeTransformation(tr);
    
    evt.body->setVisible(true);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accessWidget("display_skeleton")), true);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accessWidget("display_anchor")), true);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accessWidget("display_mesh")), true);


    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(accessWidget("display_skeletonMI")), true);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(accessWidget("display_anchorMI")), true);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(accessWidget("display_meshMI")), true);
  }

  //definition des objets actifs de l'IHM
  gtk_widget_set_sensitive(accessWidget("file_saveMI"), sensitive);
  gtk_widget_set_sensitive(accessWidget("file_save_asMI"), sensitive);

  gtk_widget_set_sensitive(accessWidget("file_save"), sensitive);
  gtk_widget_set_sensitive(accessWidget("file_save_as"), sensitive);

  gtk_widget_set_sensitive(accessWidget("display_wireframe"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_skeleton"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_anchor"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_mesh"), sensitive);

  gtk_widget_set_sensitive(accessWidget("display_wireframeMI"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_skeletonMI"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_anchorMI"), sensitive);
  gtk_widget_set_sensitive(accessWidget("display_meshMI"), sensitive);

  gtk_widget_set_sensitive(accessWidget("file_undo"), sensitive);
  gtk_widget_set_sensitive(accessWidget("file_redo"), sensitive);

  _updateJointTreeView();

}

//fonction reflexes sur l'edition des ancres
void
Editor::_onAnchorCB(const AnchorController::AnchorEvent& /* evt */) {
  _updateAnchorListView();
}

//fonction reflexe a la selction d'une articulation
void
Editor::_onJointSelectCB(const JointController::SelectEvent& evt) {
  
  if(evt.joint) {
    JointController::ConstraintEvent e;
    e.jointConstraint = evt.joint->accessConstraint();
    _onJointConstraintCB(e);
  } 
  else {
    for(size_t i = 3; i--;) {
      _osdAxis[i]->setVisible(false);
    }
  }
  
  ArRef<Joint> joint = evt.joint;
  GtkTreeView* jointTreeView = GTK_TREE_VIEW(accessWidget("joint_tree"));
  GtkTreeModel* model = gtk_tree_view_get_model(jointTreeView);
  GtkTreeIter* iter_result = new GtkTreeIter();
  bool valid;
	
  if(joint != Joint::nullRef()) {
    //recherche et selection dans l'arbre des articulations
    const char* JN = (joint->getName()).c_str();
    valid = _search_joint_in_treeview(model, JN , NULL , &iter_result);
    if(valid) {
      GtkTreeSelection* select = gtk_tree_view_get_selection(jointTreeView);
      GtkTreePath* treepath = gtk_tree_model_get_path(model,iter_result);
      gtk_tree_selection_select_path(select,treepath);
    }
  }
  gtk_tree_iter_free(iter_result);
}

//recherche d'une articualtion dans l'arbre des articulations
bool
Editor::_search_joint_in_treeview(GtkTreeModel* model,const char* JN, GtkTreeIter* iter, GtkTreeIter** iter_out) {
  GtkTreeIter* iter_old = new GtkTreeIter();
  
  gboolean valid;
  bool result = false;
  gchar* str_data;
  
  if(iter == NULL) {	
    iter = new GtkTreeIter();
    valid = gtk_tree_model_get_iter_first(model, iter);
    iter_old = gtk_tree_iter_copy(iter);
  }
  else {
    iter_old = gtk_tree_iter_copy(iter);
  }
  
  gtk_tree_model_get(model, iter, COLUMN_JOINT_NAME, &str_data,-1);
  if( strcmp( JN , str_data) == 0 ) {
    *iter_out = gtk_tree_iter_copy(iter);
    return true;
  }
  else {
    if(gtk_tree_model_iter_has_child(model,iter)) {
      gint nb;
      nb = gtk_tree_model_iter_n_children(model,iter);
      for(gint i = 0; i < nb  ; i++) {
	valid = gtk_tree_model_iter_nth_child(model,iter,iter_old, i );
	result = (gboolean)_search_joint_in_treeview(model,JN,iter,iter_out);
        if(result == true) {return result;}
      }
    }
  }
  return result;
}


//fonction reflexe au changement de contrainte d'une articualtion -> impact sur la visualisation
void
Editor::_onJointConstraintCB(const JointController::ConstraintEvent& evt) {
  for(size_t i = 3; i--;) {
    _osdAxis[i]->setVisible(false); 

    double in, out;
    _jointCtrl->getAxisValues((AxisType)i, in, out);
    _editing = true;
    _osdAxis[i]->setButtonValues(in, out);
    _editing = false;
  }

  if(evt.jointConstraint != JointConstraint::nullRef()) {
    
    if(evt.jointConstraint->getClass()->isA(JointConstraint1DOF::CLASS())) {
      switch(ar_down_cast<JointConstraint1DOF>(evt.jointConstraint)->getAxisType()) {
      case JointConstraint1DOF::AXIS_X :
	_osdAxis[AXIS_X]->setPosition(32, 32);
	_osdAxis[AXIS_X]->setVisible(true);
	break;
      case JointConstraint1DOF::AXIS_Y :
	_osdAxis[AXIS_Y]->setPosition(32, 32);
	_osdAxis[AXIS_Y]->setVisible(true);
	break;
      case JointConstraint1DOF::AXIS_Z :
	_osdAxis[AXIS_Z]->setPosition(32, 32);
	_osdAxis[AXIS_Z]->setVisible(true);
	break;
      default :
	break;
      }
    }
    if(evt.jointConstraint->getClass()->isA(JointConstraint2DOF::CLASS())) {
      switch(ar_down_cast<JointConstraint2DOF>(evt.jointConstraint)->getPlaneType()) {
      case JointConstraint2DOF::PLANE_XY :
	_osdAxis[AXIS_X]->setPosition(32, 32);
	_osdAxis[AXIS_X]->setVisible(true);
	_osdAxis[AXIS_Y]->setPosition(32, 77);
	_osdAxis[AXIS_Y]->setVisible(true);
	break;
      case JointConstraint2DOF::PLANE_XZ :
	_osdAxis[AXIS_X]->setPosition(32, 32);
	_osdAxis[AXIS_X]->setVisible(true);
	_osdAxis[AXIS_Z]->setPosition(32, 77);
	_osdAxis[AXIS_Z]->setVisible(true);
	break;
      case JointConstraint2DOF::PLANE_YZ :
	_osdAxis[AXIS_Y]->setPosition(32, 32);
	_osdAxis[AXIS_Y]->setVisible(true);
	_osdAxis[AXIS_Z]->setPosition(32, 77);
	_osdAxis[AXIS_Z]->setVisible(true);
	break;
      default :
	break;
      }
    }
    if(evt.jointConstraint->getClass()->isA(JointConstraint3DOF::CLASS())) {
      ArRef<JointConstraint3DOF> constraint = ar_down_cast<JointConstraint3DOF>(evt.jointConstraint);
      constraint->applyChanges();
    }
  }
}

void
Editor::_onJointValueCB(const JointController::ValueEvent& evt) {
  if(!_editing) {
    _editing = true;
    _osdAxis[evt.axis]->setButtonValues(evt.in, evt.out);
    _editing = false;
  }
}

void
Editor::_onOSDValueCB(const Axis::ValueEvent& evt) {
  if(!_editing) {
    _editing = true;
    for(size_t i = 0; i < 3; i++) {
      if(evt.source == _osdAxis[i]) {
	_jointCtrl->setAxisValues((AxisType)i, evt.in, evt.out);
	_editing = false;
	return;
      }
    }
  }
}

//fonction reflexe au changement de taille de la pile d'undo/redo -> determine si les boutons sont actifs
void
Editor::_onSizeCB(const UndoRedo::SizeEvent& evt) {
  if(evt.undoSize) {
    gtk_widget_set_sensitive(accessWidget("file_undo"),true); 
    gtk_widget_set_sensitive(accessWidget("file_undoMI"),true); 
  }
  else{ 
    gtk_widget_set_sensitive(accessWidget("file_undo"), false); 
    gtk_widget_set_sensitive(accessWidget("file_undoMI"), false);
  }
  if(evt.redoSize) {
    gtk_widget_set_sensitive(accessWidget("file_redo"),true); 
    gtk_widget_set_sensitive(accessWidget("file_redoMI"),true);
  }
  else{
    gtk_widget_set_sensitive(accessWidget("file_redo"), false); 
    gtk_widget_set_sensitive(accessWidget("file_redoMI"), false);
  }
}
