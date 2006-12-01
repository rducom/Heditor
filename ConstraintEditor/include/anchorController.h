#ifndef _ANCHOR_CONTROLLER_H_
#define _ANCHOR_CONTROLLER_H_

#include "AReVi/arObject.h"
#include "AReVi/Lib3D/material3D.h"
#include "AReVi/Shapes/sphere3D.h"
using namespace AReVi;

namespace hLib {
class Anchor;
}
using namespace hLib;

/**
 * Classe permettant la gestion des ancres sur la vue ARéVi
 */
class AnchorController : public ArObject {
public :
  AR_CLASS(AnchorController)
  AR_CONSTRUCTOR(AnchorController)

  /**
   * 
   */
  virtual void postChanges(void);

  /**
   * Méthode permettant de retourner le nombre d'ancres du modèle chargé
   * @return Le nombre d'ancres du modèle chargé
   */
  virtual size_t getNbAnchors(void) const;

  /**
   * Méthode permettant d'ajouter une ancre au modèle chargé
   * @param anchor L'ancre à ajouter
   */
  virtual void addAnchor(ArRef<Anchor> anchor);

  /**
   * Méthode permettant d'enlever une ancre au modèle chargé
   * @param anchor L'ancre à enlever
   */
  virtual void removeAnchor(ArRef<Anchor> anchor);
  
  /**
   * Méthode permettant de supprimer toutes les ancres du modèle chargé
   */
  virtual void clearAnchors(void);

  /**
   * Méthode permettant de récupérer en consultation une ancre dont l'indice est passé en paramètre
   * @param id L'indice de l'ancre
   * @return L'ancre
   */
  virtual ArConstRef<Anchor> getAnchor(size_t id) const;

  /**
   * Méthode permettant de récupérer en une ancre dont l'indice est passé en paramètre
   * @param id L'indice de l'ancre
   * @return L'ancre
   */
  virtual ArRef<Anchor> accessAnchor(size_t id);

  AR_CALLBACK(AnchorController, Anchor, _anchorCB, StlVector<ArRef<Anchor> > anchors; )

  /**
   * Méthode permettant de surligné une ancre dans la vue ARéVi
   * @param anchor L'ancre à surligner
   */
  virtual void highLightAnchor(ArRef<Anchor> anchor);

  /**
   * Méthode permettant de récupérer l'ancre surlignée en consultation 
   * @return L'ancre surlignée
   */
  virtual ArConstRef<Anchor> getHighLightedAnchor(void) const;

  /**
   * Méthode permettant de récupérer l'ancre surlignée
   * @return L'ancre surlignée
   */
  virtual ArRef<Anchor> accessHighLightedAnchor(void);

  AR_CALLBACK(AnchorController, HighLight, _highLightCB,
	      ArRef<Anchor> anchor;
	      )

protected :

  /**
   * Méthode décrivant l'action résultante de la manipulation d'une ancre
   */
  virtual void _onAnchor(void);

  /**
   * Méthode décrivant l'action résultante du surlignage d'une ancre
   * @param  
   */
  virtual void _onHighLight(void);

protected :
  /** Vector des ancres du modèle */
  StlVector<ArRef<Anchor> > _anchors;
  /** Ancre surlignée */
  ArRef<Anchor> _highLighted;

  /** Sphère représentative des ancres */
  ArRef<Sphere3D> _sphere;
  /** Texture de la sphère */
  ArRef<Material3D> _material;

  /** Gestionnaire de Callback pour les actions sur les ancres */
  CallbackManager<AnchorController, AnchorEvent> _anchorCB;
  /** Gestionnaire de Callback pour le surlignage des ancres */
  CallbackManager<AnchorController, HighLightEvent> _highLightCB;

};

#endif // _ANCHOR_CONTROLLER_H_
