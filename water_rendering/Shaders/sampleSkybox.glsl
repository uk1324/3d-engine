#include "Utils/perlin2d.glsl"

float fbm(vec2 p) {
    return (octave01(p, 7) - 0.5) * 2.0;
}

float noise(vec2 p) {
    return (perlin01(p) - 0.5) * 2.0;
}

vec3 sampleClouds(vec2 p, float time) {
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
    vec2 uv = p;
    float q = fbm(uv * cloudscale * 0.5 + time);
    vec3 result;
    //time = 0.0;
    //ridged noise shape
    float r = 0.0;
    uv *= cloudscale;
    uv -= q - time;
    float weight = 0.8;
    for (int i = 0; i < 8; i++) {
        r += abs(weight * noise(uv));
        uv = m * uv + time;
        weight *= 0.7;
    }

    result = vec3(q * r);

    return result;
}

//vec3 skyboxSunColor = vec3(97, 98, 47) / 255.0;
vec3 skyboxSunColor = vec3(1.0);

vec3 sampleSkybox(vec3 direction) {
	vec3 color;
	vec3 c0 = vec3(0.518001, 0.670929, 0.918919);
	vec3 c1 = vec3(0.329154, 0.622619, 0.888031);
	float angle = atan(direction.y, length(vec2(direction.x, direction.z))) / (3.141 / 2);
	if (direction.y > 0.0) {
        color = mix(c0, c1, angle);
        vec3 intersectWithPlane = direction;
        intersectWithPlane /= intersectWithPlane.y;

        vec2 pos = intersectWithPlane.xz;
        
        float dist = length(pos);
        vec3 result = sampleClouds(pos, time / 30.0);
        result *= smoothstep(15.0, 0.5, dist);
        color += result;
		//color = mix(c0, c1, angle);
		//vec3 intersectWithPlane = direction;
		//// make y == 1.0
		//intersectWithPlane /= intersectWithPlane.y;
		//vec2 pos = intersectWithPlane.xz;
		//pos += time / 125.0;
		////float noise = octave01(pos, 4);
		////noise *= clamp(perlin01(pos / 4.0) - 0.3, 0.0, 1.0);
		////noise *= smoothstep(15.0, 0.8, dist);
  //     
	} else {
		color = mix(c0, c0 / 3.0, -angle);
	}
	//color += smoothstep(0., 0.1, -angle) * c0;

	color += pow(clamp(dot(direction, -directionalLightDirection), 0, 1), 140.0) * skyboxSunColor * 2.0;
	return color;
}

//// Some very barebones but fast atmosphere approximation
//vec3 extra_cheap_atmosphere(vec3 raydir, vec3 sundir) {
//	sundir.y = max(sundir.y, -0.07);
//	float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
//	float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
//	float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
//	float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
//	float mymie = sundt * special_trick * 0.2;
//	vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
//	vec3 bluesky = vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
//	vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
//	bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
//	return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0)) + mymie * suncolor;
//}
//
//// Calculate where the sun should be, it will be moving around the sky
//vec3 getSunDirection() {
//	//return normalize(vec3(sin(time * 0.1), 1.0, cos(time * 0.1)));
//	return normalize(vec3(sin(0.0), 1.0, cos(0.0)));
//}
//
//// Get atmosphere color for given direction
//vec3 getAtmosphere(vec3 dir) {
//	return extra_cheap_atmosphere(dir, getSunDirection()) * 0.5;
//}
//
//// Get sun color for given direction
//float getSun(vec3 dir) {
//	return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
//}
//
//vec3 sampleSkybox(vec3 direction) {
//	return getAtmosphere(direction) + getSun(direction);
//}