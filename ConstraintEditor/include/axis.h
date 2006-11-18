#ifndef _AXIS_H_
#define _AXIS_H_

#include "AReVi/arObject.h"
#include "AReVi/Lib3D/osdSprite.h"
#include "AReVi/Lib3D/renderer3D.h"
using namespace AReVi;

#include "types.h"

class AxisButton;

class Axis : public ArObject {
public :
  AR_CLASS(Axis)
  AR_CONSTRUCTOR_1(Axis,
		   AxisType, axisType)

  virtual
  void
  addRenderer(ArRef<Renderer3D> renderer);

  virtual
  void
  removeRenderer(ArRef<Renderer3D> renderer);

  virtual
  unsigned int
  getNbRenderer(void) const;

  virtual
  ArConstRef<Renderer3D>
  getRenderer(unsigned int index) const;

  virtual
  ArRef<Renderer3D>
  accessRenderer(unsigned int index);

  virtual
  void
  setVisible(bool visible);

  virtual
  void
  setSize(unsigned int w, unsigned int h);

  virtual
  void
  getSize(unsigned int& w, unsigned int& h) const;

  virtual
  void
  setButtonSize(unsigned int w, unsigned int h);

  virtual
  void
  getButtonSize(unsigned int& w, unsigned int& h) const;

  virtual
  void
  setPosition(int x, int y);

  virtual
  void
  setBounds(double min, double max);

  virtual
  void
  getBounds(double& min, double& max) const;

  virtual
  void
  setButtonValues(double in, double out);

  virtual
  void
  getButtonValues(double& in, double& out) const;

  virtual
  void
  attachTo(ArRef<Axis> axis);
  
  AR_CALLBACK(Axis, Value, _valueCB,
	      double in;
	      double out;
	      )

protected :

  virtual
  void
  _onAxisMotionCB(const OSD::MotionEvent& evt);

  virtual
  void
  _onButtonMotionCB(const OSD::MotionEvent& evt);

  virtual
  void
  _onValueChanged(void);

protected :
  StlVector<ArRef<Renderer3D> > _renderers;
 
  ArRef<OSDSprite> _axis;
  ArRef<AxisButton> _inOsd;
  ArRef<AxisButton> _outOsd;

  unsigned int _sw, _sh, _bw, _bh;
  double _rangeMin, _rangeMax;
  double _valueIn, _valueOut;
  int _motionMask;

  CallbackManager<Axis, ValueEvent> _valueCB;
};

#endif // _AXIS_H_
