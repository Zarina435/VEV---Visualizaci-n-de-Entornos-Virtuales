#include "tools.h"
#include "avatar.h"
#include "scene.h"

Avatar::Avatar(const std::string &name, Camera * cam, float radius) :
	m_name(name), m_cam(cam), m_walk(false) {
	Vector3 P = cam->getPosition();
	m_bsph = new BSphere(P, radius);
}

Avatar::~Avatar() {
	delete m_bsph;
}

void Avatar::setCamera(Camera *thecam) {
	m_cam = thecam;
}

Camera *Avatar::getCamera() const {
	return m_cam;
}


bool Avatar::walkOrFly(bool walkOrFly) {
	bool walk = m_walk;
	m_walk = walkOrFly;
	return walk;
}

bool Avatar::getWalkorFly() const {
	return m_walk;
}

//
// AdvanceAvatar: advance 'step' units
//
// @@ TODO: Change function to check for collisions. If the destination of
// avatar collides with scene, do nothing.
//
// Return: true if the avatar moved, false if not.

bool Avatar::advance(float step) {
	//Desde el nodo raíz.
	Node *rootNode = Scene::instance()->rootNode();
	/* =================== PUT YOUR CODE HERE ====================== */
	//Nos movemos hacia adelante, en caso de que colisione con algún objeto nos moveremos hacia atrás.
	if (m_walk){
		m_cam->walk(step);
	}
	else{
		m_cam->fly(step);
	}
	//El avatar se ha desplazado.
	bool avatar_desplazado = true;
	//Actualizamos la nueva posición de la cámara.
	m_bsph->setPosition(m_cam->getPosition());
	//Comprobamos si colisiona con algún objeto.
	const Node *chocan = rootNode->checkCollision(m_bsph);
	//En caso de que colisione.
	if(chocan != 0){
		//Nos movemos hacia atrás.
		if (m_walk){
			m_cam->walk(-step);
		}
		else{
			m_cam->fly(-step);
		}	
		//Volvemos a actualizar la posición de la cámara.
		m_bsph->setPosition(m_cam->getPosition());
		//El avatar no se ha desplazado.
		avatar_desplazado = false;
	}
	//devolvemos si se ha movido o no.
	return avatar_desplazado;
	/* =================== END YOUR CODE HERE ====================== */
}

void Avatar::leftRight(float angle) {
	if (m_walk)
		m_cam->viewYWorld(angle);
	else
		m_cam->yaw(angle);
}

void Avatar::upDown(float angle) {
	m_cam->pitch(angle);
}

void Avatar::panX(float step) {
	m_cam->panX(step);
}

void Avatar::panY(float step) {
	m_cam->panY(step);
}

void Avatar::print() const { }
