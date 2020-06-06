/*
 * persona.c
 * Fichero recogiendo las funciones relacionadas con personas.
 * Abril de 2020
 */
 #include <stdlib.h>
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
