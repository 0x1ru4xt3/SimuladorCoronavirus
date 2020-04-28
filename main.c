//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <sys/time.h>

#define SEED      0

// PROBABILIDADES POR EDAD
#define EDAD1 0.004  // < 50
#define EDAD2 0.013  // 50 - 60
#define EDAD3 0.036  // 60 - 70
#define EDAD4 0.080  // 70 - 80
#define EDAD5 0.148  // > 80

int TIEMPO;
int ESCHEIGHT;
int ESCWIDTH;
int RADIO;
float PROBRADIO;
int POBLACION;
int EDADMEDIA;
int BATX;

// OBJETO PERSONA
struct persona {
	int edad;
	int estado;
	int diasContaminado;
	float probMuerte;
	int pos[2];
	int vel[2];
};

// CALCULAR UNA EDAD ENTRE 0 y 100
// (Par: int edad media de la poblacion)
int numeroRandom(int medEdad) {
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    struct timeval tv; // Para que no salgan todo el rato los mismos valores se coge el tiempo actual y se utliza como seed,
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    T = gsl_rng_default; // Generar el setup
    r = gsl_rng_alloc (T);
    gsl_rng_set(r, mySeed);
    double u = gsl_rng_uniform(r); // ! He creado la variable medEdad que es la metida como parametro al programa// el numero que genera es entre 0 y 1, multiplicamos por 100.
    gsl_rng_free (r);
    double aux=u*100;
    int auxa=aux;
    return auxa;

}

// CALCULAR NUMERO ENTRE 0 y 1 (DECISION DE MUERTE)
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
	per.edad = numeroRandom(EDADMEDIA);
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

// MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
// (Par: struct persona)
void moverPersona(struct persona pers){
	// SE CONTROLA PRIMERO UN EJE, DESPUES EL OTRO, SE CONTROLA
	// QUE NO SE SALGA DE LOS LIMITES DEL ESCENARIO, Y SE ELIGE
	// LA VELOCIDAD DE LA SIGUIENT RONDA EN BASE A SU POSICION:
	// SI ESTA EN EL BORDE REBOTA, SI NO SE MUEVE LIBREMENTE :)

	if(pers.pos[0] + pers.vel[0] >= ESCHEIGHT){
		pers.pos[0] = ESCHEIGHT;
		pers.vel[0] = rand()%5+(-5);
	} else if(pers.pos[0] + pers.vel[0] <= 0){
		pers.pos[0] = 0;
		pers.vel[0] = rand()%5;
	} else {
		pers.pos[0] += pers.vel[0];
		pers.vel[0] = rand()%10+(-5);
	}

	if(pers.pos[1] + pers.vel[1] >= ESCWIDTH){
		pers.pos[1] = ESCWIDTH;
		pers.vel[1] = rand()%5+(-5);
	} else if(pers.pos[1] + pers.vel[0] <= 0){
		pers.pos[1] = 0;
		pers.vel[1] = rand()%5;
	} else {
		pers.pos[1] += pers.vel[1];
		pers.vel[1] = rand()%10+(-5);
	}
}

// DECISION DE INFECTAR UNA PERSONA por RADIO DE CONTAGIADO
// (Par: struct persona, ints radio del infectado)
int infecPersona(struct persona per, int rangox, int rangoy){
	// SI NO ESTA INFECTADO y NO LO HA ESTADO
	if(per.estado == 0){
		// SI ESTA DENTRO DEL RANGO DE EJE X DEL INFECTADO
		if(per.pos[0] <= rangox+RADIO && per.pos[0] >= rangox-RADIO){
			// SI ESTA DENTRO DEL RANGO DE EJE Y DEL INFECTADO
			if(per.pos[1] <= rangoy+RADIO && per.pos[1] >= rangoy-RADIO){
				float deci = (rand()%100) /100.0;
				if(deci>PROBRADIO){
					per.estado = 1;
					return 1;
				}
			}
		}
	}
	return 0;
}

// DECISION DE MUERTE DE UNA PERSONA
// (Par: struct persona)
int matarPersona(struct persona per){
	float deci = calcProb();
	if(deci <= per.probMuerte)
		return 0;
	else {
		per.diasContaminado++;
		if(per.estado == 1 && per.diasContaminado >= 5){
			per.estado = 2;
			return 1;
		} else if(per.estado == 2 && per.diasContaminado >= 15){
			per.estado = 3;
			return 2;
		}
	}
}

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

	TIEMPO 		= atoi(argv[1]);
	ESCHEIGHT 	= atoi(argv[2]);
	ESCWIDTH 	= atoi(argv[3]);
	RADIO 		= atoi(argv[4]);
	PROBRADIO 	= atof(argv[5]);
	POBLACION 	= atoi(argv[6]);
	EDADMEDIA 	= atoi(argv[7]);
	BATX 		= atoi(argv[8]);

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
	posic=fopen("historialposic.txt","w+");
	dias=fopen("historialdias.txt", "w+");

	// INICIALIZACION ARRAY PERSONAS
	struct persona *personas;
    personas  = malloc(POBLACION*sizeof(struct persona));

	// IMPRESION DE VARIABLES INTRODUCIDAS POR PARAMETRO
	printf("STATUS: DATOS INTRODUCIDOS: \n\tTIEMPO %d\n\t POBLACION: %d\n\tANCHO ESC: %d  ALTO_ESC: %d\n\tRADIO CONTAGIO: %d  PROB DE CONTAGIO RADIO: %f\n",
			TIEMPO, POBLACION, ESCHEIGHT, ESCWIDTH, RADIO, PROBRADIO);

	// CREAR POBLACION
	printf("STATUS: Creando población...\n");
	for(i=0; i<POBLACION; i++)
		personas[i] = crearPersona();

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
			moverPersona(personas[i]);
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
					contagiadosRonda += infecPersona(personas[e], rangox, rangoy);

				// DECIDIR SI SE MUERE O SE RECUPERA
				if(matarPersona(personas[i]) == 0){			// SE MUERE
					for(e=i; e<pobActual-1; e++)
						personas[e] = personas[e+1];
					muertosRonda++;
					contagiadosTotales--;
					pobActual--;
				} else if(matarPersona(personas[i]) == 2){	// SE CURA
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
