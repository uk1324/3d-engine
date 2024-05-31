#version 430 core

layout(location = 0) in vec2 vertexPosition; 
layout(location = 1) in vec2 vertexTexturePosition; 
layout(location = 2) in vec2 instanceMin; 
layout(location = 3) in vec2 instanceMax; 
layout(location = 4) in vec4 instanceColor; 

/*generated end*/

out vec4 color;

void main() {
	gl_Position = vec4(mix(instanceMin, instanceMax, vertexTexturePosition), 0.0, 1.0);
	color = instanceColor;
}
