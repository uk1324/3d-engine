#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 

/*generated end*/

out vec3 position;

void main() {
	position = vertexPosition;
	gl_Position = instanceTransform * vec4(vertexPosition, 1.0);
}