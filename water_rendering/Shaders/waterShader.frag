#version 430 core

uniform vec3 cameraPosition; 
uniform vec3 directionalLightDirection; 
uniform vec3 scatteringColor; 
uniform bool displayLodLevels; 

in vec3 unnormalizedNormal; 
in vec3 fragmentWorldPosition; 
in flat int iterations; 
out vec4 fragColor;

/*generated end*/

#include "SkyboxSettingsData.glsl"

uniform float time;

#include "sampleSkybox.glsl"

#define WATER_DEPTH 1.0 

uniform int maxIterations; 

void main() {
    if (displayLodLevels) {
        fragColor = vec4(vec3(float(iterations) / float(maxIterations)), 1.0);
        return;
    }

    vec3 normal = normalize(unnormalizedNormal);
    vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);

    vec3 ray = viewDirection;
    float fresnel = (0.04 + (1.0 - 0.04) * (pow(1.0 - max(0.0, dot(-normal, ray)), 5.0)));
    fresnel = clamp(fresnel, 0.7, 1.0);

    vec3 reflectionDirection = normalize(reflect(ray, normal));

    vec3 reflection = sampleSkybox(reflectionDirection);
    vec3 waterHitPos = fragmentWorldPosition;

    //vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
    //vec3 scattering = vec3(41) / 255 * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
    vec3 scattering = scatteringColor * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

    vec3 color;
    color = fresnel * reflection + (1.0 - fresnel) * scattering;

    fragColor = vec4(color, 1.0);
}