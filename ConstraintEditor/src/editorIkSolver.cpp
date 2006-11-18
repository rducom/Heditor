#include "hLib/core/joint.h"
#include "hLib/core/jointConstraint.h"

#include "editorIkSolver.h"

AR_CLASS_DEF(EditorIkSolver, IkCcd) 

EditorIkSolver::EditorIkSolver(ArCW& arCW) 
: IkCcd(arCW) {
}

EditorIkSolver::~EditorIkSolver(void) {
}

void
EditorIkSolver::_checkHingeX(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double min, max;
  ar_down_cast<JointConstraint1DOF>(joint->getConstraint())->getLimitValues(min, max);

  rx = _clampWithPreviousValue(min, max, prx, rx);

  joint->setDeltaRotation(Quaterniond(Vector3d(1.0, 0.0, 0.0), rx));
}

void
EditorIkSolver::_checkHingeY(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double min, max;
  ar_down_cast<JointConstraint1DOF>(joint->getConstraint())->getLimitValues(min, max);

  ry = _clampWithPreviousValue(min, max, pry, ry);

  joint->setDeltaRotation(Quaterniond(Vector3d(0.0, 1.0, 0.0), ry));
}

void
EditorIkSolver::_checkHingeZ(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double min, max;
  ar_down_cast<JointConstraint1DOF>(joint->getConstraint())->getLimitValues(min, max);

  rz = _clampWithPreviousValue(min, max, prz, rz);

  joint->setDeltaRotation(Quaterniond(Vector3d(0.0, 0.0, 1.0), rz));
}

void
EditorIkSolver::_checkPivotXY(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double* max[2];
  double* min[2];

  ar_down_cast<JointConstraint2DOF>(joint->getConstraint())->getLimitValues(min, max);

  double xmin = *min[0], xmax = *max[0], ymin = *min[1], ymax = *max[1];

  rx = _clampWithPreviousValue(xmin, xmax, prx, rx);
  ry = _clampWithPreviousValue(ymin, ymax, pry, ry);

  Quaterniond qx(Vector3d(1.0, 0.0, 0.0), rx);
  Quaterniond qy(Vector3d(0.0, 1.0, 0.0), ry);

  joint->setDeltaRotation(qx * qy);
}

void
EditorIkSolver::_checkPivotXZ(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double* max[2];
  double* min[2];

  ar_down_cast<JointConstraint2DOF>(joint->getConstraint())->getLimitValues(min, max);

  double xmin = *min[0], xmax = *max[0], zmin = *min[1], zmax = *max[1];

  rx = _clampWithPreviousValue(xmin, xmax, prx, rx);
  rz = _clampWithPreviousValue(zmin, zmax, prz, rz);

  Quaterniond qx(Vector3d(1.0, 0.0, 0.0), rx);
  Quaterniond qz(Vector3d(0.0, 0.0, 1.0), rz);

  joint->setDeltaRotation(qx * qz);
}

void
EditorIkSolver::_checkPivotYZ(ArRef<Joint> joint, Vector3d /* requestedTranslation */, Quaterniond requestedRotation) {
  double prx, pry, prz;
  joint->getDeltaRotation().getEulerAngles(prx, pry, prz);

  requestedRotation = joint->getDeltaRotation() * requestedRotation;

  double rx, ry, rz;
  requestedRotation.getEulerAngles(rx, ry, rz);

  double* max[2];
  double* min[2];

  ar_down_cast<JointConstraint2DOF>(joint->getConstraint())->getLimitValues(min, max);

  double ymin = *min[0], ymax = *max[0], zmin = *min[1], zmax = *max[1];

  ry = _clampWithPreviousValue(ymin, ymax, pry, ry);
  rz = _clampWithPreviousValue(zmin, zmax, prz, rz);

  Quaterniond qy(Vector3d(0.0, 1.0, 0.0), ry);
  Quaterniond qz(Vector3d(0.0, 0.0, 1.0), rz);

  joint->setDeltaRotation(qy * qz);
}

double
EditorIkSolver::_clampWithPreviousValue(double min, double max, double previousValue,	double value) {
  if(previousValue - value < degreeToRadian(- 300.0)) {
    value -= M_2PI;
  } else if(previousValue - value > degreeToRadian(300)) {
    value += M_2PI;
  }
  if(max <min) {
    double tmp = min;
    min = max;
    max = tmp;
  }
  return (value < min) ? min : ((value > max) ? max : value);
}
