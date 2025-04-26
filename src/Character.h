#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "shader.h"

#include "ft2build.h"
#include FT_FREETYPE_H 
#include FT_OUTLINE_H  // 轮廓处理（含 FT_Outline_Embolden）

#include <iostream>
#include <map>
#include <string>

struct Character {
	GLuint     TextureID;  // 字形纹理的ID
	glm::ivec2 Size;       // 字形大小
	glm::ivec2 Bearing;    // 从基准线到字形左部/顶部的偏移值
	GLuint     Advance;    // 原点距下一个字形原点的距离
};

struct FontManager {
	FT_Library library;
	FT_Face face;

	bool init(const char* fontPath) {
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		if(FT_Init_FreeType(&library)) {
			std::cerr << "ERROR: Failed to init FreeType" << std::endl;
			return false;
		}
		if(FT_New_Face(library,fontPath,0,&face)) {
			std::cerr << "ERROR: Failed to load font" << std::endl;
			return false;
		}
		FT_Set_Pixel_Sizes(face,0,48); // 默认大小
		return true;
	}

	~FontManager() {
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}
};
extern FontManager fontMagager;
extern std::map<GLuint,Character> Characters;

Character getCharacterMap(unsigned int unicode);
void renderCharacter(Shader& s,const std::string& text,GLfloat x,GLfloat y,GLfloat scale,glm::vec3 color);
