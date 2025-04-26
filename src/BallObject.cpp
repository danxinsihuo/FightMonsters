#include "BallObject.h"
#include <iostream>

BallObject::BallObject()
	: GameObject(),Radius(12.5f),Stuck(true),Sticky(GL_FALSE),PassThrough(GL_FALSE)
{

}
BallObject::BallObject(glm::vec2 pos,GLfloat radius,glm::vec2 velocity,Texture2D sprite)
	: GameObject(pos,glm::vec2(radius * 2,radius * 2),sprite,glm::vec3(1.0f),velocity),Radius(radius),Stuck(true),
	Sticky(GL_FALSE),PassThrough(GL_FALSE)
{

}

glm::vec2 BallObject::Move(GLfloat dt,GLuint window_width)
{
	// 如果没有被固定在挡板上
	if(!this->Stuck)
	{
		// 移动球
		this->Position += this->Velocity * dt;
		// 检查是否在窗口边界以外，如果是的话反转速度并恢复到正确的位置
		if(this->Position.x <= 0.0f)
		{
			this->Velocity.x = -this->Velocity.x;
			this->Position.x = 0.0f;
		} else if(this->Position.x + this->Size.x >= window_width)
		{
			this->Velocity.x = -this->Velocity.x;
			this->Position.x = window_width - this->Size.x;
		}
		if(this->Position.y <= 0.0f)
		{
			this->Velocity.y = -this->Velocity.y;
			this->Position.y = 0.0f;
		}

	}
	return this->Position;
}
void BallObject::Reset(glm::vec2 position,glm::vec2 velocity)
{
	this->Position = position;
	this->Velocity = velocity;
	this->Stuck = GL_TRUE;
	this->Sticky = GL_FALSE;
	this->PassThrough = GL_FALSE;
}

CircleVertex generateCircleVertex(float cx,float cy,float radius,int segments)
{
	std::vector<float> vertex;
	std::vector<unsigned int> indices;

	vertex.push_back(cx);
	vertex.push_back(cy);
	vertex.push_back(0.5f);
	vertex.push_back(0.5f);

	for(int i=0;i<=segments;i++){
		float angle = 2.0f*glm::pi<float>() * float(i) / float(segments);
		float x = cx + radius * cosf(angle);
		float y = cy + radius * sinf(angle);

		float s = 0.5f + 0.5*cosf(angle);
		float t = 0.5f + 0.5*sinf(angle);

		vertex.push_back(x);
		vertex.push_back(y);
		vertex.push_back(s);
		vertex.push_back(t);
	}

	for(int i=1;i<=segments;i++){
		indices.push_back(0);
		indices.push_back(i);
		indices.push_back(i+1);
	}
	return {vertex,indices};
}

void BallObject::ownDraw(Shader& shader,float Width,float Height)
{
	CircleVertex circle = generateCircleVertex(0,0,Radius,64);

	unsigned int circleVAO,circleVBO,circleEBO;
	glGenVertexArrays(1,&circleVAO);
	glGenBuffers(1,&circleVBO);
	glGenBuffers(1,&circleEBO);

	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER,circleVBO);
	glBufferData(GL_ARRAY_BUFFER,circle.vertex.size()*sizeof(float),circle.vertex.data(),GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void *)(2*sizeof(float)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,circleEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,circle.indices.size()*sizeof(unsigned int),circle.indices.data(),GL_STATIC_DRAW);

	glBindVertexArray(0);

	shader.Use();
	shader.SetInteger("circleTex",0);
	glm::mat4 model(1.0f);
	model = glm::translate(model,glm::vec3(Position.x+Radius,Height-Position.y-Radius,0.0f));
	glm::mat4 projection = glm::ortho(0.0f,Width,0.0f,Height);
	shader.SetMatrix4("projection",projection);
	shader.SetMatrix4("model",model);
	glActiveTexture(GL_TEXTURE0);
	Sprite.Bind();
	glBindVertexArray(circleVAO);
	glDrawElements(GL_TRIANGLES,circle.indices.size(),GL_UNSIGNED_INT,0);
}
