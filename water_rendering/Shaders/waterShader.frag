#version 430 core

uniform vec3 cameraPosition; 
uniform vec3 directionalLightDirection; 
uniform vec3 scatteringColor; 

in vec3 unnormalizedNormal; 
in vec3 fragmentWorldPosition; 
out vec4 fragColor;

/*generated end*/

uniform float time;

#include "sampleSkybox.glsl"

//void main() {
//	vec3 normal = normalize(unnormalizedNormal);
//	vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
//	vec3 reflectionDirection = reflect(-viewDirection, normal);
//
//	vec3 color;
//
//	float schlickFresnel = pow((1.0 - dot(normal, viewDirection)), 5.0);
//	schlickFresnel = clamp(schlickFresnel, 0.0, 1.0);
//
//	color += sampleSkybox(reflectionDirection) * schlickFresnel;
//
//	float specular = dot(reflectionDirection, directionalLightDirection);
//	specular = pow(specular, 30.0);
//	specular *= schlickFresnel;
//	specular = clamp(specular, 0, 1);
//	color += specular * skyboxSunColor;
//
//	float diffuse = dot(directionalLightDirection, normal);
//	diffuse = clamp(diffuse, 0, 1);
//
//	vec3 waterColor = vec3(14, 135, 204) / 255.0 / 5;
//	color += (diffuse + 0.3) * waterColor;
//
//	fragColor = vec4(color, 1.0);
//}
//

// https://www.shadertoy.com/view/MdXyzX 
//#define WATER_DEPTH 1.0 
//void main() {
////	vec3 color;
////
////	color = vec3(1, 0, 0);
////
////	fragColor = vec4(normalize(unnormalizedNormal), 1.0);
// // calculate fresnel coefficient
////    vec3 normal = normalize(unnormalizedNormal);
////    vec3 N = normal;
////    vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
////    vec3 ray = viewDirection;
////    float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));
////    //fresnel = clamp(fresnel, 0.6, 5.0);
////    //fresnel = clamp(fresnel, 0.6, 5.0);
////    // reflect the ray and make sure it bounces up
////    vec3 R = normalize(reflect(ray, N));
////    R.y = abs(R.y);
////  
////    // calculate the reflection and approximate subsurface scattering
////    vec3 reflection = sampleSkybox(R);
////    vec3 waterHitPos = fragmentWorldPosition;
////    vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
////   // vec3 scattering = scatteringColor * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);
////
////    // return the combined result
////    vec3 C = fresnel * reflection + (1.0 - fresnel) * scattering;
////    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
////    //C = vec3(viewDirection);
////    fragColor = vec4(C, 1.0);
//
//
//
//	
//}

//void main() {
//	vec3 normal = normalize(unnormalizedNormal);
//	vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
//	vec3 reflectionDirection = reflect(-viewDirection, normal);
//
//	vec3 color;
//
//	float schlickFresnel = pow((1.0 - max(0.0, dot(normal, viewDirection))), 5.0);
//	schlickFresnel = clamp(schlickFresnel, 0.0, 1.0);
//
//	color = vec3(sampleSkybox(-reflectionDirection));
//	//color = vec3(pow(dot(reflectionDirection, directionalLightDirection), 25.0));
//	//color = vec3(1);
//	//color = reflectionDirection;
//	fragColor = vec4(color, 1.0);
//}
//

#define WATER_DEPTH 1.0 
void main() {
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
    vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

    // return the combined result
    vec3 C = fresnel * reflection + (1.0 - fresnel) * scattering;
    //fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
    fragColor = vec4(C, 1.0);
}