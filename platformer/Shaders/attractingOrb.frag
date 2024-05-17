#version 430 core

uniform float time; 

in vec2 worldPosition; 

in vec2 playerWorldPos; 
in vec2 orbWorldPos; 
in float t; 
out vec4 fragColor;

/*generated end*/

#define PI 3.14159265359 

vec2 cdiv(vec2 a, vec2 b) {
	float denominator = b.x * b.x + b.y * b.y;
	return vec2(
		a.x * b.x + a.y * b.y,
		a.y * b.x - b.y * a.x
	) / denominator;
}

#include "Utils/noise2.glsl"
#include "Utils/noise3.glsl"

float falloff(float x, float equalToZeroAfter) {
    x /= equalToZeroAfter;
	if (x <= 0.0) {
		return 1.0;
	}
	if (x >= 1.0) {
		return 0.0;
	}
    return (x-2.0)*x+1.0;
}

void main() {
	float attractionT = smoothstep(0.0, 1.0, t);
	// Attenuate with distance so that the player doesn't interact with far away orbs.
	attractionT *= smoothstep(250.0, 100.0, distance(playerWorldPos, orbWorldPos));

	vec2 v = normalize(playerWorldPos - orbWorldPos);
	float poleOffset = 5.0;
	// Animate from a dipole around the orb to a pole at the orb and a pole at the player positions.
	vec2 sourcePosition = mix(orbWorldPos + v * poleOffset , playerWorldPos, attractionT);
	vec2 sinkPosition = mix(orbWorldPos - v * poleOffset, orbWorldPos, attractionT);

	vec2 z = cdiv(sinkPosition - worldPosition, sourcePosition - worldPosition);

	float r = length(z);
	float a = atan(z.y, z.x);
	float n = octave01(vec3(worldPosition / 10.0, r + time / 5.0), 4);
	a += n * 0.5;
	r += n * 0.1;

	float isolineMovementT = time * 0.1;
	float k = 
		smoothstep(1.0, 0.0, abs(fract(log(r) + isolineMovementT) - 0.5)) * 
		smoothstep(1.0, 0.0, abs(fract((a * 7.0)/ PI + isolineMovementT) - 0.5));

	vec3 col;

	col = vec3(1.0 / (k * k) / 40.09);
	col *= vec3(0.0, 1.0, 1.0);

	float alpha = distance(worldPosition, orbWorldPos) + perlin01(worldPosition / 40.0) * 5.5;
	alpha = falloff(alpha, 100.0);
	fragColor = vec4(col, alpha);
}
