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
	float d = 1.0 - length(p);
	vec2 normal = normalize(p);
	mat2 rot = mat2(vec2(cos(rotation), sin(rotation)), vec2(-sin(rotation), cos(rotation)));
	normal *= rot;

	fragColor = spikeColor(d, time, worldPosition, normal);
}
