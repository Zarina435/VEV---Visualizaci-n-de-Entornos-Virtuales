#version 120

varying vec4 f_color;
varying vec2 f_texCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform float uCloudOffset; // The offset of the cloud texture

void main() {

	// The final color must be a linear combination of both
	// textures with a factor of 0.5, e.g:
	//
	vec4 texColor0 = texture2D(texture0, f_texCoord);
	vec4 texColor1 = texture2D(texture1, f_texCoord);
	vec4 color= 0.5 * texColor0 + 0.5 * texColor1;

	gl_FragColor = color;
}
