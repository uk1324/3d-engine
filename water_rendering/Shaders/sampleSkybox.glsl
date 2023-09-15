#include "Utils/perlin2d.glsl"

float sampleClouds(vec2 p, float time) {
    const mat2 m = mat2(1.6, 1.2, -1.2, 1.6);
    const float cloudscale = 1.1;
    const float speed = 0.03;
    const float clouddark = 0.5;
    const float cloudlight = 0.3;
    const float cloudcover = 0.2;
    const float cloudalpha = 8.0;
    const float skytint = 0.5;
    const vec3 skycolour1 = vec3(0.2, 0.4, 0.6);
    const vec3 skycolour2 = vec3(0.4, 0.7, 1.0);
    time /= 4.0;
   
    float r = 0.0;
    float weight = 0.8;
    p /= 20.0;
    time /= 20.0;
    for (int i = 0; i < 6; i++) {
        r += abs(weight * (perlin01(p) - 0.5) * 2.0);
        p = m * p + time;
        weight *= 0.7;
    }

    r *= octave01(p / 5.0 + r + time / 0.1, 7);
    r *= perlin01(p / 10.0);
    r *= 2.0;

    float ret = r;
    return ret;
}

vec3 skyboxSunColor = vec3(1.0);

vec3 blend(vec3 old, vec3 new, float newAlpha) {
    return old * (1.0 - newAlpha) + new * newAlpha;
}

vec3 sampleSkybox(vec3 direction) {
	vec3 color;
	vec3 c0 = vec3(0.518001, 0.670929, 0.918919);
	vec3 c1 = vec3(0.329154, 0.622619, 0.888031);
	float angle = atan(direction.y, length(vec2(direction.x, direction.z))) / (3.141 / 2);
    float a = 0.0;

    color += pow(clamp(dot(direction, -directionalLightDirection), 0, 1), 140.0) * skyboxSunColor * 2.0;

	if (direction.y > 0.0) {
        color += mix(c0, c1, angle);
        vec3 intersectWithPlane = direction;
        intersectWithPlane /= intersectWithPlane.y;

        vec2 pos = intersectWithPlane.xz;
        
        float dist = length(pos);
        float result = sampleClouds(pos, time / 30.0);
        a = result;
        result *= smoothstep(15.0, 0.5, dist);
        color = mix(color, vec3(1.0), result);
	} else {
		color = mix(c0, c0 / 3.0, -angle);
	}

	return color;
}