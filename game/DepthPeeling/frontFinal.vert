#version 430 core

//layout(location = 0) in vec3 vertexPosition; 
//layout(location = 1) in vec3 vertexNormal; 
//layout(location = 2) in mat4 instanceTransform; 

layout(location = 0) in vec2 vertexPosition; 

/*generated end*/

out vec3 normal;

void main() {
	//gl_Position = instanceTransform * vec4(vertexPosition, 1.0);
	gl_Position = vec4(vertexPosition, 0, 1);
}