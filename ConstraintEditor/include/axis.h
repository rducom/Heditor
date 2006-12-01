#ifndef _AXIS_H_
#define _AXIS_H_

#include "AReVi/arObject.h"
#include "AReVi/Lib3D/osdSprite.h"
#include "AReVi/Lib3D/renderer3D.h"
using namespace AReVi;

#include "types.h"

class AxisButton;

/**
 * Classe décrivant les axes de réglages des contraintes imprimés en mode OSD
 */
class Axis : public ArObject {
public :
  AR_CLASS(Axis)
  AR_CONSTRUCTOR_1(Axis, AxisType, axisType)

  /**
   * Méthode permettant d'ajouter un rendu pour l'axe
   * @param renderer Le rendu éà ajouter
   */
  virtual void addRenderer(ArRef<Renderer3D> renderer);

  /**
   * Méthode permettant d'enlever un rendu pour l'axe
   * @param renderer Le rendu à enlever
   */
  virtual void removeRenderer(ArRef<Renderer3D> renderer);

  /**
   * Méthode permettant de retourner le nombre de rendu de l'Axis
   * @return Le nombre de rendu de l'Axis
   */
  virtual unsigned int getNbRenderer(void) const;

  /**
   * Méthode permettant de récupérer un rendu en consultation dont l'indice est passé en paramètre
   * @param index Indice du rendu à retourner
   * @return Le rendu
   */
  virtual ArConstRef<Renderer3D> getRenderer(unsigned int index) const;

  /**
   * Méthode permettant de récupérer un rendu dont l'indice est passé en paramètre
   * @param index Indice du rendu à retourner
   * @return Le rendu
   */
  virtual ArRef<Renderer3D> accessRenderer(unsigned int index);

  /**
   * Méthode permettant de détermier si l'Axis est visible ou non
   * @param visible true si l'Axis doit être visible, flase sinon
   */
  virtual void setVisible(bool visible);

  /**
   * Méthode permettant de définir la taille de l'Axis
   * @param w Largeur de l'Axis
   * @param h Hauteur de l'Axis
   */
  virtual void setSize(unsigned int w, unsigned int h);

  /**
   * Méthode permettant de recupérer la taille de l'Axis
   * @param w Largeur de l'Axis
   * @param h Hauteur de l'Axis
   */
  virtual void getSize(unsigned int& w, unsigned int& h) const;

  /**
   * Méthode permettant de définir la taille des bouttons de l'Axis
   * @param w Largeur des bouttons
   * @param h Hauteur des bouttons
   */
  virtual void setButtonSize(unsigned int w, unsigned int h);

  /**
   * Méthode permettant de récupérer la taille des bouttons de l'Axis
   * @param w Largeur des bouttons
   * @param h Hauteur des bouttons
   */
  virtual void getButtonSize(unsigned int& w, unsigned int& h) const;

  /**
   * Méthode permettant de positionner l'Axis dans la fenêtre ARéVi
   * @param x Position en X
   * @param y Position en Y
   */
  virtual void setPosition(int x, int y);

  /**
   * Méthode permettant de définir les valeur maximum et minimum de l'Axis
   * @param min Valeur minimale de l'Axis
   * @param max Valeur maximale de l'Axis
   */
  virtual void setBounds(double min, double max);

  /**
   * Méthode permettant de récupérer les valeur maximum et minimum de l'Axis
   * @param min Valeur minimale de l'Axis
   * @param max Valeur maximale de l'Axis
   */
  virtual void getBounds(double& min, double& max) const;

  /**
   * Méthode permettant d'affecter des valeurs aux boutons
   * @param in Valeur du bouton inférieur
   * @param out Valeur du bouton supérieur
   */
  virtual void setButtonValues(double in, double out);

  /**
   * Méthode permettant de récupérer des valeurs aux boutons
   * @param in Valeur du bouton inférieur
   * @param out Valeur du bouton supérieur
   */
  virtual void getButtonValues(double& in, double& out) const;

  /**
   * Méthode permettant d'attacher l'Axis à une autre
   * @param axis L'Axis à laquelle attacher celle-ci
   */
  virtual void attachTo(ArRef<Axis> axis);
  
  AR_CALLBACK(Axis, Value, _valueCB,
	      double in;
	      double out;
	      )

protected :

  /**
   * Méthode de déclenchement sur le déplacement de l'Axis
   * @param evt Evènement de déplacement
   */
  virtual void _onAxisMotionCB(const OSD::MotionEvent& evt);

  /**
   * Méthode de déclenchement sur le déplacement d'un boutton
   * @param evt Evènement de déplacement d'un boutton
   */
  virtual void _onButtonMotionCB(const OSD::MotionEvent& evt);

  /**
   * Méthode de déclenchement sur un changement de valeur de l'Axis
   */
  virtual void _onValueChanged(void);

protected :
  /** Vector des rendus */
  StlVector<ArRef<Renderer3D> > _renderers;
 
  /** Représentation de l'Axis en OSD */
  ArRef<OSDSprite> _axis;
  /** Boutton inférieur */
  ArRef<AxisButton> _inOsd;
  /** Boutton supérieur */
  ArRef<AxisButton> _outOsd;

  /** ... je sais plus du tout */
  unsigned int _sw, _sh, _bw, _bh;
  /** Valeurs maximale et minimale */
  double _rangeMin, _rangeMax;
  /** Valeurs inférieure et supérieure */
  double _valueIn, _valueOut;
  /** Surement un truc à voir avec le déplacement de l'Axis ou de ses bouttons */
  int _motionMask;

  /** Gestionnaire de déclencheur sur un changement de valeur */
  CallbackManager<Axis, ValueEvent> _valueCB;
};

#endif // _AXIS_H_
