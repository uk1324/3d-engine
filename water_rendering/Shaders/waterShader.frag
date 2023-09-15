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

uniform float time;

#include "sampleSkybox.glsl"

#define WATER_DEPTH 1.0 

uniform int maxIterations; 

void main() {
    if (displayLodLevels) {
        fragColor = vec4(vec3(float(iterations) / float(maxIterations)), 1.0);
        return;
    }
//	vec3 color;
//
//	color = vec3(1, 0, 0);
//
//	fragColor = vec4(normalize(unnormalizedNormal), 1.0);
 // calculate fresnel coefficient
    vec3 normal = normalize(unnormalizedNormal);
    vec3 N = normal;
    vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
    vec3 ray = viewDirection;
    float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));
    fresnel = clamp(fresnel, 0.7, 1.0);

    // reflect the ray and make sure it bounces up
    vec3 R = normalize(reflect(ray, N));
    R.y = abs(R.y);
  
    // calculate the reflection and approximate subsurface scattering
    vec3 reflection = sampleSkybox(R);
    vec3 waterHitPos = fragmentWorldPosition;
    //vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
    vec3 scattering = scatteringColor * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

    // return the combined result
    vec3 C = fresnel * reflection + (1.0 - fresnel) * scattering;
    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
    fragColor = vec4(C, 1.0);
}