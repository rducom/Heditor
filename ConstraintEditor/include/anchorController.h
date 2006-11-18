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

class AnchorController : public ArObject {
public :
  AR_CLASS(AnchorController)
  AR_CONSTRUCTOR(AnchorController)
    //AR_CONSTRUCTOR_1(AnchorController,ArRef<UndoRedo>,undoRedo)

  // Anchors management

  virtual
  void
  postChanges(void);

  virtual
  size_t
  getNbAnchors(void) const;

  virtual
  void
  addAnchor(ArRef<Anchor> anchor);

  virtual
  void
  removeAnchor(ArRef<Anchor> anchor);

  virtual
  void
  clearAnchors(void);

  virtual
  ArConstRef<Anchor>
  getAnchor(size_t id) const;

  virtual
  ArRef<Anchor>
  accessAnchor(size_t id);

  AR_CALLBACK(AnchorController, Anchor, _anchorCB, StlVector<ArRef<Anchor> > anchors; )

  virtual
  void
  highLightAnchor(ArRef<Anchor> anchor);

  virtual
  ArConstRef<Anchor>
  getHighLightedAnchor(void) const;

  virtual
  ArRef<Anchor>
  accessHighLightedAnchor(void);

  AR_CALLBACK(AnchorController, HighLight, _highLightCB,
	      ArRef<Anchor> anchor;
	      )

protected :

  virtual
  void
  _onAnchor(void);

  virtual
  void
  _onHighLight(void);

protected :
  StlVector<ArRef<Anchor> > _anchors;
  ArRef<Anchor> _highLighted;

  ArRef<Sphere3D> _sphere;
  ArRef<Material3D> _material;

  CallbackManager<AnchorController, AnchorEvent> _anchorCB;
  CallbackManager<AnchorController, HighLightEvent> _highLightCB;

  //ArRef<UndoRedo> _undoRedo;
};

#endif // _ANCHOR_CONTROLLER_H_
