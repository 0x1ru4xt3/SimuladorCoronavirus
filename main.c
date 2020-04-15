#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ESCHEIGHT 50
#define ESCWIDTH  50
#define POBLACION 50
#define RADIO     5

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

	// PROGRAMA 
	while(1) {
		muertosRonda = 0;
		repuestas = 0;

		// PARA CADA PERSONA
		for(i=0; i<POBLACION; i++){
			// MOVER PERSONA

			// COMPROBAR RADIO DE CONTAGIO Y (si esta contagiado) CONTAGIAR Y CAMBIAR COLOR

			// DECIDIR SI SE MUERE
			muertosRonda += 1;
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
