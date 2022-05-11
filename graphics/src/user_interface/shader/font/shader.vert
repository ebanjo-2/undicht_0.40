#version 450

layout(location = 0) in vec2 aPos;

layout(location = 1) in vec2 aUV;
layout(location = 2) in vec2 aUVSize;
layout(location = 3) in vec2 aOffset;
layout(location = 4) in float aAdvance;

layout (location = 0) out vec2 uv;


void main() {

		vec2 pos = aPos * aUVSize + aOffset;
		pos.x += aAdvance;
		
	  uv = aUV + aPos * aUVSize;
	  
    gl_Position = vec4(pos, 0.0, 1.0);

}
