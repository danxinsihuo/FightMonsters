#version 330 core

layout (location = 0 ) out vec4 color;
in vec2 aTexCoord;

uniform float aspect;  // = width / height

uniform sampler2D circleTex;

void main(){

	// 把坐标中心移到中点
    vec2 coord = aTexCoord - vec2(0.5);

    // 补偿长宽比，使横纵尺度一致
    // coord.x *= aspect;

	float dist = length(coord);

    if (dist <= 0.5) {
        color = texture(circleTex,aTexCoord);  // 圆内
    } else {
        discard; // 圆外透明
    }
}