#version 430 core

uniform vec3 cameraWorldPosition; 

in vec3 fragNormal; 
in float colormapValue01; 
in vec3 fragmentWorldPosition; 

in vec3 rangeScale; 
in vec2 rangeTranslation; 
out vec4 fragColor;

/*generated end*/

uniform sampler1D colormap;

uniform float test;
uniform bool test1;

void main() {
    vec3 normal = normalize(fragNormal);
    if (!gl_FrontFacing) {
        normal = -normal;
    }

    vec3 directionalLightDir = normalize(vec3(1, 1, 0));
    vec3 lightColor = vec3(1.0);

    float ambient = 0.3;
    vec3 ambientColor = ambient * lightColor;
  	
    float diffuse = max(dot(normal, directionalLightDir), 0.0);
    vec3 diffuseColor = diffuse * lightColor;
    
    vec3 objectColor = texture(colormap, colormapValue01).rgb;
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraWorldPosition - fragmentWorldPosition);
    vec3 reflectDir = reflect(-directionalLightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularColor = specularStrength * spec * lightColor;  
        
    vec3 result = (ambientColor + diffuseColor + specularColor) * objectColor;

    // Normal has the slope rotated by 90deg.
    {
        float width = 0.03;
        float interpolationWidth = width / 5.0f;
        vec2 gridCellSize = rangeScale.xz;
        vec2 pos = fragmentWorldPosition.xz + rangeTranslation;
	    vec2 posInCell = mod(abs(pos), gridCellSize) - vec2(gridCellSize / 2.0);
        float distanceX = abs(posInCell.x);
	    float distanceY = abs(posInCell.y);

        float slopeDistanceX = length(normal.xy) / abs(normal.y) * distanceX;
        float slopeDistanceY = length(normal.zy) / abs(normal.y) * distanceY;

        float heightGridCellSize = rangeScale.y;
        float heightInCell = abs(mod(abs(fragmentWorldPosition.y + test), heightGridCellSize) - heightGridCellSize / 2.0);
        float slopeDistanceHeight = length(normal) / length(normal.xz) * heightInCell;

        float d = min(min(slopeDistanceX, slopeDistanceY), slopeDistanceHeight);
        result *= smoothstep(width - interpolationWidth, width, abs(d));
    }

    fragColor = vec4(result, 1.0);
}
