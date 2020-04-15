#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ESCHEIGHT 50
#define ESCWIDTH  50
#define POBLACION 50
#define RADIO     5

// PROBABILIDADES POR EDAD
#define EDAD1 0.004  // < 50
#define EDAD2 0.013  // 50 - 60
#define EDAD3 0.036  // 60 - 70
#define EDAD4 0.08   // 70 - 80
#define EDAD5 0.148  // > 80

// CALCULO DE PROBABILIDADES (shorturl.at/abILW)
float distrNormal(float v1, float v2, float sigma, float mi){
	return cos(2*3.14*v2)*sqrt(-2.*log(v1))*sigma + mi;	
}

// FUNCION DE PROGRAMA PRINCIPAL
int main(int argc, char** argv) {
	// OBJETO PERSONA
	struct persona {
		int edad;
		int estado;
		float probMuerte;
		int pos[2];
		int vel[2];
	};

	// INICIALIZACIONES
	persona personas[POBLACION];
	int pobActual = POBLACION;
	int muertosRonda, repuestas, mediaEdad;
	int diasTranscurridos = 0;
	int i, e;

	// CREAR POBLACION
	for(i=0; i<POBLACION; i++){
		personas[i].edad = ;
		personas[i].estado = 0;
		personas[i].probMuerte = ;
		personas[i].pos[0] = ;
		personas[i].pos[1] = ;
		personas[i].vel[0] = ;
		personas[i].vel[1] = ;
	}
	
	// PRIMER INFECTADO!
	personas[rand() % POBLACION].estado = 1;

	// BUCLE PRINCIPAL 
	while(1) {
		muertosRonda = 0;
		repuestas = 0;

		// PARA CADA PERSONA
		for(i=0; i<POBLACION; i++){
			// MOVER PERSONA

			// COMPROBAR SI ESTA CONTAGIADO: SI NO, COMPROBAR RADIO DE CONTAGIO Y (si es el caso) CONTAGIAR Y CAMBIAR COLOR
			if(personas[i].estado == 1 || personas[i].estado ==2){
				
			}

			// DECIDIR SI SE MUERE
			if(es el caso) muertosRonda += 1;
		}		

		// REPONER PERSONAS
		repuestas = ;

		// ACTUALIZAR LENGTH ARRAY (pobActual - muertos + personas repuestas)
		pobActual = ;

		// RULAR TIEMPO
		diasTranscurridos = ;

		// MEDIA DE EDAD DE LAS PERSONAS
		mediaEdad = 0;
		for(i=0; i<POBLACION; i++){
			mediaEdad += personas[i].edad;
		}
		mediaEdad = mediaEdad/POBLACION;
	}
}
