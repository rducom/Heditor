/*
*
*Gestion de la l'IHM permettant l'edition des ancres
*
*/

#include "hLib/core/joint.h"

#include "editAnchor.h"

AR_CLASS_DEF(EditAnchor, GtkAReViWidget)

EditAnchor::EditAnchor(ArCW& arCW) 
: GtkAReViWidget(arCW, "./data/ihm/edit_anchor.glade", "edit_anchor") {
  GtkListStore* model;
  GtkCellRenderer* renderer;
  GtkComboBox* combo;
  model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
  //initialisation de la boite de dialogue
  combo = GTK_COMBO_BOX(accessWidget("link"));
  renderer = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, true);
  gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combo), renderer, "text", COLUMN_LINK_NAME);
  gtk_combo_box_set_model(combo, GTK_TREE_MODEL(model));
}

EditAnchor::~EditAnchor(void) {
}

/*
*Remplissage de la boite de dialogue
*/
void
EditAnchor::fill(ArRef<Anchor> anchor, ArRef<Body> body) {
  GtkTreeIter iter;
  GtkComboBox* combo = GTK_COMBO_BOX(accessWidget("link"));
  GtkListStore* model =  GTK_LIST_STORE(gtk_combo_box_get_model(combo));
  gtk_list_store_clear(model);

  //recuperation de la liste des articulation ainsi que leur nom
  int id = 0;
  for(int i = 0; i < body->getNbJoints(); i++) {
    gtk_list_store_append(model, &iter);
    gtk_list_store_set(model, &iter, COLUMN_LINK_NAME, body->getJoint(i)->getName().c_str(), COLUMN_LINK_DATA, body->getJoint(i).c_ptr(), -1);
    if(anchor.valid() && anchor->getLinkedTo() == body->getJoint(i)) {
      id = i;
    }
  }
  gtk_combo_box_set_active(combo, id);

  if(anchor.valid()) {
    gtk_entry_set_text(GTK_ENTRY(accessWidget("name")), anchor->getName().c_str());
    
  } else {
    gtk_entry_set_text(GTK_ENTRY(accessWidget("name")), "no name");
  }
}
