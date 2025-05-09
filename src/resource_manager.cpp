#include "resource_manager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "stb_image/stb_image.h"

// Instantiate static variables
std::map<std::string,Texture2D>    ResourceManager::Textures;
std::map<std::string,Shader>       ResourceManager::Shaders;


Shader ResourceManager::LoadShader(const GLchar* vShaderFile,const GLchar* fShaderFile,const GLchar* gShaderFile,std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile,fShaderFile,gShaderFile);
	return Shaders[name];
}

Shader& ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const GLchar* file,GLboolean alpha,std::string name)
{
	Textures[name] = loadTextureFromFile(file,alpha);
	return Textures[name];
}

Texture2D& ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear()
{
	// (Properly) delete all shaders	
	for(auto iter : Shaders)
		glDeleteProgram(iter.second.ID);
	// (Properly) delete all textures
	for(auto iter : Textures)
		glDeleteTextures(1,&iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const GLchar* vShaderFile,const GLchar* fShaderFile,const GLchar* gShaderFile)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		// Open files
		std::ifstream vertexShaderFile(vShaderFile);
		if(!vertexShaderFile.is_open()){
			std::cout<< "\nFile "<< vShaderFile <<" not exists!"<<std::endl;
			system("pause");
			exit(0);
		}
		std::ifstream fragmentShaderFile(fShaderFile);
		if(!fragmentShaderFile.is_open()){
			std::cout<< "\nFile "<< fShaderFile <<" not exists!"<<std::endl;
			system("pause");
			exit(0);
		}

		std::stringstream vShaderStream,fShaderStream;
		// Read file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();
		// close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();
		// Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// If geometry shader path is present, also load a geometry shader
		if(gShaderFile != nullptr && gShaderFile != "")
		{
			std::ifstream geometryShaderFile(gShaderFile);
			if(!geometryShaderFile.is_open()){
				std::cout<< "\nFile "<< gShaderFile <<" not exists!"<<std::endl;
				system("pause");
				exit(0);
			}
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	} catch(std::exception e)
	{
		std::cout << "\nERROR::SHADER: Failed to read shader files" << std::endl;
	}
	const GLchar* vShaderCode = vertexCode.c_str();
	const GLchar* fShaderCode = fragmentCode.c_str();
	const GLchar* gShaderCode = geometryCode.c_str();
	// 2. Now create shader object from source code
	Shader shader;
	shader.Compile(vShaderCode,fShaderCode,(gShaderFile != nullptr && gShaderFile != "")? gShaderCode : nullptr);
	return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const GLchar* file,GLboolean alpha)
{
	// Create Texture object
	Texture2D texture;
	if(alpha)
	{
		texture.Internal_Format = GL_RGBA;
		texture.Image_Format = GL_RGBA;
	}
	if(!std::filesystem::exists(file)){
		std::cout<< "\nFile "<< file <<" not exists!"<<std::endl;
		system("pause");
		exit(0);
	}

	stbi_set_flip_vertically_on_load(true);

	// Load image
	int width,height;
	//unsigned char* image = SOIL_load_image(file, &width, &height, 0, texture.Image_Format == GL_RGBA ? SOIL_LOAD_RGBA : SOIL_LOAD_RGB);
	unsigned char* image = stbi_load(file,&width,&height,0,texture.Image_Format == GL_RGBA ? STBI_rgb_alpha : STBI_rgb);

	// Now generate texture
	texture.Generate(width,height,image);
	// And finally free image data
	//SOIL_free_image_data(image);
	stbi_image_free(image);
	return texture;
}