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
          realloc(arr, sizeof(arr) * 2);
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
	int muertosNodo, curadosNodo, contagiadosNodo;
	int muertosTotales = 0;
	int curadosTotales = 0;
	int contagiadosTotales = 0;
	int diasTranscurridos = 0;
	int desv = 0;
	int pobActual = POBLACION;
	int edadMedia = EDADMEDIA;
	char linea1[30], linea2[30];

	//PARA MPI
 	int *proc=malloc(world_size*sizeof(int)); //Con este puntero guardaremos los nodos que van a trabajar.

	//Todos los nodos van a funcionar.
	for(i=0;i<world_size;i++)
		proc[i]=i;

	if(world_rank==0){
		//Para sacar el tamaño de los cuadrantes.
		int totalcuadrados, control; //Numero total de puntos a controlar por cada nodo.
		int *factores;//Aqui guardaremos los factores.
		int cuenteofactores=-1;
		int nX, nY;//Se guarda el tamaño de X,Y
		int capacidadarray = CAPACIDADINICIAL;
		int tamanoActArray=0;
		factores=malloc(capacidadarray*sizeof(int));//Aqui guardaremos los factores.

		int *coordenadasX, *coordenadasY; //Aqui guardaremos las coordenadas X,Y de las que es responsable cada nodo.

		coordenadasX=malloc(world_size*sizeof(int));//guardaremos desde que punto del eje X se encarga cada nodo.
		coordenadasY=malloc(world_size*sizeof(int));//guardaremos desde que punto del eje Y se encarga cada nodo.

		totalcuadrados=(ESCHEIGHT*ESCWIDTH)/world_size;//Numero total de cuadrados a controlar por cada nodo.
		//Ahora hay que sacar las dimensiones de los cuadrados:
		//1-Sacar los factores:
		for(control=2;control<=totalcuadrados/2;control++){
			if(totalcuadrados % control == 0){
				cuenteofactores++;
				//Entonces el numero encontrado es factor.
				//Comprobar que el numero no se pasa de las dimensiones
				push(factores,tamanoActArray,control,tamanoActArray,capacidadarray);
				//factores[cuenteofactores]=control;

			}
		}
		if(cuenteofactores!=-1){//No es primo
			//La mejor opcion siempre tiene que estar en la mitad ya que estan ordenados.
			int indice=cuenteofactores/2;
			if(cuenteofactores%2==0){
				nX=factores[indice-1];
				nY=factores[indice];
			}else{
				nX=factores[indice];
				nY=factores[indice+1];
			}

		}else{//Habra que mirar que hacer

		}
		//Mirar cuantos huecos va a haber.
		//En X:
		int huecosX = round(ESCWIDTH/nX);
		//En Y:
		int huecosY = round(ESCHEIGHT/nY);

		for(i=0;i<world_size;i++){
			//EJE X (Para calcular las cordenadas) y saber a quien hay que enviarle cada persona.
			coordenadasX[i]=(i%huecosX)*nX;
			//EJE Y (Para calcular las cordenadas) y saber a quien hay que enviarle cada persona.
			coordenadasY[i]=(i/huecosX)*nY;
		}
	}

	desv = calculo_desv(EDADMEDIA);
	srand(SEED);

	// INICIALIZACION FICHEROS (MPI)
	int posic;
	MPI_Offset offset1;
	MPI_File posiFile;
	MPI_Status statPosic;
	posic = MPI_File_open( MPI_COMM_WORLD, "historialposic.txt", MPI_MODE_RDWR | MPI_MODE_CREATE, MPI_INFO_NULL, &posiFile);

	FILE *dias;
	dias  = fopen("historialdias.txt", "w+");

	// INICIALIZACION ARRAY PERSONAS
	struct persona *personas;
	personas = malloc(POBLACION*sizeof(struct persona));

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
		printf("STATUS: Creando población...\n");
		for(i=0; i<POBLACION; i++){
			personas[i] = crearPersona(EDADMEDIA, ESCWIDTH, ESCHEIGHT,desv);
			if(i==0){
				// PRIMER INFECTADO! Ahora el primer infectado sera el primero creado.
				printf("STATUS: PRIMER INFECTADO!\n");
				personas[0].estado = 1;
				contagiadosTotales++;
			}
			//Ahora habra que asignarselo a un nodo.
			int posiX=personas[i].pos[0]/nX;//Cuadrante X
			int posiY=personas[i].pos[1]/nY;//Cuadrante Y.
			int posiFinal=posiX+(huecosX*posiY);//para saber a que servidor hay que mandarselo.
			//push(int *arr, int index, int value, int *size, int *capacity){
			//Hay que llevar un control de la longitud y del tamano actual de todos los arrays (MENUDO MARRON)
			pushpersona(carga[posiFinal],tamanoarray[posiFinal],personas[i],&tamanoarray[posiFinal],&capacidadArray[posiFinal]);//Esto lo que deberia de hacer es meter a cada persona en su posicion correcta y redimensionar los arrays automaticamente (en teoria)
			tamanoarray[posiFinal]++;//Aumentar el indice del numero de personas de ese proc.
		}

	}

	//Ahora habria que compartir toda la informacion con los demas nodos. Scatter?

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
		for(i=0; i<pobActual; i++){
			// AQUI HABRA QUE VER QUIEN LO TIENE QUE CALCULAR.
			moverPersona(&personas[i], ESCWIDTH, ESCHEIGHT);
			// FICHERO: GUARDAR CAMBIOS DE PERSONA
			if(diasTranscurridos%BATX==0)
				// ESCRIBIR EN FICHERO CON MPI
				sprintf(linea1, "%d,%d,%d:", personas[i].pos[0], personas[i].pos[1], personas[i].estado);
				// AQUI FALTA EL OFFSET
				// offset1 = ()
				MPI_File_seek(posiFile, offset1, MPI_SEEK_SET);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
        	}

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

		// FICHERO: SALTAR DE LINEA TRAS MOVER TODAS LAS PERSONAS
		if(diasTranscurridos%BATX==0)
			if(world_rank == 0) {
				snprintf(linea1, sizeof("\n"), "\n");
				offset1 = 0;
				MPI_File_seek(posiFile, offset1, MPI_SEEK_END);
				MPI_File_write(posiFile, linea1, sizeof(linea1), MPI_CHAR, &statPosic);
			}

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

	    // INFECTADOS: COMPROBAR RADIO DE CONTAGIOS y DECISIONES DE MUERTE o SUPERVIVENCIA
        for(i=0; i<pobActual; i++){
			if(personas[i].estado == 1 || personas[i].estado == 2){
				rangox = personas[i].pos[0];
				rangoy = personas[i].pos[1];

				// DECIDIR SI SE CONTAGIA CADA INDIVIDUO EN BASE AL RADIO DE UN CONTAGIADO
				for(e=0; e<pobActual; e++)
					contagiadosNodo += infecPersona(&personas[e], rangox, rangoy, RADIO, PROBRADIO);

				// DECIDIR SI SE MUERE O SE RECUPERA
				int samatao = matarPersona(&personas[i]);
				if(samatao == 0){				// SE MUERE
					for(e=i; e<pobActual-1; e++)
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
		pobActual -= muertosRonda;
		MPI_Reduce(&muertosNodo, &muertosRonda, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
		contagiadosTotales -= (curadosRonda + muertosRonda);
		contagiadosRonda = 0;
		MPI_Reduce(&contagiadosNodo, &contagiadosRonda, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

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

/*
struct persona **crear_matriz(int filas, int column) {
    struct persona *data = (struct persona *)malloc(filas*column*sizeof(struct persona));
    struct persona **array= (struct persona **)malloc(filas*sizeof(struct persona*));
    int i;
    for (i=0; i<filas; i++)
        array[i] = &(data[column*i]);

    return array;
}
*/
