#version 430 core

layout(location = 0) in vec4 vertexPosition; 
layout(location = 1) in vec3 vertexColor; 

uniform mat4 viewProjection; 

out vec3 color; 

/*generated end*/

void main() {
	gl_Position = viewProjection * vertexPosition;
	color = vertexColor;
}
