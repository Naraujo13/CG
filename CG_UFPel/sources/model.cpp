#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#define STEPS 40 //number of steps of an animations
#define LOD 100	//Level of detail of the curve
//Constructor
Model::Model(const char *textPath, const char *textSampler, GLuint programID, Mesh &modelMesh, glm::vec3 pos)
{
	texture = loadDDS(textPath);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	mesh = &modelMesh;
	modelMatrixID = glGetUniformLocation(programID, "M");
	modelMatrix = glm::mat4(glm::translate(glm::mat4(1), pos));
	state = 0;
	timeBtwn = 0;
	lastTransformed = glfwGetTime();
}

//Getters
GLuint Model::getModelMatrixID() {
	return modelMatrixID;
}
GLuint Model::getTextureID() {
	return textureID;
}
GLuint* Model::getTexture() {
	return &texture;
}
glm::mat4 Model::getModelMatrix() {
	return modelMatrix;
}
Mesh* Model::getMesh() {
	return mesh;
}
std::vector<Transformation> * Model::getTransformationQueue() {
	return &transformationQueue;
}
int Model::getState() {
	return state;
}

//Setter
void Model::setModelMatrix(glm::mat4 matrix) {
	Model::modelMatrix = matrix;
}
void Model::setState(int newState) {
	Model::state = newState;
}
long double Model::getLastTransformed() {
	return lastTransformed;
}

long double Model::getTimeBtwn() {
	return timeBtwn;
}
//Others


void Model::addCompTransformation(struct translation *t, struct rotation *r, struct scale *s, struct shear *h, long double time){
	//Split and push all the transformations to queue with the time between them
	int steps = ceil (time * STEPS);
	if (steps <= 0)
		steps = 1;
	long double stepTime = (long double)time / (long double)steps - 0.005;

	//std::cout << "Steps:" << steps << "\tStep time: " << stepTime << std::endl;

	glm::mat4 stepTransformation(1.0);

	//Translação
	if (t != NULL) {
		stepTransformation = glm::translate(stepTransformation, t->translationVec / glm::vec3(steps, steps, steps));
	}
	//Rotação
	if (r != NULL) {
		//std::cout << "Rotation step dgs: " << (r->rotationDegrees) << std::endl;
		stepTransformation = glm::rotate(stepTransformation, (float)(r->rotationDegrees / steps), r->rotationVec);
	}
	//Escala
	if (s != NULL) {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(s->scaleVec.x, raiz) / 2 + 0.5, pow(s->scaleVec.y, raiz) / 2 + 0.5, pow(s->scaleVec.z, raiz) / 2 + 0.5));
	}
	if (h != NULL) {
		h->shearVec /= glm::vec3(steps, steps, steps);
		glm::mat4 shearMatrix(1.0);
		shearMatrix[0][1] = h->shearVec.x;
		shearMatrix[0][2] = h->shearVec.y;
		shearMatrix[0][3] = h->shearVec.z;
		stepTransformation = stepTransformation * shearMatrix;
	}

	/*
	//Transformation 1
	if (type == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation / glm::vec3(steps, steps, steps));
	}
	else if (type == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation.x, raiz) / 2 + 0.5, pow(transformation.y, raiz) / 2 + 0.5, pow(transformation.z, raiz) / 2 + 0.5));
	
	}
	else if (type == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees / steps, transformation);
	}
	
	//Transformation 2
	if (type2 == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation2 / glm::vec3(steps, steps, steps));
	}
	else if (type2 == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation2.x, raiz) / 2 + 0.5, pow(transformation2.y, raiz) / 2 + 0.5, pow(transformation2.z, raiz) / 2 + 0.5));
	}
	else if (type2 == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees2 / steps, transformation2);
	}

	//Transformation 3
	if (type3 == 'T') {			//Translação
		stepTransformation = glm::translate(stepTransformation, transformation3 / glm::vec3(steps, steps, steps));
	}
	else if (type3 == 'S') {
		double raiz = (1.0 / (double)steps);
		stepTransformation = glm::scale(stepTransformation, glm::vec3(pow(transformation3.x, raiz) / 2 + 0.5, pow(transformation3.y, raiz) / 2 + 0.5, pow(transformation3.z, raiz) / 2 + 0.5));
	}
	else if (type3 == 'R') {
		stepTransformation = glm::rotate(stepTransformation, rotationDegrees3 / steps, transformation3);
	}
	*/


	//Push all transformations
	int firstFlag = 1;
	for (int i = 0; i < steps; i++) {
		if (firstFlag != 0) {
			transformationQueue.push_back(Transformation(stepTransformation, 0));
			firstFlag=0;
		}
		else {
			transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
	}
}

void Model::rotationAroundPoint(struct rotationAP *p) {
	//Split and push all the transformations to queue with the time between them
	int steps = ceil(p->time * STEPS);
	if (steps <= 0)
		steps = 1;
	long double stepTime = (long double)p->time / (long double)steps - 0.005;
	
	//Gest current pos
	glm::vec3 currentPos;
	currentPos.x = modelMatrix[3][0];
	currentPos.y = modelMatrix[3][1];
	currentPos.z = modelMatrix[3][2];

	//Calculate vector to point
	glm::vec3 toPoint = p->point - currentPos;

	//Transformation matrix
	glm::mat4 stepTransformation(1.0f);


	struct translation t;
	t.translationVec = toPoint * glm::vec3(2);

	glm::vec3 rotationAxis = glm::cross(t.translationVec, vec3(0.0, 0.0, 1.0));
	struct rotation r;
	r.rotationDegrees = p->rotationAngle;
	r.rotationVec = rotationAxis;

	addCompTransformation(&t, &r, NULL, NULL, p->time);
}

/*Bezier evaluation function, given 3 control points and a t, returns the bezier curve point at t (B(t))*/
glm::vec3 evaluateBezierCurve(struct bezier b, double t) {
	glm::vec3 temp1, temp2, temp3;
	temp1 = pow(glm::vec3(1 - t), glm::vec3(2)) * b.controlPoints[0];
	temp2 = (glm::vec3(2 * t*(1 - t))) * b.controlPoints[1];
	temp3 = glm::vec3(pow(t, 2)) * b.controlPoints[2];
	return temp1 + temp2 + temp3;
}

/*Calls bezier evaluation function for a wide range of T, constructing the curve and pushing the transformations to the queue*/
void Model::bezierCurve(struct bezier b) {
	glm::vec3 currentPos;
	glm::vec3 newPoint;
	struct translation trans;
	trans.time = 0;		//MUDAR
	//Gets current pos

	currentPos.x = modelMatrix[3][0];
	currentPos.y = modelMatrix[3][1];
	currentPos.z = modelMatrix[3][2];
	b.controlPoints[0] = currentPos;

	//double step = 0.005 * b.time;
	//if (step < 0.0000001)
		//step = 1;

	std::cout << "Bezier points | vector:" << std::endl;

	for (double t = 0; t < 1; t += 0.01) {
		
		//Calculate next position
		newPoint = evaluateBezierCurve(b, t);
		std::cout << "Point: (" << newPoint.x << "," << newPoint.y << "," << newPoint.z << ")";

		//Calculate vector from current to next
		trans.translationVec = newPoint - currentPos;
		std::cout << "\tVector: (" << trans.translationVec.x << "," << trans.translationVec.y << "," << trans.translationVec.z << ")" << std::endl;
			
		//Puts translation into queue
		addCompTransformation(&trans, NULL, NULL, NULL, b.time/100);

		//Update current pos
		currentPos += trans.translationVec;
	}
}


//Auxiliar para bsplineTest
glm::vec3 GetPoint(int i, struct bspline l) {
	// return 1st point
	if (i<0) {
		return	l.controlPoints[0];
	}
	// return last point
	if (i<4)
		return l.controlPoints[i];

	return l.controlPoints[3];
}


void Model::BSplineTest(struct bspline l) {
	glm::vec3 currentPos;
	currentPos.x = modelMatrix[3][0];
	currentPos.y = modelMatrix[3][1];
	currentPos.z = modelMatrix[3][2];
	l.controlPoints[0] = currentPos;

	struct translation trans;

	for (int i = -3, j = 0; j != 5; ++j, ++i) {

		float t = (float)i / LOD;

		// the t value inverted
		float it = 1.0f - t;

		// calculate blending functions
		float b0 = it*it*it / 6.0f;
		float b1 = (3 * t*t*t - 6 * t*t + 4) / 6.0f;
		float b2 = (-3 * t*t*t + 3 * t*t + 3 * t + 1) / 6.0f;
		float b3 = t*t*t / 6.0f;

		// sum the control points mulitplied by their respective blending functions
		float x = b0 * GetPoint(i + 0, l).x +
			b1 * GetPoint(i + 1, l).x +
			b2 * GetPoint(i + 2, l).x +
			b3 * GetPoint(i + 3, l).x;

		float y = b0 * GetPoint(i + 0, l).y +
			b1 * GetPoint(i + 1, l).y +
			b2 * GetPoint(i + 2, l).y +
			b3 * GetPoint(i + 3, l).y;

		float z = b0 * GetPoint(i + 0, l).z +
			b1 * GetPoint(i + 1, l).z +
			b2 * GetPoint(i + 2, l).z +
			b3 * GetPoint(i + 3, l).z;
		
		std::cout << "Point: (" << x << "," << y << "," << z << ")";

		//Calculates vector
		trans.translationVec = glm::vec3(x,y,z) - currentPos;

		std::cout << "\tVector: (" << trans.translationVec.x << "," << trans.translationVec.y << "," << trans.translationVec.z << ")" << std::endl;

		//Puts translation to queue
		addCompTransformation(&trans, NULL, NULL, NULL, l.time/5 );

		//Update current pos
		currentPos += trans.translationVec;

		// specify the point
		//glVertex3f(x, y, z);
	}
}

void Model::applyTransformation() {
	if (transformationQueue.empty()){
		Model::state = 0;
		std::cout << "empty" << std::endl;
		return;
	}
	else if (glfwGetTime() > lastTransformed + timeBtwn) {
		//Aplica operação
		setModelMatrix(modelMatrix * transformationQueue.front().getTransformation());

		//Apaga operação e atualiza controle
		transformationQueue.erase(transformationQueue.begin());
		lastTransformed = glfwGetTime();
		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();
	
		else
			timeBtwn = 0;
	}
}
