#include "AReVi/arClass.h"
#include "AReVi/Lib3D/urlTexture.h"

#include "axis.h"
#include "axisButton.h"

AR_CLASS_NOVOID_DEF(Axis, ArObject)

Axis::Axis(ArCW& arCW, AxisType axisType) 
: ArObject(arCW),
  _renderers(),
  _axis(),
  _inOsd(),
  _outOsd(),
  _sw(160),
  _sh(16),
  _bw(24),
  _bh(24),
  _rangeMin(0.0),
  _rangeMax(100.0),
  _motionMask(0),
  _valueCB(thisRef()) {

  StlString inStr, outStr, slideStr;

  inStr = "data/in.png";
  outStr = "data/out.png";

  switch(axisType) {
  case AXIS_X :
    slideStr = "data/xaxis.png";
    break;
  case AXIS_Y :
    slideStr = "data/yaxis.png";
    break;
  case AXIS_Z :
    slideStr = "data/zaxis.png";
    break;
  default :
    break;
  }

  //  ArRef<URLTexture> slideTex = new_URLTexture(slideStr, true, true);
  ArRef<URLTexture> slideTex = URLTexture::NEW(slideStr, true, true);
  //  _axis = new_OSDSprite(slideTex);
  _axis = OSDSprite::NEW(slideTex);
  _axis->setLocation(OSD::LOC_FREE);
  _axis->resizeWithRenderer(false);
  _axis->setRatio(0.0);
  _axis->setSize(_sw, _sh);
  _axis->setDepth(-0.1);
  _axis->setMotionInteraction(false);
  _axis->addMotionCB(thisRef(), &Axis::_onAxisMotionCB);

  //  ArRef<URLTexture> inTex = new_URLTexture(inStr, true, true);
  ArRef<URLTexture> inTex = URLTexture::NEW(inStr, true, true);
  //  _inOsd = new_AxisButton(inTex, true);
  _inOsd = AxisButton::NEW(inTex, true);
  _inOsd->setSize(_bw, _bh);
  _inOsd->setDepth(0.1);
  _inOsd->addMotionCB(thisRef(), &Axis::_onButtonMotionCB);

  //  ArRef<URLTexture> outTex = new_URLTexture(outStr, true, true);
  ArRef<URLTexture> outTex = URLTexture::NEW(outStr, true, true);
  //  _outOsd = new_AxisButton(outTex, false);
  _outOsd = AxisButton::NEW(outTex, false);
  _outOsd->setSize(_bw, _bh);
  _outOsd->setDepth(0.1);
  _outOsd->addMotionCB(thisRef(), &Axis::_onButtonMotionCB);

  setButtonValues(_rangeMin, _rangeMax);
}

Axis::~Axis(void) {
}

void
Axis::addRenderer(ArRef<Renderer3D> renderer) {
  for(size_t i = _renderers.size(); i--;) {
    if(renderer == _renderers[i]) {
      return;
    }
  }

  _renderers.push_back(renderer);

  renderer->addOSD(_axis);
  renderer->addOSD(_inOsd);
  renderer->addOSD(_outOsd);
}

void
Axis::removeRenderer(ArRef<Renderer3D> renderer) {
  for(size_t i = _renderers.size(); i--;) {
    if(renderer == _renderers[i]) {
      renderer->addOSD(_axis);
      renderer->addOSD(_inOsd);
      renderer->addOSD(_outOsd);
      StlVectorFastErase(_renderers, i);
      return;
    }
  }
}

unsigned int
Axis::getNbRenderer(void) const {
  return _renderers.size();
}

ArConstRef<Renderer3D>
Axis::getRenderer(unsigned int index) const {
  return _renderers[index];
}

ArRef<Renderer3D>
Axis::accessRenderer(unsigned int index) {
  return _renderers[index];
}

void
Axis::setVisible(bool visible) {
  for(size_t i = _renderers.size(); i--;) {
    if(visible) {
      _renderers[i]->addOSD(_axis);
      _renderers[i]->addOSD(_inOsd);
      _renderers[i]->addOSD(_outOsd);
    } else {
      _renderers[i]->removeOSD(_axis);
      _renderers[i]->removeOSD(_inOsd);
      _renderers[i]->removeOSD(_outOsd);
    }
  }
}

void
Axis::setSize(unsigned int w, unsigned int h) {
  double in, out;
  getButtonValues(in, out);  

  int x, y;
  _axis->getPosition(Renderer3D::nullRef(), x, y);
  
  _sw = w;
  _sh = h;
  _axis->setSize(w, h);
  setButtonValues(in, out);
}

void
Axis::getSize(unsigned int& w, unsigned int& h) const {
  w = _sw;
  h = _sh;
}

void
Axis::setButtonSize(unsigned int w, unsigned int h) {
  double in, out;
  getButtonValues(in, out);  

  _bw = w;
  _bh = h;
  _inOsd->setSize(w, h);
  _outOsd->setSize(w, h);
  setButtonValues(in, out);
}

void
Axis::getButtonSize(unsigned int& w, unsigned int& h) const {
  w = _bw;
  h = _bh;
}

void
Axis::setPosition(int x, int y) {
  _axis->setPosition(x, y);
}

void
Axis::setBounds(double min, double max) {
  if(min > max) {
    return;
  }

  double oldIn, oldOut;
  getButtonValues(oldIn, oldOut);

  _rangeMin = min;
  _rangeMax = max;
  
  if(oldIn < min) {
    oldIn = min;
  }

  if(oldOut > max) {
    oldOut = max;
  }
  
  setButtonValues(oldIn, oldOut);
}

void
Axis::getBounds(double& min, double& max) const { 
  min = _rangeMin;
  max = _rangeMax;
}

void
Axis::setButtonValues(double in, double out) {
  int xmin, xmax, ymin, ymax, xslide, yslide;
  _axis->getPosition(Renderer3D::nullRef(), xslide, yslide);

  xmin = (int)(xslide - (int)_bw + (int)_sw * (in - _rangeMin) / (_rangeMax - _rangeMin));
  //ymin = (int)(yslide + (int)_sh * 0.5 - (int)_bh * 0.5);
  ymin = (int)(yslide + (int)_sh * 0.5 - (int)_bh);
  xmax = (int)(xslide + (int)_sw * (out - _rangeMin) / (_rangeMax - _rangeMin));
  //ymax = (int)(yslide + (int)_sh * 0.5 - (int)_bh * 0.5);
  ymax = (int)(yslide + (int)_sh * 0.5);

  _inOsd->setPosition(xmin, ymin);
  _outOsd->setPosition(xmax, ymax);

  _valueIn = in;
  _valueOut = out;
  _inOsd->setValue(in);
  _outOsd->setValue(out);

  _onValueChanged();
}

void
Axis::getButtonValues(double& in, double& out) const {
  int xmin, xmax, ymin, ymax, xslide, yslide;
  _inOsd->getPosition(Renderer3D::nullRef(), xmin, ymin);
  _outOsd->getPosition(Renderer3D::nullRef(), xmax, ymax);
  _axis->getPosition(Renderer3D::nullRef(), xslide, yslide);
  
  in  = _rangeMin + (_rangeMax - _rangeMin) * (xmin + (int)_bw - xslide) / (int)_sw;
  out = _rangeMin + (_rangeMax - _rangeMin) * (xmax - xslide) / (int)_sw;
}

void
Axis::attachTo(ArRef<Axis> axis) {
  _axis->attachTo(axis->_axis);
}

void
Axis::_onAxisMotionCB(const OSD::MotionEvent& /* evt */) {
  setButtonValues(_valueIn, _valueOut);
}

void
Axis::_onButtonMotionCB(const OSD::MotionEvent& /* evt */) {
  if(!_motionMask) {
    _motionMask++;

    double in, out;
    getButtonValues(in, out);

    if(in < _rangeMin) in = _rangeMin;
    if(in > _rangeMax) in = _rangeMax;
    if(out < _rangeMin) out = _rangeMin;
    if(out > _rangeMax) out = _rangeMax;

    setButtonValues(in, out);

    _motionMask--;
  }
}

void
Axis::_onValueChanged(void) {
  if(!_valueCB.empty()) {
    Axis::ValueEvent evt;
    evt.in = _valueIn;
    evt.out = _valueOut;
    _valueCB.fire(evt);
  }
}
