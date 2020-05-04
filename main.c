//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "persona.h"

#define SEED 0

// CALCULAR LA MEDIA DE EDAD
// (Par: struct persona, int poblacion actual)
int mediaEdad(struct persona *per, int pobl){
	int i;
	int media = 0;
	for(i=0; i<pobl; i++){
		media += per[i].edad;
	}
	return (media/pobl);
}

// FUNCION DE PROGRAMA PRINCIPAL
int main(int argc, char** argv) {
	if(argc!=9) {
		fprintf(stderr,"Funcionamiento: %s <tiempoASimular> <tamanoAncho> <tamanoAlto> <radio> <probRadio> <poblacion> <edadMedia> <batch>\n", argv[0]);
		exit(1);
	}

	int TIEMPO 		= atoi(argv[1]);
	int ESCHEIGHT 	= atoi(argv[2]);
	int ESCWIDTH 	= atoi(argv[3]);
	int RADIO 		= atoi(argv[4]);
	float PROBRADIO = atof(argv[5]);
	int POBLACION 	= atoi(argv[6]);
	int EDADMEDIA 	= atoi(argv[7]);
	int BATX 		= atoi(argv[8]);

	if (PROBRADIO > 0.9 || PROBRADIO < 0 || TIEMPO < BATX || TIEMPO < 1 || RADIO >= ESCWIDTH || RADIO >= ESCHEIGHT) {
        fprintf(stderr,"Error de parámetros: \n\t- La probabilidad de contagio debe estar comprendido entre 0 y 1.\n\t- El tiempo a simular debe ser mayor que 1.\n\t- El batch no puede ser mayor que el tiempo a simular.\n\t- El radio de contagio debe ser menor que el tamaino del lienzo.\n");
		exit(1);
	}

	srand(SEED);

	// INICIALIZACIONES VARIABLES
    printf("STATUS: Inicializando variables...\n");
	int i, e, j;
   	int rangox, rangoy;
	int muertosRonda, curadosRonda, contagiadosRonda;
	int muertosTotales = 0;
	int curadosTotales = 0;
	int contagiadosTotales = 0;
	int diasTranscurridos = 0;
	int pobActual = POBLACION;
	int edadMedia = EDADMEDIA;

	// INICIALIZACION FICHEROS
	FILE *dias, *posic;
	posic = fopen("historialposic.txt","w+");
	dias  = fopen("historialdias.txt", "w+");

	// INICIALIZACION ARRAY PERSONAS
	struct persona *personas;
    personas  = malloc(POBLACION*sizeof(struct persona));

	// IMPRESION DE VARIABLES INTRODUCIDAS POR PARAMETRO
	printf("STATUS: DATOS INTRODUCIDOS: \n\tTIEMPO %d\n\tPOBLACION: %d\n\tANCHO ESC: %d  ALTO_ESC: %d\n\tRADIO CONTAGIO: %d  PROB DE CONTAGIO RADIO: %f\n",
			TIEMPO, POBLACION, ESCHEIGHT, ESCWIDTH, RADIO, PROBRADIO);

	// CREAR POBLACION
	printf("STATUS: Creando población...\n");
	for(i=0; i<POBLACION; i++)
		personas[i] = crearPersona(EDADMEDIA, ESCWIDTH, ESCHEIGHT);

	// PRIMER INFECTADO!
	printf("STATUS: PRIMER INFECTADO!\n");
	int aux = rand()%POBLACION;
	personas[aux].estado = 1;
	contagiadosTotales++;

	// BUCLE PRINCIPAL
	printf("STATUS: Iniciando programa...\n");
	while(diasTranscurridos < TIEMPO) {
		muertosRonda = 0;
		curadosRonda = 0;
		contagiadosRonda = 0;

		// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
		for(i=0; i<pobActual; i++){
			moverPersona(&personas[i], ESCWIDTH, ESCHEIGHT);
			// FICHERO: GUARDAR CAMBIO DE PERSONA
			if(diasTranscurridos%BATX==0)
				fprintf(posic,"%d,%d,%d:",personas[i].pos[0],personas[i].pos[1],personas[i].estado);
        }
		// FICHERO: SALTAR DE LINEA TRAS MOVER TODAS LAS PERSONAS
		if(diasTranscurridos%BATX==0)
			fprintf(posic,"\n");

    	// INFECTADOS: COMPROBAR RADIO DE CONTAGIOS y DECISIONES DE MUERTE o SUPERVIVENCIA
        for(i=0; i<pobActual; i++){
            if(personas[i].estado == 1 || personas[i].estado == 2){
				rangox = personas[i].pos[0];
				rangoy = personas[i].pos[1];

				// DECIDIR SI SE CONTAGIA CADA INDIVIDUO EN BASE AL RADIO DE UN CONTAGIADO
				for(e=0; e<pobActual; e++)
					contagiadosRonda += infecPersona(&personas[e], rangox, rangoy, RADIO, PROBRADIO);

				// DECIDIR SI SE MUERE O SE RECUPERA
				int samatao = matarPersona(&personas[i]);
				if(samatao == 0){				// SE MUERE
					for(e=i; e<pobActual-1; e++)
						personas[e] = personas[e+1];
					muertosRonda++;
					contagiadosTotales--;
					pobActual--;
				} else if(samatao == 2){			// SE CURA
					curadosRonda++;
					contagiadosTotales--;
				}
	        }
		}

		// ACTUALIZAR EDAD MEDIA
		edadMedia = mediaEdad(personas, pobActual);

	    // ACTUALIZAR VALORES TOTALES
		contagiadosTotales += contagiadosRonda;
		curadosTotales += curadosRonda;
		muertosTotales += muertosRonda;
	   	if(diasTranscurridos%BATX==0){//Si es multiplo de lo metido significa que se va a guardar en el fichero los datos con el formato establecido
			fprintf(dias, "%d:%d,%d,%d\n", diasTranscurridos,contagiadosTotales,curadosTotales,muertosTotales);
		}

		// RULAR TIEMPO
		diasTranscurridos++;

	    // VISUALIZAR PROGRESO
	    printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n",
	            diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);

		// CONTROLAR SI SE DEBE FINALIZAR EL PROGRAMA
        if(contagiadosTotales == 0) break;
        if(pobActual == 0) break;
	}

	// LIBERAR MEMORIA y CERRAR ARCHIVOS AL ACABAR PROGRAMA
	printf("STATUS: Liberando memoria alocada...\n");
	free(personas);
	fclose(dias);
	fclose(posic);
    printf("STATUS: Fin del programa.\n");

}
