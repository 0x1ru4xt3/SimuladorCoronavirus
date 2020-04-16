#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


#define SEED 0
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
	srand(seed);

	// INICIALIZACIONES
	//persona personas[POBLACION];
	//persona contagios[POBLACION]
	persona *personas; //PARA GUARDAR TODAS LAS PERSONAS,SANOTES,ENFERMOS Y PALMADOS
	persona *contagios; //PARA GUARDAR A LOS CONTAGIADOS
	
	personas=malloc(POBLACION*sizeof(persona));
	contagios=malloc(POBLACION*sizeof(persona));

	int pobActual = POBLACION;
	int muertosRonda, repuestas, mediaEdad,contagiados;
	int diasTranscurridos = 0;
	int i, e,j;

	if(argc!=2){
		fprintf(stderr,"No has metido bien el argumento");
		exit(1);
	}
	int tiempo=atoi(argv[1]);//Le pasamos cuantos dias queremos por el primer parametro.
	// CREAR POBLACION
	for(i=0; i<POBLACION; i++){
		personas[i].edad = rand()%100;
		mediaedad+=mediaedad+personas[i].edad;
		personas[i].estado = 0; //5 estados: 0-Perfectamente, 1-Contagiado pero sin sintomas, 2-Contagiado con sintomas, 3-Recuperado 4-Muerto
		if(persona[i].edad<50){ //Inicializacion de las probabilidades de muerte dependiendo de la edad
			personas[i].probMuerte = EDAD1;
		}else if(persona[i].edad>=50 && persona[i].edad<60){
			personas[i].probMuerte = EDAD2;
		}else if(persona[i].edad>=60 && persona[i].edad<70){
			personas[i].probMuerte = EDAD3;
		}else if(persona[i].edad>=80 && persona[i].edad<80){
			personas[i].probMuerte = EDAD4;
		}else {
			personas[i].probMuerte = EDAD5;
		}
		personas[i].pos[0] = rand()%ESCHEIGHT; //Calculo de la posicion inicial.
		personas[i].pos[1] = rand()%ESCWIDTH;
		personas[i].vel[0] = rand()%5;//Solo se podra mover por lo menos 5 posiciones para que no sea usain bolt
		personas[i].vel[1] = rand()%5;
	}
	mediaedad=mediaedad/POBLACION;
	// PRIMER INFECTADO!
	int aux=rang()%POBLACION;
	personas[aux].estado = 1;
	contagios[contagiados]=persona[aux];
	contagiados++;
	// BUCLE PRINCIPAL 
	while(diastranscurridos<=tiempo) {
		muertosRonda = 0;
		repuestas = 0;

		// Mover a las personas
		for(i=0; i<POBLACION; i++){
			// MOVER PERSONA
			if(personas[i].pos[0]+personas[i].vel[0]>ESCHEIGHT){
				personas[i].pos[0]=ESCHEIGHT; //Ha llegado al limite
			}else{
				personas[i].pos[0]=personas[i].pos[0]+personas[i].vel[0]; //Movimiento en uno de los ejes
			}
			if(personas[i].pos[1]+personas[i].vel[1]>ESCWIDTH){
				personas[i].pos[0]=ESCWIDTH; //Ha llegado al limite.
			}else{
				personas[i].pos[1]=personas[i].pos[1]+personas[i].vel[1]; //Movimiento en el otro eje
			}
			personas[i].vel[0] = rand()%5;//Solo se podra mover por lo menos 5 posiciones para que no sea usain bolt
			personas[i].vel[1] = rand()%5;//Solo se podra mover por lo menos 5 posiciones para que no sea usain bolt
			}
		}
		//Cogemos a los infectados y miramos si alguno esta cerca.
		for(i=0; i<contagiados; i++){
			rangox=contagios[i].pos[0];
			rangoy=contagios[i].pos[1];
			for(j=0;j<POBLACION;j++){
				if(personas[i].estado==0 && personas[i].pos[0]<=rangox+RADIO&&personas[i].pos[0]>=rangox-RADIO){ //ESTA dentro del eje x.
					if(personas[i].pos[1]<=rangoy+RADIO&&personas[i].pos[1]>=rangoy+RADIO){//Esta dentro del rango de Y.
						//Habra que calcular la probabilidad de que se infecte (la normal?).
						personas[i].estado=1;
						contagios[contagiados]=personas[i];
						contagiados++;
					}
			}
			// DECIDIR SI SE MUERE
			if(es el caso) muertosRonda += 1;
		}
		// REPONER PERSONAS
		repuestas = ;

		// ACTUALIZAR LENGTH ARRAY (pobActual - muertos + personas repuestas)
		pobActual = ;

		// RULAR TIEMPO
		diasTranscurridos++;
	}
	free(personas);
	free(contagios);
	free(sanos);
}
