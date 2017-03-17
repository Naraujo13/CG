#pragma once
#ifndef ENEMY_HPP
#define ENEMY_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include "mesh.hpp"
#include "model.hpp"

// Include GLFW
#include <glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <glerror.hpp>

class Enemy : public Model {
private:
	long double lastTimeMoved;
	long double stepTime;
	long double speedPerSecond;
	long double speedPerStep;

	int currentHealth;
	int maxHealth;
public:
	//Construtor
	Enemy::Enemy(const char * textPath, const char * textSample, GLuint programID, std::vector<Mesh> meshes, glm::vec3 position, std::string type, long double speed, int health);
	//Movement
	void moveEnemy(std::string direction);
	void checkBoundaries();
	//Health
	void wasHit();
};

#endif