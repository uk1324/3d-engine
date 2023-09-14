#version 430 core

uniform vec3 cameraPosition; 
uniform vec3 directionalLightDirection; 

in vec3 unnormalizedNormal; 
in vec3 fragmentWorldPosition; 
out vec4 fragColor;

/*generated end*/

uniform float time;

#include "sampleSkybox.glsl"

void main() {
	vec3 normal = normalize(unnormalizedNormal);
	vec3 viewDirection = normalize(fragmentWorldPosition - cameraPosition);
	vec3 reflectionDirection = reflect(-viewDirection, normal);

	vec3 color;

	float schlickFresnel = pow((1.0 - dot(normal, viewDirection)), 5.0);
	schlickFresnel = clamp(schlickFresnel, 0.0, 1.0);

	color += sampleSkybox(reflectionDirection) * schlickFresnel;

	float specular = dot(reflectionDirection, directionalLightDirection);
	specular = pow(specular, 30.0);
	specular *= schlickFresnel;
	specular = clamp(specular, 0, 1);
	color += specular * skyboxSunColor;

	float diffuse = dot(directionalLightDirection, normal);
	diffuse = clamp(diffuse, 0, 1);

	vec3 waterColor = vec3(14, 135, 204) / 255.0 / 5;
	color += (diffuse + 0.3) * waterColor;
	//color = vec3(specular);
	fragColor = vec4(color, 1.0);
}
