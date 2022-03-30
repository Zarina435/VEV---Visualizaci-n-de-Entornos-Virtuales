#version 120

uniform int active_lights_n; // Number of active lights (< MG_MAX_LIGHT)
uniform vec3 scene_ambient; // Scene ambient light

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

uniform sampler2D texture0;

varying vec3 f_position;      // camera space
varying vec3 f_viewDirection; // camera space
varying vec3 f_normal;        // camera space
varying vec2 f_texCoord;

float lambert_factor(vec3 n, vec3 l){
	return max(dot(n,l),0.0);
}

//m es el brillo del materia (theMaterial.shininess).
float specular_factor( const vec3 n, const vec3 l, const vec3 v, float m){
	vec3 r= normalize(2*dot(n,l)*n-l); //Aplicamos fórmula y normalizamos.

	//Factor especular= (n*l)max(0,(r*v)^m)m*i.
	float factor_especular=0.0;
	float base= dot(r,v); //Calculamos la base de la potencia.
	//Comprobamos que la base de la potencia no es 0.
	if (base>0.0){ 
		//Aplicamos la fórmula.
		 factor_especular=pow(base,m);
	}
	return factor_especular;
}

void aporte_direccional(in int i, in vec3 l, in vec3 n, in vec3 v, inout vec3 acumulador_difuso, inout vec3 acumulador_especular){
	//Calcular Lambert
	float NoL= lambert_factor(n,l);
	if(NoL>0.0){
		acumulador_difuso= acumulador_difuso+NoL*theLights[i].diffuse*theMaterial.diffuse;

		float especular= specular_factor(n,l,v,theMaterial.shininess);
		acumulador_especular= acumulador_especular+NoL*especular*theMaterial.specular*theLights[i].specular;
	}
	
}

void aporte_posicional(in int i, in vec3 l, in vec3 n, in vec3 v, in float d, inout vec3 acumulador_difuso, inout vec3 acumulador_especular){
	if(d>0.0){
		float NoL= lambert_factor(n,l);
		if (NoL>0.0){
			//Calculamos la atenuación. Primero el denominador de la fracción.
			float fdist= theLights[i].attenuation[0]+theLights[i].attenuation[1]*d+theLights[i].attenuation[2]*d*d; //Calculamos el denominador.
			//Comprobamos que el denominador no sea 0.
			if(fdist>0.0){ //Si el denominador no es 0.
				//Terminamos de calcular la atenuación.
				fdist=1/fdist; //Hacemos la división.
				acumulador_difuso= acumulador_difuso+(NoL*theMaterial.diffuse*theLights[i].diffuse*fdist);

				float especular= specular_factor(n,l,v,theMaterial.shininess);
				acumulador_especular= acumulador_especular+NoL*especular*theMaterial.specular*theLights[i].specular*fdist;
			}
			
		}
	}
}

void aporte_spot(in int i, in vec3 l, in vec3 n, in vec3 v, inout vec3 acumulador_difuso, inout vec3 acumulador_especular){


	vec3 direccion= normalize(theLights[i].spotDir); //Dirección de la luz.
	float cos= dot(direccion,-l); //Calculamos el coseno entre la dirección de la luz y el vector de la luz.
	float cspot=0.0;

	//Si el coseno del cutOff es mayor, es que el ángulo del cosCutOff es menor y está dentro del cono.
	if(cos>theLights[i].cosCutOff){ //dentro del cono.

		if(cos>0.0){ //La base de la potencia no es 0.

			float NoL=lambert_factor(n,l);
			if (NoL>0.0){
				cspot= pow(cos, theLights[i].exponent); //Aplicamos la fórmula.
				float especular= specular_factor(n,l,v,theMaterial.shininess);

				acumulador_difuso= acumulador_difuso+ NoL*theMaterial.diffuse*theLights[i].diffuse*cspot;
				acumulador_especular= acumulador_especular+ NoL*especular*theMaterial.specular*theLights[i].specular*cspot;

			}
		}
	}

	
}

void main() {

	//n es el vector normal.
    //l es el vector de la luz.
	//v es el vector que va a la cámara.

	vec3 L,N,V;
	vec3 acumulador_difuso;
	acumulador_difuso=vec3(0.0,0.0,0.0);
	vec3 acumulador_especular;
	acumulador_especular=vec3(0.0,0.0,0.0);

	N=normalize(f_normal); //normal en el sistema de la cámara y normalizada.
	v=normalize(f_viewDirection); // Vector que va desde el vértice a la cámara.
	


	for (int i=0; i<active_lights_n;++i){
		
		//Luz direccional.
		if (theLights[i].position.w==0.0){
			//Vector de la luz direccional
			L= normalize(-1.0*theLights[i].position.xyz);
			aporte_direccional(i,L,N,V,acumulador_difuso,acumulador_especular);
		
		}
		else{
			L= theLights[i].position.xyz-positionEye; // Del vértice a la luz
			float d= length(L); //distancia euclídea.
			L= normalize(L); //normalizamos L.
			//Luz posicional.
			if(theLights[i].cosCutOff==0){
				aporte_posicional(i,L,N,V,d,acumulador_difuso,acumulador_especular);
			}
			//Spotlight
			else{
				aporte_spot(i,L,N,V,acumulador_difuso,acumulador_especular);
			}
			
		}

	}
	gl_FragColor.rgb= scene_ambient+ acumulador_difuso+ acumulador_especular;
	gl_FragColor.a=1.0;

	vec4 texColor = texture2D(texture0, f_texCoord);
	gl_FragColor *= texColor;
	
}
