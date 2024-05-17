#version 430 core

in vec2 worldPosition; 

in mat3x2 clipToWorld; 
in vec2 cameraPosition; 
in float time; 
out vec4 fragColor;

/*generated end*/

#define ANIMATE
float t = time / 100.0;

vec2 hash(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float perlin01(vec2 p) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

    vec2  i = floor(p + (p.x + p.y) * K1);
    vec2  a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x);
    vec2  o = vec2(m, 1.0 - m);
    vec2  b = a - o + K2;
    vec2  c = a - 1.0 + 2.0 * K2;
    vec3  h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    vec3  n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
    return (dot(n, vec3(70.0)) + 1.0) * 0.5;
}

float octave01(vec2 p, int octaves) {
    float amplitude = .5;
    float frequency = 0.;
    float value = 0.0;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * perlin01(p);
        p *= 2.;
        amplitude *= .5;
    }
    return value;
}

vec2 hash2( vec2 p )
{
	// texture based white noise
	//return textureLod( iChannel0, (p+0.5)/256.0, 0.0 ).xy;
	
    // procedural white noise	
	return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 voronoi( in vec2 x )
{
    vec2 ip = floor(x);
    vec2 fp = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = hash2( ip + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( t + 6.2831*o );
        #endif	
        vec2 r = g + o - fp;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
		vec2 o = hash2( ip + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( t+ 6.2831*o );
        #endif	
        vec2 r = g + o - fp;

        if( dot(mr-r,mr-r)>0.00001 )
        md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
        //md = max((r-mr).x, ((r-mr).y));
    }

    return vec3( md, mr );
}

float g(vec2 p) {
    float v = smoothstep(0.0, 0.2, voronoi(p).x);
    v = min(voronoi(p).x, 0.2) / 0.2;
    return v;
}

vec3 getNormal(vec2 p )
{
    const float eps = 0.01; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3( g(p-h.xy) - g(p+h.xy),
                            2.0*h.x,
                            g(p-h.yx) - g(p+h.yx) ) );
}

void main() {
    // paralax
    vec2 p = worldPosition / 1800.0 - cameraPosition / (1800.0 * 6);

    vec3 col = vec3(0.0);
    float d = voronoi(p * 8.0).x;

    float g = dot(getNormal(p * 8.0), normalize(vec3(1.0, 1.0, 0.0)));
    col = vec3(g);

    vec3 background = vec3(0.0);

    col = mix(col + vec3(0.25), background, smoothstep(0.19, 0.2, d));
    col *= 0.05;
    fragColor = vec4(col,1.0);
}