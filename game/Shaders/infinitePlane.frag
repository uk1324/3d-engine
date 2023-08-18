#version 430 core

uniform mat4 inverseViewProjection; 
uniform vec2 screenSize; 
uniform float time; 
uniform float detail; 
out vec4 fragColor;

/*generated end*/

#include "Utils/posFromDepth.glsl"
#include "Utils/sampleFiltered.glsl"

vec3 sampleTexture(vec2 p) {
	ivec2 gridPos = ivec2(floor(p));

	if (gridPos.x % 2 == gridPos.y % 2) {
		return vec3(1);
	} else {
		return vec3(0);
	}
}

float HexDist(vec2 p) {
	p = abs(p);
    
    float c = dot(p, normalize(vec2(1,1.73)));
    c = max(c, p.x);
    
    return c;
}

vec4 HexCoords(vec2 uv) {
	vec2 r = vec2(1, 1.73);
    vec2 h = r*.5;
    
    vec2 a = mod(uv, r)-h;
    vec2 b = mod(uv-h, r)-h;
    
    vec2 gv = dot(a, a) < dot(b,b) ? a : b;
    
    float x = atan(gv.x, gv.y);
    float y = .5-HexDist(gv);
    vec2 id = uv-gv;
    return vec4(x, y, id.x,id.y);
}

vec3 sampl(vec2 uv) {
    vec3 col = vec3(0);

    uv *= 10.;

    vec4 hc = HexCoords(uv+100.);

    float c = smoothstep(.01, .03, hc.y*sin(hc.z*hc.w+ time));

    col += c;
    return col;
}

GENERATE_SAMPLE_FILTERED(sampleTextureFiltered, sampl)

void main() {
	vec3 worldPos = posFromDepth(gl_FragCoord.xy / screenSize, gl_FragCoord.z, inverseViewProjection);
	
	vec2 p = worldPos.xz / 20.0;

    vec3 color;

    if (gl_FragCoord.x / screenSize.x > 0.5) {
        color = sampleTextureFiltered(p, dFdx(p), dFdy(p), detail);
    } else {
        color = sampl(p);
    }
    color = sampleTextureFiltered(p, dFdx(p), dFdy(p), detail);

	fragColor = vec4(color, 1);
}
