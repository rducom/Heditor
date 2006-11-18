/*
*
*Gestion de l'IHM permettant l'edition des contraintes d'articulation
*
*/

#include "AReVi/arClass.h"

#include "editConstraint.h"
#include "keyOrientation.h"

AR_CLASS_NOVOID_DEF(EditConstraint, GtkAReViWidget)

EditConstraint::EditConstraint(ArCW& arCW, ArRef<UndoRedo> undoRedo) 
: GtkAReViWidget(arCW, "./data/ihm/edit_constraint.glade", "edit_constraint"),
  _editing(false),
  _jointCtrl(),
  _undoRedo(undoRedo),
  _spin(false) {

  //initialisation des differentes visualisaaation en fonction des contraintes
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(accessWidget("none")), true);
  gtk_notebook_set_current_page(GTK_NOTEBOOK(accessWidget("notebook")), 8);

  //connecitons des signaux avec l'IHM
  g_signal_connect(accessWidget("x"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("y"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("z"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("xy"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("xz"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("yz"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("xyz"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("locked"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);
  g_signal_connect(accessWidget("none"), "toggled", G_CALLBACK(&EditConstraint::_onConstraintToggled), this);

  g_signal_connect(accessWidget("notebook"), "switch-page", G_CALLBACK(&EditConstraint::_fillNotebookPage), this);

  g_signal_connect(accessWidget("x_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("x_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("y_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("y_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("z_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("z_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xy_x_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xy_x_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xy_y_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xy_y_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xz_x_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xz_x_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xz_z_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xz_z_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("yz_y_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("yz_y_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("yz_z_min"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("yz_z_max"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xyz_pitch"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);
  g_signal_connect(accessWidget("xyz_yaw"), "value-changed", G_CALLBACK(&EditConstraint::_onSpinButtonValueChanged), this);

  g_signal_connect(accessWidget("addKey"), "clicked", G_CALLBACK(&EditConstraint::_onAddKey), this);
  g_signal_connect(accessWidget("removeKey"), "clicked", G_CALLBACK(&EditConstraint::_onRemoveKey), this);

  // Setup key list view
  GtkCellRenderer* keyCelRenderer;
  GtkTreeViewColumn* keyCol;
  GtkTreeView* keyTreeView = GTK_TREE_VIEW(accessWidget("keyOrientation"));

  GtkListStore* keyModel = gtk_list_store_new(3, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER);
  gtk_tree_view_set_model(keyTreeView, GTK_TREE_MODEL(keyModel));
  keyCelRenderer = gtk_cell_renderer_text_new();
  keyCol = gtk_tree_view_column_new_with_attributes("Yaw", keyCelRenderer, "text", COLUMN_KEY_YAW, NULL);
  gtk_tree_view_append_column(keyTreeView, keyCol);
  keyCelRenderer = gtk_cell_renderer_text_new();
  keyCol = gtk_tree_view_column_new_with_attributes("Pitch", keyCelRenderer, "text", COLUMN_KEY_PITCH, NULL);
  gtk_tree_view_column_set_sort_column_id(keyCol, COLUMN_KEY_YAW);
  gtk_tree_view_append_column(keyTreeView, keyCol);

  GtkTreeSelection* keySelection = gtk_tree_view_get_selection(keyTreeView);
  g_signal_connect(keySelection,"changed", G_CALLBACK(&EditConstraint::_onSelectKey), this);

}

EditConstraint::~EditConstraint(void) {

}

/*
*Affichage de la boite d'edition en fonction de la contrainte presente sur l'articulation
*/
void
EditConstraint::fill(ArRef<JointController> jointCtrl) {
  if(_jointCtrl) {
    _jointCtrl->removeValueCB(thisRef(), &EditConstraint::_onValueChangedCB);
    _jointCtrl->removeSelectCB(thisRef(), &EditConstraint::_onSelectJointCB);
    _jointCtrl->removeChangeMotionCB(thisRef(), &EditConstraint::_onChangeMotionCB);
  }

  _jointCtrl = jointCtrl;

  _joint = _jointCtrl->accessSelectedJoint();

  //test s'il y a une articulation
  if(_joint == Joint::nullRef()) {
    gtk_widget_set_sensitive(accessWidget("dockSrc"), false);
  } else {
    gtk_widget_set_sensitive(accessWidget("dockSrc"), true);
  }

  ArRef<JointConstraint> jointConstraint = _jointCtrl->getJointConstraint();
  _jointCtrl->getAxisValues(AXIS_X, _xmin, _xmax);
  _jointCtrl->getAxisValues(AXIS_Y, _ymin, _ymax);
  _jointCtrl->getAxisValues(AXIS_Z, _zmin, _zmax);

  _jointCtrl->addValueCB(thisRef(), &EditConstraint::_onValueChangedCB);
  _jointCtrl->addSelectCB(thisRef(), &EditConstraint::_onSelectJointCB);
  _jointCtrl->addChangeMotionCB(thisRef(), &EditConstraint::_onChangeMotionCB);

  GtkWidget* w = NULL;
  if(jointConstraint != JointConstraint::nullRef()) {
    //cas contrainte 1DOF
    if(jointConstraint->getClass()->isA(JointConstraint1DOF::CLASS())) {
      switch(ar_down_cast<JointConstraint1DOF>(jointConstraint)->getAxisType()) {
      case JointConstraint1DOF::AXIS_X :
	w = accessWidget("x");
	break;
      case JointConstraint1DOF::AXIS_Y :
	w = accessWidget("y");
	break;
      case JointConstraint1DOF::AXIS_Z :
	w = accessWidget("z");
	break;
      default :
	break;
      }
    } else  if(jointConstraint->getClass()->isA(JointConstraint2DOF::CLASS())) {
      //cas contrainte 2DOF
      switch(ar_down_cast<JointConstraint2DOF>(jointConstraint)->getPlaneType()) {
      case JointConstraint2DOF::PLANE_XY :
	w = accessWidget("xy");
	break;
      case JointConstraint2DOF::PLANE_XZ :
	w = accessWidget("xz");
	break;
      case JointConstraint2DOF::PLANE_YZ :
	w = accessWidget("yz");
	break;
      default :
	break;
      }
      
    } else  if(jointConstraint->getClass()->isA(JointConstraint3DOF::CLASS())) {
      //cas contrainte 3DOF
      w = accessWidget("xyz");
    } else if(jointConstraint->getClass()->isA(JointConstraintLocked::CLASS())) {
      //cas contrainte bloque
      w = accessWidget("locked");
    }
  }
  else{ w = accessWidget("none"); }

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), true);

  //sauvegarde si changement detecte
  bool save = _undoRedo->testSave(_jointCtrl->accessSelectedJoint()->accessSkeleton());
  if(save) {
    _undoRedo->saveState(_jointCtrl);
  }

}

/*
*Fonction reflexe en cas de changement des valeurs de la contrainte
*/
void
EditConstraint::_onValueChangedCB(const JointController::ValueEvent& evt) {
  if(!_editing) {
    _editing = true;

    switch(evt.axis) {
    case AXIS_X :
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("x_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("x_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xy_x_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xy_x_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xz_x_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xz_x_max")), evt.out);
      break;
    case AXIS_Y :
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("y_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("y_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xy_y_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xy_y_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("yz_y_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("yz_y_max")), evt.out);
      break;
    case AXIS_Z :
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("z_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("z_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xz_z_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("xz_z_max")), evt.out);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("yz_z_min")), evt.in);
      gtk_spin_button_set_value(GTK_SPIN_BUTTON(accessWidget("yz_z_max")), evt.out);
      break;
    default :
      break;
    }

    _editing = false;
  }
}

/*
*Fonction reflexe sur la selection d'une articulation
*/
void
EditConstraint::_onSelectJointCB(const JointController::SelectEvent& /* evt */) {

  fill(_jointCtrl);

}

/*
*Fonction reflexe sur le changement d'une KO pour les contrainte 3DOF
*/
void
EditConstraint::_onChangeMotionCB(const JointController::ChangeMotionEvent& evt) {
  
  if( evt.key != KeyOrientation::nullRef() && !_spin) {
    _updateKeyListView();
  }

}

/*
*Selection du type de contrainte
*/
void
EditConstraint::_onConstraintToggled(GtkToggleButton* button, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;

  gint page_num = 0;

  if(GTK_WIDGET(button) == e->accessWidget("x")) {
    page_num = 0;
    //    ArRef<JointConstraint1DOF> jointConstraint = new_JointConstraint1DOF();
    ArRef<JointConstraint1DOF> jointConstraint = JointConstraint1DOF::NEW();
    jointConstraint->setAxisType(JointConstraint1DOF::AXIS_X);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("y")) {
    page_num = 1;
    //    ArRef<JointConstraint1DOF> jointConstraint = new_JointConstraint1DOF();
    ArRef<JointConstraint1DOF> jointConstraint = JointConstraint1DOF::NEW();
    jointConstraint->setAxisType(JointConstraint1DOF::AXIS_Y);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("z")) {
    page_num = 2;
    //    ArRef<JointConstraint1DOF> jointConstraint = new_JointConstraint1DOF();
    ArRef<JointConstraint1DOF> jointConstraint = JointConstraint1DOF::NEW();
    jointConstraint->setAxisType(JointConstraint1DOF::AXIS_Z);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("xy")) {
    page_num = 3;
    //    ArRef<JointConstraint2DOF> jointConstraint = new_JointConstraint2DOF();
    ArRef<JointConstraint2DOF> jointConstraint = JointConstraint2DOF::NEW();
    jointConstraint->setPlaneType(JointConstraint2DOF::PLANE_XY);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("xz")) {
    page_num = 4;
    //    ArRef<JointConstraint2DOF> jointConstraint = new_JointConstraint2DOF();
    ArRef<JointConstraint2DOF> jointConstraint = JointConstraint2DOF::NEW();
    jointConstraint->setPlaneType(JointConstraint2DOF::PLANE_XZ);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("yz")) {
    page_num = 5;
    //    ArRef<JointConstraint2DOF> jointConstraint = new_JointConstraint2DOF();
    ArRef<JointConstraint2DOF> jointConstraint = JointConstraint2DOF::NEW();
    jointConstraint->setPlaneType(JointConstraint2DOF::PLANE_YZ);
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("xyz")) {
    page_num = 6;
    //    ArRef<JointConstraint3DOF> jointConstraint = new_JointConstraint3DOF();
    ArRef<JointConstraint3DOF> jointConstraint = JointConstraint3DOF::NEW();
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("locked")) {
    page_num = 7;
    //    ArRef<JointConstraintLocked> jointConstraint = new_JointConstraintLocked();
    ArRef<JointConstraintLocked> jointConstraint = JointConstraintLocked::NEW();
    e->_jointCtrl->setJointConstraint(jointConstraint);
  } else if(GTK_WIDGET(button) == e->accessWidget("none")) {
    page_num = 8;
    e->_jointCtrl->setJointConstraint(JointConstraint::nullRef());
  }

  gtk_widget_set_sensitive(e->accessWidget("notebook"), true);
  gtk_notebook_set_current_page(GTK_NOTEBOOK(e->accessWidget("notebook")), page_num);

  e->_jointCtrl->removeValueCB(ArRef<EditConstraint>(e), &EditConstraint::_onValueChangedCB);
  e->_jointCtrl->removeSelectCB(ArRef<EditConstraint>(e), &EditConstraint::_onSelectJointCB);

}


/*
*Initialisation des valeurs des boutons de reglages des contraintes
*/
void
EditConstraint::_fillNotebookPage(GtkNotebook* /* notebook */, GtkNotebookPage* /* page */, guint /* page_num */, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;

  double xmin, xmax, ymin, ymax, zmin, zmax;
  //recuperation des valeurs
  e->_jointCtrl->getAxisValues(AXIS_X, xmin, xmax);
  e->_jointCtrl->getAxisValues(AXIS_Y, ymin, ymax);
  e->_jointCtrl->getAxisValues(AXIS_Z, zmin, zmax);

  e->_editing = true;

  if(e->_jointCtrl->getJointConstraint() != JointConstraint::nullRef()){
    if(e->_jointCtrl->getJointConstraint()->getClass()->isA(JointConstraint1DOF::CLASS())) {
      //cas d'une contrainte 1DOF
      switch(ar_down_cast<JointConstraint1DOF>(e->_jointCtrl->getJointConstraint())->getAxisType()) {
      case JointConstraint1DOF::AXIS_X :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("x_min")), xmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("x_max")), xmax);
	e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);
	break;
      case JointConstraint1DOF::AXIS_Y :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("y_min")), ymin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("y_max")), ymax);
	e->_jointCtrl->setAxisValues(AXIS_Y, ymin, ymax);
	break;
      case JointConstraint1DOF::AXIS_Z :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("z_min")), zmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("z_max")), zmax);
	e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
	break;
      default :
	break;
      }
    }
    
    if(e->_jointCtrl->getJointConstraint()->getClass()->isA(JointConstraint2DOF::CLASS())) {
      //cas d'une contrainte 2DOF
      switch(ar_down_cast<JointConstraint2DOF>(e->_jointCtrl->getJointConstraint())->getPlaneType()) {
      case JointConstraint2DOF::PLANE_XY :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xy_x_min")), xmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xy_x_max")), xmax);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xy_y_min")), ymin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xy_y_max")), ymax);
	e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);
	e->_jointCtrl->setAxisValues(AXIS_Y, ymin, ymax);
	break;
      case JointConstraint2DOF::PLANE_XZ :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_x_min")), xmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_x_max")), xmax);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_z_min")), zmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_z_max")), zmax);
	e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);
	e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
	break;
      case JointConstraint2DOF::PLANE_YZ :
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_x_min")), xmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_x_max")), xmax);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_z_min")), zmin);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xz_z_max")), zmax);
	e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);
	e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
	break;
      default :
	break;
      }
    }
    
    if(e->_jointCtrl->getJointConstraint()->getClass()->isA(JointConstraint3DOF::CLASS())) {
      //cas d'une contrainte 3DOF
      e->_updateKeyListView();
    }
    
    if(e->_jointCtrl->getJointConstraint()->getClass()->isA(JointConstraintLocked::CLASS())) {
    }
  }
  else{}
  
  e->_editing = false;
}

/*
*Gestion des reglages des valeurs de contraintes
*/
void
EditConstraint::_onSpinButtonValueChanged(GtkSpinButton*, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;
  e->_spin = true;

  if(!e->_editing) {
    GtkNotebook* notebook = GTK_NOTEBOOK(e->accessWidget("notebook"));

    double xmin, xmax, ymin, ymax, zmin, zmax;
    GtkSpinButton* low;
    GtkSpinButton* up;
    GtkTreeView* keyTreeView = GTK_TREE_VIEW(e->accessWidget("keyOrientation"));
    GtkTreeSelection* selection = gtk_tree_view_get_selection(keyTreeView);
    GtkTreeModel* model = gtk_tree_view_get_model(keyTreeView);
    GtkListStore* store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    StlList<ArRef<KeyOrientation> > keys;
    StlList<Util3D::Dbl3 > lKO;
    StlVector<Util3D::Dbl3 > vKO;
    ArRef<KeyOrientation> key;

    //Gestion selon le type de contrainte
    switch(gtk_notebook_get_current_page(notebook)) {
    case 0 :
      low = GTK_SPIN_BUTTON(e->accessWidget("x_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("x_max"));
      xmin = gtk_spin_button_get_value(low);
      xmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, xmin, up, xmax);
      e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);
      break;
    case 1 :
      low = GTK_SPIN_BUTTON(e->accessWidget("y_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("y_max"));
      ymin = gtk_spin_button_get_value(low);
      ymax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, ymin, up, ymax);
      e->_jointCtrl->setAxisValues(AXIS_Y, ymin, ymax);
      break;
    case 2 :
      low = GTK_SPIN_BUTTON(e->accessWidget("z_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("z_max"));
      zmin = gtk_spin_button_get_value(low);
      zmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, zmin, up, zmax);
      e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
      break;
    case 3 :
      low = GTK_SPIN_BUTTON(e->accessWidget("xy_x_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("xy_x_max"));
      xmin = gtk_spin_button_get_value(low);
      xmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, xmin, up, xmax);
      e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);

      low = GTK_SPIN_BUTTON(e->accessWidget("xy_y_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("xy_y_max"));
      ymin = gtk_spin_button_get_value(low);
      ymax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, ymin, up, ymax);
      e->_jointCtrl->setAxisValues(AXIS_Y, ymin, ymax);
      break;
    case 4 :
      low = GTK_SPIN_BUTTON(e->accessWidget("xz_x_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("xz_x_max"));
      xmin = gtk_spin_button_get_value(low);
      xmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, xmin, up, xmax);
      e->_jointCtrl->setAxisValues(AXIS_X, xmin, xmax);

      low = GTK_SPIN_BUTTON(e->accessWidget("xz_z_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("xz_z_max"));
      zmin = gtk_spin_button_get_value(low);
      zmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, zmin, up, zmax);
      e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
      break;
    case 5 :
      low = GTK_SPIN_BUTTON(e->accessWidget("yz_y_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("yz_y_max"));
      ymin = gtk_spin_button_get_value(low);
      ymax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, ymin, up, ymax);
      e->_jointCtrl->setAxisValues(AXIS_Y, ymin, ymax);

      low = GTK_SPIN_BUTTON(e->accessWidget("yz_z_min"));
      up = GTK_SPIN_BUTTON(e->accessWidget("yz_z_max"));
      zmin = gtk_spin_button_get_value(low);
      zmax = gtk_spin_button_get_value(up);

      e->_adjustSpinner(low, zmin, up, zmax);
      e->_jointCtrl->setAxisValues(AXIS_Z, zmin, zmax);
      break;
    case 6 :
      low = GTK_SPIN_BUTTON(e->accessWidget("xyz_yaw"));
      up = GTK_SPIN_BUTTON(e->accessWidget("xyz_pitch"));
      ymin = gtk_spin_button_get_value(low);
      ymax = gtk_spin_button_get_value(up);

      gtk_tree_selection_get_selected(selection, &model, &iter);

      gtk_list_store_set(store, &iter, COLUMN_KEY_YAW, (int)ymin, COLUMN_KEY_PITCH, (int)ymax, -1);

      e->_extractKeyOrientation(lKO, vKO);

      e->_jointCtrl->accessC3DOF()->getKeyOrientation() = lKO;
      e->_jointCtrl->accessC3DOF()->updateShape();
      ar_down_cast<JointConstraint3DOF>(e->_jointCtrl->getJointConstraint())->accessKeyOrientation() = vKO;
      ar_down_cast<JointConstraint3DOF>(e->_jointCtrl->getJointConstraint())->applyChanges();

      break;
    default :
      break;
    }
    e->_jointCtrl->removeValueCB(ArRef<EditConstraint>(e), &EditConstraint::_onValueChangedCB);
    e->_jointCtrl->removeSelectCB(ArRef<EditConstraint>(e), &EditConstraint::_onSelectJointCB);
  }
  e->_spin = false;
}

/*
*Ajustement des valeurs de contraintes 1DOF et 2DOF (max forcement superieur a min et vice et versa)
*/
void
EditConstraint::_adjustSpinner(GtkSpinButton* spinLow, double low, GtkSpinButton* spinUp, double up) {
  _editing = true;

  if(low > up) {
    low = up;
    gtk_spin_button_set_value(spinLow, low);
  }

  if(up < low) {
    up = low;
    gtk_spin_button_set_value(spinUp, up);
  }

  _editing = false;
}

/*
*Recuperation des coordonnees des KO en radian
*/
void
EditConstraint::_extractKeyOrientation(StlList<Util3D::Dbl3 >& list, StlVector<Util3D::Dbl3 >& vector) {
  list.clear();
  vector.clear();
  double x, y, z;
  double yaw, pitch;
  int iyaw, ipitch;
  GtkTreeIter iter;
  GtkTreeView* keyTreeView = GTK_TREE_VIEW(accessWidget("keyOrientation"));
  GtkTreeModel* keyModel = gtk_tree_view_get_model(keyTreeView);
  bool ok = gtk_tree_model_get_iter_first(keyModel, &iter);
  while(ok) {
    gtk_tree_model_get(keyModel, &iter, COLUMN_KEY_YAW, &iyaw, -1);
    gtk_tree_model_get(keyModel, &iter, COLUMN_KEY_PITCH, &ipitch, -1);
    if(iyaw > 180) {iyaw -= 360;}
    if(ipitch > 180) {ipitch -= 360;}
    yaw = (double)iyaw * M_PI / 180.0;
    pitch = (double)ipitch * M_PI / 180.0;
    sphericalToCart3D(x, y, z, 1.0, yaw, pitch);
    Util3D::Dbl3 dblTmp;
    dblTmp.x = x;
    dblTmp.y = y;
    dblTmp.z = z;
    list.push_back(dblTmp);
    vector.push_back(dblTmp);
    ok = gtk_tree_model_iter_next(keyModel, &iter);
  }
}

/*
*Ajout d'une KO
*/
void
EditConstraint::_onAddKey(GtkButton*, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;
  e->_jointCtrl->accessC3DOF()->addKeyOrientation();
  e->_updateKeyListView();
}

/*
*Suppression d'une KO
*/
void
EditConstraint::_onRemoveKey(GtkButton*, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;
  e->_jointCtrl->accessC3DOF()->removeKeyOrientation();
  e->_updateKeyListView();
}

/*
*Selection d'une KO
*/
void
EditConstraint::_onSelectKey(GtkTreeSelection* selection, gpointer data) {
  EditConstraint* e = (EditConstraint*)data;
  
  GtkTreeModel* model;
  GtkTreeIter iter;

  if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
    KeyOrientation* kPtr;
    gtk_tree_model_get(model, &iter, COLUMN_KEY_DATA, &kPtr, -1);
    //Selection  sur la fenetre de visualisation
    e->_jointCtrl->accessC3DOF()->unselect();
    e->_jointCtrl->accessC3DOF()->select(ArRef<KeyOrientation>(kPtr));
    int yaw, pitch;
    //Recuperation des donnees
    gtk_tree_model_get(model, &iter, COLUMN_KEY_YAW, &yaw, -1);
    gtk_tree_model_get(model, &iter, COLUMN_KEY_PITCH, &pitch, -1);
    //Initialisation des valeurs pour leurs editions
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xyz_yaw")), (double)yaw);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->accessWidget("xyz_pitch")), (double)pitch);
  }
}

/*
*Regeneration de la liste des KO
*/
void 
EditConstraint::_updateKeyListView(void){
  GtkTreeIter iter;
  GtkTreeView* keyTreeView = GTK_TREE_VIEW(accessWidget("keyOrientation"));
  GtkListStore* keyModel = GTK_LIST_STORE(gtk_tree_view_get_model(keyTreeView));
  
  gtk_list_store_clear(keyModel);
  
  StlList<Util3D::Dbl3 > keyOrientations = _jointCtrl->accessC3DOF()->getKeyOrientation();
  StlVector<Util3D::Dbl2 > angleKeys;
  
  for(StlList<Util3D::Dbl3 >::iterator itDbl = keyOrientations.begin() ; itDbl != keyOrientations.end() ; itDbl++) {
    double yaw, pitch, dist;
    cart3DToSpherical(dist, yaw, pitch, itDbl->x, itDbl->y, itDbl->z);
    Util3D::Dbl2 dblTmp;
    dblTmp.x = pitch;
    dblTmp.y = yaw;
    angleKeys.push_back(dblTmp);
  }
  
  StlList<ArRef<KeyOrientation> > keys = _jointCtrl->accessC3DOF()->accessKeyOrientation();
  StlList<ArRef<KeyOrientation> >::iterator it = keys.begin();

  for(size_t i = 0 ; i < angleKeys.size() ; i++) {
    double pitch = angleKeys[i].x * 180/M_PI;
    double yaw = angleKeys[i].y * 180/M_PI;
    if(pitch < 0) { pitch += 360; }
    if(yaw < 0){ yaw += 360; }
    gtk_list_store_append(keyModel, &iter);
    gtk_list_store_set(keyModel, &iter, COLUMN_KEY_YAW, (int)yaw, COLUMN_KEY_PITCH, (int)pitch, COLUMN_KEY_DATA, (*it).c_ptr(), -1);
    it++;
  }
  
}

