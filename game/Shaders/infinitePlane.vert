#version 430 core

layout(location = 0) in vec4 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 

/*generated end*/

void main() {
	gl_Position = instanceTransform * vertexPosition;
}
