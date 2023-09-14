#version 430 core

uniform vec3 cameraPosition; 

in vec3 unnormalizedNormal; 
in vec3 fragmentWorldPosition; 
out vec4 fragColor;

/*generated end*/

void main() {
	// fragColor = vec4((normal + 1.0) / 2.0, 1);
	vec3 normal = normalize(unnormalizedNormal);
	vec3 viewDirection = normalize(cameraPosition - fragmentWorldPosition);
	vec3 directionalLightDir = normalize(vec3(0, 1, 0));
	vec3 reflectionDirection = reflect(-directionalLightDir, normal);
	vec3 color;
	float specular = clamp(pow(dot(reflectionDirection, viewDirection), 40.0), 0, 1);
	specular = clamp(specular, 0, 1);
	color += vec3(specular);
	float diffuse = dot(directionalLightDir, normal);
	diffuse = clamp(diffuse, 0, 1);
	vec3 waterColor = vec3(14,135,204) / 255.0;
	color += (diffuse + 0.3) * waterColor;
	fragColor = vec4(color, 1.0);
	//fragColor = vec4((normal + 1.0) / 2.0, 1.0);
}
