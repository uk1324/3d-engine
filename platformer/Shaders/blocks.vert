#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 

uniform mat3x2 clipToWorld; 

out vec2 worldPosition; 

/*generated end*/

void main() {
	worldPosition = (vertexTexturePosition - vec2(0.5)) * 2.0;
	worldPosition = clipToWorld * vec3(worldPosition, 1.0);
	gl_Position = vec4(vertexPosition, 0.0, 1.0);
}
