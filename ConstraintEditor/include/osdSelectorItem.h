#ifndef _OSD_SELECTOR_ITEM_H_
#define _OSD_SELECTOR_ITEM_H_

#include "AReVi/Lib3D/osdSprite.h"
#include "AReVi/Lib3D/texture.h"
using namespace AReVi;

class OSDSelector;

class OSDSelectorItem : public OSDSprite {
public :
  AR_CLASS(OSDSelectorItem)
  AR_CONSTRUCTOR_1(OSDSelectorItem,
		   ArConstRef<Texture>, texture)

  AR_CALLBACK(OSDSelectorItem, Highlight, _highlightCB,
	      bool highlighted;
	      )

  AR_CALLBACK(OSDSelectorItem, Select, _selectCB,
	      bool selected;
	      )

protected :

  virtual
  void
  _onHighLight(void) const;

  virtual
  void
  _onDeHighLight(void) const;

  virtual
  void
  _onSelect(void) const;

  virtual
  void
  _onDeSelect(void) const;

  virtual
  void
  _highlight(bool highlight);

  virtual
  void
  _select(bool select);

protected :
  friend class OSDSelector;

  CallbackManager<OSDSelectorItem, HighlightEvent> _highlightCB;
  CallbackManager<OSDSelectorItem, SelectEvent> _selectCB;
};

#endif // _OSD_SELECTOR_ITEM_H_
