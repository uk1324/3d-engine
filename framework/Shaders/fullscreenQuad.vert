#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 

out vec2 fragmentTexturePosition; 

/*generated end*/

void main() {
	gl_Position = vec4(vertexPosition, 0, 1);
	fragmentTexturePosition = vertexTexturePosition;
}
