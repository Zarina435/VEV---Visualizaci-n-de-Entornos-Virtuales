#include <cmath>
#include "intersect.h"
#include "constants.h"
#include "tools.h"

/* | algo           | difficulty | */
/* |----------------+------------| */
/* | BSPherePlane   |          1 | */
/* | BBoxBBox       |          2 | */
/* | BBoxPlane      |          4 | */

// @@ TODO: test if a BSpheres intersects a plane.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int BSpherePlaneIntersect(const BSphere *bs, Plane *pl) {
	/* =================== PUT YOUR CODE HERE ====================== */
	//Distancia plano al centro.
	float dist = pl->distance(bs->getPosition());   
	//Radio de la esfera. 
    float radio = bs->getRadius();      
    //INTERSECTAN, la distancia es menor al radio.
    if (dist < radio) {
        return IINTERSECT;
    }
    else {
        //dentro o fuera?? mirar la direccion del vector normal
        side = pl->whichSide(bs->getPosition());
        if (side == -1) {
             return -IREJECT;
        }
        else if (side == 1) {
            return +IREJECT;
        }
    }
	/* =================== END YOUR CODE HERE ====================== */
}


// @@ TODO: test if two BBoxes intersect.
//! Returns :
//    IINTERSECT intersect
//    IREJECT don't intersect

int  BBoxBBoxIntersect(const BBox *bba, const BBox *bbb ) {
	/* =================== PUT YOUR CODE HERE ====================== */
	//Comprobar si se solapan en todos los ejes. Si lo hacen, es que intersectan.
	//X
	if((bba->m_max.x()>=bbb->m_min.x())&(bbb->m_max.x()>=bba->m_min.x())){
		//Y
		if((bba->m_max.y()>=bbb->m_min.y())&(bbb->m_max.y()>=bba->m_min.y())){
			//Z
			if((bba->m_max.z()>=bbb->m_min.z())&(bbb->m_max.z()>=bba->m_min.z())){
				return IINTERSECT;
			}
		}
	}
	return IREJECT;
	/* =================== END YOUR CODE HERE ====================== */
}

// @@ TODO: test if a BBox and a plane intersect.
//! Returns :
//   +IREJECT outside
//   -IREJECT inside
//    IINTERSECT intersect

int  BBoxPlaneIntersect (const BBox *theBBox, Plane *thePlane) {
	/* =================== PUT YOUR CODE HERE ====================== */
	Vector3 cercano;
	Vector3 lejano;
	//Sacar puntos cercano y lejano segun la normal.
	if(thePlane->m_n.x()>0){
		cercano.x()= theBBox->m_min.x();
		lejano.x()=theBBox->m_max.x();
	}else{
		cercano.x()= theBBox->m_max.x();
		lejano.x()=theBBox->m_min.x();
	}

	if(thePlane->m_n.y()>0){
		cercano.y()= theBBox->m_min.y();
		lejano.y()=theBBox->m_max.y();
	}else{
		cercano.y()= theBBox->m_max.y();
		lejano.y()=theBBox->m_min.y();
	}

	if(thePlane->m_n.z()>0){
		cercano.z()= theBBox->m_min.z();
		lejano.z()=theBBox->m_max.z();
	}else{
		cercano.z()= theBBox->m_max.z();
		lejano.z()=theBBox->m_min.z();
	}
	//Calcular en qué parte del plano estan los puntos más cercano y más lejano.
	int aux1= thePlane->whichSide(cercano);
	int aux2= thePlane->whichSide(lejano);
    //Si estan en el mismo lado, el BBox no intersecta con el plano.
	if(aux1==aux2){
		//Mirar en qué lado del plano están.
		if (aux1>0){
			return +IREJECT;
		}
		else{
			return -IREJECT;
		}
	}
	//Si estan en lados diferentes, es que intersecta con el plano.
	else{
		return IINTERSECT;
	}
	
	/* =================== END YOUR CODE HERE ====================== */
}

// Test if two BSpheres intersect.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBSphereIntersect(const BSphere *bsa, const BSphere *bsb ) {

	Vector3 v;
	v = bsa->m_centre - bsb->m_centre;
	float ls = v.dot(v);
	float rs = bsa->m_radius + bsb->m_radius;
	if (ls > (rs * rs)) return IREJECT; // Disjoint
	return IINTERSECT; // Intersect
}


// Test if a BSpheres intersect a BBox.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int BSphereBBoxIntersect(const BSphere *sphere, const BBox *box) {

	float d;
	float aux;
	float r;

	r = sphere->m_radius;
	d = 0;

	aux = sphere->m_centre[0] - box->m_min[0];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[0] - box->m_max[0];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = (sphere->m_centre[1] - box->m_min[1]);
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[1] - box->m_max[1];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}

	aux = sphere->m_centre[2] - box->m_min[2];
	if (aux < 0) {
		if (aux < -r) return IREJECT;
		d += aux*aux;
	} else {
		aux = sphere->m_centre[2] - box->m_max[2];
		if (aux > 0) {
			if (aux > r) return IREJECT;
			d += aux*aux;
		}
	}
	if (d > r * r) return IREJECT;
	return IINTERSECT;
}

// Test if a Triangle intersects a ray.
//! Returns :
//    IREJECT don't intersect
//    IINTERSECT intersect

int IntersectTriangleRay(const Vector3 & P0,
						 const Vector3 & P1,
						 const Vector3 & P2,
						 const Line *l,
						 Vector3 & uvw) {
	Vector3 e1(P1 - P0);
	Vector3 e2(P2 - P0);
	Vector3 p(crossVectors(l->m_d, e2));
	float a = e1.dot(p);
	if (fabs(a) < Constants::distance_epsilon) return IREJECT;
	float f = 1.0f / a;
	// s = l->o - P0
	Vector3 s(l->m_O - P0);
	float lu = f * s.dot(p);
	if (lu < 0.0 || lu > 1.0) return IREJECT;
	Vector3 q(crossVectors(s, e1));
	float lv = f * q.dot(l->m_d);
	if (lv < 0.0 || lv > 1.0) return IREJECT;
	uvw[0] = lu;
	uvw[1] = lv;
	uvw[2] = f * e2.dot(q);
	return IINTERSECT;
}

/* IREJECT 1 */
/* IINTERSECT 0 */

const char *intersect_string(int intersect) {

	static const char *iint = "IINTERSECT";
	static const char *prej = "IREJECT";
	static const char *mrej = "-IREJECT";
	static const char *error = "IERROR";

	const char *result = error;

	switch (intersect) {
	case IINTERSECT:
		result = iint;
		break;
	case +IREJECT:
		result = prej;
		break;
	case -IREJECT:
		result = mrej;
		break;
	}
	return result;
}
