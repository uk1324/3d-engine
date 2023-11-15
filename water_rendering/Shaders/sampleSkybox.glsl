#include "Utils/perlin2d.glsl"

float sampleClouds(vec2 p, float time) {
    const mat2 m = mat2(1.6, 1.2, -1.2, 1.6);
    time /= 80.0;

    float r = 0.0;
    float weight = 0.8;
    p /= 20.0;
    for (int i = 0; i < 6; i++) {
        r += abs(weight * (perlin01(p) - 0.5) * 2.0);
        p = m * p + time;
        weight *= 0.7;
    }

    r *= octave01(p / 5.0 + r + time / 0.1, 7);
    r *= perlin01(p / 10.0);
    r *= 2.0;

    return r;
}

vec3 sampleSkybox(vec3 direction) {
    vec3 color;
    //vec3 c0 = vec3(0.518001, 0.670929, 0.918919);
    //vec3 c1 = vec3(0.329154, 0.622619, 0.888031);
    vec3 c0 = skyboxBottom;
    vec3 c1 = skyboxTop;
    float angle = atan(direction.y, length(vec2(direction.x, direction.z))) / (3.141 / 2);
    float a = 0.0;

    color += pow(clamp(dot(direction, -skyboxDirectionalLightDirection), 0, 1), 140.0) * skyboxSunColor * 2.0;

    if (direction.y > 0.0) {
        color += mix(c0, c1, angle);
        vec3 intersectWithPlane = direction;
        intersectWithPlane /= intersectWithPlane.y;

        vec2 pos = intersectWithPlane.xz;

        float dist = length(pos);
        float result = sampleClouds(pos, skyboxTime / 30.0);
        a = result;
        result *= smoothstep(15.0, 0.5, dist);
        color = mix(color, vec3(1.0), result);
    } else {
        color = mix(c0, c0 / 3.0, -angle);
    }

    return color;
}