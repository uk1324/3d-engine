#version 430 core

uniform vec3 cameraPosition; 

in vec3 fragmentPosition; 
in vec3 normal; 
out vec4 fragColor;

/*generated end*/

#include "SkyboxSettingsData.glsl"
#include "sampleSkybox.glsl"

uniform sampler2D heightMap;

void main() {
	//float height = texture(heightMap, fragmentPosition.xz / 100.0).r;
	//fragColor = texture(heightMap, fragmentPosition.xz / 100.0 * 5.0);
	//fragColor = vec4(vec3(abs(height) / 256), 1.0);
	//fragColor = vec4(fragmentPosition.xz / 100.0 * 5.0, 0.0, 1.0);
//	float height = texture(heightMap, fragmentPosition.xz / 200.0).r;
//	fragColor = vec4(vec3(abs(height) / 2), 1.0);
	vec2 h = vec2(0.0, 0.01);
	float y00 = texture(heightMap, fragmentPosition.xz / 200.0).r * 2.5;
	float y10 = texture(heightMap, fragmentPosition.xz / 200.0 + h.yx).r * 2.5;
	float y01 = texture(heightMap, fragmentPosition.xz / 200.0 + h.xy).r * 2.5;

	float dydx = (y10 - y00) / h.y;
	float dydz = (y01 - y00) / h.y;
	vec3 normal = cross(vec3(1.0, dydx, 0.0), vec3(0.0, dydz, 1.0));
	normal = normalize(normal);
	normal = -normal;

	//normal = (normal + 1.0) / 2.0;
	float diffuse = dot(normal, normalize(vec3(-1.0, -1.0, 0.0)));
	diffuse = clamp(diffuse, 0.0, 1.0);
	diffuse += 0.1;
	fragColor = vec4(vec3(diffuse), 1.0);

	vec3 N = normal;
	vec3 viewDirection = normalize(fragmentPosition - cameraPosition);
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

	//fragColor = vec4((R + 1.0) / 2.0, 1.0);
	//fragColor = vec4((N + 1.0) / 2.0, 1.0);
	fragColor = vec4(reflection, 1.0);
}
