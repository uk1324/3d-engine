#version 430 core

uniform float time; 

in vec2 position; 
in vec2 worldPosition; 

in float rotation; 
out vec4 fragColor;

/*generated end*/

float sdBox( in vec2 p, in vec2 b ) {
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}

#include "spikeShared.glsl"

void main() {
	vec2 p = vec2(1.0) -  position;
	float d;
	d = max(p.x, p.y);

	vec2 normal;
	// Translating to make the normals align on sides.
	// TODO: This is 0.8 = 1.0 - spikeSizeRatio
	normal = normalize(0.8 - position);
	mat2 rot = mat2(vec2(cos(rotation), sin(rotation)), vec2(-sin(rotation), cos(rotation)));
	normal *= rot;

	fragColor = spikeColor(d, time, worldPosition, normal);
}
