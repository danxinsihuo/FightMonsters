#include "GameLevel.h"
#include <fstream>
#include <sstream>
#include "resource_manager.h"

void GameLevel::Load(const GLchar *file,GLuint levelWidth,GLuint levelHeight)
{
	// 清空过期数据
	this->Bricks.clear();
	// 从文件中加载
	GLuint tileCode;
	GameLevel level;
	std::string line;
	std::ifstream fstream(file);
	std::vector<std::vector<GLuint>> tileData;
	if(fstream)
	{
		while(std::getline(fstream,line)) // 读取关卡文件的每一行
		{
			std::istringstream sstream(line);
			std::vector<GLuint> row;
			while(sstream >> tileCode) // 读取被空格分隔的每个数字
				row.push_back(tileCode);
			tileData.push_back(row);
		}
		if(tileData.size() > 0)
			this->init(tileData,levelWidth,levelHeight);
	}
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
	for(GameObject &tile : this->Bricks)
		if(!tile.Destroyed)
			tile.Draw(renderer);
}

GLboolean GameLevel::IsCompleted()
{
	for(GameObject &tile : this->Bricks)
		if(!tile.IsSolid && !tile.Destroyed)
			return GL_FALSE;
	return GL_TRUE;
}

void GameLevel::init(std::vector<std::vector<GLuint>> tileData,GLuint levelWidth,GLuint levelHeight)
{
	// 计算每个维度的大小
	GLuint height = tileData.size();
	GLuint width = tileData[0].size();
	GLfloat unit_width = levelWidth / static_cast<GLfloat>(width);
	GLfloat unit_height = levelHeight / static_cast<GLfloat>(height);
	// 基于tileDataC初始化关卡     
	this->solid = 0;
	this->soft = 0;
	for(GLuint y = 0; y < height; ++y)
	{
		for(GLuint x = 0; x < width; ++x)
		{
			// 检查砖块类型
			if(tileData[y][x] == 1)
			{
				glm::vec2 pos(unit_width * x,unit_height * y);
				glm::vec2 size(unit_width,unit_height);
				GameObject obj(pos,size,
					ResourceManager::GetTexture("block"),
					glm::vec3(0.8f,0.8f,0.7f)
				);
				obj.IsSolid = GL_TRUE;
				this->Bricks.push_back(obj);
				this->solid++;
			} else if(tileData[y][x] > 1)
			{
				glm::vec3 color = glm::vec3(1.0f); // 默认为白色
				glm::vec2 pos(unit_width * x,unit_height * y);
				glm::vec2 size(unit_width,unit_height);
				if(tileData[y][x] == 2)
				{
					this->Bricks.push_back(
						GameObject(pos,size,ResourceManager::GetTexture("stranger1"),color)
					);
				} else if(tileData[y][x] == 3){
					this->Bricks.push_back(
						GameObject(pos,size,ResourceManager::GetTexture("stranger2"),color)
					);
				} else if(tileData[y][x] == 4){
					this->Bricks.push_back(
						GameObject(pos,size,ResourceManager::GetTexture("stranger3"),color)
					);
				} else if(tileData[y][x] == 5){
					this->Bricks.push_back(
						GameObject(pos,size,ResourceManager::GetTexture("stranger4"),color)
					);
				}

				this->soft++;
			}
		}
	}
}