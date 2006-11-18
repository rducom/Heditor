/*
*
*Classe de controle de l'humanoide
*
*/

#include "AReVi/arClass.h"

#include "bodyController.h"

AR_CLASS_DEF(BodyController, ArObject)

BodyController::BodyController(ArCW& arCW)
: ArObject(arCW),
  _body(),
  _bodyCB(thisRef()) {
}

BodyController::~BodyController(void) {
}


/*
*Initialise l'humanoide
*/
void
BodyController::setBody(ArRef<Body> body) {
  _body = body;
  _onBody();
}

/*
*Retourne une reference constante sur l'humanoide
*/
ArConstRef<Body>
BodyController::getBody(void) const {
  return _body;
}

/*
*Retourne une reference sur l'humanoide
*/
ArRef<Body>
BodyController::accessBody(void) {
  return _body;
}

/*
*Emet le signal de  modification de l'humanoide
*/
void
BodyController::_onBody(void) {
  if(!_bodyCB.empty()) {
    BodyEvent evt;
    evt.body = _body;
    _bodyCB.fire(evt);
  }
}
