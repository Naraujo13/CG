#include "model.hpp"
#include <GL/glew.h>
#include <model.hpp>
#include <texture.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <AntTweakBar.h>
#include <iostream>
#include <glm/gtx/transform2.hpp>

#define STEPS 40 //number of steps of an animations
#define LOD 100//Level of detail of the curve
//Constructor
Model::Model(const char *textPath, const char *textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type)
{
	texture = loadDDS(textPath);
	textureID = glGetUniformLocation(programID, "myTextureSampler");
	this->meshes = meshes;
	modelMatrixID = glGetUniformLocation(programID, "M");
	modelMatrix = glm::mat4(glm::translate(glm::mat4(1), position));
	state = 0;
	timeBtwn = 0;
	lastTransformed = glfwGetTime();
	geometry = false;
	lastUsedGeometry = -1.55;
	geometryStart = glfwGetTime();
	this->type = type;
	this->position = glm::vec3(
		this->modelMatrix[3][0],
		this->modelMatrix[3][1],
		this->modelMatrix[3][2]);
	this->alive = true;
	this->calculateSize();
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
std::vector<Mesh> * Model::getMeshes() {
	return &meshes;
}
std::vector<Transformation> * Model::getTransformationQueue() {
	return &transformationQueue;
}
int Model::getState() {
	return state;
}
long double Model::getLastTransformed() {
	return lastTransformed;
}

long double Model::getTimeBtwn() {
	return timeBtwn;
}
bool Model::getGeometry() {
	return geometry;
}
long double Model::getGeometryStart() {
	return geometryStart;
}
long double Model::getLastUsedGeometry() {
	return lastUsedGeometry;
}
glm::vec3 Model::getPosition() {
	return glm::vec3(
		this->modelMatrix[3][0],
		this->modelMatrix[3][1],
		this->modelMatrix[3][2]);
}
bool Model::isAlive() {
	return alive;
}
std::string Model::getType() {
	return type;
}
long double Model::getTimeOfDeath() {
	return timeOfDeath;
}

//Setter
void Model::setAlive(bool newState) {
	this->alive = newState;
	if (!newState)
		timeOfDeath = glfwGetTime();
}
void Model::setModelMatrix(glm::mat4 matrix) {
	Model::modelMatrix = matrix;
}
void Model::setState(int newState) {
	Model::state = newState;
}
void Model::setGeometry(bool newState) {
	Model::geometry = newState;
	long double time = glfwGetTime();
	if (Model::geometry)
		Model::geometryStart = time;
	else {
		long double previous = Model::lastUsedGeometry;
		Model::lastUsedGeometry = Model::lastUsedGeometry + (time - Model::geometryStart);
	}
}

void Model::setGeometryStart(long double time) {
	geometryStart = time;
}

void Model::setLastUsedGeometry(long double time) {
	lastUsedGeometry = time;
}

//Others


void Model::addCompTransformation(struct translation *t, struct rotation *r, struct scale *s, struct shear *h, struct extraProjection3D *p3D, long double time){
	//Split and push all the transformations to queue with the time between them
	int steps = ceil (time * STEPS);
	if (steps <= 0)
		steps = 1;
	long double stepTime = (long double)time / (long double)steps - 0.005;

	glm::mat4 stepTransformation(1.0);

	//Transla��o
	if (t != NULL) {
		stepTransformation = glm::translate(stepTransformation, t->translationVec / glm::vec3(steps, steps, steps));
	}
	//Rota��o
	if (r != NULL) {
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

	if (p3D != NULL) {
		stepTransformation = glm::proj3D(stepTransformation, p3D->projVector / glm::vec3(steps));
	}

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

	addCompTransformation(&t, &r, NULL, NULL, NULL,  p->time);
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


	for (double t = 0; t < 1; t += 0.001) {
		
		//Calculate next position
		newPoint = evaluateBezierCurve(b, t);

		//Calculate vector from current to next
		trans.translationVec = newPoint - currentPos;
			
		//Puts translation into queue
		addCompTransformation(&trans, NULL, NULL, NULL, NULL, b.time/1000);

		//Update current pos
		currentPos += trans.translationVec;
	}
}


//Auxiliar para bsplineTest
glm::vec3 GetPoint(int i, struct bspline l) {
	//retorna primeiro ponto
	if (i<0) {
		return	l.controlPoints[0];
	}
	//retorna ultimo ponto
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
	int lod = LOD * l.time * 1.2f / 4;
	double intermediaryTime = l.time / 7;

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

			//Calculates vector
			trans.translationVec = glm::vec3(x, y, z) - currentPos;

			//Puts translation to queue
			addCompTransformation(&trans, NULL, NULL, NULL, NULL, intermediaryTime / lod);

			//Update current pos
			currentPos += trans.translationVec;
		}
	}
}

void Model::applyTransformation() {
	if (transformationQueue.empty()){
		Model::state = 0;
		return;
	}
	else if (glfwGetTime() > lastTransformed + timeBtwn) {
		//Aplica opera��o
		setModelMatrix(modelMatrix * transformationQueue.front().getTransformation());

		//Apaga opera��o e atualiza controle
		transformationQueue.erase(transformationQueue.begin());
		lastTransformed = glfwGetTime();
		if (!transformationQueue.empty())
			timeBtwn = transformationQueue.front().getTimeBtwn();
	
		else
			timeBtwn = 0;
	}
	this->position = glm::vec3(
		this->modelMatrix[3][0],
		this->modelMatrix[3][1],
		this->modelMatrix[3][2]);
}
double Model::getScalingFactor() {
	glm::mat4 m = this->modelMatrix;
	return sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] * m[2][0]);

}

void Model::calculateSize() {
	GLfloat
		min_x, max_x,
		min_y, max_y,
		min_z, max_z;
	min_x = max_x = (*(meshes[0].getIndexedVertices()))[0].x;
	min_y = max_y = (*(meshes[0].getIndexedVertices()))[0].y;
	min_z = max_z = (*(meshes[0].getIndexedVertices()))[0].z;

	for (auto it = meshes.begin(); it != meshes.end(); ++it) {
		for (int i = 0; i < (*it->getIndexedVertices()).size(); i++) {
			if ((*it->getIndexedVertices())[i].x < min_x) min_x = (*it->getIndexedVertices())[i].x;
			if ((*it->getIndexedVertices())[i].x > max_x) max_x = (*it->getIndexedVertices())[i].x;
			if ((*it->getIndexedVertices())[i].y < min_y) min_y = (*it->getIndexedVertices())[i].y;
			if ((*it->getIndexedVertices())[i].y > max_y) max_y = (*it->getIndexedVertices())[i].y;
			if ((*it->getIndexedVertices())[i].z < min_z) min_z = (*it->getIndexedVertices())[i].z;
			if ((*it->getIndexedVertices())[i].z > max_z) max_z = (*it->getIndexedVertices())[i].z;
		}
	}

	this->size.x = max_x - min_x;
	this->size.y = max_y - min_y;
	this->size.z = max_z - min_z;

}

glm::vec3 Model::getSize() {
	//return meshes[0].getSize();
	return this->size * glm::vec3(this->getScalingFactor()) * glm::vec3(2.8f, 3.0f, 3.5f);
}
