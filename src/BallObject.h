#pragma once

#include "GL/glew.h"
#include "GameObject.h"
#include "texture.h"
#include "glm/glm.hpp"
#include <vector>

struct CircleVertex{
	std::vector<float> vertex;
	std::vector<unsigned int> indices;
};

CircleVertex generateCircleVertex(float cx,float cy,float radius,int segments);

class BallObject: public GameObject
{
public:
	// 球的状态
	GLfloat   Radius;
	GLboolean Stuck;
	GLboolean Sticky,PassThrough;

	BallObject();
	BallObject(glm::vec2 pos,GLfloat radius,glm::vec2 velocity,Texture2D sprite);

	glm::vec2 Move(GLfloat dt,GLuint window_width);
	void      Reset(glm::vec2 position,glm::vec2 velocity);

	void ownDraw(Shader& render,float Width,float Height);
};