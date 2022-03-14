#version 120

uniform mat4 modelToCameraMatrix;
uniform mat4 cameraToClipMatrix;
uniform mat4 modelToWorldMatrix;
uniform mat4 modelToClipMatrix;

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient;  // rgb

uniform struct light_t {
	vec4 position;    // Camera space
	vec3 diffuse;     // rgb
	vec3 specular;    // rgb
	vec3 attenuation; // (constant, lineal, quadratic)
	vec3 spotDir;     // Camera space
	float cosCutOff;  // cutOff cosine
	float exponent;
} theLights[4];     // MG_MAX_LIGHTS

uniform struct material_t {
	vec3  diffuse;
	vec3  specular;
	float alpha;
	float shininess;
} theMaterial;

attribute vec3 v_position; // Model space
attribute vec3 v_normal;   // Model space
attribute vec2 v_texCoord;

varying vec4 f_color;
varying vec2 f_texCoord;


void main() {
	vec3 acumulador_difuso;
	acumulador_difuso=vec3(0.0,0.0,0.0);
	for (int i=0; i<active_lights_n;++i){
		//acumulador difuso= acumulador difuso+Calculo del color. Color difuso del material*color difuso de la luz i-esima*irradiancia de la luz i-esima
		acumulador_difuso= acumulador_difuso+theMaterial.diffuse*theLights[i].diffuse*irradiancia_de_la_luz(i)
	}
	f_color
	//Coordenadas de textura que se pasan del vertex-shader al fragment shader
	f_texCoord= v_texCoord;
	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}
