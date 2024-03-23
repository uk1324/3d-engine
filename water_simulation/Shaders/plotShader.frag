#version 430 core

uniform vec3 cameraWorldPosition; 

in vec3 fragNormal; 
in float colormapValue01; 
in vec3 fragmentWorldPosition; 
out vec4 fragColor;

/*generated end*/

uniform sampler1D colormap;

void main() {
    vec3 normal = normalize(fragNormal);
    if (!gl_FrontFacing) {
        normal = -normal;
    }

    vec3 directionalLightDir = normalize(vec3(1, 1, 0));
    vec3 lightColor = vec3(1.0);

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    //vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(normal, directionalLightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    vec3 objectColor = texture(colormap, colormapValue01).rgb;
    // TODO: Draw height isolines.
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraWorldPosition - fragmentWorldPosition);
    vec3 reflectDir = reflect(-directionalLightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
//    vec2 s = mod(fragmentWorldPosition.xz, 1);
//    s = smoothstep(0.0, 0.03, s);
//    result *= min(s.x, s.y);

    {
        vec2 pos = fragmentWorldPosition.xz;
        float gridCellSize = 1.0;
	    vec2 posInCell = mod(abs(pos), gridCellSize) - vec2(gridCellSize / 2.0);

        float distanceX = abs(posInCell.x);
	    float distanceY = abs(posInCell.y);
        float xSlopeAngle = atan(abs(normal.x / normal.y));
        float ySlopeAngle = atan(abs(normal.z / normal.y));
        float xSlopeLength = distanceX / abs(cos(xSlopeAngle));
        float ySlopeLength = distanceY / abs(cos(ySlopeAngle));
        distanceX = xSlopeLength;
        distanceY = ySlopeLength;

        float width = 0.03;
	    float interpolationWidth = width / 5.0f;

        distanceX = smoothstep(width - interpolationWidth, width, distanceX);
	    distanceY = smoothstep(width - interpolationWidth, width, distanceY);
        result *= min(distanceY, distanceX);
    }

    //result = normal;
    fragColor = vec4(result, 1.0);

//    vec3 directionalLightDir = normalize(vec3(1, 1, 0));
//
//    vec3 viewDirection = normalize(fragmentWorldPosition - cameraWorldPosition);
//    //float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(N, -ray)), 5.0)));
//    //float fresnel = r0 + (1.0 - r0) * pow((1.0 - dot(N, -ray)), 5.0);
//    //float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(N, -ray)), 5.0)));
//    //float fresnel = r0 + (1.0 - r0) * pow((1.0 - clamp(dot(N, -ray), 0.0, 1.0)), 5.0);
//    //float fresnel = r0 + (1.0 - r0) * pow((1.0 - dot(N, -ray)), 5.0);
//    //fresnel = clamp(fresnel, 0.0, 1.0);
//
//    //fresnel = clamp(fresnel, 0.4, 1.0);
//
//    // reflect the ray and make sure it bounces up
//    vec3 reflection = normalize(reflect(directionalLightDir, normal));
//    //R.y = abs(R.y);
//    float specular = pow(clamp(dot(reflection, viewDirection), 0, 1), 1.0);
//    //specular = 0.0;
//	float diffuse = clamp(dot(normalize(normal), directionalLightDir), 0, 1);
//	float ambient = 0.2;
//	diffuse = clamp(diffuse + ambient + specular, 0.0, 1.0);
//	//fragColor = vec4((normalize(normal) + vec3(1.0)) / 2.0, 1);
//	vec3 color = texture(colormap, colormapValue01).rgb;
//    color = cameraWorldPosition;
//	fragColor = vec4(vec3(clamp(diffuse, 0, 1)) * color, 1);
//	//fragColor = vec4(normal, 1.0);
}
