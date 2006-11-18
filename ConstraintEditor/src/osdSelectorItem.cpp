#include "AReVi/arClass.h"

#include "osdSelectorItem.h"

AR_CLASS_NOVOID_DEF(OSDSelectorItem, OSDSprite)

OSDSelectorItem::OSDSelectorItem(ArCW& arCW,
				 ArConstRef<Texture> texture) 
: OSDSprite(arCW, texture),
  _highlightCB(thisRef()),
  _selectCB(thisRef()) {
  setMouseButtonInteraction(false);
  setMotionInteraction(false);
}

OSDSelectorItem::~OSDSelectorItem(void) {
}

void
OSDSelectorItem::_onHighLight(void) const {
}

void
OSDSelectorItem::_onDeHighLight(void) const {
}

void
OSDSelectorItem::_onSelect(void) const {
}

void
OSDSelectorItem::_onDeSelect(void) const {
}

void
OSDSelectorItem::_highlight(bool highlight) {
  if(highlight) {
    _onHighLight();
  } else {
    _onDeHighLight();
  }

  if(!_highlightCB.empty()) {
    HighlightEvent evt;
    evt.highlighted = highlight;
    _highlightCB.fire(evt);
  }
}

void
OSDSelectorItem::_select(bool select) {
  if(select) {
    _onSelect();
  } else {
    _onDeSelect();
  }

  if(!_selectCB.empty()) {
    SelectEvent evt;
    evt.selected = select;
    _selectCB.fire(evt);
  }
}
