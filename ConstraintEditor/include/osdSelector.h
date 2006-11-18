#ifndef _OSD_SELECTOR_H_
#define _OSD_SELECTOR_H_

#include <queue>
using namespace std;

#include "AReVi/Lib3D/osd.h"
#include "AReVi/Lib3D/renderer3D.h"
#include "AReVi/Lib3D/abstractInteractor.h"
#include "AReVi/Lib3D/window3D.h"
using namespace AReVi;

class ConstraintInteractor;
class OSDSelectorItem;

class OSDSelector : public OSD {
public :
  AR_CLASS(OSDSelector)
  AR_CONSTRUCTOR(OSDSelector)

// -----------------------------------------------------------------
// OSD management
// -----------------------------------------------------------------

  virtual
  void
  setPosition(int x,
              int y);

// -----------------------------------------------------------------
// Items management
// -----------------------------------------------------------------

  virtual
  unsigned int 
  getNbItems(void) const;

  virtual
  void
  addItem(ArRef<OSDSelectorItem> item);

  virtual
  void
  removeItem(ArRef<OSDSelectorItem> item);

  virtual
  void
  clearItems(void);

  virtual
  ArConstRef<OSDSelectorItem>
  getItem(unsigned int index) const;

  virtual
  ArRef<OSDSelectorItem>
  accessItem(unsigned int index);

  AR_CALLBACK(OSDSelector, Select, _selectCB,
	      ArRef<OSDSelectorItem> selected;
	      )
 
  virtual
  void
  setItemMaxSize(unsigned int width, unsigned int height);

// -----------------------------------------------------------------
// Selection management
// -----------------------------------------------------------------
 
  virtual
  void
  selectItem(ArRef<OSDSelectorItem> item);

  virtual
  ArConstRef<OSDSelectorItem>
  getSelectedItem(void) const;

  virtual
  ArRef<OSDSelectorItem>
  accessSelectedItem(void);

  virtual
  void
  selectNext(void);

  virtual
  void
  selectPrevious(void);

  virtual
  void
  setSelectedItemZoomFactor(float zoom);

  virtual
  float
  getSelectedItemZoomFactor(void) const;

protected :

  virtual
  void
  _addRenderer(ArPtr<Renderer3D> renderer);

  virtual
  void
  _removeRenderer(ArPtr<Renderer3D> renderer);

  virtual
  void
  _getSize(unsigned int rendererWidth,
	   unsigned int rendererHeight,
	   unsigned int& widthOut,
	   unsigned int& heightOut) const;

  virtual 
  void 
  _render(unsigned int width,
          unsigned int height) const;

  virtual
  void
  _onSelectItem(int current,
		int previous);

  virtual
  void
  _timeOutCB(const Window3D::TimeOutEvent& evt);

  virtual
  void
  _updateItemsSize(void);

  virtual
  void
  _rebuild(void);

  virtual
  void
  _reallyRemoveRenderer(void);

protected :
  ArRef<Window3D> _window;
  unsigned long _timeOut;
  unsigned long _timeOutDelay;

  StlVector<ArRef<OSDSelectorItem> > _items;
  StlVector<pair<unsigned int, unsigned int> > _initialSize;
  int _selectedItem;
  int _zoomedItem;
  unsigned int _itemMaxWidth;
  unsigned int _itemMaxHeight;
  float _zoomFactor;

  queue<bool> _cmd;
  double _rotationDt;

  bool _mustHide;

  CallbackManager<OSDSelector, SelectEvent> _selectCB;
};

#endif // _OSD_SELECTOR_H_
