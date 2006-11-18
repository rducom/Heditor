/*
*
*Classe de controlle des ancres de l'humanoide
*
*/


#include "AReVi/arClass.h"
#include "AReVi/Shapes/shapePart3D.h"
#include "AReVi/Shapes/surface3D.h"

#include "hLib/core/anchor.h"
#include "hLib/core/joint.h"

#include "anchorController.h"

AR_CLASS_DEF(AnchorController, ArObject)

AnchorController::AnchorController(ArCW& arCW)
: ArObject(arCW),
  _anchors(),
  _highLighted(),
  _material(),
  _anchorCB(thisRef()),
  _highLightCB(thisRef()){
}

AnchorController::~AnchorController(void) {
}

void
AnchorController::postChanges(void) {
  _onAnchor();
}


/*
*retourne le nombre d'ancres du squelette
*/
size_t
AnchorController::getNbAnchors(void) const {
  return _anchors.size();
}


/*
*Ajoute l'ancre passée en parametre au squelette
*/
void
AnchorController::addAnchor(ArRef<Anchor> anchor) {
  _anchors.push_back(anchor);
  _onAnchor();
}


/*
*Supprime l'ancre du squelette passée en paramtre
*/
void
AnchorController::removeAnchor(ArRef<Anchor> anchor) {
  for(size_t i = _anchors.size(); i--;) {
    if(_anchors[i] == anchor) {
      anchor->accessLinkedTo()->removeSubPart(anchor);
      if(_highLighted == anchor) {
	_highLighted = Anchor::nullRef();
      }
      StlVectorFastErase(_anchors, i);
      _onAnchor();
    }
  }
}


/*
*Supprime toutes les ancres du squelette
*/
void
AnchorController::clearAnchors(void) {
  for(size_t i = _anchors.size(); i--;) {
    if(_anchors[i]) {
      if(_anchors[i]->accessLinkedTo()) {
	_anchors[i]->accessLinkedTo()->removeSubPart(_anchors[i]);
      }
    }
  }
  _anchors.clear();
  _onAnchor();
}


/*
*retourne une reference constante sur l'ancre placée a l'index id
*/
ArConstRef<Anchor>
AnchorController::getAnchor(size_t id) const {
  return _anchors[id];
}


/*
*Retourne une reference sur l'ancre placee a l'index id
*/
ArRef<Anchor>
AnchorController::accessAnchor(size_t id) {
  return _anchors[id];
}

/*
*Surligne l'ancre passee en parametre dans la fenetre de visualisation AReVi
*/
void
AnchorController::highLightAnchor(ArRef<Anchor> anchor) {
  if(_highLighted.valid()) {
    _sphere->writeMaterial(_material);
  }

  _highLighted = anchor;

  if(!_highLighted.valid()) {
    return;
  }

  //initialisation de la couleur a appliquer
  //  ArRef<Material3D> mhl = new_Material3D();
  ArRef<Material3D> mhl = Material3D::NEW();
  mhl->setTransparency(0.666);
  mhl->setDiffuseColor(0.8,0.0,0.0);
  mhl->setEmissiveColor(0.25,0.0,0.0);
  mhl->setAmbientIntensity(0.4);

  //changement de couleur de l'ancre
  for(size_t i = 0; i < _highLighted->getNbSubParts(); i++) {
    ArRef<ShapePart3D> subPart = _highLighted->accessSubPart(i);
    if(subPart->getClass()->isA(Sphere3D::CLASS())) {
      _sphere = ar_down_cast<Sphere3D>(subPart);

      //      _material = new_Material3D();
      _material = Material3D::NEW();
      _sphere->readMaterial(_material);
      _sphere->writeMaterial(mhl);
    }
  }

  _onHighLight();
}

/*
*Retourne une reference constante sur l'ancre surlignee
*/
ArConstRef<Anchor>
AnchorController::getHighLightedAnchor(void) const {
  return _highLighted;
}

/*
*retourne une reference sur l'ancre surlignee
*/
ArRef<Anchor>
AnchorController::accessHighLightedAnchor(void) {
  return _highLighted;
}

/* 
*lance le signal de CallBack de l'evenement de manipulation d'ancre
*/
void
AnchorController::_onAnchor(void) {
  if(!_anchorCB.empty()) {
    AnchorEvent evt;
    evt.anchors = _anchors;
    _anchorCB.fire(evt);
  }
}

/*
*Emet le signal de Callback de l'evenement de surlignage d'ancre
*/
void
AnchorController::_onHighLight(void) {
  if(!_highLightCB.empty()) {
    HighLightEvent evt;
    evt.anchor = _highLighted;
    _highLightCB.fire(evt);
  }
}
