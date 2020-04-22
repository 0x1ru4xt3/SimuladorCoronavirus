//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <sys/time.h>

#define SEED      0
int ESCHEIGHT;
int ESCWIDTH;
int POBLACION;
int RADIO;
float PROBRADIO;

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

//Calcular una edad entre 0 y 100
int NumeroRandom() {
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    struct timeval tv; // Para que no salgan todo el rato los mismos valores se coge el tiempo actual y se utliza como seed,
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    T = gsl_rng_default; // Generar el setup
    r = gsl_rng_alloc (T);
    gsl_rng_set(r, mySeed);
    double u = gsl_rng_uniform(r); // el numero que genera es entre 0 y 1, multiplicamos por 100.
    gsl_rng_free (r);
    double aux=u*100;
    int auxa=aux;
    return auxa;

}
//Funcion para calcular un numero entre 0 y 1, con el cual veremos si muere o no.
float calcProb(){
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    struct timeval tv; 
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    T = gsl_rng_default; // Generar setup
    r = gsl_rng_alloc (T);
    gsl_rng_set(r, mySeed);
    double u = gsl_rng_uniform(r); // Generar numero entre 0 y 1.
    gsl_rng_free (r);
    return (float)u;
}



// CREAR PERSONA
struct persona crearPersona(){
	struct persona per;
	per.edad = NumeroRandom(); // Generar por probabilidad
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
	per.pos[0] = rand()%ESCHEIGHT;
	per.pos[1] = rand()%ESCWIDTH;
	per.vel[0] = rand()%10+(-5);
	per.vel[1] = rand()%10+(-5);

	return per;
}

// CALCULAR LA MEDIA DE EDAD
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
	if(argc!=7){
		fprintf(stderr,"%s <tiempoASimular> <tamanoAncho> <tamanoAlto> <radio> <probRadio> <poblacion>\n", argv[0]);
		exit(1);
	}

	srand(SEED);

    printf("STATUS: Inicializando array dinamico...\n");
	// INICIALIZACIONES
	struct persona *personas;
	POBLACION=atoi(argv[6]);

    personas  = malloc(POBLACION*sizeof(struct persona));

    printf("STATUS: Inicializando variables...\n");
	int tiempo = atoi(argv[1]);
	ESCHEIGHT=atoi(argv[2]);
	ESCWIDTH=atoi(argv[3]);
	RADIO=atoi(argv[4]);
	PROBRADIO=atof(argv[5]);
	printf("\n Los datos dados son: TIEMPO %d,POBLACION: %d, ANCHURA: %d, ALTO: %d, RADIO: %d, PROB DEL RADIO: %f\n",tiempo, POBLACION,ESCHEIGHT,ESCWIDTH,RADIO,PROBRADIO);
	int pobActual = POBLACION;
   	int rangox, rangoy;
	int muertosRonda, curadosRonda, repuestas, edadMedia, contagiadosRonda;
	int diasTranscurridos = 0;
	int muertosTotales = 0;
	int curadosTotales = 0;
	int contagiadosTotales = 0;
	float deci;
	int i, e, j;

	printf("STATUS: Creando población...\n");
	// CREAR POBLACION
	for(i=0; i<POBLACION; i++)
		personas[i] = crearPersona();

	edadMedia = mediaEdad(personas, POBLACION);

    printf("STATUS: PRIMER INFECTADO!\n");
	// PRIMER INFECTADO!
	int aux = rand()%POBLACION;
	personas[aux].estado = 1;
	contagiadosTotales++;

    printf("STATUS: Iniciando programa...\n");
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
			} else if(personas[i].pos[1] + personas[i].vel[0] <= 0){
				personas[i].pos[1] = 0;			// Ha llegado al limite.
				personas[i].vel[1] = rand()%5;
			} else {
				personas[i].pos[1] += personas[i].vel[1]; //Movimiento en el otro eje
				personas[i].vel[1] = rand()%10+(-5);
			}

//            printf("POSICION DE personas[%i] = {%i,%i}\n", i, personas[i].pos[0], personas[i].pos[1]);
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
						if(personas[e].pos[0] <= rangox+RADIO && personas[e].pos[0] >= rangox-RADIO){
							// SI ESTA DENTRO DEL RANGO DE EJE Y
							if(personas[e].pos[1] <= rangoy+RADIO && personas[e].pos[1] >= rangoy-RADIO){
								deci = (rand()%100) /100.0;
								if(deci>PROBRADIO){
                                    					printf("STATUS: NUEVO CONTAGIO!\n");
									personas[e].estado = 1;
									contagiadosRonda++;
								}
							}
						}
					}
				}

				// DECIDIR SI SE MUERE O SE RECUPERA
				deci = calcProb();
				//printf("La probabilidad de que muera es: %f sobre %f\n",deci,personas[i].probMuerte);
				if(deci <= personas[i].probMuerte){
                    			printf("STATUS: NUEVO FALLECIDO!\n");
					for(e=i; e<pobActual-1; e++)
						personas[e] = personas[e+1];
					muertosRonda++;
					contagiadosTotales--;
					pobActual--;
				} else {
					personas[i].diasContaminado++;
					if(personas[i].estado == 1 && personas[i].diasContaminado >= 5){
						personas[i].estado = 2;
					} else if(personas[i].estado == 2 && personas[i].diasContaminado >= 15){
                        			printf("STATUS: NUEVO SUPERVIVIENTE!\n");
						personas[i].estado = 3;
						curadosRonda++;
						contagiadosTotales--;
					}
				}
	        }
		}

		// ACTUALIZAR EDAD MEDIA
		edadMedia = mediaEdad(personas, pobActual);

		// RULAR TIEMPO
		diasTranscurridos++;

	    // ACTUALIZAR VALORES TOTALES
		contagiadosTotales += contagiadosRonda;
		curadosTotales += curadosRonda;
	    muertosTotales += muertosRonda;

	    // VISUALIZAR PROGRESO
	    printf("DIA %i: %i INFECTADOS (%i NUEVOS), %i RECUPERADOS (%i NUEVOS), %i FALLECIDOS (%i NUEVOS), %i NUEVAS PERSONAS. POBLACION: %i, EDAD MEDIA: %i\n",
	            diasTranscurridos, contagiadosTotales, contagiadosRonda, curadosTotales, curadosRonda, muertosTotales, muertosRonda, repuestas, pobActual, edadMedia);

        if(contagiadosTotales == 0) break;
        if(pobActual == 0) break;
	}

//    printf("STATUS: Liberando memoria alocada...\n");
	// LIBERAR MEMORIA AL ACABAR PROGRAMA
	free(personas);
    printf("STATUS: Fin del programa.\n");
}
