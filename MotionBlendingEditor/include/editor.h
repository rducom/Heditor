#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "AReVi/Lib3D/object3D.h"
#include "AReVi/Lib3D/osdText.h"
/* #include "AReVi/Lib3D/simpleInteractor.h" */
#include "AReVi/Shapes/shapePart3D.h"
#include "AReVi/Utils/stlString_priv.h"
using namespace AReVi;

#include "hLib/core/body.h"
#include "hLib/core/skin.h"
#include "hLib/core/skeleton.h"
#include "hLib/utils/loader.h"
#include "hLib/animation/animationBlendCycle.h"
using namespace hLib;

#include "gtkAReViWidget.h"
#include "gtkWindowPlugger.h"
using namespace GtkTools;

#include "cameraInteractor.h"
#include "animBody.h"
#include "canvasController.h"

#include <string.h>


enum AnimColumnID {
  COLUMN_ANIM_NAME,
  COLUMN_ANIM_DATA,
  COLUMN_ANIM_BODY
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


  //Menubar : 

  static void _onFileQuitMI(GtkImageMenuItem*, gpointer);
  static void _onFileCutMI(GtkImageMenuItem*, gpointer);
  static void _onFileCopyMI(GtkImageMenuItem*, gpointer);
  static void _onFilePasteMI(GtkImageMenuItem*, gpointer);
  static void _onFileHelpMI(GtkImageMenuItem*, gpointer);

  static void _onFileNew(GtkToolButton*, gpointer);
  static void _onFileOpen(GtkToolButton*, gpointer);
  static void _onFileImport(GtkToolButton*, gpointer);
  static void _onFileImportFolder(GtkToolButton*, gpointer data);
  static void _onFileSelectBody(GtkToolButton* /*tb*/, gpointer data);
  static void _onFileExport(GtkToolButton*, gpointer data);
  static void _onFileSave(GtkToolButton*, gpointer);
  static void _onFileSaveAs(GtkToolButton*, gpointer);
  static void _onFileHelp(GtkToolButton*, gpointer);    
  static void _onFileQuit(GtkToolButton*, gpointer);
  
  static void _onButtonUndo(GtkToolButton*, gpointer);
  static void _onButtonRedo(GtkToolButton*, gpointer);
  static void _onButtonLink(GtkToolButton*, gpointer data);
  static void _onButtonUnlink(GtkToolButton*, gpointer data);
  static void _onButtonDelete(GtkToolButton*, gpointer data);

  static void _onButtonBegin(GtkToolButton* /*tb*/, gpointer data);
  static void _onButtonPlay(GtkToolButton* /*tb*/, gpointer data);
  static void _onButtonPause(GtkToolButton* /*tb*/, gpointer data);
  static void _onButtonStop(GtkToolButton* /*tb*/, gpointer data);
  static void _onButtonEnd(GtkToolButton* /*tb*/, gpointer data);
  static void _onTimeLine(GtkHScale* item, gpointer data);


  static void _onScaleLine(GtkHScale* item, gpointer data);
	
	ArRef<Item>		accessCurrentItem(void) 					const {return _current;}
	void 					setCurrentItem(ArRef<Item> item);

	// Refresh blending params & AReVi Window
  static void _onSelectAnim(GtkTreeSelection*, gpointer);

  // Drag and Drop callback's
	static void DNDBeginCB(GtkWidget *widget, GdkDragContext *dc, gpointer data);
	static void DNDEndCB(	GtkWidget *widget, GdkDragContext *dc, gpointer data);

	
  //-------------------------------------------------------------------
  // Utilities functions
  //-------------------------------------------------------------------
  
  virtual bool _saveAnimation(StlString filename);

  virtual void _setFilename(StlString filename);

  virtual bool _askBeforeGoOn(void);

  virtual void _fillAnimTree(void);

  virtual void _setBody(ArRef<Body> body, char* name);

  virtual void _onTimeCB(const AnimBody::TimeEvent& evt);

  virtual void _onSetanimCB(const CanvasController::SetanimEvent& evt);
  
protected :

	/* AReVi integration*/
  ArRef<EmbeddedWindow3D>		_view; 			// AReVi viewer
  ArRef<GtkWindowPlugger>		_plugger;   // AReVi Wigdet implementation

	/* environnement controllers */
  ArRef<CanvasController>		_canvasCtrl; // Canvas garphics & manipulation handler
  ArRef<CameraInteractor>		_interactor; // AReVi 3D Interactor
	ArRef<AnimBody>           _bodyObj; 	 // Control blending manipulation

	/* Current context */
	ArRef<Item>								_current; // Main animation item currently displayed

  StlVector<HLibLoader::ExtensionInfo>	 _extensions; // Supported hLib extensions
	StlString 	_filename;				// used for file openning
  StlString		_importFilename;  // used for file importing


  StlMap<StlString, ArRef<AnimationCycle> > 	_anims; // Map of all's animations in MBE context
	StlMap<StlString, ArRef<Body> > 						_bodys; // and associated body's

	StlVector<ArRef<AnimationBlendCycle> > 			_blendCycles; // Vector of all blend's

  // Body


  //UndoRedo
  //ArRef<UndoRedo>					_undoRedo;
};

#endif // _EDITOR_H_
