/*
 * persona.c
 *
 * Fichero recogiendo las funciones relacionadas
 * con el calculo de probabilidades.
 *
 * Abril de 2020
 */
 #include <mpi.h>
 #include <gsl/gsl_math.h>
 #include <gsl/gsl_rng.h>
 #include <sys/time.h>
 #include <stdlib.h>
 #include "probabilidad.h"
 #include "persona.h"

//Mediante esta funcion se podra calcular la desviacion a utilizar dependiendo de la media deseada.
int calculo_desv(int edad){
	int resul;
	if(edad>=50){
		resul=100-edad;
		resul=resul/2;
		return resul;
	}else{
		resul=edad/2;
		return resul;
	}
}

//Solo devolvera valores entre 0 y 100.
double rand_normal(double media, double stddev)
{//Box muller method
    static double n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached)
    {
        double x, y, r;
        do
        {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;
            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);
        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            double result = n1*stddev + media;
            n2_cached = 1;
	    if(result>100){
		return 100.0;
	    }else if(result<0){
		return 0.0;
	    }else{
            	return result;
            }
	}
    }
    else
    {
        n2_cached = 0;
	double aux=n2*stddev + media;
        if(aux>100){
		return 100.0;
	}else if(aux<0){
		return 0.0;
	}else{
		return n2*stddev + media;
    	}
     }
}

// CALCULAR UNA EDAD ENTRE 0 y 100
// (Par: int edad media de la poblacion)
int numeroRandom(int medEdad) {
    const gsl_rng_type * T;
    gsl_rng * r;
    gsl_rng_env_setup();
    struct timeval tv; // Para que no salgan todo el rato los mi$
    gettimeofday(&tv,0);
    unsigned long mySeed = tv.tv_sec + tv.tv_usec;
    T = gsl_rng_default; // Generar el setup
    r = gsl_rng_alloc (T);
    gsl_rng_set(r, mySeed);
    double sigma=1.0;
    double u = gsl_rng_uniform(r); // ! He creado la variable me$
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
