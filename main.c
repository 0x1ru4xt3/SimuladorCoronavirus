#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "persona.h"
#include "probabilidad.h"

#define SEED 0

struct almacenamiento{
   int capacidad;
   struct persona actualPersona;
   struct almacenamiento *siguienteAlma;
   struct almacenamiento *ultimo;
};

struct enviarInfectados{
    int capacidad;
    int coordenadas[][2];
};

/*
204 y 234 asteriscos,
pobNodo==capacidad
^*/

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
void pushPersona(struct persona **arr, int index, struct persona *value, int *size, int *capacity){
     if(*size == *capacity){ //Redimensionarlo haciendolo mas grande
         int aux=(*capacity)*2;
         struct persona *punt;
         punt = realloc(*arr, sizeof(struct persona)*aux);
         *arr = punt;
         *capacity = *capacity * 2;
	 }

     memcpy(&((*arr)[index]), value, sizeof(struct persona));
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
    int ESCWIDTH 	= atoi(argv[2]);
	int ESCHEIGHT 	= atoi(argv[3]);
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
	struct persona persVirtual = crearPersona(100, 1, 1, 1, 1, 1);
	persVirtual.edad=101;
	MPI_Datatype dataPersona;
	crearTipoPersona(&persVirtual, &dataPersona);

	MPI_Datatype dataEnvio;
	struct envio enviotipo;
	//crearTipoEnvio(&enviotipo,&dataEnvio,&dataPersona);
	MPI_Request request;

	// Calcular de que tamano tiene que ser el area de cada nodo.
	if(world_rank==0){
		// Tamano de posicion de X e Y.
		nX=ESCWIDTH/(int)floor(sqrt(world_size));
		nY=ESCHEIGHT/(int)ceil(sqrt(world_size));
		pobNodo=round(POBLACION/world_size);

        printf("nX=%d y nY=%d\n", nX, nY);
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
	int capacidad=round(POBLACION/world_size);
    int longitud=0;
	personas = malloc(capacidad*sizeof(struct persona));

	// IMPRESION DE VARIABLES INTRODUCIDAS POR PARAMETRO
	if(world_rank == 0)
		printf("STATUS: DATOS INTRODUCIDOS: \n\tTIEMPO %d\n\tPOBLACION: %d\n\tESCENARIO: %dx%d\n\tRADIO CONTAGIO: %d  PROB DE CONTAGIO RADIO: %.2f\n",
			TIEMPO, POBLACION, ESCHEIGHT, ESCWIDTH, RADIO, PROBRADIO);

	if(world_rank == 0)
		printf("STATUS: Creando población en cada nodo...\n");
	// CREAR POBLACION EN CADA NODO
	for(i=0; i<capacidad; i++){
		struct persona persaux = crearPersona(EDADMEDIA, nX, nY, desv, NWX, NWY);
		pushPersona(&personas,longitud,&persaux,&longitud,&capacidad);
    }

	// PRIMER INFECTADO!
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

        // MOVER -----------------------------------------------------------------------------------------
		// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
		struct envio envios[4];
		struct almacenamiento cap[4];
		for(i=0;i<4;i++){
			cap[i].capacidad=0;
            cap[i].ultimo=&cap[i];
            cap[i].siguienteAlma=&cap[i];
        }

		// FUNCIONA COMPROBADO
		for(i=0; i<pobNodo; i++){
			seMueve = moverPersona(&personas[i], ESCWIDTH, ESCHEIGHT, NWX, NWY, NWX+nX, NWY+nY);

			if(seMueve != 0){
                struct almacenamiento nuev;

				cap[seMueve-1].capacidad++;
				nuev.actualPersona = personas[i];

                if(cap[seMueve-1].capacidad==1){
                    cap[seMueve-1].actualPersona=personas[i];
                }

                nuev.ultimo=&nuev;
                nuev.siguienteAlma=&nuev;
				struct almacenamiento aux;
				aux=*cap[seMueve-1].ultimo;
				cap[seMueve-1].ultimo->siguienteAlma = &nuev;
                cap[seMueve-1].ultimo->ultimo=&nuev;
				cap[seMueve-1].ultimo = &nuev;

                // SI LA PERSONA CAMBIA DE NODO
                for(e=i; e<pobNodo-1; e++)
                    personas[e] = personas[e+1];
                pobNodo--;
                longitud--;
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
		struct almacenamiento *almaux;
		for(e=0; e<4; e++){
			envios[e].capacidad=cap[e].capacidad;
			envios[e].personas=malloc(envios[e].capacidad*sizeof(struct persona));
			almaux=&cap[e];
			for(i=0;i<envios[e].capacidad;i++){
				envios[e].personas[i]=almaux->actualPersona;
				almaux=cap[e].siguienteAlma;
				//cap[e]=*cap[e].siguienteAlma;
			}
		}

		// MANDAR EL ARRAY DE PERSONAS QUE SE LE ENVIA A CADA NODO
		/// BORDE IZQUIERDO
        //MIRAR EL DATATYPE, Que dependera de la cantidad de elementos que reciba de los otros elementos.

        MPI_Datatype dataEnvio0, dataEnvio1, dataEnvio2, dataEnvio3;
        crearTipoEnvio(&envios[0],&dataEnvio0,&dataPersona);
		if(NWX > 0){
            MPI_Isend(&envios[0].capacidad,1,MPI_INT,world_rank-1,0, MPI_COMM_WORLD,&request); //Enviar capacidad del array para crear el datatype.
			MPI_Isend(&envios[0], 1, dataEnvio0, world_rank-1,1, MPI_COMM_WORLD,&request);
        }
		/// BORDE SUPERIOR
        crearTipoEnvio(&envios[1],&dataEnvio1,&dataPersona);
		if(NWY > 0){
            MPI_Isend(&envios[1].capacidad,1,MPI_INT,world_rank-(ESCWIDTH/nX),0, MPI_COMM_WORLD,&request);//Enviar capacidad del array para crear el datatype.
			MPI_Isend(&envios[1], 1, dataEnvio1, world_rank-(ESCWIDTH/nX),1, MPI_COMM_WORLD,&request);
        }
		/// BORDE DERECHO
        crearTipoEnvio(&envios[2],&dataEnvio2,&dataPersona);
		if(NWX+nX < ESCWIDTH){
            MPI_Isend(&envios[2].capacidad,1,MPI_INT, world_rank+1,0, MPI_COMM_WORLD,&request);//Enviar capacidad del array para crear el datatype.
			MPI_Isend(&envios[2], 1, dataEnvio2, world_rank+1,1, MPI_COMM_WORLD,&request);
        }
		/// BORDE INFERIOR
        crearTipoEnvio(&envios[3],&dataEnvio3,&dataPersona);
		if(NWY+nY < ESCHEIGHT){
            MPI_Isend(&envios[3].capacidad,1,MPI_INT, world_rank+(ESCWIDTH/nX),0, MPI_COMM_WORLD,&request);//Enviar capacidad del array para crear el datatype.
			MPI_Isend(&envios[3], 1, dataEnvio3, world_rank+(ESCWIDTH/nX),1, MPI_COMM_WORLD,&request);
        }

        // LIBERAR ESPACIO DEL ARRAY MALLOC
        for(e=0; e<4; e++){
            if (envios[e].capacidad > 0){
                free(envios[e].personas);
            }

            envios[e].capacidad=0;
        }

		// BARRERA
        printf("Soy fulano de %d y he pasado el COVID\n", world_rank);
		//MPI_Barrier(MPI_COMM_WORLD);

        int tam;
        MPI_Status status1,status2,status3,status4;
		// RECIBIR ARRAIS DE PERSONAS DE NODOS COLINDANTES
        if((NWX == 0 && NWY == 0) || (NWX == 0 && NWY == ESCHEIGHT-nY) || (NWY == 0 && NWX == ESCWIDTH-nX) || (NWY == ESCHEIGHT-nY && NWX == ESCWIDTH-nX)){
            // SI ES ESQUINA SOLO RECIBE 2 (En orden: SUPIZQ, INFIZQ, SUPDER, INFDER):
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status1);
            envios[0].capacidad=tam;
            envios[0].personas=malloc(envios[0].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[0],&dataEnvio0,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status2);
            envios[1].capacidad=tam;
            envios[1].personas=malloc(envios[1].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[1],&dataEnvio1,&dataPersona);
            MPI_Recv(&envios[0], 1, dataEnvio0, status1.MPI_SOURCE,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&envios[1], 1, dataEnvio1, status2.MPI_SOURCE,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} else if ((NWX == 0) || (NWY == 0) || (NWX == ESCWIDTH-nX) || (NWY == ESCHEIGHT-nY)){
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status1);
            envios[0].capacidad=tam;
            envios[0].personas=malloc(envios[0].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[0],&dataEnvio0,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status2);
            envios[1].capacidad=tam;
            envios[1].personas=malloc(envios[1].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[1],&dataEnvio1,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status3);
            envios[2].capacidad=tam;
            envios[2].personas=malloc(envios[2].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[2],&dataEnvio2,&dataPersona);

			/// SI ES MARGEN (En orden: IZQ, SUP, DER, INF):
			MPI_Recv(&envios[0], 1, dataEnvio0, status1.MPI_SOURCE,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&envios[1], 1, dataEnvio1, status2.MPI_SOURCE,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&envios[2], 1, dataEnvio2, status3.MPI_SOURCE,1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} else {
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status1);
            envios[0].capacidad=tam;
            envios[0].personas=malloc(envios[0].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[0],&dataEnvio0,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status2);
            envios[1].capacidad=tam;
            envios[1].personas=malloc(envios[1].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[1],&dataEnvio1,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status3);
            envios[2].capacidad=tam;
            envios[2].personas=malloc(envios[2].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[2],&dataEnvio2,&dataPersona);
            MPI_Recv(&tam, 1, MPI_INT, MPI_ANY_SOURCE,0, MPI_COMM_WORLD, &status4);
            envios[3].capacidad=tam;
            envios[3].personas=malloc(envios[3].capacidad*sizeof(struct persona));

            crearTipoEnvio(&envios[3],&dataEnvio3,&dataPersona);

			/// SI ES DEL CENTRO:
			MPI_Recv(&envios[0], 1, dataEnvio0, status1.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&envios[1], 1, dataEnvio1, status2.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&envios[2], 1, dataEnvio2, status3.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&envios[3], 1, dataEnvio3, status4.MPI_SOURCE, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// BARRERA
		//MPI_Barrier(MPI_COMM_WORLD);

		// JUNTAR LOS CUATRO ARRAYS RECIBIDOS CON EL ARRAY QUE TIENE EL NODO
		for (i=0; i<4; i++){
			for (e=0; e<envios[i].capacidad; e++){
				pushPersona(&personas, longitud, &envios[i].personas[e], &longitud, &capacidad);
                //pushPersona(&personas,longitud,&persaux,&longitud,&capacidad);
				pobNodo++;
			}
		}

        printf("EL nodo %d tiene %d personas\n", world_rank, pobNodo);

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

        // LIBERAR MEMODIA DE MALLOC ENVIO
        for(e=0; e<4; e++){
            if (envios[e].capacidad > 0){
                if(world_rank==0){
                    printf("HOLITA %d y la e es: %d y la edad de la persona es envios[e].personas[0].edad=%d\n",envios[e].capacidad,e, envios[e].personas[0].edad);
                }
                free(envios[e].personas);
                printf("Soy %d, estamos en el segundo free.\n", world_rank);
            }
        }

        printf("SOY UN SUPERVIVIENTE %d \n", world_rank);

        // FIN MOVER -------------------------------------------------------------------------------------------

		// BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

        // INFECCIONES -----------------------------------------------------------------------------------------
        // /// COJO MIS PERSONAS QUE ESTAN INFECTADOS EN LOS BORDES Y CREO 4 ARRAYS (1 por cada lado)
        // int coordenadasInfectadas1[pobNodo][2];
        // int coordenadasInfectadas2[pobNodo][2];
        // int coordenadasInfectadas3[pobNodo][2];
        // int coordenadasInfectadas4[pobNodo][2];
        // int cont1 = 0;
        // int cont2 = 0;
        // int cont3 = 0;
        // int cont4 = 0;
        //
        // for(i=0; i<pobNodo; i++){
        //     if(personas[i].estado == 1 || personas[i].estado == 2){
        //         // SI ESTA EN LA ORILLA (Orden: DERECHA, ABAJO, IZQUIERDA, ARRIBA)
        //         if (personas[i].pos[0] >= (world_rank*nX+(nX-RADIO))) {
        //             coordenadasInfectadas1[cont1][1] = personas[i].pos[0];
        //             coordenadasInfectadas1[cont1][2] = personas[i].pos[1];
        //             cont1++;
        //         } else if (personas[i].pos[1] >= (world_rank*nY+(nY-RADIO))) {
        //             coordenadasInfectadas2[cont2][1] = personas[i].pos[0];
        //             coordenadasInfectadas2[cont2][2] = personas[i].pos[1];
        //             cont2++;
        //         } else if (personas[i].pos[0] <= (world_rank*nX+(RADIO))) {
        //             coordenadasInfectadas3[cont3][1] = personas[i].pos[0];
        //             coordenadasInfectadas3[cont3][2] = personas[i].pos[1];
        //             cont3++;
        //         } else if (personas[i].pos[1] <= (world_rank*nY+(RADIO))) {
        //             coordenadasInfectadas4[cont4][1] = personas[i].pos[0];
        //             coordenadasInfectadas4[cont4][2] = personas[i].pos[1];
        //             cont4++;
        //         }
        //     }
        // }
        //
        // /// ENVIO ESOS ARRAYS A SU NODO
        // //MPI_Datatype dataEnvio0, dataEnvio1, dataEnvio2, dataEnvio3;
        // crearTipoCoordenadas(&envios[0],&dataEnvio0,&dataPersona);
        // //// BORDE IZQUIERDO
		// if(NWX > 0){
        //     MPI_Isend(&envios[0].capacidad,1,MPI_INT,world_rank-1,0, MPI_COMM_WORLD,&request);
		// 	MPI_Isend(&envios[0], 1, dataEnvio0, world_rank-1,1, MPI_COMM_WORLD,&request);
        // }
		// 	printf("Soy %d Y llegamos a los sends: 1\n", world_rank);
		// //// BORDE SUPERIOR
        // crearTipoEnvio(&envios[1],&dataEnvio1,&dataPersona);
		// if(NWY > 0){
        //     MPI_Isend(&envios[1].capacidad,1,MPI_INT,world_rank-(ESCWIDTH/nX),0, MPI_COMM_WORLD,&request);
		// 	MPI_Isend(&envios[1], 1, dataEnvio1, world_rank-(ESCWIDTH/nX),1, MPI_COMM_WORLD,&request);
        // }
		// printf("Soy %d Y llegamos a los sends: 2\n", world_rank);
		// //// BORDE DERECHO
        // crearTipoEnvio(&envios[2],&dataEnvio2,&dataPersona);
		// if(NWX+nX < ESCWIDTH){
        //     MPI_Isend(&envios[2].capacidad,1,MPI_INT, world_rank+1,0, MPI_COMM_WORLD,&request);
		// 	MPI_Isend(&envios[2], 1, dataEnvio2, world_rank+1,1, MPI_COMM_WORLD,&request);
        // }
		// printf("Soy %d Y llegamos a los sends: 3\n", world_rank);
		// //// BORDE INFERIOR
        // crearTipoEnvio(&envios[3],&dataEnvio3,&dataPersona);
		// if(NWY+nY < ESCHEIGHT){
        //     printf("Soy el procesador %d y world_rank+(ESCWIDTH/nX) es %d\n", world_rank, world_rank+(ESCWIDTH/nX));
        //     MPI_Isend(&envios[3].capacidad,1,MPI_INT, world_rank+(ESCWIDTH/nX),0, MPI_COMM_WORLD,&request);
		// 	MPI_Isend(&envios[3], 1, dataEnvio3, world_rank+(ESCWIDTH/nX),1, MPI_COMM_WORLD,&request);
        // }

        /// RECIBO ARRAYS DE NODOS ADYACENTES numInfectados y coordenadasInfectadas nos las envian
        int numInfectados = 0;
        int coordenadasInfectadas[numInfectados+pobNodo][2];
        // ------------------------- //
        // ------- MPI RECIF ------- //
        // ------------------------- //

	    // INFECTADOS: COMPROBAR RADIO DE CONTAGIOS y DECISIONES DE MUERTE o SUPERVIVENCIA
        for(i=0; i<pobNodo; i++){
			if(personas[i].estado == 1 || personas[i].estado == 2){
				// DECIDIR SI SE MUERE O SE RECUPERA
				samatao = matarPersona(&personas[i]);
				if(samatao == 1){					// SE MUERE
					for(e=i; e<pobNodo-1; e++)
						personas[e] = personas[e+1];
					pobNodo--;
                    longitud--;
					muertosNodo++;
				} else if(samatao == 0){			// SE CURA
					curadosNodo++;
				} else {
                    coordenadasInfectadas[numInfectados][0] = personas[i].pos[0];
                    coordenadasInfectadas[numInfectados][1] = personas[i].pos[1];
                    numInfectados++;
                }
			}
		}

        // DECIDIR SI SE CONTAGIA CADA INDIVIDUO EN BASE AL RADIO DE UN CONTAGIADO
        if(numInfectados>0){
            for(i=0; i<pobNodo; i++){
                 if(infecPersona(&personas[i], coordenadasInfectadas, numInfectados, RADIO, PROBRADIO)){
                     contagiadosNodo++;
                 }
             }
        }
        // FIN INFECCIONES -----------------------------------------------------------------------------------------

        // BARRERA
		MPI_Barrier(MPI_COMM_WORLD);

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

            // RULAR TIEMPO
            diasTranscurridos++;
		}

		// VISUALIZAR PROGRESO
		if(world_rank == 0){
			if(diasTranscurridos%BATX==0){//Si es multiplo de lo metido significa que se va a guardar en el fichero los datos con el formato establecido
				fprintf(dias, "%d:%d,%d,%d\n", diasTranscurridos, contagiadosTotales, curadosTotales,muertosTotales);
				printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);
			}
		}

        // // LIBERAR ESPACIO DEL ARRAY MALLOC
        // for(e=0; e<4; e++){
        //     if (envios[e].capacidad > 0) {
        //         for(i=0;i<envios[e].capacidad;i++){
        //             printf("Soy %d y esta es la edad de la persona %d\n", world_rank, envios[e].personas[i].edad);
        //         }
        //         printf("W.R.=%d aqui deberia hacer el fri del array %dº y la capacidad es: %d\n", world_rank, e, envios[e].capacidad);
        //         free(envios[e].personas);
        //         envios[e].capacidad=0;
        //         printf("Se ha realizado el free\n");
        //     }
        // }

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

	// FINALIZANDO PROGRAMA
	if(world_rank == 0){
		// FIN TIEMPO DE EJECUCION
		clock_t fin = clock();
		double tiempoTotal = (double)(inicio - fin) / CLOCKS_PER_SEC;

		// IMPRIMIR ESTADO
		printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS). POBLACION: %i, EDAD MEDIA: %i\n", diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, pobActual, edadMedia);
		printf("STATUS: Tiempo de ejecucion en paralelo: %.2f\n", tiempoTotal);

		// LIBERAR MEMORIA, CERRAR ARCHIVOS y CERRAR MPI AL ACABAR PROGRAMA
		printf("STATUS: Liberando memoria alocada...\n");
		printf("STATUS: Fin del programa.\n");
	}

	free(personas);
	//MPI_File_close(&posiFile);
	//fclose(dias);
	MPI_Finalize();

	return 0;
}
