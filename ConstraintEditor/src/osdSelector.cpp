#include "AReVi/arClass.h"

#include "osdSelector.h"
#include "osdSelectorItem.h"

AR_CLASS_DEF(OSDSelector, OSD)

OSDSelector::OSDSelector(ArCW& arCW)
: OSD(arCW),
  _window(),
  _timeOut(0),
  _timeOutDelay(20),
  _selectedItem(0),
  _zoomedItem(0),
  _itemMaxWidth(32),
  _itemMaxHeight(32),
  _zoomFactor(1.33),
  _rotationDt(0.0),
  _mustHide(false),
  _selectCB(thisRef()) {
  setLocation(OSD::LOC_FREE);
}

OSDSelector::~OSDSelector(void) {
}

void
OSDSelector::setPosition(int x,
			 int y) {
  OSD::setPosition(x, y);
  _rebuild();
}

// -----------------------------------------------------------------
// Items management
// -----------------------------------------------------------------

unsigned int 
OSDSelector::getNbItems(void) const {
  return _items.size();
}

void
OSDSelector::addItem(ArRef<OSDSelectorItem> item) {
  for(size_t i = _items.size(); i--;) {
    if(_items[i] == item) {
      return;
    }
  }

  item->resizeWithRenderer(false);
  item->setLocation(OSD::LOC_FREE);
  _items.push_back(item);

  if(_items.size() == 1) {
    selectItem(item);
  }

  _updateItemsSize();
}

void
OSDSelector::removeItem(ArRef<OSDSelectorItem> item) {
  for(int i = _items.size(); i--;) {
    if(_items[i] == item) {
      StlVectorFastErase(_items, i);
      /*
      if(_items.empty()) {
	_selectedItem = 0;
      } else {
	if(_selectedItem == i) {
	  selectItem(_items[0]);
	}
      }
      */
      _selectedItem = 0;
      if(_window) {
	_window->removeOSD(item);
	_rebuild();
      }
      return;
    }
  }
}

void
OSDSelector::clearItems(void) {
  while(getNbItems()) {
    removeItem(accessItem(0));
  }
}

ArConstRef<OSDSelectorItem>
OSDSelector::getItem(unsigned int index) const {
  return _items[index];
}

ArRef<OSDSelectorItem>
OSDSelector::accessItem(unsigned int index) {
  return _items[index];
}

void
OSDSelector::setItemMaxSize(unsigned int width, unsigned int height) {
  _itemMaxWidth = width;
  _itemMaxHeight = height;
  _updateItemsSize();
}

// -----------------------------------------------------------------
// Selection management
// -----------------------------------------------------------------

void
OSDSelector::selectItem(ArRef<OSDSelectorItem> item) {
  for(size_t i = _items.size(); i--;) {
    if(_items[i] == item) {
      _onSelectItem(i, _selectedItem);
      return;
    }
  }
}

ArConstRef<OSDSelectorItem>
OSDSelector::getSelectedItem(void) const {
  if(_items.empty()) {
    return OSDSelectorItem::nullRef();
  } else {
    return _items[_selectedItem];
  }
}

ArRef<OSDSelectorItem>
OSDSelector::accessSelectedItem(void) {
  if(_items.empty()) {
    return OSDSelectorItem::nullRef();
  } else {
    return _items[_selectedItem];
  }
}

void
OSDSelector::selectNext(void) {
  if(_items.empty()) {
    return;
  }

  _cmd.push(true);

  if(_window && _cmd.size() == 1) {
    _timeOut = _window->postTimeOut(_timeOutDelay);
  }
}

void
OSDSelector::selectPrevious(void) {
  if(_items.empty()) {
    return;
  }

  _cmd.push(false);

  if(_window && _cmd.size() == 1) {
    _timeOut = _window->postTimeOut(_timeOutDelay);
  }
}

void
OSDSelector::setSelectedItemZoomFactor(float zoom) {
  _zoomFactor = zoom;
  _rebuild();
}

float
OSDSelector::getSelectedItemZoomFactor(void) const {
  return _zoomFactor;
}

// -----------------------------------------------------------------
// Protected operations
// -----------------------------------------------------------------

void
OSDSelector::_addRenderer(ArPtr<Renderer3D> renderer) {
  if(!_window &&
     renderer->getClass()->isA(Window3D::CLASS())) {
    for(size_t i = _items.size(); i--;) {
      renderer->addOSD(_items[i]);
    }
    OSD::_addRenderer(renderer);
    _mustHide = false;

    _window = ar_down_cast<Window3D>(renderer);
    _window->addTimeOutCB(thisRef(),
			  &OSDSelector::_timeOutCB);
  }
}

void
OSDSelector::_removeRenderer(ArPtr<Renderer3D> /* renderer */) {
  if(_cmd.empty()) {
    _reallyRemoveRenderer();
  } else {
    _mustHide = true;
  }
}

void
OSDSelector::_getSize(unsigned int /* rendererWidth */,
		      unsigned int /* rendererHeight */,
		      unsigned int& widthOut,
		      unsigned int& heightOut) const {
  widthOut = 0;
  heightOut = 0;
}

void 
OSDSelector::_render(unsigned int /* width */,
		     unsigned int /* height */) const {
}

void
OSDSelector::_onSelectItem(int current,
			   int previous) {
  _selectedItem = current;

  _items[previous]->_select(false);
  _items[current]->_select(true);

  if(!_selectCB.empty()) {
    SelectEvent evt;
    evt.selected = _items[_selectedItem];
    _selectCB.fire(evt);
  }
}

void
OSDSelector::_timeOutCB(const Window3D::TimeOutEvent& evt) {
  if(evt.timeOut != _timeOut) {
    return;
  }

  int sgn = -1;
  if(_cmd.front()) {
    sgn = 1;
  }

  _rotationDt += sgn * M_2PI / _timeOutDelay;
  if(fabs(_rotationDt) >  M_2PI / (float)_items.size()) {
    _rotationDt = 0.0;
    _items[_zoomedItem]->_highlight(false);
    _zoomedItem = _zoomedItem - sgn;
    if(_zoomedItem == (int)_items.size()) _zoomedItem = 0;
    if(_zoomedItem < 0) _zoomedItem = _items.size() - 1;
    _cmd.pop();
    _items[_zoomedItem]->_highlight(true);
  }

  if(_cmd.empty()) {
    _window->cancelTimeOut(_timeOut);
    _timeOut = 0;
    
    if(_mustHide) {
      _reallyRemoveRenderer();
    }
  } else {
    _timeOut = _window->postTimeOut(_timeOutDelay);
  }

  _rebuild();
}

void
OSDSelector::_updateItemsSize(void) {
  _initialSize.resize(_items.size());

  for(size_t i = _items.size(); i--;) {
    float itemRatio = _items[i]->getRatio();
    unsigned int width, height;
    
    if(itemRatio == 0.0) {
      _items[i]->getSize(Renderer3D::nullRef(), width, height);
      itemRatio = (float)width / (float)height;
      
      if(width > _itemMaxWidth) {
	width = _itemMaxWidth;
	height = (unsigned int)(width / itemRatio);
      }
      
      if(height > _itemMaxHeight) {
	height = _itemMaxHeight;
	width = (unsigned int)(height * itemRatio);
      }
    } else {
      width = _itemMaxWidth;
      height = _itemMaxHeight;
    }
    
    _items[i]->setSize(width, height);
    _items[i]->setDepth(0.0);
    _initialSize[i] = make_pair(width, height);
  }

  _rebuild();
}

void
OSDSelector::_rebuild(void) {
  if(_items.empty()) {
    return;
  }

  int radius = (int)(max(4, (int)_items.size()) * 
		     max(_itemMaxWidth, _itemMaxHeight) * 
		     0.25);

  float deltaAngle = M_2PI / (float)_items.size();

  StlVector<float> angles(_items.size());
  for(int i = _items.size(); i--;) {
    int idx = (i - _zoomedItem) < 0 ? 
      i - _zoomedItem + _items.size() : 
      i - _zoomedItem;    

    angles[i] = 3.0 * M_PI / 2.0 + idx * deltaAngle + _rotationDt;
    if(angles[i] > M_2PI) angles[i] -= M_2PI;

    _items[i]->setSize(_initialSize[i].first, 
		       _initialSize[i].second);
  }

  for(size_t i = _items.size(); i--;) {
    float zoomFactor = 1.0;
    float depth = 0.0;
    float delta = fabs(angles[i] - 3.0 * M_PI / 2.0);

    if(delta < M_PI / 3.0) {
      delta = (M_PI / 3.0) - delta;
      zoomFactor += delta * (_zoomFactor - 1.0) / (M_PI / 3.0);
      depth += delta * 3.0 / M_PI;
      _items[i]->setSize((unsigned int)((float)_initialSize[i].first * zoomFactor),
			 (unsigned int)((float)_initialSize[i].second * zoomFactor));
      _items[i]->setDepth(depth);
    }

    unsigned int w = (unsigned int)((float)_initialSize[i].first * zoomFactor);
    unsigned int h = (unsigned int)((float)_initialSize[i].second * zoomFactor);

    _items[i]->setPosition((int)(_x + radius * cos(angles[i]) - (float)w * 0.5),
			   (int)(_y + radius * sin(angles[i]) - (float)h * 0.5));
  }
}

void
OSDSelector::_reallyRemoveRenderer(void) {
  for(size_t i = _items.size(); i--;) {
    _window->removeOSD(_items[i]);
  }
  OSD::_removeRenderer(_window);      
  
  _window->removeTimeOutCB(thisRef(), 
			   &OSDSelector::_timeOutCB);
  _window = Window3D::nullRef();

  if(!_items.empty()) {
    _onSelectItem(_zoomedItem,
		  _selectedItem);
  }
}
