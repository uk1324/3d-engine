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

#include "waves.glsl"

void main() {
    float dx = max(abs(dFdx(fragmentWorldPosition.x)), abs(dFdy(fragmentWorldPosition.x)));
    float dy = max(abs(dFdx(fragmentWorldPosition.y)), abs(dFdy(fragmentWorldPosition.y)));
    int iterations = clamp(int(16.0 / max(dx, dy)), 1, maxIterations);

    if (displayLodLevels) {
        fragColor = vec4(vec3(float(iterations) / float(maxIterations)), 1.0);
        return;
    }
    
    //fragColor = vec4(vec3(x * y), 1.0);
    //fragColor = vec4(vec3(), 1.0);

    //vec3 normal = normalize(unnormalizedNormal);
    //vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, 1.0, iterations);
    //vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, 1.0, 64);
    vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, 1.0, iterations);
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
    //fragColor = vec4(vec3(abs(fragmentWorldPosition.y) / 5.0), 1.0);
    //fragColor = vec4(vec3(float(iter) / 64), 1.0);
    //fragColor = vec4(vec3(max(dFdx(abs(fragmentWorldPosition.x)), dFdx(abs(fragmentWorldPosition.y)))), 1.0);
}