#include "Utils/perlin2d.glsl"

vec3 skyboxSunColor = vec3(97, 98, 47) / 255.0;

vec3 sampleSkybox(vec3 direction) {
	vec3 color;
	vec3 c0 = vec3(0.518001, 0.670929, 0.918919);
	vec3 c1 = vec3(0.329154, 0.622619, 0.888031);
	float angle = atan(direction.y, length(vec2(direction.x, direction.z))) / (3.141 / 2);
	if (direction.y > 0.0) {
		color = mix(c0, c1, angle);
		vec3 intersectWithPlane = direction;
		// make y == 1.0
		intersectWithPlane /= intersectWithPlane.y;
		vec2 pos = intersectWithPlane.xz;
		float dist = length(pos);
		pos += time / 125.0;
		float noise = octave01(pos, 4);
		noise *= clamp(perlin01(pos / 4.0) - 0.3, 0.0, 1.0);
		noise *= smoothstep(15.0, 0.8, dist);
		color += vec3(noise * 0.8);
	} else {
		color = mix(c0, c0 / 3.0, -angle);
	}
	color += pow(clamp(dot(direction, -directionalLightDirection), 0, 1), 140.0) * skyboxSunColor * 2.0;
	return color;
}