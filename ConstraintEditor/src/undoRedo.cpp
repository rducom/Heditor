/*
 *
 *Gestion de la fonction defaire/refaire
 *
 */

#include "AReVi/arClass.h"
#include "AReVi/Utils/memoryBlock.h"
#include "AReVi/Utils/memoryStream.h"

#include "hLib/core/anchor.h"
#include "hLib/core/joint.h"
#include "hLib/core/jointConstraint.h"

#include "undoRedo.h"
#include "bodyController.h"
#include "anchorController.h"
#include "jointController.h"

AR_CLASS_DEF( UndoRedo, ArObject )
  
  UndoRedo::UndoRedo(ArCW& arCW) : ArObject(arCW), _sizeCB(thisRef()) {

  _undoStack.resize(20);
  _redoStack.resize(20);

}

UndoRedo::~UndoRedo(void) {

}

/*
 *Sauvegarde l'etat actuel du squelette
 */
void
UndoRedo::saveState(ArRef<Body> body) {

  //  ArRef<MemoryBlock> memoryBlock = new_MemoryBlock();
  ArRef<MemoryBlock> memoryBlock = MemoryBlock::NEW();
  
  SerializationDependencies dep;
  body->extractDependencies(dep, true);
  dep.addReference( body );
  
  //  ArRef<MemoryOStream> output = new_MemoryOStream(memoryBlock, 0);
  ArRef<MemoryOStream> output = MemoryOStream::NEW(memoryBlock, 0);
  dep.writeToStream(output);
  
  const StlVector<ArRef<ArObject> >& refsOut = dep.getReferences();
  for(size_t i = 0; i < refsOut.size(); i++) {
    refsOut[i]->serialize(dep, output);
  }
  _undoStack.push_back( memoryBlock );

  _onSize();
  
}

void
UndoRedo::saveState(ArRef<BodyController> bodyCtrl) {

  if(bodyCtrl->getBody() != Body::nullRef()) {

    saveState(bodyCtrl->accessBody());
    
  }
}

void
UndoRedo::saveState(ArRef<JointController> jointCtrl) {

  if( jointCtrl->accessSelectedJoint() != Joint::nullRef() ) {

    if( jointCtrl->accessSelectedJoint()->accessSkeleton() != Skeleton::nullRef()) {
      ArRef<Joint> tmp = jointCtrl->accessSelectedJoint();
      ArRef<Body> body = ar_down_cast<Body>(jointCtrl->accessSelectedJoint()->accessSkeleton());
      jointCtrl->selectJoint(Joint::nullRef());
      saveState(body);
      jointCtrl->selectJoint(tmp);
    }

  }

}

/*
 *test si le squelette doit etre sauvegarde ou non
 */
bool
UndoRedo::testSave(ArRef<Skeleton> tBody) {

  SerializationDependencies dep;
    
  //  ArRef<MemoryIStream> input = new_MemoryIStream(_undoStack.back(), 0);
  ArRef<MemoryIStream> input = MemoryIStream::NEW(_undoStack.back(), 0);
  dep.readFromStream(input);
    
  const StlVector<ArRef<ArObject> >& refsIn = dep.getReferences();

  for(size_t i = 0; i < refsIn.size(); i++) {
    refsIn[i]->unserialize(dep, input);
  }
  
  //  ArRef<Skeleton> uBody = new_Skeleton();
  ArRef<Skeleton> uBody = Skeleton::NEW();

  uBody = ar_down_cast<Skeleton>(refsIn.back());

  bool save = false;

  if(uBody->getNbJoints() == tBody->getNbJoints()) { //test si le nombre d'articulations est identique
    for(int i = 0 ; i < uBody->getNbJoints() && !save ; i++ ) { //pour chaque articulation -> test si la contrainte exactement la meme
      ArRef<JointConstraint> uConstraint = uBody->accessJoint(i)->accessConstraint();
      ArRef<JointConstraint> tConstraint = tBody->accessJoint(i)->accessConstraint();

      if(uConstraint == JointConstraint::nullRef() || tConstraint == JointConstraint::nullRef()) {
	if(uConstraint == JointConstraint::nullRef() && tConstraint != JointConstraint::nullRef()) { save = true; }
	if(uConstraint != JointConstraint::nullRef() && tConstraint == JointConstraint::nullRef()) { save = true; }
      }
      else {
	if(uConstraint->getClass()->isA(JointConstraint1DOF::CLASS()) && tConstraint->getClass()->isA(JointConstraint1DOF::CLASS())) {
	  ArRef<JointConstraint1DOF> u1DOF = ar_down_cast<JointConstraint1DOF>(uConstraint);
	  ArRef<JointConstraint1DOF> t1DOF = ar_down_cast<JointConstraint1DOF>(tConstraint);
	  if(u1DOF->getAxisType() == t1DOF->getAxisType()) {
	    double uIn, uOut, tIn, tOut;
	    t1DOF->getLimitValues(tIn, tOut);
	    u1DOF->getLimitValues(uIn, uOut);
	    if(tIn != uIn || tOut != uOut) { save = true; }
	  }
	  else { save = true; }
	}else if(uConstraint->getClass()->isA(JointConstraint2DOF::CLASS()) && tConstraint->getClass()->isA(JointConstraint2DOF::CLASS())) {
	  ArRef<JointConstraint2DOF> u2DOF = ar_down_cast<JointConstraint2DOF>(uConstraint);
	  ArRef<JointConstraint2DOF> t2DOF = ar_down_cast<JointConstraint2DOF>(tConstraint);
	  if(u2DOF->getPlaneType() == t2DOF->getPlaneType()) {
	    double *uIn[2], *uOut[2], *tIn[2], *tOut[2];
	    uIn[0] = new double();
	    uIn[1] = new double();
	    uOut[0] = new double();
	    uOut[1] = new double();
	    tIn[0] = new double();
	    tIn[1] = new double();
	    tOut[0] = new double();
	    tOut[1] = new double();
	    u2DOF->getLimitValues(uIn, uOut);
	    t2DOF->getLimitValues(tIn, tOut);
	    if(*tIn[0] != *uIn[0] || *tOut[0] != *uOut[0] || *tIn[1] != *uIn[1] || *tOut[1] != *uOut[1]) { save = true; }
	  }
	}else if(uConstraint->getClass()->isA(JointConstraint3DOF::CLASS()) && tConstraint->getClass()->isA(JointConstraint3DOF::CLASS())) {
	  ArRef<JointConstraint3DOF> u3DOF = ar_down_cast<JointConstraint3DOF>(uConstraint);
	  ArRef<JointConstraint3DOF> t3DOF = ar_down_cast<JointConstraint3DOF>(tConstraint);
	  StlVector<Util3D::Dbl3> uKO = u3DOF->getKeyOrientation();
	  StlVector<Util3D::Dbl3> tKO = t3DOF->getKeyOrientation();
	  if(uKO.size() == tKO.size()) {
	    for(unsigned int j ; j < uKO.size() && !save ; j++) {
	      double uX = uKO[j].x, uY = uKO[j].y, uZ = uKO[j].z, tX = tKO[j].x, tY = tKO[j].y, tZ = tKO[j].z;
	      if(uX != tX || uY != tY || uZ != tZ) { save = true; }
	    }
	  }
	  else{ save = true; }
	}else if(uConstraint->getClass()->isA(JointConstraintLocked::CLASS()) && tConstraint->getClass()->isA(JointConstraintLocked::CLASS())) { }
	else {save = true; }
      }
    }
  }

  return save;

}

/*
 *defaire
 */
void
UndoRedo::undo(ArRef<BodyController> bodyCtrl, ArRef<AnchorController> anchorCtrl) {
  
  if( _undoStack.size() > 1 ) {

    _redoStack.push_back( _undoStack.back() );

    _undoStack.pop_back();

    SerializationDependencies dep;

    //extrait les dependances du MemoryBlock
    //    ArRef<MemoryIStream> input = new_MemoryIStream(_undoStack.back(), 0);
    ArRef<MemoryIStream> input = MemoryIStream::NEW(_undoStack.back(), 0);
    dep.readFromStream(input);

    //extraction des references
    const StlVector<ArRef<ArObject> >& refsIn = dep.getReferences();

    //deserialisation && recuperation des ancres
    StlVector<ArRef<Anchor> > anchors;
    for(size_t i = 0; i < refsIn.size(); i++) {
      refsIn[i]->unserialize(dep, input);
      if(refsIn[i]->getClass()->isA(Anchor::CLASS())) {
	anchors.push_back(ar_down_cast<Anchor>(refsIn[i]));
      }
    }
    
    bodyCtrl->setBody( ar_down_cast<Body>(refsIn.back()) );
    
    anchorCtrl->clearAnchors();

    for(unsigned int i = 0 ; i < anchors.size() ; i++ ) {
      anchorCtrl->addAnchor(anchors[i]);
    }

    _onSize();
    
  }

}

/*
 *refaire -> commentaire cf undo
 */
void
UndoRedo::redo(ArRef<BodyController> bodyCtrl, ArRef<AnchorController> anchorCtrl) {
  
  if( !_redoStack.empty() ) {

    SerializationDependencies dep;
    
    //    ArRef<MemoryIStream> input = new_MemoryIStream(_redoStack.back(), 0);
    ArRef<MemoryIStream> input = MemoryIStream::NEW(_redoStack.back(), 0);
    dep.readFromStream(input);
 
    const StlVector<ArRef<ArObject> >& refsIn = dep.getReferences();
    StlVector<ArRef<Anchor> > anchors;
    for(size_t i = 0; i < refsIn.size(); i++) {
      refsIn[i]->unserialize(dep, input);
      if(refsIn[i]->getClass()->isA(Anchor::CLASS())) {
	anchors.push_back(ar_down_cast<Anchor>(refsIn[i]));
      }
    }
    
    bodyCtrl->setBody( ar_down_cast<Body>(refsIn.back()) );
   
    anchorCtrl->clearAnchors();
    
    for(unsigned int i = 0 ; i < anchors.size() ; i++ ) {
      anchorCtrl->addAnchor(anchors[i]);
    }

    _undoStack.push_back(_redoStack.back());

    _redoStack.pop_back();

    _onSize();
    
  }

}

/*
 *vide les piles de sauvegarde
 */
void
UndoRedo::emptyStacks(void) {
  _undoStack.clear();
  _redoStack.clear();
}

/*
 *signaux sur le changement de taille de la pile
 */
void
UndoRedo::_onSize(void) {
  if(!_sizeCB.empty()) {
    SizeEvent evt;
    if(_undoStack.size() > 1) { evt.undoSize = true; }
    else{ evt.undoSize = false; }
    if(_redoStack.size() > 0) { evt.redoSize = true; }
    else{ evt.redoSize = false; }
    _sizeCB.fire(evt);
  }
}
