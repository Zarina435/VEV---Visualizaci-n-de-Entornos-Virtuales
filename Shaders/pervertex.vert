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

float lambert_factor(vec3 n, vec3 l){
	return max(dot(n,l),0.0);
}

void aporte_direccional(in int i, in vec3 l, in vec3 n, inout vec3 acumulador_difuso){
	//Calcular Lambert
	float NoL= lambert_factor(n,l);
	if(NoL>0.0){
		acumulador_difuso= acumulador_difuso+NoL*theLights[i].diffuse;
	}
}
	
void aporte_posicional(in int i, in vec3 l, in vec3 n, inout vec3 acumulador_difuso){
	if(length(l)>0.0){
		float NoL= lambert_factor(n,l);
		if (NoL>0.0){
			acumulador_difuso= acumulador_difuso+NoL*theMaterial.diffuse*theLights[i].diffuse;
		}
	}
}

//No mezclar colores con posiciones.
void main() {
	vec3 L,N,V;
	vec3 acumulador_difuso;
	acumulador_difuso=vec3(0.0,0.0,0.0);

	vec4 N4=modelToCameraMatrix*vec4(v_normal, 0.0); //Vector del vértice en el s.c. cámara. En homogéneas.
	N=normalize(N4.xyz); //Vector normal del vértice en el s.c.cámara. Normalizamos el vector3, cogemos solo las coordenadas x, y, z.

	vec4 positionEye4= modelToCameraMatrix*vec4(v_normal,1); //Posición del vértice en el s.c cámara. En homogéneas.
	vec3 positionEye= positionEye4.xyz; //Para coger las coordenadas x, y, z.

	vec3 V4= (0,0,0,1)-positionEye; //Vector que va del vértice a la cámara. Cámara-posición del vértice en el s.c. cámara. EN homogéneas.
	V=normalize(V4.xyz);  //Normalizamos el vector, cogiendo las coordenadas x, y, z.
	for (int i=0; i<active_lights_n;++i){
		
		//Luz direccional.
		if (theLights[i].position.w==0.0){
			//Vector de la luz direccional
			L= normalize(-1.0*theLights[i].position.xyz);
			aporte_direccional(i,L,N,acumulador_difuso);
		
		}
		//Luz posicional.
		else{
			L= normalize(theLights[i].position.xyz-positionEye); // Del vértice a la luz
			aporte_posicional(i,L,N,acumulador_difuso);
		}

	}
	f_color=vec4(0.0,0.0,0.0, 1.0); //Canales rojo, azul, verde y opacidad.
	f_color.rgb= acumulador_difuso;
	//Coordenadas de textura que se pasan del vertex-shader al fragment shader
	f_texCoord= v_texCoord;
	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}
