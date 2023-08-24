#version 430 core

layout(location = 0) in vec4 vertexPosition; 

uniform mat4 viewProjection; 

/*generated end*/

void main() {
	gl_Position = viewProjection * vertexPosition;
}
