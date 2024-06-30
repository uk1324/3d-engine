#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexColor; 

uniform mat4 viewProjection; 

out vec3 color; 

/*generated end*/

void main() {
	gl_Position = viewProjection * vec4(vertexPosition, 1.0);
	color = vertexColor;
}
