#version 430 core

uniform mat4 inverseViewProjection; 
uniform vec3 cameraPositionWorldSpace; 
uniform vec2 screenSize; 
out vec4 fragColor;

/*generated end*/

#include "Utils/posFromDepth.glsl"

vec4 textureAniso(vec2 p) {
    mat2 J = inverse(mat2(dFdx(p),dFdy(p)));       // dFdxy: pixel footprint in texture space
    J = transpose(J)*J;                            // quadratic form
    float d = determinant(J), t = J[0][0]+J[1][1], // find ellipse: eigenvalues, max eigenvector
          D = sqrt(abs(t*t-4.*d)),                 // abs() fix a bug: in weird view angles 0 can be slightly negative
          V = (t-D)/2., v = (t+D)/2.,                     // eigenvalues. ( ATTENTION: not sorted )
          M = 1./sqrt(V), m = 1./sqrt(v), l =log2(m*screenSize.y); // = 1./radii^2
  //if (M/m>16.) l = log2(M/16.*R.y);                     // optional
    vec2 A = M * normalize(vec2( -J[0][1] , J[0][0]-V )); // max eigenvector = main axis
    vec4 O = vec4(0);
    for (float i = -7.5; i<8.; i++)                       // sample x16 along main axis at LOD min-radius
        O += vec4(p+(i/16.)*A, 0, 0);
	//textureLod(iChannel0, p+(i/16.)*A, l);
    return O/16.;
}

float checkersTextureGradBox( in vec2 p, in vec2 ddx, in vec2 ddy )
{
    // filter kernel
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;  
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)/2.0)-0.5)-abs(fract((p+0.5*w)/2.0)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;                  
}

vec2 pri( in vec2 x )
{
    // see https://www.shadertoy.com/view/MtffWs
    vec2 h = fract(x/2.0)-0.5;
    return x*0.5 + h*(1.0-2.0*abs(h));
}


float checkersTextureGradTri( in vec2 p, in vec2 ddx, in vec2 ddy )
{
    vec2 w = max(abs(ddx), abs(ddy)) + 0.001;      // filter kernel
    vec2 i = (pri(p+w)-2.0*pri(p)+pri(p-w))/(w*w); // analytical integral (triangle filter)
    return 0.5 - 0.5*i.x*i.y;                  // xor pattern
}

const int MaxSamples = 10;  // 10*10

vec3 sampleTexture(vec2 p) {
	ivec2 gridPos = ivec2(floor(p));

	if (gridPos.x % 2 == gridPos.y % 2) {
		return vec3(1);
	} else {
		return vec3(0);
	}
}

vec3 sampleTextureWithFilter( in vec2 uvw, in vec2 ddx_uvw, in vec2 ddy_uvw)
{
    int sx = 1 + int( clamp( 4.0*length(ddx_uvw-uvw), 0.0, float(MaxSamples-1) ) );
    int sy = 1 + int( clamp( 4.0*length(ddy_uvw-uvw), 0.0, float(MaxSamples-1) ) );

	vec3 no = vec3(0.0);

    for (int j=0; j<sy; j++) {
		for (int i=0; i<sx; i++)  {
			vec2 st = vec2( float(i), float(j) ) / vec2( float(sx),float(sy) );
			no += sampleTexture( uvw + st.x*(ddx_uvw-uvw) + st.y*(ddy_uvw-uvw));
		}
	}
    
	return no / float(sx*sy);
}

void main() {
	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseViewProjection);
	
	//vec2 p = textureAniso(worldPos.xz).xy;
	vec2 p = worldPos.xz;
	ivec2 gridPos = ivec2(floor(p));

	float d = distance(cameraPositionWorldSpace, worldPos);

	vec3 color;
	if (gridPos.x % 2 == gridPos.y % 2) {
		color = vec3(1);
	} else {
		color = vec3(0);
	}

	//color = vec3(checkersTextureGradBox(p, dFdx(p), dFdy(p)));
	//color = sampleTextureWithFilter(p, dFdx(p), dFdy(p));
	color = sampleTextureWithFilter(p, p + dFdx(p), p + dFdy(p));
	//color = sampleTexture(p);

	//vec3 corner1 = color;`
	//vec3 corner2 = color + dFdx(color);`
	//vec3 corner3 = color + dFdy(color);`
	//vec3 corner4 = color + dFdx(color) + dFdy(color);`
	//color = (corner1 + corner2 + corner3 + corner4) / 4;`


	//color = mix(color, vec3(0.5), smoothstep(20, 60, d));


//	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseViewProjection);
//	
//	ivec3 gridPos = ivec3(floor(worldPos));
//
//	float d = distance(cameraPositionWorldSpace, worldPos);
//
//	vec3 color;
//	if (gridPos.x % 2 == gridPos.z % 2) {
//		color = vec3(1);
//	} else {
//		discard;
//		color = vec3(0.3);
//	}
//
//	color *= smoothstep(25, 20, d);


	// float depth = gl_FragCoord.z * 2.0 - 1.0;
	// color *= 1.0 - smoothstep(0.97, 0.98, depth);

	//color = vec3(depth);
	//fragColor vec4(p.xy, 0, 1);
	//fragColor = vec4(gl_FragCoord.z, 1, 0, 1);
	//fragColor = vec4(gl_FragCoord.xy / screenSize, 0, 1);
	//fragColor = vec4(pos, 1);
	
	//fragColor = vec4(floor(mod(pos.xz, 2)), 0, 1);
	fragColor = vec4(color, 1);
	//fragColor = vec4(vec3(worldPos.z), 1);
	//fragColor = vec4(pos.xyz, 1);
	//fragColor = vec4(mod(p, 0.1), 1);
}
