#ifndef _BODY_CONTROLLER_H_
#define _BODY_CONTROLLER_H_

#include "AReVi/arObject.h"
using namespace AReVi;

namespace hLib {
class Body;
}
using namespace hLib;

class BodyController : public ArObject {
public :
  AR_CLASS(BodyController)
  AR_CONSTRUCTOR(BodyController)

  // Body management

  virtual
  void
  setBody(ArRef<Body> body);

  virtual
  ArConstRef<Body>
  getBody(void) const;

  virtual
  ArRef<Body>
  accessBody(void);

  AR_CALLBACK(BodyController, Body, _bodyCB, ArRef<Body> body; )

protected :

  virtual
  void
  _onBody(void);

protected :
  ArRef<Body> _body;
  CallbackManager<BodyController, BodyEvent> _bodyCB;

  //UndoRedo* _undoRedo;
};

#endif // _BODY_CONTROLLER_H_
