#version 430 core

uniform float time; 
uniform vec2 cameraPosition; 
uniform vec3 color; 

in vec2 worldPosition; 
in vec2 position; 

in float t; 
out vec4 fragColor;

/*generated end*/

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
		//#ifdef ANIMATE
        o = 0.5 + 0.5*sin( time + 6.2831*o );
        //#endif	
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

#include "Utils/noise2.glsl"
#include "Utils/noise3.glsl"

//void main() {
//	vec2 p = position;
//	p -= 0.5;
//	p *= 2.0;
//
////	float r = length(p);
////	float a = atan(p.y, p.x);
////	float isolineMovementT = 0.0f;
////	float PI = 3.14159;
////
////	float k = smoothstep(1.0, 0.0, abs(fract((a * 7.0)/ PI + isolineMovementT) - 0.5));
////
////	float d = length(p);
////	d = smoothstep(0.6, 0.5, d);
////	vec3 col;
////    vec2 pos = worldPosition;
////    pos /= 10.0;
////    d = voronoi(pos).x;
////
////    //d *= pow(length(p) * 2.0, 2.0);
////    //d = min(2.0, d);
////    d += octave01(worldPosition / 10.0, 2) * 0.5;
////    d /= 1.5;
////    //d = octave01(worldPosition / 40.0, 2) * 0.1;
////    d *= length(p) * 10.0;
////    d = smoothstep(0.6, 0.05, d);
////	col = vec3(d);
//    vec3 col;
//    float n = octave01(worldPosition / 40.0, 4);
//    n -= 0.5;
//    n = abs(n);
//    float n1 = octave01(worldPosition / 15.0, 4);
//    n1 -= 0.5;
//    n1 = abs(n1);
//    n1 *= 2.0;
//    //n1 = smoothstep(0.07, 0.02, n1);
//    n = n1;
//
//    //p /= 4.0;
//    //n = min(n, n1);
//    //n = smoothstep(0.05, 0.02, n);
//    float h = length(p) - 0.125;
//    h += octave01(worldPosition / 15.0, 4) * 0.02;
//    float k = h;
//    h = abs(h);
//    h = smoothstep(0.0, 0.004, h);
//    //n *= h;
//    n *= h;
//    //n -= h;
//    //n += h;
//    //n = min(h, n);
//    col = vec3(n);
//
//    //1.0 / dep
//  
//    float g = smoothstep(h + 0.01, h, length(p));
//    g = 1.0;
//    g = smoothstep(0.004, 0.0, k);
//
//    g = 1.0;
//    float i = voronoi(p * 2.0).x;
//    i *= octave01(p * 20.0, 2) * 0.2;
//    for (int j = 0; j < 5; j++) {
//        i -= 0.5;
//        i = abs(i);
//        i *= 2.0;
//    }
//    //i += octave01(i, 0.1);
//    col = vec3(i);
//	//col = vec3(k);
//	//col *= vec3(0.0, 1.0, 0.0);
//	//fragColor = vec4(col, 1.0);
//    fragColor = vec4(col, g);
//}
//

// return distance, and cell id
vec2 voronoii(vec2 x)
{
    vec2 n = floor( x );
    vec2 f = fract( x );

	vec3 m = vec3( 8.0 );
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2  g = vec2( float(i), float(j) );
        vec2  o = hash( n + g );
      //vec2  r = g - f + o;
	    vec2  r = g - f;
		float d = dot( r, r );
        if( d<m.x )
            m = vec3( d, o );
    }

    return vec2( sqrt(m.x), m.y+m.z );
}

float smin1(float a, float b, float k)
{
    return pow((0.5 * (pow(a, -k) + pow(b, -k))), (-1.0 / k));
}

float smax0(float a, float b, float k)
{
    return smin1(a, b, -k);
}

float sa(vec2 p) {
    float v = max(voronoi(p).x, voronoi(p / 2.0).x);
    //v = smoothstep(0.0, 1.1, v);
    return v;
}

vec2 hash22(vec2 p) { 

    // Faster, but doesn't disperse things quite as nicely as other combinations. :)
    float n = sin(dot(p, vec2(41, 289)));
    //return fract(vec2(262144, 32768)*n)*.75 + .25; 
    
    // Animated.
    p = fract(vec2(262144, 32768)*n); 
    return sin( p*6.2831853 + 0.0 )*.35 + .65; 
    
}

float Voronoi(in vec2 p){
    
	vec2 g = floor(p), o; p -= g;
	
	vec3 d = vec3(1); // 1.4, etc.
    
    float r = 0.;
    
	for(int y = -1; y <= 1; y++){
		for(int x = -1; x <= 1; x++){
            
			o = vec2(x, y);
            o += hash22(g + o) - p;
            
			r = dot(o, o);
            
            // 1st, 2nd and 3rd nearest squared distances.
            d.z = max(d.x, max(d.y, min(d.z, r))); // 3rd.
            d.y = max(d.x, min(d.y, r)); // 2nd.
            d.x = min(d.x, r); // Closest.
                       
		}
	}
    
	d = sqrt(d); // Squared distance to distance.
    
    // Fabrice's formula.
    return min(2./(1./max(d.y - d.x, .001) + 1./max(d.z - d.x, .001)), 1.);
    // Dr2's variation - See "Voronoi Of The Week": https://www.shadertoy.com/view/lsjBz1
    //return min(smin(d.z, d.y, .2) - d.x, 1.);
    
}

vec2 hMap(vec2 uv){
    
    // Plain Voronoi value. We're saving it and returning it to use when coloring.
    // It's a little less tidy, but saves the need for recalculation later.
    float h = Voronoi(uv*6.);
    
    // Adding some bordering and returning the result as the height map value.
    float c = smoothstep(0., fwidth(h)*2., h - .09)*h;
    c += (1.-smoothstep(0., fwidth(h)*3., h - .22))*c*.5; 
    
    // Returning the rounded border Voronoi, and the straight Voronoi values.
    return vec2(c, h);
    
}

float g(vec2 p) {
    {
//        float v = smoothstep(0.0, 0.1, voronoi(p).x);
//        v = min(voronoi(p).x, 0.1) / 0.1;
//        return v;
        //float v = voronoi(p).x - voronoi(p - 0.5).x;
        //float v = (voronoi(p).x + voronoi(p + 1.1).x) / 2.0;
         //float v = max(voronoi(p).x, voronoi(p / 2.0).x);
         //v *= 2.0;
         //float a = voronoi(p).x - voronoi(p - 0.5).x;
         //vec3 a = voronoi(p);
         float a = max(voronoi(p).x, voronoi(p / 2.0).x);
         a *= 2.0;
         float v = a.x;
         //v = smoothstep(0.0, 0.3, v);
         //v += 51.0 * sin(time);


//        float off = 0.6;
//        off = 0.0;
//        v += off;
//        v = smoothstep(off, 1.0, v);
        return v;
    }
    //p *= 11.0;
    //float v = max(Voronoi(p / 10.0).x, Voronoi(p / 20.0).x);
    //vec2 h = vec2(0.0, 0.03);
    //return (sa(p) + sa(p + vec2(h.xy)) + sa(p - vec2(h.xy)) + sa(p + vec2(h.yx)) + sa(p - vec2(h.yx))) / 5;
    float v = smax0(voronoi(p).x, voronoi(p / 2.0).x, 4.0);
    p += 0.1;
    float v2 = smax0(voronoi(p).x, voronoi(p / 2.0).x, 4.0);
    v = (v + v2) / 2.0;
    //float v = max(voronoi(p).x, voronoi(p / 2.0).x);
    //v = smoothstep(0.0, 2.0, v);
    return v;
}

vec3 getNormal(vec2 p )
{
    const float eps = 0.05; // or some other value
    const vec2 h = vec2(eps,0);
    return normalize( vec3( g(p-h.xy) - g(p+h.xy),
                            g(p-h.yx) - g(p+h.yx),
                            2.0*h.x
                            ) );
}

vec3 nr(vec2 p, inout float edge) { 
	
    vec2 e = vec2(.05, 0);

    // Take some distance function measurements from either side of the hit point on all three axes.
	float d1 = g(p + e.xy), d2 = g(p - e.xy);
	float d3 = g(p + e.yx), d4 = g(p - e.yx);
	float d5 = g(p + e.yy), d6 = g(p - e.yy);
	float d = g(p)*2.;	// The hit point itself - Doubled to cut down on calculations. See below.
     
    // Edges - Take a geometry measurement from either side of the hit point. Average them, then see how
    // much the value differs from the hit point itself. Do this for X, Y and Z directions. Here, the sum
    // is used for the overall difference, but there are other ways. Note that it's mainly sharp surface 
    // curves that register a discernible difference.
    edge = abs(d1 + d2 - d) + abs(d3 + d4 - d) + abs(d5 + d6 - d);
    //edge = max(max(abs(d1 + d2 - d), abs(d3 + d4 - d)), abs(d5 + d6 - d)); // Etc.
    
    // Once you have an edge value, it needs to normalized, and smoothed if possible. How you 
    // do that is up to you. This is what I came up with for now, but I might tweak it later.
    edge = smoothstep(0., 1., sqrt(edge/e.x*2.));
	
    // Return the normal.
    // Standard, normalized gradient mearsurement.
    return normalize(vec3(d1 - d2, d3 - d4, d5 - d6));
}

#define PI 3.14159

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
    //denom = max(denom, 0.02);
	
    return nom / denom;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float sdPentagon( in vec2 p, in float r )
{
    const vec3 k = vec3(0.809016994,0.587785252,0.726542528); // pi/5: cos, sin, tan
    p.y = -p.y;
    p.x = abs(p.x);
    p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
    p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
	p -= vec2(clamp(p.x,-r*k.z,r*k.z),r);    
    return length(p)*sign(p.y);
}

void main() {
	vec2 p = position;
	p -= 0.5;
	p *= 2.0;

    vec3 col;

    p *= 3.0;

    col = (vec3(p, 0.0));
    const int vertexCount = 5;
    vec3 vertices[vertexCount];
    for (int i = 0; i < vertexCount; i++) {
        float t = float(i) / float(vertexCount);
        float a = t * 2.0 * 3.141592;
        vertices[i] = vec3(cos(a), sin(a), 1.0);
    }
 
//    float d = 100.0;
//    for (int i = 0; i < vertexCount; i++) {
//        d = min(d, distance(p, vertices[i].xy));
//    }
//    col = vec3(d);
//
//    d = length(p);
//
//    d += octave01(vec3(p, time), 4) * 0.51;
//
//    d = smoothstep(0.25, 1.0, d);
    float d;
    //d = sdPentagon(p, 0.25);

    //d = smoothstep(-0.6, 0.3, d);

    vec3 colr;
    float t1 = 0.005;
    float t2 = 0.11;

    //vec3 c1 = vec3(1./4., 1.0 ,1./16.);
    vec3 c1 = vec3(1./4., 1.0 ,1./16.);
    vec3 c2 = vec3(1.0 / 4.0);
    vec3 c3 = c1 * 3;

    d = length(p);

    if (t < t1) {
        float lt = smoothstep(0.0, t1, t);
        colr = mix(c1, c3, lt);
        d -= lt * 0.1;
    } else if (t < t2) {
        float lt = smoothstep(t1, t2, t);
        colr = mix(c3, c2, lt);
        d -= (1.0 - lt) * 0.1;
    } else {
        colr = mix(c2, c1, smoothstep(t2, 1.0, t));
    }

    d += octave01(vec3(p, time * 2.0), 4) * 0.5;
    //d = pow(d, 0.9);
    d = smoothstep(1.0, 0.25, d);

    //d = smoothstep(0.5, 0.25, d);
    col = vec3(d);
    float g = octave01(vec3(p * 4.0, time), 4);
    //vec3 c = vec3(1,1./4.,1./16.) * exp(4.*g - 1.);
    //vec3(1./4., 1.0 ,1./16.)




    //vec3 c = vec3(1./4., 1.0 ,1./16.) * exp(4.*g - 1.);
    vec3 c = colr * exp(4.*g - 1.);
    //vec3 c = color * exp(4.*g - 1.);
    //vec3 c = vec3(1./4) * exp(4.*g - 1.);
    //vec3 c = pow(vec4(.1, .7, .8, 1), vec4(4.*U.x));    
    col *= c / 2.0;
    //col *= color;
    //col += octave01(vec3(p * 2.0, 0.0), 4) * 0.1;
    //col *= vec3(0.7, 1.0, 0.7);
    col /= 4.0;
    //col *= vec3(0,208,98) / 255.0;
    //col = vec3(octave01(vec3(p * 2.0, 0.0), 4));

//    vec3 normal = getNormal(p * 2.0);
//    {
//        vec3 lightDir = normalize(vec3(1.0, 1.0, -1.0));
//        vec3 toLight = -lightDir;
//        vec3 camera = vec3(cameraPosition, + 5.0);
//        vec3 fragmentPosition = vec3(worldPosition, g(p * 2.0));
//        vec3 viewDir = normalize(fragmentPosition - camera);
//        vec3 toView = -viewDir;
//        vec3 halfway = normalize(toLight + toView);
//        vec3 refraction = refract(toLight, normal, 1.60);
//        vec3 reflectionDirection = normalize(reflect(toLight, normal));
//        float specular = DistributionGGX(normal, halfway, 0.1);
//        col *= (1.0 + specular * 0.1);
//        col = vec3(specular);
//    }
//    col *= (1.0 + 0.11 * dot(normal, normalize(vec3(1.0, 1.0, -1.0))));

    fragColor = vec4(col, d);
}

//void main() {
//	vec2 p = position;
//	p -= 0.5;
//	p *= 2.0;
//
//    float d = voronoi(p).x;
// 
//    vec3 col;
//    vec2 posi = p * 4.0;
//    posi *= 2.0;
//    vec3 normal = getNormal(posi);
//    float edge;
//    //vec3 normal = nr(p * 4.0, edge);
////    normal = (smoothstep(-1.0, 1.0, normal) - 0.5) * 2.0;
//  //  col = normal;
//
//    float tt = time;
//    //vec3 lightDir = normalize(vec3(cos(tt) * 0.5, sin(tt) * 0.5, -1.0));
//    vec3 lightDir = normalize(vec3(0.1, 0.1, -1.0));
//    vec3 toLight = -lightDir;
//    vec3 camera = vec3(cameraPosition, + 5.0);
//    vec3 fragmentPosition = vec3(worldPosition, g(posi));
//    vec3 viewDir = normalize(fragmentPosition - camera);
//    vec3 toView = -viewDir;
//    vec3 halfway = normalize(toLight + toView);
//    vec3 refraction = refract(toLight, normal, 1.60);
//    vec3 reflectionDirection = normalize(reflect(toLight, normal));
//
//
//    float specular = DistributionGGX(normal, halfway, 0.1);
//    float diffuse = 1.0 - specular;
//
//    vec3 F0 = vec3(0.04);
//    vec3 F = fresnelSchlick(max(dot(halfway, toView), 0.0), F0);
//
////    float roughness = 0.2;
////    col = vec3(dot(normal, lightDir));
//    //col = vec3(dot(reflectionDirection, viewDir));
//    col = vec3(dot(refraction, viewDir));
//
//    vec3 c1 = vec3(0,77,36) / 255.0;
//    vec3 c2 = vec3(0,208,98) / 255.0;
//    //col = mix(c1, c2, specular);
//
//    //col = vec3(specular) * c2 + c1 * 0.15 * F;
//    col = vec3(specular) * c2 + c1 * 0.15 * F;
//    //col = vec3(edge);
//    //col = vec3(fragmentPosition.z);
//    //col = (vec3(dot(lightDir, normal)) + 1.0) / 2.0;
//    //col = normal;
//    //col = F;
//    //col = vec3(normal);
//    float a = length(p);
//    a += octave01(posi, 2) * 0.1;
//    a -= 0.2;
//    a = smoothstep(0.02, 0.0, a);
//
//    fragColor = vec4(col, a);
//}


//void main() {
//	vec2 p = position;
//	p -= 0.5;
//	p *= 2.0;
//
////	float r = length(p);
////	float a = atan(p.y, p.x);
////	float isolineMovementT = 0.0f;
////	float PI = 3.14159;
////
////	float k = smoothstep(1.0, 0.0, abs(fract((a * 7.0)/ PI + isolineMovementT) - 0.5));
////
////	float d = length(p);
////	d = smoothstep(0.6, 0.5, d);
////	vec3 col;
////    vec2 pos = worldPosition;
////    pos /= 10.0;
////    d = voronoi(pos).x;
////
////    //d *= pow(length(p) * 2.0, 2.0);
////    //d = min(2.0, d);
////    d += octave01(worldPosition / 10.0, 2) * 0.5;
////    d /= 1.5;
////    //d = octave01(worldPosition / 40.0, 2) * 0.1;
////    d *= length(p) * 10.0;
////    d = smoothstep(0.6, 0.05, d);
////	col = vec3(d);
//
//    float d = voronoi(p).x;
//    //d -= 0.5;
//    //d = abs(d);
//    //d *= 2.0;
//    //d = abs(d - 0.5) * 2.0;
//    //d *= voronoi(p * 2.0).x / 2.0;
//    vec3 col;
//    vec3 normal = getNormal(p * 4.0);
//    normal = (smoothstep(-1.0, 1.0, normal) - 0.5) * 2.0;
//    col = normal;
//
//    //vec3 lightDir = normalize(vec3(-1.0, 1.0, 1.0));
//    float tt = time;
//    vec3 lightDir = normalize(vec3(cos(tt) * 0.5, sin(tt) * 0.5, 1.0));
//    float g = dot(normal, lightDir);
//    g += 1.0;
//    g /= 2.0;
//    col = vec3(g);
//    vec3 c1 = vec3(0,77,36) / 255.0;
//    vec3 c2 = vec3(0,208,98) / 255.0;
//
//    vec3 camera = vec3(cameraPosition, -5.0);
//    vec3 position = vec3(worldPosition, 0.0);
//    vec3 view = normalize(position - camera);
//    vec3 refraction = refract(-lightDir, normal, 1.60);
//    vec3 reflectionDirection = normalize(reflect(lightDir, normal));
//    reflectionDirection = refraction;
//    float k = dot(view, reflectionDirection);
//    k = max(k, 0.0);
//    k = pow(k, 15.0);
//    col = max(0.0, g) * 0.1 * c1 + (k + 0.1) * c2;
//
//    //col = vec3(octave01((refraction + position).xy * 0.1, 4));
//    //col = vec3(refraction);
//    //col = lightDir;
//    //col = vec3(k);
//    //col = (view + 1.0) / 2.0;
//    //col = mix(c1, c2, g);
//    //col = vec3(d);
//    //col = vec3(p - mr, 0.0);
//
//    fragColor = vec4(col, 1.0);
//}
//