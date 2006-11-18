#ifndef _UNDOREDO_H_
#define _UNDOREDO_H_

#include "AReVi/arObject.h"
using namespace AReVi;

#include "hLib/core/body.h"
#include "hLib/core/skeleton.h"
using namespace hLib;


class BodyController;

class AnchorController;

class JointController;


class UndoRedo : public ArObject {

 public :

  AR_CLASS(UndoRedo)
  AR_CONSTRUCTOR(UndoRedo)

  virtual void saveState(ArRef<Body> body);

  virtual void saveState(ArRef<BodyController> bodyCtrl);

  virtual void saveState(ArRef<JointController> jointCtrl);

  virtual bool testSave(ArRef<Skeleton> tBody);

  virtual void undo(ArRef<BodyController> bodyCtrl, ArRef<AnchorController> anchorCtrl);

  virtual void redo(ArRef<BodyController> bodyCtrl, ArRef<AnchorController> anchorCtrl);

  virtual void emptyStacks(void);

  AR_CALLBACK(UndoRedo, Size, _sizeCB, bool undoSize; bool redoSize;)

 protected:

  virtual void _onSize(void);

  CallbackManager<UndoRedo, SizeEvent> _sizeCB;
  
 private :

  StlDeque<ArRef<MemoryBlock> > _undoStack;
  StlDeque<ArRef<MemoryBlock> > _redoStack;


};


#endif //_UNDOREDO_H_
