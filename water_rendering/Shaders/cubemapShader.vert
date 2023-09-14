#version 430 core

layout(location = 0) in vec3 vertexPosition; 

uniform mat4 transform; 

out vec3 unnormalizedDirection; 

/*generated end*/

void main() {
	unnormalizedDirection = vertexPosition;
	gl_Position = transform * vec4(vertexPosition, 1.0);
}
