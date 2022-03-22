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

//m es el brillo del materia (theMaterial.shininess).
float specular_factor( const vec3 n, const vec3 l, const vec3 v, float m, int i){
	vec3 r= normalize(2*dot(n,l)*n-l); //Aplicamos fórmula y normalizamos.

	//Factor especular= (n*l)max(0,(r*v)^m)m*i.
	float base= dot(r,v) //Calculamos la base de la potencia.
	//Comprobamos que la base de la potencia no es 0.
	if (base>0.0){ 
		//Aplicamos la fórmula.
		float factor_especular=dot(n,l)*max(0,pow(base,m)*m*theLights[i].specular;
	}
	return factor_especular;
}

void aporte_direccional(in int i, in vec3 l, in vec3 n, in vec3 v, inout vec3 acumulador_difuso, inout vec3 acumulador_especular){
	//Calcular Lambert
	float NoL= lambert_factor(n,l);
	if(NoL>0.0){
		acumulador_difuso= acumulador_difuso+NoL*theLights[i].diffuse;
		acumulador_especular= acumulador_especular+ specular_factor(n,l,v,theMaterial.shininess,i);
	}
	
}
	
void aporte_posicional(in int i, in vec3 l, in vec3 n, in vec3 v, in float d, inout vec3 acumulador_difuso, inout vec3 acumulador_especular){
	if(length(l)>0.0){
		float NoL= lambert_factor(n,l);
		if (NoL>0.0){
			float fdist= theLights[i].attenuation[0]+theLights[i].attenuation[1]*d+theLights[i].attenuation[2]*d*d; //Calculamos el denominador.
			if(fdist>0.0){ //Si el denominador no es 0.
				fdist=1/fdist; //Hacemos la división.
				acumulador_difuso= acumulador_difuso+NoL*theMaterial.diffuse*theLights[i].diffuse*fdist;
			}
			
		}
	}
}

//No mezclar colores con posiciones.
void main() {
	//n es el vector normal.
    //l es el vector de la luz.
	//v es el vector que va a la cámara.
	vec3 L,N,V;
	vec3 acumulador_difuso;
	acumulador_difuso=vec3(0.0,0.0,0.0);
	vec3 acumulador_especular;
	acumulador_especular=vec3(0.0,0.0,0.0);

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
			aporte_direccional(i,L,N,V,acumulador_difuso,acumulador_especular);
		
		}
		//Luz posicional.
		else{
			L= theLights[i].position.xyz-positionEye; // Del vértice a la luz
			float d= length(L); //distancia euclídea.
			L= normalize(L); //normalizamos L.
			aporte_posicional(i,L,N,V,d,acumulador_difuso,acumulador_especular);
		}

	}
	f_color=vec4(0.0,0.0,0.0, 1.0); //Canales rojo, azul, verde y opacidad.
	f_color.rgb= acumulador_difuso;
	//Coordenadas de textura que se pasan del vertex-shader al fragment shader
	f_texCoord= v_texCoord;
	gl_Position = modelToClipMatrix * vec4(v_position, 1);
}
//Posición(1,1,1,1)-> Modelview*Posición(1,1,1,1) hay que normalizar.
