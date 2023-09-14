#version 430 core

layout(location = 0) in vec3 vertexPosition; 
layout(location = 1) in vec3 vertexNormal; 
layout(location = 2) in mat4 instanceTransform; 

/*generated end*/

out vec3 normal;

uniform float t;

void main() {

	vec3 p = vertexPosition.xyz;
	vec3 projected = normalize(vec4(p, -1)).xyz;

	vec3 a = mix(p, projected, t);

	gl_Position = instanceTransform * vec4(a, 1.0);
	// gl_Position = instanceTransform * vec4(projected, 1.0);
	//gl_Position = instanceTransform * vec4(p, 1.0);

	normal = vertexNormal;
}