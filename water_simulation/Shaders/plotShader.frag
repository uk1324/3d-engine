#version 430 core

uniform vec3 cameraWorldPosition; 

in vec3 fragNormal; 
in float colormapValue01; 
in vec3 fragmentWorldPosition; 
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
    // TODO: Draw height isolines.
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraWorldPosition - fragmentWorldPosition);
    vec3 reflectDir = reflect(-directionalLightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specularColor = specularStrength * spec * lightColor;  
        
    vec3 result = (ambientColor + diffuseColor + specularColor) * objectColor;

    float width = 0.03;
    float interpolationWidth = width / 5.0f;
    float gridCellSize = 1.0;
    {
        vec2 pos = fragmentWorldPosition.xz;
	    vec2 posInCell = mod(abs(pos), gridCellSize) - vec2(gridCellSize / 2.0);

        float distanceX = abs(posInCell.x);
	    float distanceY = abs(posInCell.y);
        // Normal has the slope rotated by 90deg. tan(x + 90deg) = 1/tan(x)
        float slopeAngleX = atan(abs(normal.x / normal.y));
        float slopeAngleY = atan(abs(normal.z / normal.y));
        // This could probably be simplified by using similar triangles and the pythagorean theorem.
        float slopeDistanceX = distanceX / abs(cos(slopeAngleX));
        float slopeDistanceY = distanceY / abs(cos(slopeAngleY));
        // Project the distances onto the slope.
        distanceX = slopeDistanceX;
        distanceY = slopeDistanceY;

        distanceX = smoothstep(width - interpolationWidth, width, distanceX);
	    distanceY = smoothstep(width - interpolationWidth, width, distanceY);
        result *= min(distanceY, distanceX);
    }

   {
        float posInCell = abs(mod(abs(fragmentWorldPosition.y + test), gridCellSize) - gridCellSize / 2.0);
        //float slopeDistanceX = float(double(abs(normal.y)) / length(dvec2(normal.xz)) * double(posInCell));

        float slopeDistanceX = length(normal) / length(normal.xz) * posInCell;
//        float slopeAngleX = abs(atan(length(normal.xz), normal.y));
//        float slopeDistanceX = posInCell / abs(sin(slopeAngleX));
        //slopeDistanceX /= normal.y;
        if (test1) {
            //slopeDistanceX = posInCell
            float slopeAngleX = abs(atan(length(normal.xz), normal.y));
            slopeDistanceX = posInCell / abs(sin(slopeAngleX));

        }
        float d = smoothstep(width - interpolationWidth, width, slopeDistanceX);

        result *= d;
   }

   {
//        float k = abs(mod(abs(fragmentWorldPosition.y), gridCellSize) - gridCellSize / 2.0);
//        vec3 f = vec3(fract (k));
//        vec3 df = vec3(fwidth(k));
//        //vec3 g = smoothstep(df * 3.0, df * 10.0, f);    
//        vec3 g = smoothstep(width - interpolationWidth, width, f); 
//        float c = g.x * g.y * g.z;    
//        result *= c;

//        float k = abs(mod(abs(fragmentWorldPosition.y), gridCellSize) - gridCellSize / 2.0);
//        vec3 f  = vec3(fract(k));    
//        vec3 df = vec3(fwidth(k));
////        vec3 g = smoothstep(width - interpolationWidth, width, f);    
////        //float c = g.x * g.y * g.z;    
////        float c = min(min(g.x, g.y), g.z);
//
//        //float c = g.x * g.y * g.z;    
//        //float c = f.x * f.y * f.z;
//        float c = f.x * f.y * f.z;
//        c = smoothstep(width - interpolationWidth, width, c);
//        result *= c;
//        //gl_FragColor = vec4(c, c, c, 1.0);
   }

    fragColor = vec4(result, 1.0);
}

//varying vec3 k;
//void main(){    
//    vec3 f  = fract (k * 100.0);    
//    vec3 df = fwidth(k * 100.0);    
//    vec3 g = smoothstep(df * 1.0, df * 2.0, f);    
//    float c = g.x * g.y * g.z;    
//    gl_FragColor = vec4(c, c, c, 1.0);
//}