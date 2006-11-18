/*
*
*Representation des contraintes a 1 et 2 degre de liberte
*
*/

#include "AReVi/arClass.h"
#include "AReVi/Lib3D/material3D.h"
#include "AReVi/Lib3D/transform3D.h"

#include "constraintSh.h"

AR_CLASS_DEF(ConstraintSh, LineSet3D)

ConstraintSh::ConstraintSh(ArCW& arCW) 
: LineSet3D(arCW),
  _color(Util3D::Dbl3(1.0, 1.0, 1.0)),
  _radius(1.0),
  //  _inSh(new_Cone3D()),
  _inSh(Cone3D::NEW()),
  //  _outSh(new_Cone3D()) {
  _outSh(Cone3D::NEW()) {
  setThickness(2);
  addSubPart(_inSh);
  addSubPart(_outSh);
  setValues(- M_PI, M_PI);
}

ConstraintSh::~ConstraintSh(void) {
}

/*
*Initialisation de la couleur en fonction de l'axe de la contrainte
*/
void
ConstraintSh::setColor(float r, float g, float b) {
  _color = Util3D::Dbl3(r, g, b);
  
  StlVector<StlVector<Util3D::Dbl3> >& colors = accessColors();
  for(size_t i = colors.size(); i--;) {
    for(size_t j = colors[i].size(); j--;) {
      colors[i][j] = _color;
    }
  }
  applyChanges(true);

  //  ArRef<Material3D> m = new_Material3D();
  ArRef<Material3D> m = Material3D::NEW();
  m->setDiffuseColor(r, g, b);
  _inSh->writeMaterial(m);
  _outSh->writeMaterial(m);
}

/*
*Retourne la couleur de la contrainte
*/
void
ConstraintSh::getColor(float& r, float& g, float& b) const {
  r = _color.x;
  g = _color.y;
  b = _color.z;
}

/*
*Initialise le rayon de la representation
*/
void
ConstraintSh::setRadius(double radius) {
  _radius = radius;
  _rebuild();
}

/*
*Retourne le rayon de la representation
*/
double
ConstraintSh::getRadius(void) const {
  return _radius;
}

/*
*Initialise les valeurs min et max de la representation
*/
void
ConstraintSh::setValues(double in, double out) {
  _in = in;
  _out = out;
  _rebuild();
}

/*
*Retourne les valeurs min et max de la representation
*/
void
ConstraintSh::getValues(double& in, double& out) const {
  in = _in;
  out = _out;
}

/*
*Regenre la representation de la contrainte
*/
void
ConstraintSh::_rebuild(void) {
  StlVector<StlVector<Util3D::Dbl3> >&  v = accessVertices();
  StlVector<StlVector<Util3D::Dbl3> >&  c = accessColors();

  v.clear();
  c.clear();

  v.push_back(StlVector<Util3D::Dbl3>());
  c.push_back(StlVector<Util3D::Dbl3>());

  double min = _in;
  double max = _out;
  if(max < min) {
    double tmp = min;
    min = max;
    max = tmp;
  }

  for(double angle = min; angle < max; angle += M_PI / 90.0) {
    v[0].push_back(Util3D::Dbl3(0.0,
				  _radius * cos(angle),
				  _radius * sin(angle)));
    c[0].push_back(_color);
  }
  v[0].push_back(Util3D::Dbl3(0.0,
				_radius * cos(max),
				_radius * sin(max)));  
  c[0].push_back(_color);
  applyChanges(true);

  _inSh->setRadius(_radius * 0.25);
  _inSh->setHeight(_radius * 0.5);
  //  ArRef<Transform3D> inTr = new_Transform3D();
  ArRef<Transform3D> inTr = Transform3D::NEW();
  inTr->preTranslate(0.0, 0.0, _radius * 0.25);
  inTr->preRoll(min + M_PI);
  inTr->preTranslate(0.0, _radius * cos(min), _radius * sin(min));
  _inSh->writeTransformation(inTr);

  _outSh->setRadius(_radius * 0.25);
  _outSh->setHeight(_radius * 0.5);
  //  ArRef<Transform3D> outTr = new_Transform3D();
  ArRef<Transform3D> outTr = Transform3D::NEW();
  outTr->preTranslate(0.0, 0.0, _radius * 0.25);
  outTr->preRoll(max);
  outTr->preTranslate(0.0, _radius * cos(max), _radius * sin(max));
  _outSh->writeTransformation(outTr);
}
