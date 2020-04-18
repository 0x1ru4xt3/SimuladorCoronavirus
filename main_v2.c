#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SEED      0
#define ESCHEIGHT 50
#define ESCWIDTH  50
#define POBLACION 50
#define RADIO     5

// PROBABILIDADES POR EDAD
#define EDAD1 0.004  // < 50
#define EDAD2 0.013  // 50 - 60
#define EDAD3 0.036  // 60 - 70
#define EDAD4 0.080  // 70 - 80
#define EDAD5 0.148  // > 80

// OBJETO PERSONA
struct persona {
	int edad;
	int estado;
	int diasContaminado;
	float probMuerte;
	int pos[2];
	int vel[2];
};

// CALCULO DE PROBABILIDADES (shorturl.at/abILW)
float distrNormal(float v1, float v2, float sigma, float mi){
	return cos(2*3.14*v2)*sqrt(-2.*log(v1))*sigma + mi;
}

// CREAR PERSONA
persona crearPersona(){
	persona per;
	per.edad = ; // Generar por probabilidad
	per.estado = 0;
	per.diasContaminado = 0;

	// PROBABILIDAD DE MUERTE EN BASE A EDAD
	if(per.edad<50)
		per.probMuerte = EDAD1;
	else if(persona[i].edad>=50 && persona[i].edad<60)
		per.probMuerte = EDAD2;
	else if(persona[i].edad>=60 && persona[i].edad<70)
		per.probMuerte = EDAD3;
	else if(persona[i].edad>=80 && persona[i].edad<80)
		per.probMuerte = EDAD4;
	else
		per.probMuerte = EDAD5;

	//CALCULO DE LA POSICION y VELOCIDAD INICIAL
	per.pos[0] = rand()%ESCHEIGHT;
	per.pos[1] = rand()%ESCWIDTH;
	per.vel[0] = rand()%10+(-5);
	per.vel[1] = rand()%10+(-5);

	return per;
}

// CALCULAR LA MEDIA DE EDAD
int mediaEdad(persona *per, int pobl){
	int i;
	int media = 0;
	for(i=0; i<pobl; i++)
		media += per[i].edad;

	return (media/pobl);
}

// FUNCION DE PROGRAMA PRINCIPAL
int main(int argc, char** argv) {
	srand(seed);

	// INICIALIZACIONES
	persona *personas;
	personas  = malloc(POBLACION*sizeof(persona));

	int pobActual = POBLACION;
	int muertosRonda, curadosRonda, repuestas, mediaEdad, contagiados;
	int diasTranscurridos = 0;
	int muertosTotales = 0;
	int curadosTotales = 0;
	int contagiadosTotales = 0;
	int i, e, j;

	if(argc!=2){
		fprintf(stderr,"%s <tiempoASimular>\n", argv[0]);
		exit(1);
	}

	int tiempo = atoi(argv[1]);

	// CREAR POBLACION
	for(i=0; i<POBLACION; i++)
		personas[i] = crearPersona();
	mediaEdad = mediaEdad(personas, POBLACION);

	// PRIMER INFECTADO!
	int aux = rang()%POBLACION;
	personas[aux].estado = 1;
	contagiadosTotales++;

	// BUCLE PRINCIPAL
	while(diasTranscurridos <= tiempo) {
		muertosRonda = 0;
		curadosRonda = 0;
		contagiadosRonda = 0;
		repuestas = 0;

		// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
		for(i=0; i<pobActual; i++){
			if(personas[i].pos[0] + personas[i].vel[0] >= ESCHEIGHT){
				personas[i].pos[0] = ESCHEIGHT;	// Ha llegado al limite
				personas[i].vel[0] = rand()%5+(-5);
			} else if(personas[i].pos[0] + personas[i].vel[0] <= 0){
				personas[i].pos[0] = 0;			// Ha llegado al limite
				personas[i].vel[0] = rand()%5;
			} else {
				personas[i].pos[0] += personas[i].vel[0]; //Movimiento en uno de los ejes
				personas[i].vel[0] = rand()%10+(-5);
			}
   
			if(personas[i].pos[1] + personas[i].vel[1] >= ESCWIDTH){
				personas[i].pos[1] = ESCWIDTH; 	// Ha llegado al limite.
				personas[i].vel[1] = rand()%5+(-5);
			} else if(personas[i].pos[0] + personas[i].vel[0] <= 0){
				personas[i].pos[1] = 0;			// Ha llegado al limite.
				personas[i].vel[1] = rand()%5;
			} else {
				personas[i].pos[1] += personas[i].vel[1]; //Movimiento en el otro eje
				personas[i].vel[1] = rand()%10+(-5);
			}
        }

		// INFECTADOS: COMPROBAR RADIO DE CONTAGIOS y DECISIONES DE MUERTE o SUPERVIVENCIA
        for(i=0; i<pobActual; i++){
            if(personas[i].estado == 1 || personas[i].estado == 2){ 
			rangox = personas[i].pos[0];
			rangoy = personas[i].pos[1];
	
			for(e=0; e<pobActual; e++){
				// SI NO ESTA INFECTADO y NO LO HA ESTADO
				if(personas[e].estado == 0){
					// SI ESTA DENTRO DEL RANGO DE EJE X
					if(personas[e].pos[0] <= rangox+RADIO && personas[e].pos[0] >= rangox-RADIO{
						// SI ESTA DENTRO DEL RANGO DE EJE Y
						if(personas[e].pos[1] <= rangoy+RADIO && personas[e].pos[1] >= rangoy+RADIO){
							//Habra que calcular la probabilidad de que se infecte (la normal?).
							personas[e].estado = 1;
							contagiadosRonda++;
						}
					}
				}   
			}
            
    
			// DECIDIR SI SE MUERE O SE RECUPERA
			if(es el caso){
				for(e=i; e<pobActual-1; e++)
					personas[e] = personas[e+1];
				muertosRonda++;
				infectadosTotales--;
				pobActual--;
			} else {
				diasContaminado++;
				if(personas[i].estado == 1 && diasContaminado >= 15){
					personas[i].estado = 2;
				} else if(personas[i].estado == 2 && diasContaminado >= 30){
					personas[i].estado = 3;
					curadosRonda++;
					infectadosTotales--;
				}
			}
        }

		// REPONER PERSONAS
		repuestas = rand() % (POBLACION - pobActual);
		for(i=0; i<repuestas; i++)
			personas[pobActual+i] = crearPersona();

		// ACTUALIZAR LENGTH ARRAY
		pobActual = pobActual + repuestas;

		// ACTUALIZAR EDAD MEDIA
		mediaEdad = mediaEdad(personas, pobActual);

		// RULAR TIEMPO
		diasTranscurridos++;

        // ACTUALIZAR VALORES TOTALES
		contagiadosTotales += contagiadosRonda;
		curadosTotales += curadosRonda;
        muertosTotales += muertosRonda;

        // VISUALIZAR PROGRESO
        printf("EN %i DIAS: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS), %i NUEVAS PERSONAS. POBLACION: %i, EDAD MEDIA: %i\n",
                diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, repuestas, pobActual, mediaEdad);
	}

	// LIBERAR MEMORIA AL ACABAR PROGRAMA
	free(personas);
}
