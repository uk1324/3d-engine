#version 430 core

uniform float time; 

in vec2 position; 
in vec2 worldPosition; 

in float rotation; 
out vec4 fragColor;

/*generated end*/

float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}


#include "spikeShared.glsl"

void main() {
	//float d = 1.0 - boxSDF(position, vec2(1.0));
//	vec2 p = position;
//	p = 1.0 - p;
//
//	float d;
////	p.x = 1.7 - p.x;
////	float d = sdBox(p, vec2(0.2));
//	p = abs(p);
//	p /= 1.2;
//	d = max(1.0 - p.x, 0.2 + p.y);
//	vec2 p = position;
//	p *= 1.2;
//	float d;
//	d = sdBox(p - vec2(2.4), vec2(1.2));
	vec2 p = vec2(1.0) -  position;
	float d;
	//d = length(p);
	d = sdBox(p * 2.0, vec2(0.5));
	float r = 0.2;
	float d1 = length(p - r) + r;
	d = max(p.x, p.y);
	//d = max(d, d1);
	//d = max(1.0 - p.x, 1.0 - p.y);
	
	//p -= vec2(-1.2, 1.2);
	//d = 1.0 - length(p);
	
	fragColor = vec4(vec3(d), d);
	mat2 rot = mat2(vec2(cos(rotation), sin(rotation)), vec2(-sin(rotation), cos(rotation)));

	vec2 normal;
	//vec2 normal = normalize(vec2(position.y, position.x));
	//normal = normalize(0.75 - position);
	// Translating to make the normals align on sides.
	normal = normalize(0.8 - position);
	normal *= rot;
//	if (p.x < p.y) {
//		normal = vec2(0.0, 1.0);
//	} else {
//		normal = vec2(1.0, 0.0);
//	}
	fragColor = spikeColor(d, time, worldPosition, normal);
}
