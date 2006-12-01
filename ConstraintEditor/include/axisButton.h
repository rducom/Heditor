#ifndef _AXIS_BUTTON_H_
#define _AXIS_BUTTON_H_

#include "AReVi/Lib3D/texture.h"
#include "AReVi/Lib3D/abstractInteractor.h"
#include "AReVi/Lib3D/osdText.h"
#include "AReVi/Lib3D/osdSprite.h"
using namespace AReVi;

#include "types.h"

/**
 * Classe décrivant les buttons OSD représentant les axes dans la fenêtre ARéVi
 */
class AxisButton : public OSDSprite {
public :
  AR_CLASS(AxisButton)
  AR_CONSTRUCTOR_2(AxisButton, ArRef<Texture>, texture, bool, textUp)

  /**
   * Méthode permettant d'affecter une valeur pour l'axe
   * @param value 
   */
  virtual void setValue(double value);

  /**
   * Gestion de l'interaction sur un évènement boutton de souris
   * @param source Source de l'évènement
   * @param button Le bouton de la souris qui a été actionné
   * @param pressed Si le bouton a été pressé
   */
  virtual void onMouseButtonInteraction(ArRef<AbstractInteractor> source, int button, bool pressed);

  /**
   * Gestion de l'interaction de mouvement de la souris
   * @param source Source de l'évènement
   * @param newX Nouvelle valeur en X
   * @param newY Nouvelle valeur en Y
   * @param begin true si l'action commence, false sinon
   * @param end true si l'action termine, false sinon
   */
  virtual void onMotionInteraction(ArRef<AbstractInteractor> source, int newX, int newY, bool begin, bool end);

protected :

  /**
   * Méthode de déclenchement de l'action set de la valeur de AxisButton
   * @param evt L'évènement déclencheur
   */
  virtual void _onSetupCB(const OSD::SetupEvent& evt);

protected :
  /** Valeur de l'axe */
  double _value;

  /** Texte à afficher en mode OSD */
  ArRef<OSDText> _text; 
  /** Flag d'affichage du texte */
  bool _textUp;
};

#endif // _AXIS_BUTTON_H_
