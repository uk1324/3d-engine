#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec4 vertexColor; 

uniform mat3x2 transform; 

/*generated end*/

out vec4 color;

void main() {
	color = vertexColor;
	gl_Position = vec4(transform * vec3(vertexPosition, 1.0), 0.0, 1.0);
}
