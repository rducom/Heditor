#ifndef _C3DOF_H_
#define _C3DOF_H_

#include "AReVi/Lib3D/object3D.h"
using namespace AReVi;

#include "hLib/core/jointConstraint.h"
using namespace hLib;

class C3DOFSh;
class KeyOrientation;

/**
 * Classe permettant de controller graphiquement les articulations à trois degrés de liberté
 */
class C3DOF : public Object3D {
public :
  AR_CLASS(C3DOF)
  AR_CONSTRUCTOR(C3DOF)
  AR_CONSTRUCTOR_1(C3DOF, ArRef<JointConstraint3DOF>, constraint)

  /**
   * Méthode permettant de définir le rayon de la sphère principale
   * @param radius Le rayon
   */
  virtual void setRadius(double radius);

  /**
   * Méthode permettant de récupérer le rayon de la sphère principale
   * @return Le rayon
   */
  virtual double getRadius(void) const;

  /**
   * Méthode permettant de créer une nouvelle KeyOrientation entre celle sélectionnée et la suivante
   */
  virtual void addKeyOrientation(void);

  /**
   * Méthode permettant de supprimer la KeyOrientation sélectionnée
   */
  virtual void removeKeyOrientation(void);

  /**
   * Méthode retournant la liste des positions des KeyOrientation
   * @return La liste des positions des KeyOrientation
   */
  virtual StlList<Util3D::Dbl3>& getKeyOrientation(void);

  /**
   * Méthode retournant la liste des KeyOrientation
   * @return La liste des KeyOrientation
   */
  virtual StlList<ArRef<KeyOrientation> > accessKeyOrientation(void);

  AR_CALLBACK(C3DOF, Change, _changeCB, StlVector<Util3D::Dbl3> keyOrientation;)
    
  /**
   * Méthode permettant de sélectionnée une KeyOrientation
   * @param keyOrientation La KeyOrientation à sélectionner
   */
  virtual void select(ArRef<KeyOrientation> keyOrientation);

  /**
   * Méthode permettant de desélectionner la KeyOrientation actuellement sélectionnée
   */
  virtual void unselect(void);

  /**
   * Méthode permettant de récupérer en consultation la KeyOrientation sélectionnée
   * @return La KeyOrientation sélectionnée
   */
  virtual ArConstRef<KeyOrientation> getSelected(void) const;

  /**
   * Méthode permettant de récupérer la KeyOrientation sélectionnée
   * @return La KeyOrientation sélectionnée
   */
  virtual ArRef<KeyOrientation> accessSelected(void);

  /**
   * Méthode permettant de rafraichir la représentation graphique de la contrainte de l'articulation
   */
  virtual void updateShape(void);

protected :

  /**
   * Méthode déclenchée lors d'un mouvement d'une KeyOrientation
   * @param evt L'évènement de mouvement d'une KeyOrientation
   */
  virtual void _keyOrientationMotionCB(const Base3D::MotionEvent& evt);

  /**
   * Méthode déclenchée lors d'un rafraichissment de la représentation graphique de la contrainte de l'articulation
   */
  virtual void _updateShape(void);

protected :
  /** Rayon de la sphère principale */
  double _radius;
  /** Indice de la KeyOrientation sélectionnée */
  int _selected;
  /** Liste des KeyOrientation */
  StlList<ArRef<KeyOrientation> > _keyOrientation;
  /** Liste des positions des KeyOrientation */
  StlList<Util3D::Dbl3> _localKO;
  /** Gestionnaire des évènements sur les changements */
  CallbackManager<C3DOF, ChangeEvent> _changeCB;

  /** Forme de la représentation graphique de la contrainte de l'articulation */
  ArRef<C3DOFSh> _sh;
};

#endif // _C3DOF_H_
