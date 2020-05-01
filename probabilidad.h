/*
 * probabilidad.h
 *
 * Fichero recogiendo las funciones relacionadas
 * con el calculo de probabilidades.
 *
 * Abril de 2020
 */

//Funcion para calcular la desviacion para que no salgan ni edades negativas ni de mas de 100.
int calculo_desv(int edad);


double rand_normal(double media, double stddev);
 // CALCULAR UNA EDAD ENTRE 0 y 100
 // (Par: int edad media de la poblacion)
 int numeroRandom(int medEdad) ;

 // CALCULAR NUMERO ENTRE 0 y 1 (DECISION DE MUERTE)
 float calcProb();
