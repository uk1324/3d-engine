#version 430 core

layout(location = 0) in vec4 vertexPosition; 
layout(location = 1) in mat4 instanceTransform; 

/*generated end*/

void main() {
	//gl_Position = instanceTransform * vertexPosition;
	//gl_Position = instanceTransform * vertexPosition;
	//vec4 a = instanceTransform * vertexPosition;
	//vec3 b = a.xyz;
	//b /= a.w;
	//gl_Position = vec4(b, 1);

	vec4 a = instanceTransform * vertexPosition;
	gl_Position = a;

	//vec4 a = instanceTransform * vertexPosition;
	//a.xyz /= a.w;
	//a.w = 1.0;
	//gl_Position = a;

	//gl_Position = vec4((instanceTransform * vertexPosition).xyz, 1);
}
