
vec3 posFromDepth(vec2 screenTexturePos /* (0, 0) to (1, 1) */, float depth, mat4 transformInverse) {
	vec4 posNdc = vec4(
		screenTexturePos * 2.0 - 1.0 /* screen texture position to NDC(<0, 1> to <-1, 1>) */, 
		depth * 2.0 - 1.0, /* depth from <0, 1> to <-1, 1> */
		1.0);
	vec4 posWorld = transformInverse * posNdc;
	return posWorld.xyz / posWorld.w;
}