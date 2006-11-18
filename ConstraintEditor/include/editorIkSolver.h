#ifndef _EDITOR_IK_SOLVER_H_
#define _EDITOR_IK_SOLVER_H_

#include "hLib/animation/ik_ccd.h"
using namespace hLib;

class EditorIkSolver : public IkCcd {
public :
  AR_CLASS(EditorIkSolver)
  AR_CONSTRUCTOR(EditorIkSolver)

protected :
  virtual
  void
  _checkHingeX(ArRef<Joint> joint,
	       Vector3d requestedTranslation,
	       Quaterniond requestedRotation);

  virtual
  void
  _checkHingeY(ArRef<Joint> joint,
	       Vector3d requestedTranslation,
	       Quaterniond requestedRotation);

  virtual
  void
  _checkHingeZ(ArRef<Joint> joint,
	       Vector3d requestedTranslation,
	       Quaterniond requestedRotation);

  virtual
  void
  _checkPivotXY(ArRef<Joint> joint,
		Vector3d requestedTranslation,
		Quaterniond requestedRotation);

  virtual
  void
  _checkPivotXZ(ArRef<Joint> joint,
		Vector3d requestedTranslation,
		Quaterniond requestedRotation);

  virtual
  void
  _checkPivotYZ(ArRef<Joint> joint,
		Vector3d requestedTranslation,
		Quaterniond requestedRotation);

  inline
  double
  _clampWithPreviousValue(double min, 
			  double max, 
			  double previousValue,
			  double value);
};

#endif // _EDITOR_IK_SOLVER_H_
