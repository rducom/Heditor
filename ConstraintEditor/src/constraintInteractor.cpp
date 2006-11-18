/*
*
*Gestion des interactions pour les vues en perspectives
*
*/

#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Lib3D/scene3D.h"
#include "AReVi/Lib3D/transform3D.h"
#include "AReVi/Lib3D/urlTexture.h"
#include "AReVi/Lib3D/simpleInteractor.h"

#include "hLib/core/body.h"
#include "hLib/core/joint.h"
//#include "keyOrientation.h"

#include "constraintInteractor.h"

AR_CLASS_NOVOID_DEF(ConstraintInteractor, AbstractInteractor)

ConstraintInteractor::ConstraintInteractor(ArCW& arCW, ArRef<BodyController> bodyController, ArRef<JointController> jointController, ArRef<AnchorController> anchorController)
: AbstractInteractor(arCW),
  init(true),
  _bodyController(bodyController),
  _jointController(jointController),
  _anchorController(anchorController),
  _mx(0),
  _my(0),
  _btn1Pressed(false),
  _body(),
  //  _solver(new_EditorIkSolver()),
  _solver(EditorIkSolver::NEW()),
  _interacted(),
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
 
  _anchorController->addAnchorCB(thisRef(), &ConstraintInteractor::_onAnchorCB);
}

ConstraintInteractor::~ConstraintInteractor(void) {
}

void
ConstraintInteractor::setLinearSpeed(double linearSpeed) {
  _linearSpeed=fabs(linearSpeed);
}


double // half-screen linear speed in 1/s
ConstraintInteractor::getLinearSpeed(void) const {
  return(_linearSpeed);
}

void
ConstraintInteractor::setAngularSpeed(double angularSpeed) {
  _angularSpeed=fabs(angularSpeed);
}

double // half-screen angular speed in rad/s
ConstraintInteractor::getAngularSpeed(void) const {
  return(_angularSpeed);
}

/*
*Initialise la fenetre sur laquelle l'interacteur doit agir
*/
void
ConstraintInteractor::setWindow(ArRef<Window3D> window) {
  AbstractInteractor::setRenderer(window);
  _window = window;
}

/*
*Initialise l'humanoide
*/
void
ConstraintInteractor::setBody(ArRef<Body> body) {
  if(_body != body) {
    _selected = Joint::nullRef();

    _body = body;

    _solver->clearGoals();
    _solver->setSkeleton(_body);
    _goals.clear();
    _interacted = Goal::nullRef();
  }
}

/*
*Retourne une reference constante sur l'humanoide
*/
ArConstRef<Body>
ConstraintInteractor::getBody(void) const {
  return _body;
}

/*
*Retourne une reference sur l'humanoide
*/
ArRef<Body>
ConstraintInteractor::accessBody(void) {
  return _body;
}

/*
*Initialise les buts a atteindre pour la cinematique inverse
*/
void
ConstraintInteractor::setGoals(StlVector<ArRef<Goal> >& goals) {
  _goals = goals;

  _solver->clearGoals();
  for(size_t i = _goals.size(); i--;) {
    _solver->addGoal(_goals[i]);
  }
  _solver->applyChanges();
}

/*
*Retourne la liste constante des buts a atteindre pour la cinematique inverse
*/
const StlVector<ArRef<Goal> >&
ConstraintInteractor::getGoals(void) {
  return _goals;
}

/*
*Retourne la liste des buts a atteindre pour la cinematique inverse
*/
StlVector<ArRef<Goal> >&
ConstraintInteractor::accessGoals(void) {
  return _goals;
}


/*
*Retourne la location de l'objet sur lequel on interragit
*/
Util3D::Base & // current location
ConstraintInteractor::_setupLocation(ArRef<Base3D> base) {
  if(!_newLocation) {
    //    _newLocation=new_Base3D();
    _newLocation=Base3D::NEW();
  }
  Util3D::Base & loc=_newLocation->accessBase();
  loc=base->accessBase();
  return(loc);
}

/*
*...
*/
void
ConstraintInteractor::_clearTimeOut(void)  { // Protected
  if(_timeOut) {
    if(_window) {
      _window->cancelTimeOut(_timeOut);
    }
    _timeOut=0;
  }
}


/*
*Gestion des interractions sur click de souris
*/
void
ConstraintInteractor::_onMouseButton(const Window3D::MouseButtonEvent& evt) {
  
  _mouseX = evt.xMouse;
  _mouseY = evt.yMouse;
   
  double yaw, pitch;
  _window->computeCursorDirection(evt.xMouse, evt.yMouse, yaw, pitch);
  //  ArRef<Base3D> ray=new_Base3D();
  ArRef<Base3D> ray=Base3D::NEW();
  ray->setLocation(_window);
  ray->yaw(yaw);
  ray->pitch(pitch);
        
  //Recuperation de l'objet sur lequel se trouve le pointeur
  double dist;
  ArRef<Object3D> obj;
  ArRef<ShapePart3D> interPartOut;
  unsigned int triIndex;
  _window->getScene()->firstRayIntersection(ray,2,true,Object3D::CLASS(),obj, interPartOut,triIndex,dist,_window);  
  

  if(interPartOut) {
    //si c'est une KeyOrientation
    if( obj->getClass()->isA(KeyOrientation::CLASS()) ) {
      _isKO = true;
    }
    else{ 
      _isKO = false;
      _anchorController->highLightAnchor(Anchor::nullRef());
      _jointController->highLightJoint(Joint::nullRef());
      if(interPartOut->getParentPart()) {
	if(interPartOut->getParentPart()->getClass()->isA(Anchor::CLASS())) {
	  _useIK = true;   
	  _useNavigate = false;
	  _useCentered = false;
	  //_selected = Joint::nullRef();
	} else if(interPartOut->getParentPart()->getClass()->isA(Joint::CLASS())) {
	  _useIK = false;
	  _useNavigate = false;
	  _useCentered = true;
	  init = true;
	  _selected = ar_down_cast< Joint >(interPartOut->getParentPart());
	  _jointController->selectJoint(ar_down_cast< Joint >(interPartOut->accessParentPart()));
	} else {
	  // if we are over some object which is'nt joint nor anchor (ex: body/skeleton)
	  _useIK = false;
	  _useNavigate = false;
	  _useCentered = true;
	  //_selected = Joint::nullRef();
	}
      }
      else {   // ShapePart3D without parent
	_useIK = true;
	_useNavigate = false;
	_useCentered = true;
	//_selected = Joint::nullRef();
      }
    }
  }
  else {   // if we are over an empty zone
    _isKO = false;
    _useIK = false;
    _useNavigate = true;
    _useCentered = false;
    //_selected = Joint::nullRef();
  }
  
  
  
  if(evt.pressed) {
    //sur click
    switch(evt.button) {
    case 1 :                 //    IK || move
      {
	_mouseBtn = 1;
	_remember_cursor = true;
	if(_isKO) {
	  if(interPartOut) {
	    ar_down_cast<KeyOrientation>(obj)->onMouseButtonInteraction(thisRef(), _mouseBtn, evt.pressed);
	    obj=_findObject(_localPoint,interPartOut,triIndex, evt.xMouse,evt.yMouse, true);
	  }
	}
	else{
	  if(_useIK) { 
	    if(interPartOut) {
	      if(interPartOut->getParentPart()) {
		if(interPartOut->getParentPart()->getClass()->isA(Anchor::CLASS())) {
		
		  _useIK = true;
		  _useNavigate = false;
		  ArConstRef<Anchor> anchor = ar_down_cast<Anchor>(interPartOut->getParentPart());
		
		  _interacted = Goal::nullRef();
		  for(size_t i = 0; i < _goals.size(); i++) {
		    if(_goals[i]->getAnchor() == anchor) {
		      _interacted = _goals[i];}
		  }
		  
		  _localPoint.x = dist;
		  _localPoint.y = 0.0;
		  _localPoint.z = 0.0;
		  ray->localToGlobalVector(_localPoint.x,_localPoint.y,_localPoint.z);
		  _window->globalToLocalVector(_localPoint.x,_localPoint.y,_localPoint.z);
		
		}
	      } else {
		_interacted = Goal::nullRef();
	      }
	    }
	  }
	  else if(_useNavigate) {
	    _clickX=_mouseX;
	    _clickY=_mouseY;
	    _clearTimeOut();
	    _lastTime=ArSystem::getTime();
	    _timeOut=evt.source->postTimeOut(NAVIGATION_DT);
	    _setupLocation(evt.source); 
	    evt.source->onMotionInteraction(thisRef(),_newLocation,true,false); 
	  }
	}
      }
      break;
    case 2 :                                            // Rotate On
      {
	_mouseBtn = 2;
	_remember_cursor = true;
      }
      break;
    case 3 :  // Center view on Joint & select it & could rotate
      {
	_mouseBtn = 3;
	_remember_cursor = true;
	if(_useCentered) {	
	  if(_selected) {
	    if(init) {
	      double x, y, z;
	      //	      ArRef<Transform3D> tr1 = new_Transform3D();
	      ArRef<Transform3D> tr1 = Transform3D::NEW();
	      interPartOut->readFlatTransformation(tr1);
            
	      //	      ArRef<Transform3D> tr2 = new_Transform3D();
	      ArRef<Transform3D> tr2 = Transform3D::NEW();
	      interPartOut->getShape()->readTransformation(tr2);
            
	      tr1->preTransform(tr2);
	      tr1->apply(x,y,z,0.0,0.0,0.0);
            
	      cout << " x=" << x << " y=" << y << " z=" << z << endl;
            
	      _selPosition = _selected->getDeltaTranslation();
	      Util3D::Base & loc=_setupLocation(_window);
	      loc.setPosition(x,y,z);
	      loc.translate(-dist,0.0,0.0);
	      _window->onMotionInteraction(thisRef(),_newLocation,false,false);
	    }
	  }
	}
      }
      break;
    case 4 :
      {
	double xc, yc, zc;
	_window->getPosition(xc, yc, zc);
	double D = sqrt(xc*xc+yc*yc+zc*zc);
	Util3D::Base & loc=_setupLocation(_window);
	loc.translate(D*0.15,0.0,0.0);
	_window->onMotionInteraction(thisRef(),_newLocation,false,false);
      }
      break;
    case 5 :
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
  } else {
    switch(evt.button) {
    case 1 :
      _mouseBtn = 0;
      _remember_cursor = false;
      _interacted = Goal::nullRef();
      _useNavigate = false;
      _useIK = false;
      break;
    case 2 :
      _mouseBtn = 0;
      _remember_cursor = false;
      _interacted = Goal::nullRef();
      _useNavigate = false;
      _useIK = false;
      break;
    case 3 :
      _mouseBtn = 0;
      _remember_cursor = false;
      init = true;
      _interacted = Goal::nullRef();
      _useNavigate = false;
      _useIK = false;
      break;
    default :
      _remember_cursor = false;
      break;
    }
  }
}

/*
*Interaction sur mouvement de la souris
*/
void
ConstraintInteractor::_onMouseMotion(const Window3D::MouseMotionEvent& evt) {

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
  
  //Recuparation du deplacement
  _window->computeCursorDirection(evt.xMouse,evt.yMouse,yaw,pitch);
  ray->setLocation(_window);
  ray->yaw(yaw);
  ray->pitch(pitch);
  //recuperation de l'objet sous le pointeur
  _window->getScene()->firstRayIntersection(ray,2,true,Object3D::CLASS(),obj, interPartOut,triIndex,dist,_window);
  if(_remember_cursor == false) {
    Window3D::Cursor cursor=Window3D::CSR_ARROW;
    _window->setWindowCursor(cursor);
  }

  if(interPartOut) {
    //si l'objet est une KeyOrientation
    if( obj->getClass()->isA(KeyOrientation::CLASS()) ) {
      if(_remember_cursor == false) {
	Window3D::Cursor cursor=Window3D::CSR_HAND;
	_window->setWindowCursor(cursor);
      }
    }
    else{ 
      _anchorController->highLightAnchor(Anchor::nullRef());
      _jointController->highLightJoint(Joint::nullRef());
      if(interPartOut->getParentPart()) {
	if(interPartOut->getParentPart()->getClass()->isA(Anchor::CLASS())) {
	  _anchorController->highLightAnchor(ar_down_cast<Anchor>(interPartOut->accessParentPart()));
	  if(_remember_cursor == false) {
	    Window3D::Cursor cursor=Window3D::CSR_HAND;
	    _window->setWindowCursor(cursor);
	  }
	} else if(interPartOut->getParentPart()->getClass()->isA(Joint::CLASS())) {
	  _jointController->highLightJoint(ar_down_cast<Joint>(interPartOut->accessParentPart()));
	  if(_remember_cursor == false) {
	    Window3D::Cursor cursor=Window3D::CSR_CROSS;
	    _window->setWindowCursor(cursor);
	  }
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
 




  //si le bouton de la souris est enfonce
  if(_mouseBtn) {
    unsigned int w, h;  
    double dx, dy, dz;
    evt.source->getRenderSize(w,h);
    ArRef<ShapePart3D> part;
    Util3D::Dbl3 localPoint;
     
    switch(_mouseBtn) {
    case 1:  // Inversed Kinematics || move 
      {
	if(_isKO) {
	  ArRef<KeyOrientation> KO = _jointController->accessC3DOF()->accessSelected();
	  const Util3D::Perspective & persp=evt.source->getPerspective();
	  double dx = 0.0;
	  double dy = _localPoint.x*(persp.getLeftSlope()-persp.getRightSlope())*((double)(_oldX-_mouseX))/(double)w;
	  double dz = _localPoint.x*(persp.getUpSlope()-persp.getDownSlope())* ((double)(_oldY-_mouseY))/(double)h;
	  evt.source->localToGlobalVector(dx,dy,dz);
	  Util3D::Base & loc = _setupLocation(ar_down_cast<Base3D>(KO));
	  loc.globalToLocalVector(dx,dy,dz);
	  loc.translate(dx,dy,dz);
	  KO->onMotionInteraction(thisRef(), _newLocation, false, false);
	}
	else {
	  if(_interacted.valid() /*&& _useIK*/) {  //IK
	     
	    const Util3D::Perspective& perspective = evt.source->getPerspective();
	    double limitZ = tan(0.5 * perspective.getFieldOfView());
	    double limitY = limitZ * perspective.getAspectRatio();
	    dx = 0.0;
	    dy = _localPoint.x * 2.0 * limitY * ((double)( _oldX - _mouseX )) / (double)w;
	    dz = _localPoint.x * 2.0 * limitZ * ((double)( _oldY - _mouseY )) / (double)h;
	     
	    evt.source->localToGlobalVector(dx, dy, dz);
	    Point3d anchorPosition = _getAnchorGlobalPosition(_interacted->getAnchor());
	    anchorPosition.x += dx;
	    anchorPosition.y += dy;
	    anchorPosition.z += dz;
	    _interacted->setPosition(anchorPosition);
	  
	    _solver->applyChanges();
	    IkCcd::SolverResult r = _solver->solve();
	    if(r.pose) { _body->applyPose(r.pose, true); }
	  }
	  if(_useNavigate)  {    // move throught space
	    // Nothing to do here, mouvement is handeled by _onTimeOut
	  }    
	}
      }
      break;
    case 2:  // move camera on Y and Z plans.
      {
	const Util3D::Perspective & persp=evt.source->getPerspective();
	double dx=0.0;
	double dy=(persp.getLeftSlope()-persp.getRightSlope())*((double)(_oldX-_mouseX))/(double)w;
	double dz=(persp.getUpSlope()-persp.getDownSlope())*((double)(_oldY-_mouseY))/(double)h;
	evt.source->localToGlobalVector(dx,dy,dz);
	Util3D::Base & loc=_setupLocation(_window);
	loc.globalToLocalVector(dx,dy,dz);
	loc.translate(dx,dy,dz);
	_window->onMotionInteraction(thisRef(),_newLocation,false,false);
      }
      break;
    case 3:  // Center view on Joint & select it & rotate
      {
	ArRef<Joint> joint;
	   
	if( _selected ) {
	  double xc,yc,zc;
	  //ArRef<Base3D> cam = ar_down_cast<Base3D>(_window);
	       
	  _window->getPosition(xc,yc,zc);
	       
	  dx=2.0*((double)(_oldX-_mouseX))/w;
	  dy=2.0*((double)(_oldY-_mouseY))/h;
	       
	  if(!_window->getCenterOfView(xc,yc,zc)) {xc=yc=zc=0.0;}
	       
	  _window->globalToLocalPosition(xc,yc,zc);
	       
	  if(xc<_window->getNearDistance()) {xc=0.5*(_window->getFarDistance()+_window->getNearDistance());}
	       
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
	       
	  //Util3D::Transform tr(loc);
	       
	  loc.extractOrientation(roll, yaw, pitch);
	  loc.setOrientation(0.0,yaw,pitch); // we cancel the roll
	  loc.translate(-_lastXc,0.0,0.0);
	       
	  _window->localToGlobalVector(_lastDx,_lastDy,_lastDz);
	  unsigned int dt=ArSystem::getTime()-_lastTime;
	  _lastTime+=dt;
	  if(dt){_lastAngle/=dt;}
	  _window->onMotionInteraction(thisRef(),_newLocation,false,false);
	}
      }
      break;
    default:
      break;
    }
  }  
}

/*
*Inrteraction clavier
*/
void
ConstraintInteractor::_onKeyboard(const Window3D::KeyboardEvent & evt) {
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
	if(found->getShapeBoundingBox())	{
	  if(found->getShapePartBoundingBox()) {found->setShapeBoundingBox(false);}
	  else {found->setShapePartBoundingBox(true);}
	}
	else	{
	  if(found->getShapePartBoundingBox()) {found->setShapePartBoundingBox(false);}
	  else {found->setShapeBoundingBox(true);}
	}
      }
      else {
	if(evt.source->getShapeBoundingBox()) {
	  if(evt.source->getShapePartBoundingBox()) {evt.source->setShapeBoundingBox(false);}
	  else {evt.source->setShapePartBoundingBox(true);}
	}
	else	{
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
	else	{
	  double dist=evt.source->getStereoViewDistance()*1.1;
	  if(dist>evt.source->getFarDistance()) {dist=evt.source->getFarDistance();}
	  evt.source->setStereoViewDistance(dist);
	}
      }
    }
    else if(evt.key=="Down")	{
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
    else if(evt.key=="Right") {
      if(evt.source->shiftPressed()) {
	if(evt.source->controlPressed()) {
	  evt.source->setNearYOffset(evt.source->getNearYOffset()- 0.1*evt.source->getNearDistance());
	}
	else	{
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
	else	{
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

/*
*Fonction reflexe aux evenement de changement de parametre des ancres
*/
void
ConstraintInteractor::_onAnchorCB(const AnchorController::AnchorEvent& evt) {
  // Create goals
  StlVector<ArRef<Goal> > goals;
  for(size_t i = evt.anchors.size(); i--;) {
    //    ArRef<Transform3D> tr = new_Transform3D();
    ArRef<Transform3D> tr = Transform3D::NEW();

    // Express anchor position in skeleton location
    evt.anchors[i]->readTransformation(tr);
    const Util3D::Transform& t3d = tr->getTransformation();
    Point3d anchorPos(t3d.matrix[Util3D::TX], t3d.matrix[Util3D::TY], t3d.matrix[Util3D::TZ]);
    ArConstRef<Joint> link = evt.anchors[i]->getLinkedTo();
    const Matrix4x4d linkMatrix = link->getAbsoluteMatrix();

    anchorPos = linkMatrix * anchorPos;

    // Then express anchor position in shape location
    _body->readTransformation(tr);
    tr->apply(anchorPos.x, anchorPos.y, anchorPos.z, anchorPos.x, anchorPos.y, anchorPos.z);

    // Shape location is the same as object location

    //    ArRef<Goal> g = new_Goal();
    ArRef<Goal> g = Goal::NEW();
    g->setAnchor(evt.anchors[i]);
    g->setLocationType(Goal::GLOBAL);
    g->setConstraints(true, false);
    g->setPosition(anchorPos);

    goals.push_back(g);
  }
  if(!goals.empty()) {
    setGoals(goals);
  }
}

/*
*Retourne la poistion globale d'une ancre
*/
Point3d
ConstraintInteractor::_getAnchorGlobalPosition(ArConstRef<Anchor> anchor) {
  //  ArRef<Transform3D> tr = new_Transform3D();
  ArRef<Transform3D> tr = Transform3D::NEW();

  anchor->readTransformation(tr);
  const Util3D::Transform& t3d = tr->getTransformation();
  Point3d anchorPos(t3d.matrix[Util3D::TX], t3d.matrix[Util3D::TY], t3d.matrix[Util3D::TZ]);

  ArConstRef<Joint> link = anchor->getLinkedTo();
  const Matrix4x4d linkMatrix = link->getAbsoluteMatrix();

  anchorPos = linkMatrix * anchorPos;

  _body->readTransformation(tr);
  tr->apply(anchorPos.x, anchorPos.y, anchorPos.z,
            anchorPos.x, anchorPos.y, anchorPos.z);

  return anchorPos;
}

/*
*Remise a 0 des but a atteindre pour la cinematique inverse
*/
void
ConstraintInteractor::_resetGoals(void) {
  AnchorController::AnchorEvent evt;

  for(size_t i = 0; i < _anchorController->getNbAnchors(); i++) {
    evt.anchors.push_back(_anchorController->accessAnchor(i));
  }

  _onAnchorCB(evt);
}

/*
*...
*/
void
ConstraintInteractor::_onTimeOut(const Window3D::TimeOutEvent & evt) {    // Protected
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
	  loc.translate(0.0,-xRatio*_linearSpeed*dt*1e-3,
			-yRatio*_linearSpeed*dt*1e-3);
	  break;
        }
      }
      _timeOut=evt.source->postTimeOut(NAVIGATION_DT);
      evt.source->onMotionInteraction(thisRef(),_newLocation, false,false);
    }
  }
}
