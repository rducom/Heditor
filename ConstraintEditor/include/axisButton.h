#ifndef _AXIS_BUTTON_H_
#define _AXIS_BUTTON_H_

#include "AReVi/Lib3D/texture.h"
#include "AReVi/Lib3D/abstractInteractor.h"
#include "AReVi/Lib3D/osdText.h"
#include "AReVi/Lib3D/osdSprite.h"
using namespace AReVi;

#include "types.h"

class AxisButton : public OSDSprite {
public :
  AR_CLASS(AxisButton)
  AR_CONSTRUCTOR_2(AxisButton,
		   ArRef<Texture>, texture,
		   bool, textUp)

  virtual
  void
  setValue(double value);

  virtual
  void
  onMouseButtonInteraction(ArRef<AbstractInteractor> source,
                           int button,
                           bool pressed);

  virtual
  void
  onMotionInteraction(ArRef<AbstractInteractor> source,
		      int newX,
		      int newY,
                      bool begin,
                      bool end);

protected :

  virtual
  void
  _onSetupCB(const OSD::SetupEvent& evt);

protected :
  double _value;

  ArRef<OSDText> _text; 
  bool _textUp;
};

#endif // _AXIS_BUTTON_H_
