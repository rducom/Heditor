/*
*
*Gestion des vue multiples et unique
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/simpleInteractor.h"
#include "gtkWindowPlugger.h"
#include "multiView.h"


AR_CLASS_DEF(MultiView, ArObject)

MultiView::MultiView(ArCW& arCW) : ArObject(arCW), _singleView(false) {
  
  for(size_t i = 0; i < 5; i++) {
    _pluggers[i] = GtkWindowPlugger::NEW();
  }
  
  _multiView = gtk_table_new(2, 2, true);
  gtk_table_set_col_spacings(GTK_TABLE(_multiView), 2);
  gtk_table_set_row_spacings(GTK_TABLE(_multiView), 2);
  for(size_t i = 0; i < 2; i++) {
    for(size_t j = 0; j < 2; j++) {
      gtk_table_attach_defaults(GTK_TABLE(_multiView), _pluggers[i * 2 + j]->getWidget(), j, j + 1, i, i + 1);
      gtk_widget_show(_pluggers[i * 2 + j]->getWidget());
    }
  }
  
  _singleView = _pluggers[PERSPECTIVE_VIEW]->getWidget();

}

MultiView::~MultiView(void) {
}

/*
*initialisation des vues
*/
void
MultiView::initViews(void) {

  for(size_t i = 0; i < 5; i++) {
    _views[i] = EmbeddedWindow3D::NEW(_pluggers[i]->getWidgetID());
    _views[i]->setOriginVisible(true);
    
    _osd[i] = OSDText::NEW();
    _osd[i]->setColor(0.0, 1.0, 0.0, 1.0);
    _osd[i]->setLocation(OSD::LOC_TOP_LEFT);
    _osd[i]->setPadding(0.01, 0.01);
    _views[i]->addOSD(_osd[i]);

    _pluggers[i]->setWindow(_views[i]);
  }

  _views[RIGHT_VIEW]->setOrientation(0.0, 0.0, - M_PI_2);
  _views[TOP_VIEW]->setOrientation(0.0, M_PI_2, 0.0);

  _views[PERSPECTIVE_VIEW]->attachTo(_views[PERSPECTIVE_VIEW_MULTI]);
  _views[PERSPECTIVE_VIEW_MULTI]->attachTo(_views[PERSPECTIVE_VIEW]);

  _osd[0]->setText("Front");
  _osd[1]->setText("Right");
  _osd[2]->setText("Top");
  _osd[3]->setText("Perspective");
  _osd[4]->setText("Perspective");

}

/*
*retourne le widget des vues multiples
*/
GtkWidget*
MultiView::getMultiViewWidget(void) {
  return _multiView;
}

/*
*retourne le widget de la vue unique
*/
GtkWidget*
MultiView::getSingleViewWidget(void) {
  return _singleView;
}

ArRef<EmbeddedWindow3D>
MultiView::getWindow(Views v) {
  return _views[v];
}

/*
*ajustement des vues par rapport a la taille de l'humanoide
*/
void
MultiView::adjustViews(ArRef<BoundingBox3D> bbox) {

  double x, y, z, sx, sy, sz;
  bbox->getCenter(x, y, z);
  bbox->getSize(sx, sy, sz);


  if(sz > sy) {_views[FRONT_VIEW]->setPosition(x - sz, y, z);}
  else {_views[FRONT_VIEW]->setPosition(x - sy, y, z);}

  if(sx > sz) {_views[RIGHT_VIEW]->setPosition(x, y + sx, z);}
  else {_views[RIGHT_VIEW]->setPosition(x, y + sz, z);}

  if(sx > sy) {_views[TOP_VIEW]->setPosition(x, y, z + 1.8*sx);}
  else {_views[TOP_VIEW]->setPosition(x, y, z + 1.8*sy);}

  if(sz > sy) {_views[PERSPECTIVE_VIEW]->setPosition(x - 1.2*sz, y, z);}
  else {_views[PERSPECTIVE_VIEW]->setPosition(x - 1.2*sy, y, z);}

  _views[PERSPECTIVE_VIEW]->cancelRotation();
  
  for(size_t i = 0; i < 5; i++) {
    _views[i]->setFarDistance(3.0 * bbox->getMaxSize());
    _views[i]->setNearDistance(0.001 * _views[i]->getFarDistance());
  }
}

