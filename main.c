#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "persona.h"
#include "probabilidad.h"

#define SEED 0
#define CAPACIDADINICIAL 5

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

// Funcion para controlar los arrays dinamicos que no sabemos cuales van a ser su longitud, se guardaran las personas.
// arr->Array. Index->donde,Value->valor,size->tamano total actual, Capacity->capacidad actual.
void pushPersona(struct persona *arr, int index, struct persona value, int *size, int *capacity){
     if(*size > *capacity){ //Redimensionarlo haciendolo mas grande
        arr = realloc(arr, sizeof(arr) * 2);
        *capacity = sizeof(arr) * 2;
//     }else if(*size<(*capacity/2)){ //Redimensionarlo haciendolo mas pequeño
//		arr = realloc(arr, sizeof(arr)/2);
//		*capacity=sizeof(arr)/2;
	 }

     arr[index] = value;
     *size = *size + 1;
}

// FUNCION DE PROGRAMA PRINCIPAL
int main(int argc, char** argv) {
	// INICIALIZACIONES MPI
	int world_rank, world_size;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	if(argc!=9) {
		if(world_rank == 0)
			fprintf(stderr,"Funcionamiento: %s <tiempoASimular> <tamanoAncho> <tamanoAlto> <radio> <probRadio> <poblacion> <edadMedia> <batch>\n", argv[0]);
		MPI_Finalize();
		exit(0);
	}

	int TIEMPO 		= atoi(argv[1]);
	int ESCHEIGHT 	= atoi(argv[2]);
	int ESCWIDTH 	= atoi(argv[3]);
	int RADIO 		= atoi(argv[4]);
	float PROBRADIO = atof(argv[5]);
	int POBLACION 	= atoi(argv[6]);
	int EDADMEDIA 	= atoi(argv[7]);
	int BATX	= atoi(argv[8]);

	if (PROBRADIO > 0.9 || PROBRADIO < 0 || TIEMPO < BATX || TIEMPO < 1 || RADIO >= ESCWIDTH || RADIO >= ESCHEIGHT) {
        fprintf(stderr,"Error de parámetros: \n\t- La probabilidad de contagio debe estar comprendido entre 0 y 1.\n\t- El tiempo a simular debe ser mayor que 1.\n\t- El batch no puede ser mayor que el tiempo a simular.\n\t- El radio de contagio debe ser menor que el tamaino del lienzo.\n");
		exit(1);
	}

	// INICIO TIEMPO DE EJECUCION
	clock_t inicio = clock();

	if(world_rank == 0)
		printf("STATUS: Inicializando variables...\n");
	// INICIALIZACIONES VARIABLES
	int muertosTotales, curadosTotales, contagiadosTotales;
	int edadMedia = EDADMEDIA;
	int edadMediaFinal;
	int i, e, j;
   	int rangox, rangoy;
	int nX, nY;
	int pobActual = POBLACION;
	int diasTranscurridos = 0;
	int muertosRonda, curadosRonda, contagiadosRonda;
	int muertosNodo, curadosNodo, contagiadosNodo, pobNodo;
	int desv = 0;
	int len, samatao, seMueve;
	char linea1[30];
	int salida = 0;

	if(world_rank == 0){
		muertosTotales = 0;
		curadosTotales = 0;
		contagiadosTotales = 0;
	}

	// PARA MPI
 	int *proc=malloc(world_size*sizeof(int)); //Con este puntero guardaremos los nodos que van a trabajar.
	struct persona persVirtual = crearPersona(100, 1, 1, 1, 1, 1);
	persVirtual.edad=101;
	MPI_Datatype dataPersona;
	crearTipoPersona(&persVirtual, &dataPersona);

	MPI_Datatype dataEnvio;
	struct envio enviotipo;
	crearTipoEnvio(&enviotipo,&dataEnvio,&dataPersona);
	MPI_Request request;

	// Todos los nodos van a funcionar.
	for(i=0;i<world_size;i++){
		proc[i]=i;
	}

	// Calcular de que tamano tienen que ser los cuadrados de cada nodo.
	if(world_rank==0){
		// Tamano de posicion de X e Y.
		nX=ESCWIDTH/(int)floor(sqrt(world_size));
		nY=ESCHEIGHT/(int)ceil(sqrt(world_size));
		pobNodo=round(POBLACION/world_size);
	}

	// PASAR A CADA NODO el tamaño de nX y de nY, broadcat.
	MPI_Bcast(&nX,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&nY,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&pobNodo,1,MPI_INT,0,MPI_COMM_WORLD);

	int NWX, NWY;
	NWX=nX*(world_rank%(int)round(sqrt(world_size)));
	NWY=nY*(world_rank/(int)round(sqrt(world_size)));

	desv = calculo_desv(EDADMEDIA);
	srand(SEED);

	// INICIALIZACION FICHEROS
	FILE *dias;
	dias = fopen("historialdias.txt", "w+");
	int posic;

	MPI_Offset offset1;
	MPI_File posiFile;
	MPI_Status statPosic;
	posic = MPI_File_open( MPI_COMM_WORLD, "historialposic.txt", MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &posiFile);

	// INICIALIZACION ARRAY PERSONAS
	struct persona *personas;
	int longitud=0;
	int capacidad=round(POBLACION/world_size);
	personas = malloc(capacidad*sizeof(struct persona));

	// IMPRESION DE VARIABLES INTRODUCIDAS POR PARAMETRO
	if(world_rank == 0)
		printf("STATUS: DATOS INTRODUCIDOS: \n\tTIEMPO %d\n\tPOBLACION: %d\n\tESCENARIO: %dx%d\n\tRADIO CONTAGIO: %d  PROB DE CONTAGIO RADIO: %.2f\n",
			TIEMPO, POBLACION, ESCHEIGHT, ESCWIDTH, RADIO, PROBRADIO);

	if(world_rank == 0)
		printf("STATUS: Creando población en cada nodo...\n");
	// CREAR POBLACION EN CADA NODO
	for(i=0; i<capacidad; i++){
		struct persona persaux = crearPersona(EDADMEDIA, nX, nY , desv, NWX, NWY); //Añadir la posición de inicio del cuadrante.
		//push(int *arr, int index, int value, int *size, int *capacity){
		//Hay que llevar un control de la longitud y del tamano actual de todos los arrays (MENUDO MARRON)
		pushPersona(personas,longitud,persaux,&longitud,&capacidad);//Esto lo que deberia de hacer es meter a cada persona en su posicion correcta y redimensionar los arrays automaticamente (en teoria)
	}

	// PRIMER INFECTADO! Ahora el primer infectado sera el primero creado.
	if(world_rank == 0){
		printf("STATUS: PRIMER INFECTADO!\n");
		personas[0].estado = 1;
		contagiadosTotales = 1;
	}

	if(world_rank == 0)
		printf("STATUS: Iniciando programa...\n");
	// BUCLE PRINCIPAL
	while(diasTranscurridos < TIEMPO) {
		if(world_rank == 0) {
			muertosRonda = 0;
			curadosRonda = 0;
			contagiadosRonda = 0;
		}

		muertosNodo = 0;
		curadosNodo = 0;
		contagiadosNodo = 0;

		// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
		struct envio envios[4];
		struct almacenamiento cap[4];
		for(i=0;i<4;i++)
			cap[i].capacidad=0;

		for(i=0; i<pobNodo; i++){
			seMueve = moverPersona(&personas[i], ESCWIDTH, ESCHEIGHT, NWX, NWY, NWX+nX, NWY+nY);
			struct almacenamiento nuev;

			if(seMueve != 0){
				cap[seMueve-1].capacidad++;
				nuev.actualPersona = personas[i];
				cap[seMueve-1].ultimo.siguienteAlma = nuev;
				cap[seMueve-1].ultimo = nuev;
			}

			// SI LA PERSONA CAMBIA DE NODO
			if(seMueve != 0){
				for(e=i; e<pobNodo-1; e++)
					personas[e] = personas[e+1];
				pobNodo--;
			}

			// FICHERO: GUARDAR CAMBIOS DE PERSONA
			if(diasTranscurridos%BATX==0){
				// ESCRIBIR EN FICHERO CON MPI
				len = sprintf(linea1, "%d,%d,%d:", personas[i].pos[0], personas[i].pos[1], personas[i].estado);
				offset1 = (world_rank*len*pobNodo) + (len*i);
				MPI_File_seek(posiFile, offset1, MPI_SEEK_SET);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
			}
		}

		// PASAR DEL LINKEDLIST A ARRAY
		for(e=0; e<4; e++){
			envios[e].capacidad=cap[e].capacidad;
			envios[e].personas=malloc(envios[e].capacidad*sizeof(struct persona));
			for(i=0;i<envios[e].capacidad;i++){
				envios[e].personas[i]=cap[e].actualPersona;
				cap[e]=cap[e].siguienteAlma;
			}
		}

		// MANDAR EL ARRAY DE PERSONAS QUE SE LE ENVIA A CADA NODO
		MPI_Isend(&envios[0], 1, dataEnvio, world_rank-1, world_rank, MPI_COMM_WORLD, &request);
		MPI_Isend(&envios[1], 1, dataEnvio, world_rank-(ESCWIDTH/nX), world_rank, MPI_COMM_WORLD, &request);
		MPI_Isend(&envios[2], 1, dataEnvio, world_rank+1, world_rank, MPI_COMM_WORLD, &request);
		MPI_Isend(&envios[3], 1, dataEnvio, world_rank+-(ESCWIDTH/nX), world_rank, MPI_COMM_WORLD, &request);

		// RECIBIR ARRAIS DE PERSONAS DE NODOS COLINDANTES
		MPI_Irecv(&envios[0], 1, dataEnvio, world_rank, MPI_ANY_SOURCE, MPI_COMM_WORLD, &request);
		MPI_Irecv(&envios[1], 1, dataEnvio, world_rank, MPI_ANY_SOURCE, MPI_COMM_WORLD, &request);
		MPI_Irecv(&envios[2], 1, dataEnvio, world_rank, MPI_ANY_SOURCE, MPI_COMM_WORLD, &request);
		MPI_Irecv(&envios[3], 1, dataEnvio, world_rank, MPI_ANY_SOURCE, MPI_COMM_WORLD, &request);

		// JUNTAR LOS CUATRO ARRAYS RECIBIDOS CON EL ARRAY QUE TIENE EL NODO
		for (i=0; i<4; i++){
			for (e=0; e<envios[i].capacidad; e++){
				pushPersona(personas, longitud, envios[i].personas[e], &longitud, &capacidad);
				pobNodo++;
			}
		}

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

		// FICHERO: SALTAR DE LINEA TRAS MOVER TODAS LAS PERSONAS
		if(diasTranscurridos%BATX==0){
			if(world_rank == 0) {
				snprintf(linea1, sizeof("\n"), "\n");
				MPI_File_seek(posiFile, offset1, MPI_SEEK_END);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
			}
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
				samatao = matarPersona(&personas[i]);
				if(samatao == 1){					// SE MUERE
					for(e=i; e<pobNodo-1; e++)
						personas[e] = personas[e+1];
					pobNodo--;
					muertosNodo++;
				} else if(samatao == 0){			// SE CURA
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
		edadMediaFinal = 0;
		edadMedia = mediaEdad(personas, pobNodo);
		MPI_Reduce(&edadMedia, &edadMediaFinal, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		edadMedia = edadMediaFinal / world_size;

		// ACTUALIZAR VALORES TOTALES
		if(world_rank == 0) {
			contagiadosTotales = contagiadosTotales - (curadosRonda + muertosRonda) + contagiadosRonda;
			curadosTotales += curadosRonda;
			muertosTotales += muertosRonda;
		}

		// RULAR TIEMPO
		diasTranscurridos++;

		// VISUALIZAR PROGRESO
		if(world_rank == 0){
			if(diasTranscurridos%BATX==0){//Si es multiplo de lo metido significa que se va a guardar en el fichero los datos con el formato establecido
				fprintf(dias, "%d:%d,%d,%d\n", diasTranscurridos, contagiadosTotales, curadosTotales,muertosTotales);
				printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);
			}
		}

		// CONTROLAR SI SE DEBE FINALIZAR EL PROGRAMA
		if(world_rank == 0){
   			if(contagiadosTotales == 0 || pobActual == 0)
				salida = 1;
			else
				salida = 0;

			MPI_Bcast(&salida,1,MPI_INT,0,MPI_COMM_WORLD);
		} else
			MPI_Bcast(&salida,1,MPI_INT,0,MPI_COMM_WORLD);

		if(salida) break;
	}

	// FIN TIEMPO DE EJECUCION
	clock_t fin = clock();
	double tiempoTotal = (double)(inicio - fin) / CLOCKS_PER_SEC;

	// FINALIZANDO PROGRAMA
	if(world_rank == 0){
		printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);
		printf("STATUS: Tiempo de ejecucion en paralelo: %.2f\n", tiempoTotal);
		// LIBERAR MEMORIA, CERRAR ARCHIVOS y CERRAR MPI AL ACABAR PROGRAMA
		printf("STATUS: Liberando memoria alocada...\n");
		printf("STATUS: Fin del programa.\n");
	}

	free(personas);
	MPI_File_close(&posiFile);
	fclose(dias);
	MPI_Finalize();

	return 0;
}
