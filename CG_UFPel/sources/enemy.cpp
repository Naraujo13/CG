#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include "mesh.hpp"
#include "model.hpp"
#include "enemy.hpp"

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>

Enemy::Enemy(const char * textPath, const char * textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type, long double speed, int health)
	: Model(textPath, textSample, programID, meshes, position, type)
{
	lastTimeMoved = glfwGetTime();
	this->speedPerSecond = speed;
	stepTime = 0.05f;
	speedPerStep = speedPerSecond / stepTime;
	
	this->maxHealth = health;
	this->currentHealth = health;

}

/* Moves Enemy */
void Enemy::moveEnemy()
{
	long double currentTime = glfwGetTime();
	if (currentTime > lastTimeMoved + stepTime) {
		lastTimeMoved = glfwGetTime();
		this->setModelMatrix(translate(this->getModelMatrix(), glm::vec3(0, speedPerStep, 0)));

		//Debug
		glm::vec3 pos = glm::vec3(this->getModelMatrix()[3][0], this->getModelMatrix()[3][1], this->getModelMatrix()[3][2]);
		/*std::cout << "Posição Atual do Inimigo: (" << pos.x << ", " << pos.y << ", "<< pos.z << ")"<< std::endl;
		if (pos.y == 10 && pos.z == -5)
		getchar();*/

		this->checkBoundaries();
	}
}

void Enemy::checkBoundaries() {

}

void Enemy::wasHit() {
	if (currentHealth > 0) {
		currentHealth--;
		if (currentHealth < 1) {
			this->setAlive(false);
			this->setGeometry(true);
		}
	}
}
