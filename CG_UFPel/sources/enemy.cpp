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
	stepTime = 0.1f;
	speedPerStep = speedPerSecond / stepTime;
	
	this->maxHealth = health;
	this->currentHealth = health;

}

/* Moves Enemy */
void Enemy::moveEnemy(std::string direction)
{
	long double currentTime = glfwGetTime();
	if (currentTime > lastTimeMoved + stepTime) {
		lastTimeMoved = glfwGetTime();
		if (direction == "Right")
			this->setModelMatrix(translate(this->getModelMatrix(), glm::vec3(speedPerStep, 0, 0)));
		else if (direction == "Left")
			this->setModelMatrix(translate(this->getModelMatrix(), glm::vec3(-speedPerStep, 0, 0)));
		else if (direction == "Down")
			this->setModelMatrix(translate(this->getModelMatrix(), glm::vec3(0, -speedPerStep/2, 0)));

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
