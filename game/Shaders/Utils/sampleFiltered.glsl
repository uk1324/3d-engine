// detail - Frequency of sampling. Not calling the variable frequency, because the value can be clamped.
// Technically if you want the sample count to be as close at it can to area covered then detail should be set to the amount of texels per one unity of uv (i think not sure). Most of the time this doesn't make sense because procedurally generated textures have infinite precision. So it's best to just set it to something that looks good.
#define GENERATE_SAMPLE_FILTERED(functionName, sampleFunctionName) \
    vec3 functionName(vec2 uv, vec2 dFdxUv, vec2 dFdyUv, float detail) { \
        const int maxSamplesPerAxis = 10; \
        \
        /* +1 because dFd* gives the offset to the next pixel and that shouldn't influence this pixel. */ \
        /* The loops have a '<' so the values of the offset never reaches uv + dFd*Uv. */ \
	    int sx = 1 + int(clamp(detail * length(dFdxUv), 0.0, float(maxSamplesPerAxis - 1))); \
        int sy = 1 + int(clamp(detail * length(dFdyUv), 0.0, float(maxSamplesPerAxis - 1))); \
        \
	    vec3 accumulated = vec3(0.0); \
        \
        for (int y = 0; y < sy; y++) { \
		    for (int x = 0; x < sx; x++)  { \
                /* Not sure it it's better to offset this by 0.5 or not. */ \
			    vec2 st = vec2(float(x), float(y)) / vec2(float(sx), float(sy)); \
			    accumulated += sampleFunctionName(uv + st.x * dFdxUv + st.y * dFdyUv); \
		    } \
	    } \
        \
	    return accumulated / float(sx * sy); \
    }