#version 430 core

in vec2 worldPosition; 

in mat3x2 clipToWorld; 
in float time; 
out vec4 fragColor;

/*generated end*/

#define ANIMATE
float t = time / 100.0;

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
//    if (v > 1.0) {
//        return 0.0;
//    }
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

void main()
{
    vec2 p = worldPosition / 1800.0;

    vec3 c = voronoi( 8.0*p );

	// isolines
    vec3 col = vec3(0.0);
    //c.x*(0.5 + 0.5*sin(64.0*c.x))*vec3(1.0);
    // borders	
    //col = mix(vec3(1, 1, 1), col, smoothstep( 0.005, 0.05, c.x ) );


//    c.x *= 5.0;
//    col = vec3(1.0 / (c.x * c.x)) / 500.0;
//    //col = vec3(1.0 / (c.x)) / 60.0;
//    col = clamp(col, vec3(0.0), vec3(1.0));
//    col /= 3.0;

    //vec3(
    //col = vec3(c.x * (c.y));
    //col = vec3(c.x - 0.05) * 0.5;
    //col = vec3(c.x * c.y);
    // feature points   
//	float dd = length( c.yz );
//	col = mix( vec3(1.0,0.6,0.1), col, smoothstep( 0.0, 0.12, dd) );
//	col += vec3(1.0,0.6,0.1)*(1.0-smoothstep( 0.0, 0.04, dd));

    float d = voronoi(p * 8.0).x;

    float g = dot(getNormal(p * 8.0), normalize(vec3(1.0, 1.0, 0.0)));
    col = vec3(g) * 0.1;

    col = mix(col + vec3(0.05), vec3(0.0), smoothstep(0.19, 0.195, d));
//    if (d > 0.2) {
//        col = vec3(0.0);
//    }
    //col = vec3(d);

	fragColor = vec4(col,1.0);
}

//vec2 random2( vec2 p ) {
//    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
//}
//
//void main() {
//    float t = time / 20.0;
//    vec2 st = worldPosition / 400.0;
//    //st.x *= u_resolution.x/u_resolution.y;
//    vec3 color = vec3(.0);
//
//    // Scale
//    st *= 3.;
//
//    // Tile the space
//    vec2 i_st = floor(st);
//    vec2 f_st = fract(st);
//
//    float m_dist = 1.;  // minimum distance
//
//    for (int y= -1; y <= 1; y++) {
//        for (int x= -1; x <= 1; x++) {
//            // Neighbor place in the grid
//            vec2 neighbor = vec2(float(x),float(y));
//
//            // Random position from current + neighbor place in the grid
//            vec2 point = random2(i_st + neighbor);
//
//			// Animate the point
//            point = 0.5 + 0.5*sin(t + 6.2831*point);
//
//			// Vector between the pixel and the point
//            vec2 diff = neighbor + point - f_st;
//
//            // Distance to the point
//            float dist = length(diff);
//
//            // Keep the closer distance
//            m_dist = min(m_dist, dist);
//        }
//    }
//
//    // Draw the min distance (distance field)
//    //m_dist = smoothstep(0.9, 1.0, m_dist);
//    color += m_dist;
////    if (m_dist > 0.3) {
////        color = vec3(1.0);
////    }
//
//    // Draw cell center
//    //color += 1.-step(.02, m_dist);
//
//    // Draw grid
//    //color.r += step(.98, f_st.x) + step(.98, f_st.y);
//
//    // Show isolines
//    // color -= step(.7,abs(sin(27.0*m_dist)))*.5;
//
//    fragColor = vec4(color,1.0);
//
//	//fragColor = vec4(worldPosition, 0.0, 1.0);
//	//fragColor = vec4(1, 0, 0, 1);
//}
//