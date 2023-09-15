#include "Utils/perlin2d.glsl"

//vec3 skyboxSunColor = vec3(97, 98, 47) / 255.0;
//
//vec3 sampleSkybox(vec3 direction) {
//	vec3 color;
//	vec3 c0 = vec3(0.518001, 0.670929, 0.918919);
//	vec3 c1 = vec3(0.329154, 0.622619, 0.888031);
//	float angle = atan(direction.y, length(vec2(direction.x, direction.z))) / (3.141 / 2);
//	if (direction.y > 0.0) {
//		color = mix(c0, c1, angle);
//		vec3 intersectWithPlane = direction;
//		// make y == 1.0
//		intersectWithPlane /= intersectWithPlane.y;
//		vec2 pos = intersectWithPlane.xz;
//		float dist = length(pos);
//		pos += time / 125.0;
//		float noise = octave01(pos, 4);
//		noise *= clamp(perlin01(pos / 4.0) - 0.3, 0.0, 1.0);
//		noise *= smoothstep(15.0, 0.8, dist);
//		color += vec3(noise * 0.8);
//	} else {
//		color = mix(c0, c0 / 3.0, -angle);
//	}
//	//color += smoothstep(0., 0.1, -angle) * c0;
//
//	color += pow(clamp(dot(direction, -directionalLightDirection), 0, 1), 140.0) * skyboxSunColor * 2.0;
//	return color;
//}

// Some very barebones but fast atmosphere approximation
vec3 extra_cheap_atmosphere(vec3 raydir, vec3 sundir) {
	sundir.y = max(sundir.y, -0.07);
	float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
	float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
	float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
	float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
	float mymie = sundt * special_trick * 0.2;
	vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
	vec3 bluesky = vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
	vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
	bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
	return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0)) + mymie * suncolor;
}

// Calculate where the sun should be, it will be moving around the sky
vec3 getSunDirection() {
	//return normalize(vec3(sin(time * 0.1), 1.0, cos(time * 0.1)));
	return normalize(vec3(sin(0.0), 1.0, cos(0.0)));
}

// Get atmosphere color for given direction
vec3 getAtmosphere(vec3 dir) {
	return extra_cheap_atmosphere(dir, getSunDirection()) * 0.5;
}

// Get sun color for given direction
float getSun(vec3 dir) {
	return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
}

vec3 sampleSkybox(vec3 direction) {
	return getAtmosphere(direction) + getSun(direction);
}