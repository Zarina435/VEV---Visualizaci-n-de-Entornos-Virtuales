#version 120

uniform mat4 modelToCameraMatrix; //modelview
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

//Atributo: Geometría del vértice.
attribute vec3 v_position; // Model space
attribute vec3 v_normal;   // Model space
attribute vec2 v_texCoord; //Coordenadas de textura del vértice.

//Pasan de vertex shader a fragment shader (no se puede escribir, solo leer).
varying vec4 f_color;
varying vec2 f_texCoord;

float Lambert_factor(vec3 n, vec3 l){
	return max(dot(n,l),0.0);
}

void aporte_direccional(in int i, in vec3 l, in vec3 n, inout vec3 acumulador){
	//Calcular Lambert
	float NoL= Lambert_factor(n,l);
	if(NoL>0.0){
		acumulador= acumulador+NoL+theMaterial.diffuse*theLights[i].diffuse;
	}
}
	

//No mezclar colores con posiciones.
void main() {
	vec3 L,N;
	vec3 acumulador_difuso;

	acumulador_difuso=vec3(0.0,0.0,0.0);

	//Normal del vértice en el espacio de la cámara.
	vec4 N4=modelToCameraMatrix*vec4(v_normal, 0.0);
	N=normalize(N4.xyz); //en el s.c.camara
	for (int i=0; i<active_lights_n;++i){
		//acumulador difuso= acumulador difuso+Calculo del color. Color difuso del material*color difuso de la luz i-esima*irradiancia de la luz i-esima
		//LUz direccional?? (x,y,z,0.0)
		
		if (theLights[i].position.w==0.0){
			//Vector de la luz direccional
			L= normalize(-1.0*theLights[i].position.xyz);

			aporte_direccional(i,L,N,acumulador_difuso);
		
		}

	}
	f_color=vec4(0.0,0.0,0.0, 1.0); //Canales rojo, azul, verde y opacidad.
	f_color.rgb= acumulador_difuso;
	//Coordenadas de textura que se pasan del vertex-shader al fragment shader
	f_texCoord= v_texCoord;
	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}
