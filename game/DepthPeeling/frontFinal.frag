#version 430 core
out vec4 fragColor;

/*generated end*/

in vec3 normal;

uniform samplerRect ColorTex;
uniform vec3 BackgroundColor;

void main() {
	vec4 frontColor = texture(ColorTex, gl_FragCoord.xy);
	//fragColor.rgb = (frontColor + vec4(BackgroundColor * frontColor.a, 1)).rgb;
	//if (frontColor.a == 1) {
	//	discard;
	//}
	fragColor = frontColor;
	//fragColor = vec4(vec3(frontColor.a), 1);
}
