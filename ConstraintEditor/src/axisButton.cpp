#include "AReVi/arClass.h"
#include "AReVi/Lib3D/boundingBox3D.h"

#include "axisButton.h"

AR_CLASS_NOVOID_DEF(AxisButton, OSDSprite)

AxisButton::AxisButton(ArCW& arCW,
		       ArRef<Texture> texture,
		       bool textUp)
: OSDSprite(arCW, texture),
  //  _text(new_OSDText()) {
  _text(OSDText::NEW()) {
  setLocation(OSD::LOC_FREE);
  resizeWithRenderer(false);
  setRatio(1.0);

  addSetupCB(thisRef(), &AxisButton::_onSetupCB);
  setValue(0.0);

  _text->setDepth(1.0);
  _text->setLocation(OSD::LOC_FREE);
  _text->setFont("9x15bold");
  _text->setMouseButtonInteraction(false);
  _text->setMotionInteraction(false);
  _text->attachTo(thisRef());
  _textUp = textUp;
}

AxisButton::~AxisButton(void) {
}

/*
*Change la valeur de du texte a afficher
*/
void
AxisButton::setValue(double value) {
  _value = value;
  _text->setText(intToStr((int)value));
}

/*
*Gestion des evenements sur click de souris
*/
void
AxisButton::onMouseButtonInteraction(ArRef<AbstractInteractor> /* source */,
				     int /* button */,
				     bool pressed) {
  if(pressed) {
    for(size_t i = _renderers.size(); i--;) {
      _renderers[i]->addOSD(_text);
    }
  } else {
    for(size_t i = _renderers.size(); i--;) {
      _renderers[i]->removeOSD(_text);
    }
  }
}



/*
*Gestion des evenement sur mouvement de souris
*/
void
AxisButton::onMotionInteraction(ArRef<AbstractInteractor> /* source */,
				int newX,
				int /* newY */,
				bool /* begin */,
				bool /* end */) {
  int x, y;
  getPosition(Renderer3D::nullRef(), x, y);
  setPosition(newX, y);
}

/*
*Initialisation de l'affichage sur evenement
*/
void
AxisButton::_onSetupCB(const OSD::SetupEvent& /* evt */) {
  unsigned int w, h;
  getSize(Renderer3D::nullRef(), w, h);

  int x, y;
  getPosition(Renderer3D::nullRef(), x, y);

  if(_textUp) {
    _text->setPosition(x, y - (int)h);
  } else {
    _text->setPosition(x, y + (int)h);
  }
}
