#include "Character.h"

std::map<GLuint,Character> Characters;
FontManager fontMagager;

Character getCharacterMap(unsigned int unicode) {
	auto it = Characters.find(unicode);
	if(it != Characters.end()) return it->second;

	if(FT_Load_Char(fontMagager.face,unicode,FT_LOAD_RENDER))
	{
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
	}
	if(fontMagager.face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		FT_Outline_Embolden(&fontMagager.face->glyph->outline,1 << 6);  // 加粗强度（26.6固定点）
	}

	FT_Render_Glyph(fontMagager.face->glyph,FT_RENDER_MODE_NORMAL);
	// 生成纹理
	GLuint texture;
	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		fontMagager.face->glyph->bitmap.width,
		fontMagager.face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		fontMagager.face->glyph->bitmap.buffer
	);
	// 设置纹理选项
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	// 储存字符供之后使用
	Character character = {
		texture,
		glm::ivec2(fontMagager.face->glyph->bitmap.width,fontMagager.face->glyph->bitmap.rows),
		glm::ivec2(fontMagager.face->glyph->bitmap_left,fontMagager.face->glyph->bitmap_top),
		fontMagager.face->glyph->advance.x
	};
	Characters.insert(std::pair<GLuint,Character>(unicode,character));
	return character;
}


void renderCharacter(Shader& s,const std::string& text,GLfloat x,GLfloat y,GLfloat scale,glm::vec3 color) {
	unsigned int charVAO,charVBO;
	glGenVertexArrays(1,&charVAO);
	glGenBuffers(1,&charVBO);
	glBindVertexArray(charVAO);
	glBindBuffer(GL_ARRAY_BUFFER,charVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat) * 6 * 4,NULL,GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,4 * sizeof(GLfloat),0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	// 激活对应的渲染状态
	s.Use();
	s.SetVector3f("textColor",color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(charVAO);

	// 遍历文本中所有的字符
	std::string::const_iterator c;
	for(int i = 0; i < text.size();)
	{
		unsigned int c = 0;
		unsigned char cha = static_cast<unsigned char>(text[i]);

		if(cha <= 0x7F) {
			// ASCII字符（1字节）
			c = text[i];
			i += 1;
		} else if(cha <= 0xDF) {
			// 2字节UTF-8
			c = ((cha & 0x1F) << 6) | (text[i + 1] & 0x3F);
			i += 2;
		} else if(cha <= 0xEF) {
			// 3字节UTF-8（中文字符）
			c = ((cha & 0x0F) << 12) | ((text[i + 1] & 0x3F) << 6) | (text[i + 2] & 0x3F);
			i += 3;
		} else {
			i++; // 跳过无效字节
		}
		// 查找字符
		Character ch;
		auto it = Characters.find(c);
		//std::cout << "c:" << c << std::endl;
		if(it == Characters.end()) {
			ch = getCharacterMap(c);

		} else {
			ch = it->second;
		}

		GLfloat xpos = x + ch.Bearing.x * scale;
		//std::cout << "xpos:" << xpos << std::endl;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// 对每个字符更新VBO
		GLfloat vertices[6][4] = {
			{xpos,ypos + h,0.0,0.0},
			{xpos,ypos,0.0,1.0},
			{xpos + w,ypos,1.0,1.0},

			{xpos,ypos + h,0.0,0.0},
			{xpos + w,ypos,1.0,1.0},
			{xpos + w,ypos + h,1.0,0.0}
		};
		// 在四边形上绘制字形纹理
		glBindTexture(GL_TEXTURE_2D,ch.TextureID);
		// 更新VBO内存的内容
		glBindBuffer(GL_ARRAY_BUFFER,charVBO);
		glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(vertices),vertices);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		// 绘制四边形
		glDrawArrays(GL_TRIANGLES,0,6);
		// 更新位置到下一个字形的原点，注意单位是1/64像素
		x += (ch.Advance >> 6) * scale; // 位偏移6个单位来获取单位为像素的值 (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D,0);
}
