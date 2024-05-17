#version 430 core

uniform float time; 

in vec2 worldPosition; 

in vec2 playerWorldPos; 
in vec2 orbWorldPos; 
in float t; 
out vec4 fragColor;

/*generated end*/

vec2 cdiv(vec2 a, vec2 b) {
	float denominator = b.x * b.x + b.y * b.y;
	return vec2(
		a.x * b.x + a.y * b.y,
		a.y * b.x - b.y * a.x
	) / denominator;
}

vec2 cartesianToPolar(vec2 v) {
	return vec2(atan(v.y, v.x), length(v));
}

#include "Utils/noise2.glsl"
#include "Utils/noise3.glsl"

float trunc_fallof(float x, float m) {
    x /= m;
	if (x <= 0.0) {
		return 1.0;
	}
	if (x >= 1.0) {
		return 0.0;
	}
    return (x-2.0)*x+1.0;
}

void main() {
	vec2 p = worldPosition;

	float attractionT = mix(0.0, 1.0, smoothstep(0.0, 1.0, t));
	attractionT *= smoothstep(250.0, 100.0, distance(playerWorldPos, orbWorldPos));
	//attractionT = 1.0;

	vec2 v = normalize(playerWorldPos - orbWorldPos);

	//vec2 sourcePosition = playerWorldPos;
	//vec2 sinkWorldPosition = orbWorldPos;

	vec2 sourcePosition = mix(orbWorldPos + v * 5.0, playerWorldPos, attractionT);
	vec2 sinkPosition = mix(orbWorldPos - v * 5.0, orbWorldPos, attractionT);

	p = cdiv(sinkPosition - p, sourcePosition - p);
	//p = cdiv(orbWorldPos - p, (playerWorldPos - p) * attractionT);
	//p = cdiv(orbWorldPos - p, mix(orbWorldPos - p, playerWorldPos - p, attractionT));
	//vec2 a = cdiv(orbWorldPos - p, playerWorldPos - p);
	//vec2 b = cdiv(orbWorldPos - p, playerWorldPos - p);
	//p = cdiv(orbWorldPos - p, mix(orbWorldPos - p, playerWorldPos - p, attractionT));
	//p = cdiv(orbWorldPos - p, playerWorldPos - p);
	p = cartesianToPolar(p);
	vec3 col;

	//float d = p.x;
	float iTime = 0.0;
	iTime += time;
	float r = p.y;
	float a = p.x;
	//float n = octave01(worldPosition / 40.0, 4);
	//float n = octave01(vec3(worldPosition / 40.0, 0.0), 4);
	float n = octave01(vec3(worldPosition / 10.0, r + iTime / 5.0), 4);
	float k1 = smoothstep(1.,0.,abs(fract(log(r)+iTime*.1)-.5)) * smoothstep(1.,0.,abs(fract((a*7.)/3.14+(iTime*.1))-.5));

	a += n * 0.5;
	vec2 aa = worldPosition - playerWorldPos;
	//a += mix(atan(aa.y, aa.x), 0.0, attractionT);
	r += n * 0.1;
	//a += r  * 4.0;

	//r += perlin01(worldPosition / 40.0) * 0.1;
	//float k = smoothstep(1.,0.,abs(fract(log(r)-iTime*.1)-.5)) * smoothstep(1.,0.,abs(fract((a*7.)/3.14+(iTime*.1))-.5));

	float k = smoothstep(1.,0.,abs(fract(log(r)+iTime*.1)-.5)) * smoothstep(1.,0.,abs(fract((a*7.)/3.14+(iTime*.1))-.5));
	//float k = smoothstep(1.,0.,abs(fract(log(r)-iTime*.1)-.5));
	col = vec3(1.0 / (k * k) / 40.09);
	col *= vec3(0.0, 1.0, 1.0);
	//col = vec3(mod(d, 5.0));

	//col = vec3(mod(distance(playerWorldPos, worldPosition), 10.0));
	float alpha = distance(worldPosition, orbWorldPos) + perlin01(worldPosition / 40.0) * 5.5;
	//alpha = 1.0 / (alpha * alpha) * 500.0;
	alpha = trunc_fallof(alpha, 100.0);
	//alpha = smoothstep(80, 30, alpha);
	//col = vec3(perlin01(worldPosition / 40.0));
	//col = vec3(k1);
	//alpha = 1.0;
	fragColor = vec4(col, alpha);
}
