#ifndef _BODY_CONTROLLER_H_
#define _BODY_CONTROLLER_H_

#include "AReVi/arObject.h"
using namespace AReVi;

namespace hLib {
class Body;
}
using namespace hLib;

/**
 * Classe décrivant le contrôle du modèle chargé
 */
class BodyController : public ArObject {
public :
  AR_CLASS(BodyController)
  AR_CONSTRUCTOR(BodyController)

  // Body management

  /**
   * Classe permettant d'affecter un modèle au BodyController
   * @param body Le modèle que doit contrôler le BodyController
   */
  virtual void setBody(ArRef<Body> body);

  /**
   * Méthode permettant de récupérer en consultation le modèle que contrôle le BodyController
   * @return Le modèle
   */
  virtual ArConstRef<Body> getBody(void) const;

  /**
   * Méthode permettant de récupérer le modèle que contrôle le BodyController
   * @return Le modèle
   */
  virtual ArRef<Body> accessBody(void);

  AR_CALLBACK(BodyController, Body, _bodyCB, ArRef<Body> body; )

protected :

  /**
   * Méthode déclenchée lors d'un changement du modèle
   * @param  
   */
  virtual void _onBody(void);

protected :
  /** Le modèle chargé */
  ArRef<Body> _body;
  /** Gestionnaire des évènements sur le modèle */
  CallbackManager<BodyController, BodyEvent> _bodyCB;

};

#endif // _BODY_CONTROLLER_H_
