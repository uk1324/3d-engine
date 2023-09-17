#version 430 core

uniform vec3 cameraPosition; 
uniform vec3 directionalLightDirection; 
uniform vec3 scatteringColor; 
uniform bool displayLodLevels; 
uniform bool useAnalyticalDerivatives; 

in vec3 unnormalizedNormal; 
in vec3 fragmentWorldPosition; 
in flat int iterations; 
out vec4 fragColor;

/*generated end*/

#include "SkyboxSettingsData.glsl"

uniform float time;

#include "waterDepth.glsl"
#include "sampleSkybox.glsl"

//#define WATER_DEPTH 1.0 

uniform int maxIterations; 

#include "waves.glsl"

vec3 aces_tonemap(vec3 color) {  
  mat3 m1 = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
  );
  mat3 m2 = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
  );
  vec3 v = m1 * color;  
  vec3 a = v * (v + 0.0245786) - 0.000090537;
  vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
  return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));  
}

void main() {
    float dx = max(abs(dFdx(fragmentWorldPosition.x)), abs(dFdy(fragmentWorldPosition.x)));
    float dy = max(abs(dFdx(fragmentWorldPosition.y)), abs(dFdy(fragmentWorldPosition.y)));
    int iterations = clamp(int(16.0 / max(dx, dy)), 1, maxIterations);

//    if (displayLodLevels) {
//        fragColor = vec4(vec3(float(iterations) / float(maxIterations)), 1.0);
//        return;
//    }
    
    //fragColor = vec4(vec3(x * y), 1.0);
    //fragColor = vec4(vec3(), 1.0);

    //vec3 normal = normalize(unnormalizedNormal);
    //vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, 1.0, iterations);
    //vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, 1.0, 64);
    vec3 normal = sampleWaveNormal(fragmentWorldPosition.xz / 10.0, WATER_DEPTH, iterations);
    vec2 d = derivatives(fragmentWorldPosition.xz / 10.0, iterations);
    vec4 j = jacobian(fragmentWorldPosition.xz / 10.0, iterations);

    normal = normalize(normal);
    vec3 N = normal;
    vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
    vec3 ray = viewDirection;
    float waterIndexOfRefraction = 1.33;
    float airIndexOfRefrection = 1.0;
    float r0 = pow((airIndexOfRefrection - waterIndexOfRefraction) / (airIndexOfRefrection + waterIndexOfRefraction), 2.0);
    //float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(N, -ray)), 5.0)));
    float fresnel = r0 + (1.0 - r0) * pow((1.0 - dot(N, -ray)), 5.0);
    //float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(N, -ray)), 5.0)));
    //float fresnel = r0 + (1.0 - r0) * pow((1.0 - clamp(dot(N, -ray), 0.0, 1.0)), 5.0);
    //float fresnel = r0 + (1.0 - r0) * pow((1.0 - dot(N, -ray)), 5.0);
    //fresnel = clamp(fresnel, 0.0, 1.0);

    //fresnel = clamp(fresnel, 0.4, 1.0);

    // reflect the ray and make sure it bounces up
    vec3 R = normalize(reflect(ray, N));
    R.y = abs(R.y);
  
    // calculate the reflection and approximate subsurface scattering
    vec3 reflection = sampleSkybox(R);
    vec3 waterHitPos = fragmentWorldPosition;
    //vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
    //float WATER_DEPTH = 1.0;
    //vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y / 10.0 + WATER_DEPTH) / WATER_DEPTH);
    vec3 scattering = scatteringColor * (0.2 + (waterHitPos.y / 10.0 + WATER_DEPTH) / WATER_DEPTH);

    // return the combined result
    vec3 C;
    if (displayLodLevels) {
        C = fresnel * reflection + (1.0 - fresnel) * scattering;
    } else {
        C = vec3(fresnel);
    }
    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
    //fragColor = vec4(vec3(fresnel), 1.0);
    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
    //fragColor = vec4(reflection, 1.0);
    C = vec3(fresnel);
    C = scattering;
    C = fresnel * reflection + (1.0 - fresnel) * scattering;

    float foam = smoothstep(1.5 / 6, 0.6, length(d));
    //float foam = step(2.5 / 6, length(d));
    C = mix(C, vec3(1.0), foam);
    //C += vec3(foam);
    //C = reflection;
    //C = vec3(fresnel < 1.0 && fresnel > 0.0);
    fragColor = vec4(C, 1.0);

    //float h = j.x * j.w - j.y * j.z;
    //float h = (j.x + 1.0) * (j.w + 1.0) - (j.y + 1.0) * (j.z + 1.0);
//    fragColor = j;
//    //h = abs(0.01 - length(d));
//    fragColor = vec4(vec3(d, 0.0), 1.0);
//    //fragColor = vec4(vec3(length(d) > 2.5 / 6), 1.0);
//    //fragColor = vec4(vec3(smoothstep(2.5 / 6, 0.6, length(d))), 1.0);
//    float foam = smoothstep(2.1 / 6, 0.6, length(d));
    
    //fragColor = vec4(vec3()), 1.0);
    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
//    C = vec3(dot(N, -ray));
//    C = viewDirection;
    //fragColor = vec4(C, 1.0);

//    vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
//    vec3 ray = viewDirection;
//    float fresnel = (0.04 + (1.0 - 0.04) * (pow(1.0 - max(0.0, dot(-normal, ray)), 5.0)));
//    //fresnel = clamp(fresnel, 0.99, 1.0);
//
//    vec3 reflectionDirection = normalize(reflect(ray, normal));
//
//    vec3 reflection = sampleSkybox(reflectionDirection);
//    vec3 waterHitPos = fragmentWorldPosition;
//
//    //vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
//    //vec3 scattering = vec3(41) / 255 * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
//    vec3 scattering = scatteringColor * (0.2 + (waterHitPos.y));
//
//    vec3 color;
//    color = fresnel * reflection + (1.0 - fresnel) * scattering;
//    fragColor = vec4(color, 1.0);
//    vec2 der = sampleDerivatives(fragmentWorldPosition.xz / 10.0, iterations);
//    if (displayLodLevels) {
//        fragColor = vec4(vec3(length(der)), 1.0);
////        vec4 a = jacobian(fragmentWorldPosition.xz / 10.0, iterations);
////        float d = (a.x) * (a.w) - (a.y) * (a.z);
////        //fragColor = vec4(vec3(-a.xxx / 1000000.0), 1.0);
////        fragColor = vec4(vec3(-a.xxx / 1000000.0), 1.0);
////        fragColor = vec4(a.xyz, 1.0);
//    } else {
//        fragColor = vec4((normal + 1.0) / 2.0, 1.0);
//    }
//    //fragColor = vec4(a.xyz, 1.0);
//    //fragColor = vec4(color + vec3(length(der)), 1.0);
//    //fragColor = vec4(color + vec3(max(abs(length(der) - 0.01), 0.0)), 1.0);
//    float h = 0.3;
//    vec3 foam = vec3((h - length(der) / h / 40.0));
//    if (fragmentWorldPosition.y < 0.0) {
//        foam = vec3(0.0);
//    }
//    foam = abs(foam);
//    //fragColor = vec4((vec3(length(der) / 5.0)), 1.0);
//    float f = 0.005 - length(der) / 50.0;
//    f /= 0.005;
    //f = pow(f / 0.2, 2.0);
    //fragColor = vec4((vec3(f)), 1.0);
    //fragColor = vec4(color + vec3(f), 1.0);
    //fragColor = vec4(color, 1.0);
    //fragColor = vec4((vec3(0.2 - length(der) / 5.0)), 1.0);
    //fragColor = vec4((normal + 1.0) / 2.0, 1.0);
    //fragColor = vec4(normal, 1.0);
    //fragColor = vec4(vec3(abs(fragmentWorldPosition.y) / 5.0), 1.0);
    //fragColor = vec4(vec3(float(iter) / 64), 1.0);
    //fragColor = vec4(vec3(max(dFdx(abs(fragmentWorldPosition.x)), dFdx(abs(fragmentWorldPosition.y)))), 1.0);
}