/*
 * persona.c
 *
 * Fichero recogiendo las funciones relacionadas
 * con el calculo de probabilidades.
 *
 * Abril de 2020
 */
 #include <gsl/gsl_math.h>
 #include <gsl/gsl_rng.h>
 #include <sys/time.h>

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
