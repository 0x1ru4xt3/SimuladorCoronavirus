#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "persona.h"
#include "probabilidad.h"

#define SEED 0
#define CAPACIDADINICIAL 5

/* Seguramente haya que usar MPI Isend y MPI_Irecv con el parametro en recv MPI_ANY_SOURCE para escuchar a todos los nodos
de manera no bloqueante.

*/

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

//Funcion para controlar los arrays dinamicos que no sabemos cuales van a ser su longitud.
//arr->Array. Index->donde,Value->valor,size->tamano total actual, Capacity->capacidad actual.
void push(int *arr, int index, int value, int *size, int *capacity){
     if(*size > *capacity){
          realloc(arr, sizeof(arr) * 2);
          *capacity = sizeof(arr) * 2;
     }
     arr[index] = value;
     *size = *size + 1;
}

//Funcion para controlar los arrays dinamicos que no sabemos cuales van a ser su longitud, se guardaran las personas.
//arr->Array. Index->donde,Value->valor,size->tamano total actual, Capacity->capacidad actual.
void pushpersona(struct persona *arr, int index, struct persona value, int *size, int *capacity){
     if(*size > *capacity){
          *arr = realloc(arr, sizeof(arr) * 2); //COMO SOLUCIONAR ESTO ? 
          *capacity = sizeof(arr) * 2;
     }

     arr[index] = value;
     *size = *size + 1;
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

	// INICIALIZACIONES MPI
	int world_rank, world_size;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// INICIALIZACIONES VARIABLES
	if(world_rank == 0)
		printf("STATUS: Inicializando variables...\n");
	int i, e, j;
   	int rangox, rangoy;
	int muertosRonda, curadosRonda, contagiadosRonda;
	int muertosNodo, curadosNodo, contagiadosNodo, pobNodo;
	int muertosTotales = 0;
	int curadosTotales = 0;
	int contagiadosTotales = 0;
	int diasTranscurridos = 0;
	int desv = 0;
	int pobActual = POBLACION;
	int edadMedia = EDADMEDIA;
	int len;
	char linea1[30], linea2[30];

	//PARA MPI
 	int *proc=malloc(world_size*sizeof(int)); //Con este puntero guardaremos los nodos que van a trabajar.

	//Todos los nodos van a funcionar.
	for(i=0;i<world_size;i++)
		proc[i]=i;

	//Calcular de que tamano tienen que ser los cuadrados de cada nodo.
	if(world_rank==0){
		// Tamano de posicion de X e Y.
		int nX=ESCWIDTH/sqrt(world_size);
		int nY=ESCHEIGHT/sqrt(world_size);
	}
	MPI_Bcast(&nX,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&nY,1,MPI_INT,0,MPI_COMM_WORLD);
	int NWX, NWY;
	NWX=nX*(world_rank%(ESCWIDTH/nX));
	NWY=nY*(world_rank%(ESCWIDTH/nY));

	//PASAR A CADA NODO el tamaño de nX y de nY, broadcat.
	desv = calculo_desv(EDADMEDIA);
	srand(SEED);

	// INICIALIZACION FICHEROS
	FILE *dias;
	dias  = fopen("historialdias.txt", "w+");
	int posic;
	
	MPI_Offset offset1;
	MPI_File posiFile;
	MPI_Status statPosic;
	posic = MPI_File_open( MPI_COMM_WORLD, "historialposic.txt", MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &posiFile);

	// INICIALIZACION ARRAY PERSONAS
	struct persona *personas;
	int longitud=0;
	int capacidad=POBLACION/world_size;
	personas = malloc(capacidad*sizeof(struct persona)); //MIRAR TAMAÑO DE ESTO!

	// IMPRESION DE VARIABLES INTRODUCIDAS POR PARAMETRO
	if(world_rank == 0){
		printf("STATUS: DATOS INTRODUCIDOS: \n\tTIEMPO %d\n\tPOBLACION: %d\n\tESCENARIO: %dx%d\n\tRADIO CONTAGIO: %d  PROB DE CONTAGIO RADIO: %.2f\n",
			TIEMPO, POBLACION, ESCHEIGHT, ESCWIDTH, RADIO, PROBRADIO);

		// El primer nodo genera toda la poblacion y lo va a ir distribuyendo a un array de arrays (nº de arrays=nº de nodos) para luego repartirlos entre los procesadores.
		//Array de arrays con los datos de la gente:
		struct persona* carga[24];
		int tamanoarray[24]; //Aqui guardaremos cuantas personas tiene que calcular cada persona
		int capacidadArray[24];

		for(i=0;i<24;i++){
			tamanoarray[i]=0;
			capacidadArray[i]=CAPACIDADINICIAL;
			carga[i]=malloc(CAPACIDADINICIAL*sizeof(struct persona));//Si esto funciona es un puto milagro.
		}

		printf("STATUS: Creando población en cada nodo\n");
	}

		//HABRA QUE PONERLO EN CADA NODO con POBLACION/wolrd_size
	for(i=0; i<POBLACION/world_size; i++){
		struct *persona persaux = crearPersona(EDADMEDIA, nX, nY ,desv, NWX, NWY); //Añadir la posición de inicio del cuadrante.
		if(i==0 && world_rank==0){
			// PRIMER INFECTADO! Ahora el primer infectado sera el primero creado.
			printf("STATUS: PRIMER INFECTADO!\n");
			persaux.estado = 1;
			contagiadosTotales++;
		}
		//push(int *arr, int index, int value, int *size, int *capacity){
		//Hay que llevar un control de la longitud y del tamano actual de todos los arrays (MENUDO MARRON)
		pushpersona(personas,longitud,persaux,longitud,capacidad);//Esto lo que deberia de hacer es meter a cada persona en su posicion correcta y redimensionar los arrays automaticamente (en teoria)
	}


	if(world_rank == 0)
		printf("STATUS: Iniciando programa...\n");
	// BUCLE PRINCIPAL
	while(diasTranscurridos < TIEMPO) {
		muertosRonda = 0;
		curadosRonda = 0;
		contagiadosRonda = 0;
		muertosNodo = 0;
		curadosNodo = 0;
		contagiadosNodo = 0;

		// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
		for(i=0; i<pobNodo; i++){
			moverPersona(&personas[i], ESCWIDTH, ESCHEIGHT, NWX, NWY, NWX+nX, NWY+nY);
			// FICHERO: GUARDAR CAMBIOS DE PERSONA
			if(diasTranscurridos%BATX==0)
				// ESCRIBIR EN FICHERO CON MPI
				len = sprintf(linea1, "%d,%d,%d:", personas[i].pos[0], personas[i].pos[1], personas[i].estado);
				//offset1 = (world_rank*len*nº de personas que controla el procesador) + (len*i);
				MPI_File_seek(posiFile, offset1, MPI_SEEK_SET);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
        	}

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

		// FICHERO: SALTAR DE LINEA TRAS MOVER TODAS LAS PERSONAS
		if(diasTranscurridos%BATX==0)
			if(world_rank == 0) {
				snprintf(linea1, sizeof("\n"), "\n");
				MPI_File_seek(posiFile, offset1, MPI_SEEK_END);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
			}

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

	    // INFECTADOS: COMPROBAR RADIO DE CONTAGIOS y DECISIONES DE MUERTE o SUPERVIVENCIA
        for(i=0; i<pobNodo; i++){
			if(personas[i].estado == 1 || personas[i].estado == 2){
				rangox = personas[i].pos[0];
				rangoy = personas[i].pos[1];

				// DECIDIR SI SE CONTAGIA CADA INDIVIDUO EN BASE AL RADIO DE UN CONTAGIADO
				for(e=0; e<pobNodo; e++)
					contagiadosNodo += infecPersona(&personas[e], rangox, rangoy, RADIO, PROBRADIO);

				// DECIDIR SI SE MUERE O SE RECUPERA
				int samatao = matarPersona(&personas[i]);
				if(samatao == 0){				// SE MUERE
					for(e=i; e<pobNodo-1; e++)
						personas[e] = personas[e+1];
					muertosNodo++;
				} else if(samatao == 2){			// SE CURA
					curadosNodo++;
				}
			}
		}

		// FUNCION MPI: RECOGER VALORES DE NODOS Y SUMAR
		curadosRonda = 0;
		MPI_Reduce(&curadosNodo, &curadosRonda, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		muertosRonda = 0;
		MPI_Reduce(&muertosNodo, &muertosRonda, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		contagiadosRonda = 0;
		MPI_Reduce(&contagiadosNodo, &contagiadosRonda, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		pobActual = 0;
		MPI_Reduce(&pobNodo, &pobActual, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		pobActual -= muertosRonda;
		contagiadosTotales = contagiadosTotales - (curadosRonda + muertosRonda) + contagiadosRonda;

		// ACTUALIZAR EDAD MEDIA
		edadMedia = mediaEdad(personas, pobActual);

		// ACTUALIZAR VALORES TOTALES
		contagiadosTotales += contagiadosRonda;
		curadosTotales += curadosRonda;
		muertosTotales += muertosRonda;

		// RULAR TIEMPO
		diasTranscurridos++;

		// VISUALIZAR PROGRESO
		if(world_rank == 0)
			if(diasTranscurridos%BATX==0){//Si es multiplo de lo metido significa que se va a guardar en el fichero los datos con el formato establecido
				fprintf(dias, "%d:%d,%d,%d\n", diasTranscurridos,contagiadosTotales,curadosTotales,muertosTotales);
				printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);
			}

		// CONTROLAR SI SE DEBE FINALIZAR EL PROGRAMA
   		if(contagiadosTotales == 0) break;
   		if(pobActual == 0) break;
	}

	// FINALIZANDO PROGRAMA
	if(world_rank == 0){
		printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);

		// LIBERAR MEMORIA, CERRAR ARCHIVOS y CERRAR MPI AL ACABAR PROGRAMA
		printf("STATUS: Liberando memoria alocada...\n");
		printf("STATUS: Fin del programa.\n");
	}

	free(personas);
	MPI_File_close(&posiFile);
	fclose(dias);
	MPI_Finalize();
}
