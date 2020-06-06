/*
 * persona.c
 * Fichero recogiendo las funciones relacionadas con personas.
 * Abril de 2020
 */
 #include <stdlib.h>
 #include <mpi.h>
 #include "persona.h"
 #include "probabilidad.h"

 // PROBABILIDADES POR EDAD
 #define EDAD1 0.004  // < 50
 #define EDAD2 0.013  // 50 - 60
 #define EDAD3 0.036  // 60 - 70
 #define EDAD4 0.080  // 70 - 80
 #define EDAD5 0.148  // > 80

// CREAR PERSONA
struct persona crearPersona(int edadMedia, int escAncho, int escAlto,int dev,int posX, int posY){ //pasar coordenadas de alguna manera.
	struct persona per;
	per.edad =(int) rand_normal(edadMedia,dev);
	per.estado = 0;
	per.diasContaminado = 0;

	// PROBABILIDAD DE MUERTE EN BASE A EDAD
	if(per.edad<50)
		per.probMuerte = EDAD1;
	else if(per.edad>=50 && per.edad<60)
		per.probMuerte = EDAD2;
	else if(per.edad>=60 && per.edad<70)
		per.probMuerte = EDAD3;
	else if(per.edad>=80 && per.edad<80)
		per.probMuerte = EDAD4;
	else
		per.probMuerte = EDAD5;

	//CALCULO DE LA POSICION y VELOCIDAD INICIAL
	per.pos[0] = rand()%escAlto+posX;
	per.pos[1] = rand()%escAncho+posY;
	per.vel[0] = rand()%10+(-5);
	per.vel[1] = rand()%10+(-5);

	return per;
}

// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
// (Par: struct persona)
int moverPersona(Persona *pers, int escAncho, int escAlto, int cordX, int cordY, int limiteAncho, int limiteAlto){
	// SE CONTROLA PRIMERO UN EJE, DESPUES EL OTRO, SE CONTROLA
	// QUE NO SE SALGA DE LOS LIMITES DEL ESCENARIO, Y SE ELIGE
	// LA VELOCIDAD DE LA SIGUIENT RONDA EN BASE A SU POSICION:
	// SI ESTA EN EL BORDE REBOTA, SI NO SE MUEVE LIBREMENTE :)

    int respuesta = 0;

	if (pers->pos[0] + pers->vel[0] >= escAncho){
		pers->pos[0] = escAncho;
        pers->vel[0] = rand()%5+(-5);
    } else if(pers->pos[0] + pers->vel[0] <= 0){
        pers->pos[0] = 0;
		pers->vel[0] = rand()%5;
	} else if(pers->pos[0] + pers->vel[0] <= cordX){
        pers->pos[0] += pers->vel[0];
		respuesta = 1;
    } else if(pers->pos[0] + pers->vel[0] >= limiteAncho){
        pers->pos[0] += pers->vel[0];
        respuesta = 3;
	} else {
		pers->pos[0] += pers->vel[0];
	}

	if (pers->pos[1] + pers->vel[1] >= escAlto){
		pers->pos[1] = escAlto;
		pers->vel[1] = rand()%5+(-5);
	} else if(pers->pos[1] + pers->vel[0] <= 0){
		pers->pos[1] = 0;
		pers->vel[1] = rand()%5;
    } else if(pers->pos[1] + pers->vel[1] <= cordY){
        pers->pos[1] += pers->vel[1];
        respuesta = 2;
    } else if(pers->pos[1] + pers->vel[1] >= limiteAlto){
        pers->pos[1] += pers->vel[1];
        respuesta = 4;
	} else {
		pers->pos[1] += pers->vel[1];
	}

    return respuesta;
}

// DECISION DE INFECTAR UNA PERSONA por RADIO DE CONTAGIADO
// (Par: struct persona, ints radio del infectado)
int infecPersona(Persona *per, int rangox, int rangoy, int radio, float probRadio){
    float deci;
	// SI NO ESTA INFECTADO y NO LO HA ESTADO
	if(per->estado == 0){
		// SI ESTA DENTRO DEL RANGO DE EJE X DEL INFECTADO
		if(per->pos[0] <= rangox+radio && per->pos[0] >= rangox-radio){
			// SI ESTA DENTRO DEL RANGO DE EJE Y DEL INFECTADO
			if(per->pos[1] <= rangoy+radio && per->pos[1] >= rangoy-radio){
				deci = (rand()%100) /100.0;
				if(deci>probRadio){
					per->estado = 1;
					return 1;
				}
			}
		}
	}
	return 0;
}

// DECISION DE MUERTE DE UNA PERSONA
// (Par: struct persona)
int matarPersona(Persona *per){
	float deci = calcProb();
    printf("FUNCION matarPersona: deci=%.2f, per-probMuerte=%.2f\n", deci, per->probMuerte);
	if(deci <= per->probMuerte)
		return 1;
	else {
		per->diasContaminado = per->diasContaminado +1;
		if(per->estado == 1 && per->diasContaminado >= 5){
			per->estado = 2;
			return 2;
		} else if(per->estado == 2 && per->diasContaminado >= 15){
			per->estado = 3;
			return 0;
		}
	}
}

// CREAR DATATYPE PARA MANDAR
void crearTipoEnvio(Envio *envio, MPI_Datatype *MPI_DATOS, MPI_Datatype *persona){
    MPI_Datatype tipo[2];
    tipo[0]=MPI_INT;
	tipo[1]=*persona;

    int tam[2];
	tam[0]=1;
	tam[1]=1;

    MPI_Aint dist[6],dir1,dir2;
	dist[0]=0;
	MPI_Get_address(&envio->capacidad, &dir1);
	MPI_Get_address(envio->personas, &dir2);
	dist[1]=dir2-dir1;
}

// CREAR DATATYPE PARA MANDAR
void crearTipoPersona(Persona *pers, MPI_Datatype *MPI_DATOS){
	//Declaracion de los tipos del struct
	MPI_Datatype tipo[2];
	tipo[0]=MPI_INT;
	tipo[1]=MPI_INT;
	tipo[2]=MPI_INT;
    tipo[3]=MPI_FLOAT;
    tipo[4]=MPI_INT;
    tipo[5]=MPI_INT;

	//Declaracion de los tamaños de el struct
	int tam[2];
	tam[0]=1;
	tam[1]=1;
	tam[2]=1;
    tam[3]=1;
	tam[4]=2;//array de dos posciones de int
    tam[5]=2;

	//Declaracion de las distancias
	MPI_Aint dist[6],dir1,dir2;
	dist[0]=0;
	MPI_Get_address(&pers->edad,&dir1);
	MPI_Get_address(&pers->estado,&dir2);
	dist[1]=dir2-dir1;
	MPI_Get_address(&pers->diasContaminado,&dir2);
	dist[2]=dir2-dir1;
    MPI_Get_address(&pers->probMuerte,&dir2);
	dist[3]=dir2-dir1;
    MPI_Get_address(pers->pos,&dir2);
	dist[4]=dir2-dir1;
    MPI_Get_address(pers->vel,&dir2);
    dist[5]=dir2-dir1;

	//Creacion del tipo
	MPI_Type_create_struct(6,tam,dist,tipo,MPI_DATOS);
	MPI_Type_commit(MPI_DATOS);
}
