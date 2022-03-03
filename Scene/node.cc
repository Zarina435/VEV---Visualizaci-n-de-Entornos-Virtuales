#include <cstdio>
#include <cassert>
#include "node.h"
#include "nodeManager.h"
#include "intersect.h"
#include "bboxGL.h"
#include "renderState.h"

using std::string;
using std::list;
using std::map;

// Recipe 1: iterate through children:
//
//    for(auto it = m_children.begin(), end = m_children.end();
//        it != end; ++it) {
//        auto theChild = *it;
//        theChild->print(); // or any other thing
//    }
/*  push(modelview)
	cargar transformacion en la modelview
	si es hoja-> dibujo
	sino-> llamar recursivamente a los hijos*/
/*	si hoja-> push
			  cargar transformacion global
			  dibujo
			  pop*/
Node::Node(const string &name) :
	m_name(name),
	m_parent(0),
	m_gObject(0),
	m_light(0),
	m_shader(0),
	m_placement(new Trfm3D),
	m_placementWC(new Trfm3D),
	m_containerWC(new BBox),
	m_checkCollision(true),
	m_isCulled(false),
	m_drawBBox(false) {}

Node::~Node() {
	delete m_placement;
	delete m_placementWC;
	delete m_containerWC;
}

static string name_clone(const string & base) {

	static char MG_SC_BUFF[2048];
	Node *aux;

	int i;
	for(i = 1; i < 1000; i++) {
		sprintf(MG_SC_BUFF, "%s#%d", base.c_str(), i);
		string newname(MG_SC_BUFF);
		aux = NodeManager::instance()->find(newname);
		if(!aux) return newname;
	}
	fprintf(stderr, "[E] Node: too many clones of %s\n.", base.c_str());
	exit(1);
	return string();
}

Node* Node::cloneParent(Node *theParent) {

	string newName = name_clone(m_name);
	Node *newNode = NodeManager::instance()->create(newName);
	newNode->m_gObject = m_gObject;
	newNode->m_light = m_light;
	newNode->m_shader = m_shader;
	newNode->m_placement->clone(m_placement);
	newNode->m_parent = theParent;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		newNode->m_children.push_back(theChild->cloneParent(this));
	}
	return newNode;
}


Node *Node::clone() {
	return cloneParent(0);
}

///////////////////////////////////
// transformations

void Node::attachGobject(GObject *gobj ) {
	if (!gobj) {
		fprintf(stderr, "[E] attachGobject: no gObject for node %s\n", m_name.c_str());
		exit(1);
	}
	if (m_children.size()) {
		fprintf(stderr, "EW] Node::attachGobject: can not attach a gObject to node (%s), which already has children.\n", m_name.c_str());
		exit(1);
	}
	m_gObject = gobj;
	propagateBBRoot();
}

GObject *Node::detachGobject() {
	GObject *res = m_gObject;
	m_gObject = 0;
	return res;
}

GObject *Node::getGobject() {
	return m_gObject;
}

void Node::attachLight(Light *theLight) {
	if (!theLight) {
		fprintf(stderr, "[E] attachLight: no light for node %s\n", m_name.c_str());
		exit(1);
	}
	m_light = theLight;
}

Light * Node::detachLight() {
	Light *res = m_light;
	m_light = 0;
	return res;
}

void Node::attachShader(ShaderProgram *theShader) {
	if (!theShader) {
		fprintf(stderr, "[E] attachShader: empty shader for node %s\n", m_name.c_str());
		exit(1);
	}
	m_shader = theShader;
}

ShaderProgram *Node::detachShader() {
	ShaderProgram *theShader = m_shader;
	m_shader = 0;
	return theShader;
}

ShaderProgram *Node::getShader() {
	return m_shader;
}

void Node::setDrawBBox(bool b) { m_drawBBox = b; }
bool Node::getDrawBBox() const { return m_drawBBox; }

bool Node::isCulled() const { return m_isCulled; }

void Node::setCheckCollision(bool b) {
	m_checkCollision = b;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCheckCollision(b);
	}
}

bool Node::getCheckCollision() const { return m_checkCollision; }

///////////////////////////////////
// transformations

void Node::initTrfm() {
	Trfm3D I;
	m_placement->swap(I);
	// Update Geometric state
	updateGS();
}

void Node::setTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] setTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->clone(M);
	// Update Geometric state
	updateGS();
}

void Node::addTrfm(const Trfm3D * M) {
	if (!M) {
		fprintf(stderr, "[E] addTrfm: no trfm for node %s\n", m_name.c_str());
		exit(1);
	}
	m_placement->add(M);
	// Update Geometric state
	updateGS();
}

void Node::translate(const Vector3 & P) {
	static Trfm3D localT;
	localT.setTrans(P);
	addTrfm(&localT);
};

void Node::rotateX(float angle ) {
	static Trfm3D localT;
	localT.setRotX(angle);
	addTrfm(&localT);
};

void Node::rotateY(float angle ) {
	static Trfm3D localT;
	localT.setRotY(angle);
	addTrfm(&localT);
};

void Node::rotateZ(float angle ) {
	static Trfm3D localT;
	localT.setRotZ(angle);
	addTrfm(&localT);
};

void Node::scale(float factor ) {
	static Trfm3D localT;
	localT.setScale(factor);
	addTrfm(&localT);
};

///////////////////////////////////
// tree operations

Node *Node::parent() {
	if (!m_parent) return this;
	return m_parent;
}

/**
 * nextSibling: Get next sibling of node. Return first sibling if last child.
 *
 */

Node *Node::nextSibling() {
	Node *p = m_parent;
	if(!p) return this;
	list<Node *>::iterator end = p->m_children.end();
	list<Node *>::iterator it = p->m_children.begin();
	while(it != end && *it != this) ++it;
	assert(it != end);
	++it;
	if (it == end) return *(p->m_children.begin());
	return *it;
}

/**
 * firstChild: Get first child of node. Return itself if leaf node.
 *
 * @param theNode A pointer to the node
 */

Node *Node::firstChild() {
	if (!m_children.size()) return this;
	return *(m_children.begin());
}
	/*Vector3 Trfm3D::transformPoint(const Vector3 & P) const;
 Vector3 Trfm3D::transformVector(const Vector3 & V) const;
 void Trfm3D::setRotAxis(const Vector3 & V, const Vector3 & P, float angle );*/
//Nota: Para visualizar los resultados de este apartado utilizaremos el programa browser_go 
Node * Node::cycleChild(size_t idx) {

	size_t m = idx % m_children.size();
	size_t i = 0;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		if (i == m) return *it;
		i++;
	}
	return 0;
}

// @@ TODO:
//
// Add a child to node
// Print a warning (and do nothing) if node already has an gObject.

void Node::addChild(Node *theChild) {

	if (theChild == 0) return;
	if (m_gObject) {
		/* =================== PUT YOUR CODE HERE ====================== */
		// node has a gObject, so print warning
         printf("ERROR");
		/* =================== END YOUR CODE HERE ====================== */
	} else {
		/* =================== PUT YOUR CODE HERE ====================== */
		// node does not have gObject, so attach child
		//No es un nodo hoja.
		theChild->m_parent=this;
		m_children.push_back(theChild);
		updateGS();
		/* =================== END YOUR CODE HERE ====================== */

	}
}

void Node::detach() {

	Node *theParent;
	theParent = m_parent;
	if (theParent == 0) return; // already detached (or root node)
	m_parent = 0;
	theParent->m_children.remove(this);
	// Update bounding box of parent
	theParent->propagateBBRoot();
}

// @@ TODO: auxiliary function
//
// given a node:
// - update the BBox of the node (updateBB)
// - Propagate BBox to parent until root is reached
//
// - Preconditions:
//    - the BBox of thisNode's children are up-to-date.
//    - placementWC of node and parents are up-to-date

void Node::propagateBBRoot() {
	/* =================== PUT YOUR CODE HERE ====================== */
	//Actualizamos la BBox.
	updateBB();
	//m_parent->updateBB();
	//Si tiene padre, propagamos hacia arriba. Si no tiene padre, significa que es la raíz y no hay que propagar.
	if(m_parent){
		m_parent->propagateBBRoot();
	}

	/* =================== END YOUR CODE HERE ====================== */
}

// @@ TODO: auxiliary function
//
// given a node, update its BBox to World Coordinates so that it includes:
//  - the BBox of the geometricObject it contains (if any)
//  - the BBox-es of the children
//
// Note: Bounding box is always in world coordinates
//
// Precontitions:
//
//     * m_placementWC is up-to-date
//     * m_containerWC of children are up-to-date
//
// These functions can be useful
//
// Member functions of BBox class (from Math/bbox.h):
//
//    void init(): Set BBox to be the void (empty) BBox
//    void clone(const BBox * source): Copy from source bounding box
//    void transform(const Trfm3D * T): Transform BBox by applying transformation T
//    void include(BBox *B): Change BBox so that it also includes BBox B
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

void Node::updateBB () {
	/* =================== PUT YOUR CODE HERE ====================== */
	//Si es un nodo hoja.
	if(m_gObject){
		m_containerWC->clone(m_gObject->getContainer());
		m_containerWC->transform(m_placementWC);
	}
	//Si no es un nodo hoja.
	else{
		m_containerWC->init();
		for(auto it = m_children.begin(), end = m_children.end();it != end; ++it) {
        auto theChild = *it;
		m_containerWC->include(theChild->m_containerWC);
    }
		
	}
	
	/* =================== END YOUR CODE HERE ====================== */
}

// @@ TODO: Update WC (world coordinates matrix) of a node and
// its bounding box recursively updating all children.
//
// given a node:
//  - update the world transformation (m_placementWC) using the parent's WC transformation
//  - recursive call to update WC of the children
//  - update Bounding Box to world coordinates
//
// Precondition:
//
//  - m_placementWC of m_parent is up-to-date (or m_parent == 0)
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

/*	Soy el nodo root? Tengo padre?? (si mi padre es 0, es que soy el root)
	m_placementWC=m_placement
	si-> llamar recursivamente a upadateWC con tus hijos si los tienes
	no-> m_placementWC= COMPOSICION(m_placementWC_de_mi_padre, m_placement)*/
void Node::updateWC() {
	/* =================== PUT YOUR CODE HERE ====================== */
	if (m_parent==0){
		m_placementWC->clone(m_placement);
	}
	else{
		m_placementWC->clone(m_parent->m_placementWC);
		m_placementWC->add(m_placement);
		
	}
	for(auto it = m_children.begin(), end = m_children.end();it != end; ++it) {
        	auto theChild = *it;
        	theChild->updateWC(); // or any other thing
		}
	updateBB();
	
	/* =================== END YOUR CODE HERE ====================== */
}

// @@ TODO:
//
//  Update geometric state of a node.
//
// given a node:
// - Update WC transformation of sub-tree starting at node (updateWC)
// - Propagate Bounding Box to root (propagateBBRoot), starting from the parent, if parent exists.

void Node::updateGS() {
	/* =================== PUT YOUR CODE HERE ====================== */
	//Actualizar, y si es padre propagar.
	updateWC();
	if(m_parent){
		m_parent->propagateBBRoot();
	}
	/* =================== END YOUR CODE HERE ====================== */
}


// @@ TODO:
// Draw a (sub)tree.
//
// These functions can be useful:
//
// RenderState *rs = RenderState::instance();
//
// rs->addTrfm(RenderState::modelview, T); // Add T transformation to modelview
// rs->push(RenderState::modelview); // push current matrix into modelview stack
// rs->pop(RenderState::modelview); // pop matrix from modelview stack to current
//
// gobj->draw(); // draw geometry object (gobj)
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

void Node::draw() {

	ShaderProgram *prev_shader = 0;
	RenderState *rs = RenderState::instance();

	if (m_isCulled) return;

	// Set shader (save previous)
	if (m_shader != 0) {
		prev_shader = rs->getShader();
		rs->setShader(m_shader);
	}

	// Print BBoxes
	if(rs->getBBoxDraw() || m_drawBBox)
		BBoxGL::draw( m_containerWC );

	/* =================== PUT YOUR CODE HERE ====================== */
	/*SI SOY UN NODO HOJA
		DIBUJAR MI OBJETO
	SINO 
		PASAR A DIBUJAR MIS HIJOS	*/
	
	if (m_gObject){
		//NODO HOJA
		rs->push(RenderState::modelview);
	    rs->addTrfm(RenderState::modelview, this->m_placementWC);
		m_gObject->draw(); //dibujar objeto
		rs->pop(RenderState::modelview);
	}else{
		//NODO INTERMEDIO, recorrer la lista de sus hijos
		for(list<Node *>::iterator it = m_children.begin(), end = m_children.end();it != end; ++it) {
        		Node *theChild = *it;
        		theChild->draw(); // or any other thing
		}
		
	}
	/* =================== END YOUR CODE HERE ====================== */

	if (prev_shader != 0) {
		// restore shader
		rs->setShader(prev_shader);
	}
	//rs->pop(RenderState::modelview);
}

// Set culled state of a node's children

void Node::setCulled(bool culled) {
	m_isCulled = culled;
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		Node *theChild = *it;
		theChild->setCulled(culled); // Recursive call
	}
}

// @@ TODO: Frustum culling. See if a subtree is culled by the camera, and
//          update m_isCulled accordingly.

void Node::frustumCull(Camera *cam) {
	/* =================== PUT YOUR CODE HERE ====================== */

	/* =================== END YOUR CODE HERE ====================== */
}

// @@ TODO: Check whether a BSphere (in world coordinates) intersects with a
// (sub)tree.
//
// Return a pointer to the Node which collides with the BSphere. 0
// if not collision.
//
// Note:
//
//    See Recipe 1 at the beggining of the file in for knowing how to
//    iterate through children.

const Node *Node::checkCollision(const BSphere *bsph) const {
	if (!m_checkCollision) return 0;
	/* =================== PUT YOUR CODE HERE ====================== */

	/* =================== END YOUR CODE HERE ====================== */
}
void Node::print_trfm(int sep) const {
	std::string delim(sep,' ');
	printf("%sNode:%s\n", delim.c_str(), m_name.c_str());
	m_placementWC->print(delim);
	for(auto it = m_children.begin(), end = m_children.end();
		it != end; ++it) {
		const Node *theChild = *it;
		theChild->print_trfm(sep + 1);
	}
}
/* empieza con el root, mira a ver si hay interseccion. 
Si no es un nodo hoja, mira a ver que pasa con sus hijos, hay colision?
cuando haya una colisión con un objeto salgo, no hay que checkear nada más.

SI HAY COLISIÓN CON OBJETO-> SALGO //Caso básico de la recursión
NO HAY COLISIÓN CON OBJETO-> MIRAR HIJOS (Recursivamente) //profundidad y anchura . 
							SI no hay colision con ese, con sus hijos tampoco (recorrido en anchura mejor)
							
							
con el nodo root miramos si colisiona la esfera  con el BBox, se llam recursivamente
a la función que lo checkea. Mirar con que hijos colisiona. Hay que encontrar el nodo
hoja con el que colisiona.*/