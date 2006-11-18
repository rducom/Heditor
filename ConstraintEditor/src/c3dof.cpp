/*
*
*Classe de gestion de la representation des contraintes a trois degres de liberte
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/scene3D.h"
#include "AReVi/Contrib/arMath.h"

#include "c3dof.h"
#include "c3dofSh.h"
#include "keyOrientation.h"

AR_CLASS_NOVOID_DEF(C3DOF, Object3D)

C3DOF::C3DOF(ArCW& arCW)
: Object3D(arCW),
  _changeCB(thisPtr()),
  //  _sh(new_C3DOFSh()) {
  _sh(C3DOFSh::NEW()) {
  setShape(_sh);

  setRadius(1.0);

  // Create 3 defaults key orientations
  StlVector<Util3D::Dbl3> pos;
  pos.push_back(Util3D::Dbl3(1.0, 0.0, 0.0));
  pos.push_back(Util3D::Dbl3(0.0, 1.0, 0.0));
  pos.push_back(Util3D::Dbl3(0.0, 0.0, 1.0));

  for(size_t i = 0; i < pos.size(); i++) {
    _localKO.push_back(Util3D::Dbl3(0.0, 0.0, 0.0));
    
    //    _keyOrientation.push_back(new_KeyOrientation(thisRef()));
    _keyOrientation.push_back(KeyOrientation::NEW(thisRef()));
    _keyOrientation.back()->addMotionCB(thisRef(), &C3DOF::_keyOrientationMotionCB);
    _keyOrientation.back()->setPosition(pos[i].x, pos[i].y, pos[i].z);
  }
  
  select(_keyOrientation.front());
  
  _updateShape();
}

C3DOF::C3DOF(ArCW& arCW, ArRef<JointConstraint3DOF> constraint) 
: Object3D(arCW),
  _changeCB(thisPtr()), 
  //  _sh(new_C3DOFSh()) {
  _sh(C3DOFSh::NEW()) {
  setShape(_sh);

  setRadius(1.0);

  StlVector<Util3D::Dbl3>& pos = constraint->accessKeyOrientation();

  if( pos.size() < 3 ) {
    //si il y a moins de 3 KO, la contrainte n'est pas valide et on initialise des valeurs par defaut
    pos.clear();
    pos.push_back(Util3D::Dbl3(1.0, 0.0, 0.0));
    pos.push_back(Util3D::Dbl3(0.0, 1.0, 0.0));
    pos.push_back(Util3D::Dbl3(0.0, 0.0, 1.0));
    for(size_t i = 0; i < pos.size(); i++) {
      _localKO.push_back(Util3D::Dbl3(0.0, 0.0, 0.0));
      
      //      _keyOrientation.push_back(new_KeyOrientation(thisRef()));
      _keyOrientation.push_back(KeyOrientation::NEW(thisRef()));
      _keyOrientation.back()->addMotionCB(thisRef(), &C3DOF::_keyOrientationMotionCB);
      _keyOrientation.back()->setPosition(pos[i].x, pos[i].y, pos[i].z);
    }
  }
  else {
    //si la contrainte est valide, on recupere ses KO pour la representation
     for(size_t i = 0; i < pos.size(); i++) {
       _localKO.push_back(pos[i]);
       
       //       _keyOrientation.push_back(new_KeyOrientation(thisRef()));
       _keyOrientation.push_back(KeyOrientation::NEW(thisRef()));
       _keyOrientation.back()->addMotionCB(thisRef(), &C3DOF::_keyOrientationMotionCB);
       _keyOrientation.back()->setPosition(pos[i].x, pos[i].y, pos[i].z);
     }
  }

  select(_keyOrientation.front());
  
  _updateShape();

}

C3DOF::~C3DOF(void) {
}

/*
*Initialise le rayon de la representation de la contrainte 3DOF
*/
void
C3DOF::setRadius(double radius) {
  if(radius <= 0.0) {
    return;
  }

  _radius = radius;
  double keyRadius = radius / 5.0;

  for(StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin(); it != _keyOrientation.end(); it++) {
    //    ArRef<Base3D> b = new_Base3D();
    ArRef<Base3D> b = Base3D::NEW();
    b->setLocation(*it);
    globalToLocalLocation(b);
    double x, y, z;
    b->getPosition(x, y, z);
    Vector3d v(x, y, z);
    v.normalize();
    v *= _radius;
    b->setPosition(v.x(), v.y(), v.z());
    localToGlobalLocation(b);
    (*it)->setLocation(b);
    (*it)->setRadius(keyRadius);
  }
  _updateShape();
}

/*
*Retourne le rayon de la representation de la contrainte 3DOF
*/
double
C3DOF::getRadius(void) const {
  return _radius;
}

/*
*ajoute une KO a la contrainte
*/
void
C3DOF::addKeyOrientation(void) {
  size_t in = _selected;

  //  ArRef<Base3D> b = new_Base3D();
  ArRef<Base3D> b = Base3D::NEW();
  double x, y, z;

  StlList<ArRef<KeyOrientation> >::iterator itIn, itOut;
  itIn = _keyOrientation.begin();

  StlList<Util3D::Dbl3>::iterator itInLKO, itOutLKO;  
  itInLKO = _localKO.begin();

  for(size_t i = 0; i < in; i++) {
    itIn++;
    itInLKO++;
  }

  itOut = itIn;
  itOut++;
  if(itOut == _keyOrientation.end()) {
    itOut = _keyOrientation.begin();
  }

  itOutLKO = itInLKO;
  itOutLKO++;

  b->setLocation(*itIn);
  globalToLocalLocation(b);
  b->getPosition(x, y, z);
  Vector3d inVec(x, y, z);

  b->setLocation(*itOut);
  globalToLocalLocation(b);
  b->getPosition(x, y, z);
  Vector3d outVec(x, y, z);

  Vector3d tmp = lerp(inVec, outVec, 0.5);
  tmp.normalize();
  tmp *= _radius;
  x = tmp.x();
  y = tmp.y();
  z = tmp.z();

  _localKO.insert(itOutLKO, Util3D::Dbl3(0.0, 0.0, 0.0));

  //  ArRef<KeyOrientation> ko = new_KeyOrientation(thisRef());
  ArRef<KeyOrientation> ko = KeyOrientation::NEW(thisRef());
  if(itOut == _keyOrientation.begin()) {
    itOut = _keyOrientation.end();
  }
  _keyOrientation.insert(itOut, ko);
  ko->addMotionCB(thisRef(), &C3DOF::_keyOrientationMotionCB);
  localToGlobalPosition(x, y, z);
  ko->setPosition(x, y, z);

  setRadius(_radius);
}

/*
*supprime une KO a la contrainte
*/
void
C3DOF::removeKeyOrientation(void) {
  if(_keyOrientation.size() == 3) {
    return;
  }

  StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin();
  StlList<Util3D::Dbl3>::iterator itLKO = _localKO.begin();
  for(int i = 0; i < _selected; i++) {
    it++;
    itLKO++;
  }

  _localKO.erase(itLKO);

  ArRef<KeyOrientation> ko = *it;

  (*it)->removeMotionCB(thisRef(), &C3DOF::_keyOrientationMotionCB);
  it = _keyOrientation.erase(it);
  if(it == _keyOrientation.end()) {
    it = _keyOrientation.begin();
  }
  select(*it);

  _updateShape();

  ko.destroy();
}

/*
*Retourne la liste des coordonnees des KO
*/
StlList<Util3D::Dbl3>&
C3DOF::getKeyOrientation(void) {
  return _localKO;
}

/*
*Retourne la liste des reference des KO
*/
StlList<ArRef<KeyOrientation> >
C3DOF::accessKeyOrientation(void) {
  return _keyOrientation;
}

/*
*Selection la KO passee en parametre
*/
void
C3DOF::select(ArRef<KeyOrientation> keyOrientation) {
  StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin();
  _selected = 0;
  while(*it != keyOrientation && it != _keyOrientation.end()) {
    it++;
    _selected++;
  }

  if(it != _keyOrientation.end()) {
    Scene3D::accessGlobalScene()->selectObject(*it);
  }
}

/*
*Annule la selection
*/
void
C3DOF::unselect(void) {
  StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin();
  for(int i = 0; i < _selected; i++) {
    it++;
  }

  if(it != _keyOrientation.end()) {
    Scene3D::accessGlobalScene()->unselectObject(*it);
  }
}

/*
*Retourne une reference constante sur la KO selectionnee
*/
ArConstRef<KeyOrientation>
C3DOF::getSelected(void) const {
  StlList<ArRef<KeyOrientation> >::const_iterator it = _keyOrientation.begin();
  for(int i = 0; i < _selected; i++) {
    it++;
  }
  return *it;
}

/*
*Retourne ue reference sur la KO selctionnee
*/
ArRef<KeyOrientation>
C3DOF::accessSelected(void) {  
  StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin();
  for(int i = 0; i < _selected; i++) {
    it++;
  }
  return *it;
}

/*
*Regenere la representation de la contrainte 3DOF
*/
void
C3DOF::updateShape(void) {
  StlList<ArRef<KeyOrientation> >::iterator itKey  = _keyOrientation.begin();
  for(StlList<Util3D::Dbl3 >::iterator it  = _localKO.begin(); it != _localKO.end() ; it++) {
    (*itKey)->setPosition(it->x, it->y, it->z);
    itKey++;
  }
  setRadius(_radius);
}

/*
*Fonction reflexe au mouvement d'une KO
*/
void
C3DOF::_keyOrientationMotionCB(const Base3D::MotionEvent& evt) {
  StlList<ArRef<KeyOrientation> >::iterator it = _keyOrientation.begin();
  StlList<Util3D::Dbl3>::iterator itLKO = _localKO.begin();

  while(*it != evt.source) {
    it++;
    itLKO++;
  }

  //  ArRef<Base3D> b = new_Base3D();
  ArRef<Base3D> b = Base3D::NEW();
  b->setLocation(evt.source);
  globalToLocalLocation(b);
  double x, y, z;
  b->getPosition(x, y, z);
  
  *itLKO = Util3D::Dbl3(x, y, z);

  _updateShape();
}

/*
*Regenre la representaion de la contrainte 3DOF
*/
void
C3DOF::_updateShape(void) {
  if(_keyOrientation.size() > 2) {
    _sh->setKeyOrientation(_localKO);
  }
  
  if(!_changeCB.empty()) {
    ChangeEvent evt;
    for(StlList<Util3D::Dbl3>::iterator it = _localKO.begin() ; it != _localKO.end() ; it++) {
      evt.keyOrientation.push_back(*it);
    }
    _changeCB.fire(evt);
  }
}
