#version 430 core

uniform float time; 

in vec2 position; 
in vec2 worldPosition; 

in float rotation; 
out vec4 fragColor;

/*generated end*/

#include "spikeShared.glsl"

void main() {
	vec2 p = position;
	//p.x = 1.0 - p.x;
	float d = 1.0 - length(p);
	vec2 normal = normalize(p);
	mat2 rot = mat2(vec2(cos(rotation), sin(rotation)), vec2(-sin(rotation), cos(rotation)));
	normal *= rot;

	fragColor = spikeColor(d, time, worldPosition, normal);
	vec3 c;
	c = vec3(atan(p.y, p.x)) / (3.14 / 2);
	//c = vec3(p, 0.0);
	//fragColor = vec4(c, 1.0);
}
