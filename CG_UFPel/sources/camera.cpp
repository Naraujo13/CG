#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <glm/gtx/transform2.hpp>
#include <random>

#include <camera.hpp>
#include <modelManager.hpp>

#include <glm/gtx/spline.hpp>

#define STEPS 40 //number of steps of an animations
#define LOD 40//Level of detail of the curve

//Construtor
Camera::Camera(GLuint programID, glm::mat4 ViewMatrix, glm::mat4 ProjectionMatrix)
{
	ViewMatrixID = glGetUniformLocation(programID, "V");
	Camera::ViewMatrix = ViewMatrix;
	Camera::ProjectionMatrix = ProjectionMatrix;
	Camera::timeBtwn = 0;
	Camera::lastTransformed = 0;
	Camera::state = 0;
}

//Getters
glm::mat4 Camera::getProjectionMatrix() {
	return ProjectionMatrix;
}

glm::mat4 Camera::getViewMatrix() {
	return ViewMatrix;
}

GLuint Camera::getViewMatrixID() {
	return ViewMatrixID;
}

std::vector<Transformation> * Camera::getTransformationQueue() {
	return &transformationQueue;
}
int Camera::getState() {
	return state;
}
long double Camera::getLastTransformed() {
	return lastTransformed;
}

long double Camera::getTimeBtwn() {
	return timeBtwn;
}

//Setters
void Camera::setViewMatrix(glm::mat4 ViewMatrix) {
	Camera::ViewMatrix = ViewMatrix;
}

void Camera::setProjectionMatrix(glm::mat4 ProjectionMatrix) {
	Camera::ProjectionMatrix = ProjectionMatrix;
}
void Camera::setState(int newState) {
	Camera::state = newState;
}

//Transformations

void Camera::aroundPoint(struct rotationAP *cp) {

	if (cp != NULL) {
		//Animation time control
		int steps = ceil(cp->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)cp->time / (long double)steps - 0.005;

		//Transformation
		int firstFlag = 1;
		glm::mat4 rotationMatrix(1.0f);
		glm::mat4 toPointMatrix(1.0f);
		glm::mat4 fromPointMatrix(1.0f);

		glm::vec3 currentPos = glm::vec3(ViewMatrix[3][0], ViewMatrix[3][1], ViewMatrix[3][2]);
		glm::vec3 aux = (cp->point - (currentPos*glm::vec3(-1))) ;

		toPointMatrix = glm::translate(toPointMatrix, aux);
		rotationMatrix = glm::rotate(rotationMatrix, (float)cp->rotationAngle / steps, cp->rotationAxis);
		fromPointMatrix = glm::translate(fromPointMatrix, aux*glm::vec3(-1));

		// Push all transformations
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(fromPointMatrix, 0));
				transformationQueue.push_back(Transformation(rotationMatrix, 0));
				transformationQueue.push_back(Transformation(toPointMatrix, 0));
				firstFlag = 0;
			}
			else {
				transformationQueue.push_back(Transformation(fromPointMatrix, stepTime));
				transformationQueue.push_back(Transformation(rotationMatrix, 0));
				transformationQueue.push_back(Transformation(toPointMatrix, 0));
			}
		}
	}
}

void Camera::BSpline(struct bspline bs)
{
	double steps = ceil(bs.time * STEPS);
	if (steps <= 0)
		steps = 1;
	long double stepTime = (long double)bs.time / (long double)steps - 0.005;

	glm::vec3 currentPos = bs.controlPoints[0];
	//currentPos.x = -ViewMatrix[3][0];
	//currentPos.y = -ViewMatrix[3][1];
	//currentPos.z = -ViewMatrix[3][2];
	//bs.controlPoints[0] = currentPos;

	std::cout << "Pontos de controle: " << std::endl;
	std::cout << "P0: (" << bs.controlPoints[0].x << "," << bs.controlPoints[0].y << "," << bs.controlPoints[0].z << ")" << std::endl;
	std::cout << "P1: (" << bs.controlPoints[1].x << "," << bs.controlPoints[1].y << "," << bs.controlPoints[1].z << ")" << std::endl;
	std::cout << "P2: (" << bs.controlPoints[2].x << "," << bs.controlPoints[2].y << "," << bs.controlPoints[2].z << ")" << std::endl;
	std::cout << "P3: (" << bs.controlPoints[3].x << "," << bs.controlPoints[3].y << "," << bs.controlPoints[3].z << ")" << std::endl;
	std::cout << "Pontos da curva:" << std::endl;

	glm::vec3 newPos;

	for (double t=0.0f; t <= steps; t++) {

		//Calculate new point
		newPos = glm::cubic(bs.controlPoints[3], bs.controlPoints[2], bs.controlPoints[1], bs.controlPoints[0], t/steps);
		std::cout << "Ponto " << t << " com t = " << t/steps <<": (" << newPos.x << "," << newPos.y << "," << newPos.z << ")" << std::endl;

		newPos = newPos - currentPos;
		
		//Creates translation
		struct translation trans;

		transformationQueue.push_back(Transformation(glm::translate(glm::mat4(1.0f),-newPos),(double)bs.time/steps));

		currentPos += newPos;

	}
}


//Auxiliar para bsplineTest
glm::vec3 Camera::GetPoint(int i, struct bspline l) {
	//retorna primeiro ponto
	if (i<0) {
		return	l.controlPoints[0];
	}
	//retorna ultimo ponto
	if (i<4)
		return l.controlPoints[i];

	return l.controlPoints[3];
}


void Camera::BSplineTest(struct bspline l) {
	glm::vec3 currentPos;
	currentPos.x = -ViewMatrix[3][0];
	currentPos.y = -ViewMatrix[3][1];
	currentPos.z = -ViewMatrix[3][2];
	l.controlPoints[0] = currentPos;
	

	struct translation trans;
	int lod = LOD * l.time * 1.2f / 4;
	double intermediaryTime = l.time / 7;
	trans.time = intermediaryTime / lod;

	for (int i = -3, j = 0; j != 5; ++j, ++i) {

		for (int k = 0; k != lod; ++k) {
			float t = (float)k / (lod);

			// t complement
			float it = 1.0f - t;

			// calculate b functions
			float b0 = it*it*it / 6.0f;
			float b1 = (3 * t*t*t - 6 * t*t + 4) / 6.0f;
			float b2 = (-3 * t*t*t + 3 * t*t + 3 * t + 1) / 6.0f;
			float b3 = t*t*t / 6.0f;

			// sum the control points mulitplied by their b
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

			std::cout << "Point: (" << x << "," << y << "," << z << ")" << std::endl;

			//Calculates vector
			trans.translationVec = glm::vec3(x, y, z) - currentPos;

			//std::cout << "\tVector: (" << trans.translationVec.x << "," << trans.translationVec.y << "," << trans.translationVec.z << ")" << std::endl;

			//Puts translation to queue
			transformationQueue.push_back(Transformation( glm::translate(glm::mat4(1.0f),trans.translationVec * glm::vec3(-1)),trans.time));
			//addCompTransformation(&trans, NULL, NULL);

			//Update current pos
			currentPos += trans.translationVec;
		}
	}
}


/*Bezier evaluation function, given 3 control points and a t, returns the bezier curve point at t (B(t))*/
glm::vec3 Camera::evaluateBezierCurve(struct bezier b, double t) {
	glm::vec3 temp1, temp2, temp3;
	temp1 = pow(glm::vec3(1 - t), glm::vec3(2)) * b.controlPoints[0];
	temp2 = (glm::vec3(2 * t*(1 - t))) * b.controlPoints[1];
	temp3 = glm::vec3(pow(t, 2)) * b.controlPoints[2];
	return temp1 + temp2 + temp3;
}

/*Calls bezier evaluation function for a wide range of T, constructing the curve and pushing the transformations to the queue*/
void Camera::bezierCurve(struct bezier b) {
	glm::vec3 currentPos;
	glm::vec3 newPoint;
	struct translation trans;
	trans.time = 0;		//MUDAR
	//Gets current pos

	currentPos.x = -ViewMatrix[3][0];
	currentPos.y = -ViewMatrix[3][1];
	currentPos.z = -ViewMatrix[3][2];
	b.controlPoints[0] = currentPos;


	double steps = ceil(b.time * STEPS);
	if (steps <= 0)
		steps = 1;

	trans.time = b.time / steps;



	for (double t = 0; t < steps; t ++) {
		
		//Calculate next position
		newPoint = evaluateBezierCurve(b, t/steps);
		//std::cout << "Point: (" << newPoint.x << "," << newPoint.y << "," << newPoint.z << ")";

		//Calculate vector from current to next
		trans.translationVec = newPoint - currentPos;
		//std::cout << "\tVector: (" << trans.translationVec.x << "," << trans.translationVec.y << "," << trans.translationVec.z << ")" << std::endl;
			
		//Puts translation into queue
		//addCompTransformation(&trans, NULL, NULL);
		transformationQueue.push_back(Transformation(glm::translate(glm::mat4(1.0f), trans.translationVec * glm::vec3(-1)), trans.time));

		//Update current pos
		currentPos += trans.translationVec;
	}
}


void Camera::addCompTransformation(struct translation *t, struct rotation *r, struct cameraLookAt *lk){

	//Split and push all the transformations to queue with the time between them	
	int firstFlag = 1;
	glm::mat4 stepTransformation(1.0);

	//std::cout << "Before type test!" << std::endl;

	//Translation
	if (t != NULL) {

		//Animation time control
		int steps = ceil(t->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)t->time / (long double)steps - 0.005;

		stepTransformation = glm::translate(stepTransformation, t->translationVec / glm::vec3(steps, steps, steps) * glm::vec3(-1));
		// Push all transformations
		for (int i = 0; i < steps; i++) {

			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else {
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
			}
		}
	}
	//Rotation
	else if (r != NULL) {
		//Animation time control
		int steps = ceil(r->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)r->time / (long double)steps - 0.005;

		stepTransformation = glm::rotate(stepTransformation, (float) r->rotationDegrees / steps, r->rotationVec);
		// Push all transformations
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else 
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
		}
		
	}
	//LookAt
	if (t== NULL && r == NULL && lk != NULL) {
		//Animation time control
		int steps = ceil(lk->time * STEPS);
		if (steps <= 0)
			steps = 1;
		long double stepTime = (long double)lk->time / (long double)steps - 0.005;

		//std::cout << "Before animating!" << std::endl;
		stepTransformation = glm::lookAt(lk->eye, lk->center, lk->up);
		//std::cout << "After animating!" << std::endl;
		// Push all transformations
		Transformation t(stepTransformation, 0);
		t.setLookAtFlag(true);
		transformationQueue.push_back(t);
		/*
		for (int i = 0; i < steps; i++) {
			if (firstFlag != 0) {
				transformationQueue.push_back(Transformation(stepTransformation, 0));
				firstFlag = 0;
			}
			else {
				transformationQueue.push_back(Transformation(stepTransformation, stepTime));
			}
		}
		*/
	}
}

void Camera::applyTransformation() {
	if (transformationQueue.empty()) {
		Camera::state = 0;
		std::cout << "Camera transformation queue empty." << std::endl;
		return;
	}
	else if (glfwGetTime() > lastTransformed + timeBtwn) {
		
		do {
			//Aplica operação
			if (!transformationQueue.front().getLookAtFlag())
				setViewMatrix(transformationQueue.front().getTransformation() * ViewMatrix);
			else
				setViewMatrix(transformationQueue.front().getTransformation());

			//Apaga operação e atualiza controle
			transformationQueue.erase(transformationQueue.begin());
		} while (!transformationQueue.empty() && transformationQueue.front().getTimeBtwn() == 0);

			lastTransformed = glfwGetTime();

		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();

		else
			timeBtwn = 0;
	}
}
