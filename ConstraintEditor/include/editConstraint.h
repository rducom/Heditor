#ifndef _EDIT_CONSTRAINT_H_
#define _EDIT_CONSTRAINT_H_

#include "gtkAReViWidget.h"
using namespace GtkTools;

#include "hLib/core/joint.h"
using namespace hLib;

#include "jointController.h"
#include "undoRedo.h"

enum KeyColumnID {
  COLUMN_KEY_PITCH,
  COLUMN_KEY_YAW,
  COLUMN_KEY_DATA
};

class EditConstraint : public GtkAReViWidget {
public :
  AR_CLASS(EditConstraint)
  AR_CONSTRUCTOR_1(EditConstraint,ArRef<UndoRedo>,undoRedo)

  virtual
  void
  fill(ArRef<JointController> jointCtrl);

protected :

  virtual
  void
  _onValueChangedCB(const JointController::ValueEvent& evt); 

  virtual
  void
  _onSelectJointCB(const JointController::SelectEvent& evt);

  virtual void _onChangeMotionCB(const JointController::ChangeMotionEvent& evt);

  static
  void
  _onConstraintToggled(GtkToggleButton*, gpointer);

  static
  void
  _onApply(GtkButton*, gpointer);

  static 
  void
  _onCancel(GtkButton*, gpointer);

  static
  void
  _fillNotebookPage(GtkNotebook*, GtkNotebookPage*, guint, gpointer);

  static
  void
  _onSpinButtonValueChanged(GtkSpinButton*, gpointer);

  virtual
  void
  _adjustSpinner(GtkSpinButton* spinLow, double low, GtkSpinButton* spinUp, double up);

  virtual
  void
  _extractKeyOrientation(StlList<Util3D::Dbl3>& list, StlVector<Util3D::Dbl3>& vector);

  static
  void 
  _onAddKey(GtkButton*, gpointer data);

  static
  void 
  _onRemoveKey(GtkButton*, gpointer data);

  static
  void
  _onSelectKey(GtkTreeSelection*, gpointer);

  virtual 
  void 
  _updateKeyListView(void);

protected :
  bool _editing;
  ArRef<JointController> _jointCtrl;

  double _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;
  ArRef<Joint> _joint;
  ArRef<UndoRedo> _undoRedo;
  bool _spin;
};

#endif // _EDIT_CONSTRAINT_H_
