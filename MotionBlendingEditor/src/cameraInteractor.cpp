#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Lib3D/scene3D.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Lib3D/urlTexture.h"

#include "hLib/core/joint.h"

#include "cameraInteractor.h"


AR_CLASS_NOVOID_DEF(CameraInteractor, AbstractInteractor)

CameraInteractor::CameraInteractor(ArCW& arCW)
  : AbstractInteractor(arCW),
    _init(true),
    _mx(0),
    _my(0),
    _btn1Pressed(false),
    //    _selected(new_Object3D()),
    _selected(Object3D::NEW()),
    _body(),
    _remember_cursor(false),
    _mouseX(0),
    _mouseY(0),
    _mouseBtn(0),
    _clickX(0),
    _clickY(0),
    _localPoint(0.0,0.0,0.0),
    _timeOut(0),
    _lastTime(ArSystem::getTime()),
    _newLocation(),
    _newX(0),
    _newY(0),
    _linearSpeed(1.0),
    _angularSpeed(M_PI) {

}

CameraInteractor::~CameraInteractor(void) {
}

void
CameraInteractor::setLinearSpeed(double linearSpeed) {
  _linearSpeed=fabs(linearSpeed);
}


double // half-screen linear speed in 1/s
CameraInteractor::getLinearSpeed(void) const {
  return(_linearSpeed);
}

void
CameraInteractor::setAngularSpeed(double angularSpeed) {
  _angularSpeed=fabs(angularSpeed);
}

double // half-screen angular speed in rad/s
CameraInteractor::getAngularSpeed(void) const {
  return(_angularSpeed);
}

void
CameraInteractor::setWindow(ArRef<Window3D> window) {
  AbstractInteractor::setRenderer(window);
  _window = window;
}

void
CameraInteractor::setBody(ArRef<Body> body) {
  if(_body != body) {
    _body = body;
    _selected->setShape( _body);
  }
}

ArConstRef<Body>
CameraInteractor::getBody(void) const {
  return _body;
}

ArRef<Body>
CameraInteractor::accessBody(void) {
  return _body;
}

Util3D::Base & // current location
CameraInteractor::_setupLocation(ArRef<Base3D> base) {
  if(!_newLocation) {
    //    _newLocation=new_Base3D();
    _newLocation=Base3D::NEW();
  }
  Util3D::Base & loc=_newLocation->accessBase();
  loc=base->accessBase();
  return(loc);
}

void
CameraInteractor::_clearTimeOut(void) { // Protected
  if(_timeOut) {
    if(_window) {
      _window->cancelTimeOut(_timeOut);
    }
    _timeOut=0;
  }
}

void
CameraInteractor::_onMouseButton(const Window3D::MouseButtonEvent& evt) {
  
  _mouseX = evt.xMouse;
  _mouseY = evt.yMouse;
  
  double yaw, pitch;
  _window->computeCursorDirection(evt.xMouse, evt.yMouse, yaw, pitch);
  //  ArRef<Base3D> ray=new_Base3D();
  ArRef<Base3D> ray=Base3D::NEW();
  ray->setLocation(_window);
  ray->yaw(yaw);
  ray->pitch(pitch);
  
  double dist;
  ArRef<Object3D> obj;
  ArRef<ShapePart3D> interPartOut;
  unsigned int triIndex;
  _window->getScene()->firstRayIntersection(ray,2,true,Object3D::CLASS(),obj, interPartOut,triIndex,dist,_window);  
  
  if(evt.pressed) {
    switch(evt.button) {
    case 1 : // move
      {
	_mouseBtn = 1;
	_remember_cursor = true;
	if(_useCentered) {	
	  if(_selected) {
	    if(_init) {
	      double x, y, z;
	      _selected->getPosition(x, y, z);
	      
	      Util3D::Base & loc=_setupLocation(_window);
	      loc.setPosition(x, y, z);
	      loc.translate(-dist,0.0,0.0);
	      _window->onMotionInteraction(thisRef(),_newLocation,false,false);
	      _init = false;
	    }
	  }
	}
      }
      break;
    case 3 :  // Center view on Object & select it & could rotate
      {
	_mouseBtn = 3;
	_remember_cursor = true;
	double x, y, z;
	_selected->getPosition(x, y, z);
// 	Util3D::Base & loc=_setupLocation(_window);
	_setupLocation(_window);
	_window->onMotionInteraction(thisRef(),_newLocation,false,false);
      }
      break;
    case 4 :  // zoom +
      {
	double xc, yc, zc;
	_window->getPosition(xc, yc, zc);
	double D = sqrt(xc*xc+yc*yc+zc*zc);
	Util3D::Base & loc=_setupLocation(_window);
	loc.translate(D*0.15,0.0,0.0);
	_window->onMotionInteraction(thisRef(),_newLocation,false,false);
      }
      break;
    case 5 :  // zoom -
      {
	double xc, yc, zc;
	_window->getPosition(xc, yc, zc);
	double D = sqrt(xc*xc+yc*yc+zc*zc);
	Util3D::Base & loc=_setupLocation(_window);
	loc.translate(-D*0.15,0.0,0.0);
	_window->onMotionInteraction(thisRef(),_newLocation,false,false);
	break;
      }
    default :
      break;
    }
  } 
  else {
    switch(evt.button) {
    case 1 :
      _mouseBtn = 0;
      _remember_cursor = false;
      _useNavigate = true;
      break;
    case 3 :
      _mouseBtn = 0;
      _remember_cursor = false;
      _init = true;
      _useNavigate = true;
      break;
    default :
      _remember_cursor = false;
      break;
    }
  }
}


void
CameraInteractor::_onMouseMotion(const Window3D::MouseMotionEvent& evt) {
  
  int _oldX=_mouseX;
  int _oldY=_mouseY;
  _mouseX=evt.xMouse;
  _mouseY=evt.yMouse;
  
  double yaw, pitch, dist;
  unsigned int triIndex;
  //  ArRef<Base3D> ray=new_Base3D();
  ArRef<Base3D> ray=Base3D::NEW();
  ArRef<Object3D> obj;
  ArRef<ShapePart3D> interPartOut;

  _window->computeCursorDirection(evt.xMouse,evt.yMouse,yaw,pitch);
  ray->setLocation(_window);
  ray->yaw(yaw);
  ray->pitch(pitch);
  _window->getScene()->firstRayIntersection(ray,2,true,Object3D::CLASS(),obj, interPartOut,triIndex,dist,_window);
  if(_remember_cursor == false) {
    Window3D::Cursor cursor=Window3D::CSR_ARROW;
    _window->setWindowCursor(cursor);
  }

  if(interPartOut) {
    if(interPartOut->getParentPart()) {
      if(interPartOut->getParentPart()->getClass()->isA(ShapePart3D::CLASS())) {
	if(_remember_cursor == false) {
	  Window3D::Cursor cursor=Window3D::CSR_HAND;
	  _window->setWindowCursor(cursor);
	}
      } 
    }
  }
  else {
      if(_remember_cursor == false) {
	Window3D::Cursor cursor=Window3D::CSR_MOVE;
	_window->setWindowCursor(cursor);
      }
  }
    
  if(_mouseBtn) {
      unsigned int w, h;  
      double dx, dy, dz;
      dz = 0.0;
      evt.source->getRenderSize(w,h);
      ArRef<ShapePart3D> part;
      Util3D::Dbl3 localPoint;
  
      switch(_mouseBtn) {
      case 1:  // move camera on Y and Z plans.
	{
	  const Util3D::Perspective& persp = evt.source->getPerspective();
	  double dx=0.0;
	  double dy=20*(persp.getLeftSlope()-persp.getRightSlope())* ((double)(_oldX-_mouseX))/(double)w;
	  double dz=20*(persp.getUpSlope()-persp.getDownSlope())* ((double)(_oldY-_mouseY))/(double)h;
	  evt.source->localToGlobalVector(dx,dy,dz);
	  Util3D::Base & loc=_setupLocation(_window);
	  loc.globalToLocalVector(dx,dy,dz);
	  loc.translate(dx,dy,dz);
	  _window->onMotionInteraction(thisRef(),_newLocation,false,false);
	}
	break;
      case 3:  // Center view on Object3D & select it & rotate
	{
	  double xc,yc,zc;
	  _window->getPosition(xc,yc,zc);
	  dx = ((double)(_oldX-_mouseX))/w;
	  dy = ((double)(_oldY-_mouseY))/h;
            
	  if(!_window->getCenterOfView(xc,yc,zc)) {xc=yc=zc=0.0;}
            
	  _window->globalToLocalPosition(xc,yc,zc);
            
	  if(xc < _window->getNearDistance()) {xc = 0.5*(_window->getFarDistance()+_window->getNearDistance());}
            
	  double _lastDx=0.0;
	  double _lastDy=dy;
	  double _lastDz=-dx;
	  double r = sqrt(_lastDy*_lastDy+_lastDz*_lastDz);
	  if(r){  _lastDy/=r;  _lastDz/=r; }
	  double _lastAngle=r*M_PI;
	  double _lastXc=xc;
	  double roll, yaw, pitch;
            
	  Util3D::Base & loc=_setupLocation(_window);
			
	  loc.translate(_lastXc,0.0,0.0);
	  loc.rotate(_lastDx,_lastDy,_lastDz,_lastAngle);
	  loc.extractOrientation(roll, yaw, pitch);
	  loc.setOrientation(0.0,yaw,pitch);
	  loc.translate(-_lastXc,0.0,0.0);
          
	  _window->localToGlobalVector(_lastDx,_lastDy,_lastDz);
	  unsigned int dt=ArSystem::getTime()-_lastTime;
	  _lastTime+=dt;
	  if(dt){_lastAngle/=dt;}
	  _window->onMotionInteraction(thisRef(),_newLocation,false,false);
	}
	break;
      default:
	break;
      }
    }
}

void
CameraInteractor::_onKeyboard(const Window3D::KeyboardEvent & evt) {
  _mouseX=evt.xMouse;
  _mouseY=evt.yMouse;
  if(evt.pressed) {
    if(evt.key=="Home") {
      Util3D::Base & loc=_setupLocation(evt.source);
      loc.cancelRotation();
      loc.setPosition(0.0,0.0,0.0);
      evt.source->onMotionInteraction(thisRef(),_newLocation,true,true);
    }
    else if((evt.key=="d")||(evt.key=="D")) {
      if(evt.source->getClass()->isA(Viewer3D::CLASS())) {
	ArRef<Viewer3D> v=ar_down_cast<Viewer3D>(evt.source);
	v->setDecoration(!v->getDecoration());
      }
    }
    else if((evt.key=="w")||(evt.key=="W")) {
      ArRef<Object3D> found;
      Util3D::Dbl3 localPoint;
      ArRef<ShapePart3D> part;
      unsigned int triIndex;
      found=_findObject(localPoint,part,triIndex,_mouseX,_mouseY,false);
      if(found) {found->setWireFrame(!found->getWireFrame());}
      else {evt.source->setWireFrame(!evt.source->getWireFrame());}
    }
    else if((evt.key=="l")||(evt.key=="L")) {
      evt.source->setHeadLight(!evt.source->getHeadLight());
    }
    else if((evt.key=="o")||(evt.key=="O")) {
      evt.source->setOriginVisible(!evt.source->getOriginVisible());
    }
    else if((evt.key=="b")||(evt.key=="B")) {
      ArRef<Object3D> found;
      Util3D::Dbl3 localPoint;
      ArRef<ShapePart3D> part;
      unsigned int triIndex;
      found=_findObject(localPoint,part,triIndex,_mouseX,_mouseY,false);
      if(found) {
	if(found->getShapeBoundingBox()) {
	  if(found->getShapePartBoundingBox()) {found->setShapeBoundingBox(false);}
	  else {found->setShapePartBoundingBox(true);}
	}
	else {
	  if(found->getShapePartBoundingBox()) {found->setShapePartBoundingBox(false);}
	  else {found->setShapeBoundingBox(true);}
	}
      }
      else {
	if(evt.source->getShapeBoundingBox()) {
	  if(evt.source->getShapePartBoundingBox()) {evt.source->setShapeBoundingBox(false);}
	  else {evt.source->setShapePartBoundingBox(true);}
	}
	else {
	  if(evt.source->getShapePartBoundingBox()) {evt.source->setShapePartBoundingBox(false);}
	  else {evt.source->setShapeBoundingBox(true);}
	}
      }
    }
    else if((evt.key=="s")||(evt.key=="S")) {
      switch(evt.source->getStereoMode()) {
      case Renderer3D::STEREO_NONE:
	{
	  evt.source->setStereoMode(Renderer3D::STEREO_SPLIT);
	  break;
	}
      case Renderer3D::STEREO_SPLIT:
	{
	  evt.source->setStereoMode(Renderer3D::STEREO_COLOR);
	  break;
	}
      case Renderer3D::STEREO_COLOR:
	{
	  evt.source->setStereoMode(Renderer3D::STEREO_NONE);
	  break;
	}
      }
    }
    else if((evt.key=="g")||(evt.key=="G")) {evt.source->setStereoGlance(!evt.source->getStereoGlance());}
    else if((evt.key=="r")||(evt.key=="R")) {
      ArRef<Window3D> w = ar_down_cast<Window3D>(evt.source);
      w->setKeyRepeat(!w->getKeyRepeat());
    }
    else if(evt.key=="Up") {
      if(evt.source->shiftPressed()) {
	if(evt.source->controlPressed()) {
	  evt.source->setNearZOffset(evt.source->getNearZOffset()+ 0.1*evt.source->getNearDistance());
	}
	else {
	  double dist=evt.source->getStereoViewDistance()*1.1;
	  if(dist>evt.source->getFarDistance()) {dist=evt.source->getFarDistance();}
	  evt.source->setStereoViewDistance(dist);
	}
      }
    }
    else if(evt.key=="Down") {
      if(evt.source->shiftPressed()) {
	if(evt.source->controlPressed()) {
	  evt.source->setNearZOffset(evt.source->getNearZOffset()- 0.1*evt.source->getNearDistance());
	}
	else {
	  double dist=evt.source->getStereoViewDistance()/1.1;
	  if(dist<evt.source->getNearDistance()) {dist=evt.source->getNearDistance();}
	  evt.source->setStereoViewDistance(dist);
	}
      }
    }
    else if(evt.key=="Right")	{
      if(evt.source->shiftPressed()) {
	if(evt.source->controlPressed()) {
	  evt.source->setNearYOffset(evt.source->getNearYOffset()- 0.1*evt.source->getNearDistance());
	}
	else {
	  double separ=evt.source->getStereoEyeSeparation()*1.1;
	  if(separ>evt.source->getFarDistance()) {separ=evt.source->getFarDistance();}
	  evt.source->setStereoEyeSeparation(separ);
	}
      }
    }
    else if(evt.key=="Left") {
      if(evt.source->shiftPressed()) {
	if(evt.source->controlPressed()) {
	  evt.source->setNearYOffset(evt.source->getNearYOffset()+ 0.1*evt.source->getNearDistance());
	}
	else {
	  double separ=evt.source->getStereoEyeSeparation()/1.1;
	  if(separ<0.01*evt.source->getNearDistance()) {separ=0.01*evt.source->getNearDistance();}
	  evt.source->setStereoEyeSeparation(separ);
	}
      }
    }
    else if((evt.key=="h")||(evt.key=="H")) {
      cerr << endl;
      cerr << "Keyboard interaction for ``" << getAlias() << "'':" << endl;
      cerr << "          [h] display this help" << endl;
      cerr << "          [l] toggle head light" << endl;
      cerr << "          [o] toggle origin visibility" << endl;
      cerr << "          [b] toggle scene/object shape/part bounding boxes" << endl;
      cerr << "          [w] toggle scene/object wire frame" << endl;
      cerr << "          [d] toggle decoration" << endl;
      cerr << "          [s] toggle stereo view" << endl;
      cerr << "          [g] toggle stereo glance" << endl;
      cerr << "          [r] toggle key repeat" << endl;
      cerr << "   [Shift+Up] increase stereo view distance" << endl;
      cerr << " [Shift+Down] decrease stereo view distance" << endl;
      cerr << "[Shift+Right] increase stereo eye separation" << endl;
      cerr << " [Shift+Left] decrease stereo eye separation" << endl;
      cerr << "       [Home] reset location" << endl;
      cerr << "        [Esc] close" << endl;
      cerr << endl;
    }
    else if(evt.key=="\x1b") {
      evt.source->simulateClose();
    }
  }
}

void
CameraInteractor::_onTimeOut(const Window3D::TimeOutEvent & evt) {      // Protected
  if(evt.timeOut==_timeOut) {
    if(_useNavigate) {
      unsigned int dt=ArSystem::getTime()-_lastTime;
      _lastTime+=dt;
      unsigned int w,h;
      evt.source->getRenderSize(w,h);
      double xRatio=(2.0*(_mouseX - _clickX ))/w;
      double yRatio=(2.0*(_mouseY - _clickY ))/h;
      Util3D::Base & loc=_setupLocation(_window);
      switch(_mouseBtn) {
      case 1:
	{
	  loc.yaw(-xRatio*_angularSpeed*dt*1e-3);
	  loc.translate(-yRatio*_linearSpeed*dt*1e-3,0.0,0.0);
	  break;
	}
      case 2:
      case 3:
	{
	  cerr << "on est tout a la fin" << endl;
	  loc.translate(0.0,-xRatio*_linearSpeed*dt*1e-3,-yRatio*_linearSpeed*dt*1e-3);
	  break;
	}
      }
      _timeOut=evt.source->postTimeOut(NAVIGATION_DT);
      evt.source->onMotionInteraction(thisRef(),_newLocation, false,false);
    }
  }
}
