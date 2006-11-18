#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Lib3D/osdText.h"
#include "AReVi/Shapes/shapePart3D.h"
#include "AReVi/Utils/stlString_priv.h"
using namespace AReVi;

#include "hLib/core/body.h"
#include "hLib/core/skin.h"
#include "hLib/core/skeleton.h"
#include "hLib/utils/loader.h"
using namespace hLib;

#include "gtkAReViWidget.h"
#include "gtkWindowPlugger.h"
using namespace GtkTools;

#include "planeInteractor.h"
#include "multiView.h"
#include "anchorController.h"
#include "bodyController.h"
#include "jointController.h"
#include "planeInteractor.h"
#include "constraintInteractor.h"
#include "editConstraint.h"
#include "editAnchor.h"
#include "axis.h"
#include "undoRedo.h"

#include <string.h>

enum JointColumnID {
  COLUMN_JOINT_NAME,
  COLUMN_JOINT_DATA
};

enum AnchorColumnID {
  COLUMN_ANCHOR_NAME,
  COLUMN_ANCHOR_LINK,
  COLUMN_ANCHOR_DATA
};

class Editor : public GtkAReViWidget {
public :
  AR_CLASS(Editor)
  AR_CONSTRUCTOR(Editor)

protected :

  //-------------------------------------------------------------------
  // CALLBACKS
  //-------------------------------------------------------------------

  void _connectSignals(void);


  //Menubar : Fichier

  static void _onFileNewMI(GtkImageMenuItem*, gpointer);

  static  void _onFileOpenMI(GtkImageMenuItem*, gpointer);

  static void _onFileImportMI(GtkImageMenuItem*, gpointer);

  static void _onFileSaveMI(GtkImageMenuItem*, gpointer);

  static void _onFileSaveAsMI(GtkImageMenuItem*, gpointer);

  static void _onFileQuitMI(GtkImageMenuItem*, gpointer);

  //Menubar : Edition

  static void _onFileUndoMI(GtkImageMenuItem*, gpointer);

  static void _onFileRedoMI(GtkImageMenuItem*, gpointer);

  //Menubar : Affichage

  static void _onDisplayAdjustBestMI(GtkImageMenuItem*, gpointer);

  static void _onDisplaySplitViewMI(GtkCheckMenuItem*, gpointer);

  static void _onDisplayWireframeMI(GtkCheckMenuItem*, gpointer);

  static void _onDisplaySkeletonMI(GtkCheckMenuItem*, gpointer);

  static void _onDisplayAnchorMI(GtkCheckMenuItem*, gpointer);

  static void _onDisplayMeshMI(GtkCheckMenuItem*, gpointer);

  //Menubar : Aide

  static void _onFileHelpMI(GtkImageMenuItem*, gpointer);


  // Toolbar

  static void _onFileNew(GtkToolButton*, gpointer);

  static void _onFileOpen(GtkToolButton*, gpointer);

  static void _onFileSave(GtkToolButton*, gpointer);

  static void _onFileSaveAs(GtkToolButton*, gpointer);

  static void _onFileUndo(GtkToolButton*, gpointer);

  static void _onFileRedo(GtkToolButton*, gpointer);

  static void _onFileImport(GtkToolButton*, gpointer);

  static void _onDisplaySplitView(GtkToggleToolButton*, gpointer);

  static void _onDisplayAdjustBest(GtkToolButton*, gpointer);

  static void _onFileHelp(GtkToolButton*, gpointer);

  static void _onFileQuit(GtkToolButton*, gpointer);

  // Toolbar : Options d'affichage

  static void _onDisplayWireframe(GtkToggleButton*, gpointer);

  static void _onDisplaySkeleton(GtkToggleButton*, gpointer);

  static void _onDisplayAnchor(GtkToggleButton*, gpointer);

  static void _onDisplayMesh(GtkToggleButton*, gpointer);

  // Joint tree view callbacks
  /*
  static
  void
  _onActivateJoint(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
  */
  static void _onSelectJoint(GtkTreeSelection*, gpointer);

  // Anchor list view callbacks

  static void _onActivateAnchor(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);

  static void _onSelectAnchor(GtkTreeSelection*, gpointer);

  static void _onAddAnchor(GtkButton*, gpointer);

  static void _onRemoveAnchor(GtkButton*, gpointer);

  //-------------------------------------------------------------------
  // Utilities functions
  //-------------------------------------------------------------------
  
  virtual bool _saveBody(StlString filename);

  virtual void _setFilename(StlString filename);

  // true -> can continue operations
  // false -> user cancel
  virtual bool _askBeforeGoOn(void);

  virtual void _updateJointTreeView(void);

  virtual void _fillJointTree(ArConstRef<Joint> node, GtkTreeIter* parentIter);

  virtual void _updateAnchorListView(void);

  virtual bool _search_joint_in_treeview(GtkTreeModel* model,const char* JN, GtkTreeIter* iter, GtkTreeIter** iter_out);  

  virtual void _onBodyCB(const BodyController::BodyEvent& evt);

  virtual void _onAnchorCB(const AnchorController::AnchorEvent& evt);


  //-------------------------------------------------------------------
  // Constraints in viewer 
  //-------------------------------------------------------------------

  virtual void _onJointSelectCB(const JointController::SelectEvent& evt);
  
  virtual void _onJointConstraintCB(const JointController::ConstraintEvent& evt);

  virtual void _onJointValueCB(const JointController::ValueEvent& evt);

  virtual void _onOSDValueCB(const Axis::ValueEvent& evt);

  virtual void _onSizeCB(const UndoRedo::SizeEvent& evt);

/*   virtual void _constraintChangeCB(const C3DOF::ChangeEvent& evt); */

protected :
  // AReVi viewer
  ArRef<MultiView> _multiView;

  ArRef<OSDText> _osdFilename;
  ArRef<PlaneInteractor> _planeInteractors[3];
  ArRef<ConstraintInteractor> _constraintInteractors[2];

  // Dialog windows
  ArRef<EditAnchor> _editAnchor;
  ArRef<EditConstraint> _editConstraint;

  // Editor variables
  StlVector<HLibLoader::ExtensionInfo> _extensions;
  StlString _filename;
  StlString _importFilename;

  // Body
  ArRef<Object3D> _bodyObj;
  ArRef<AnchorController> _anchorCtrl;
  ArRef<BodyController> _bodyCtrl;
  ArRef<JointController> _jointCtrl;
  StlVector<ArRef<Skin> > _parts;

  //UndoRedo
  ArRef<UndoRedo> _undoRedo;

  // Constraints
  ArRef<Axis> _osdAxis[3];
  bool _editing;
};

#endif // _EDITOR_H_
