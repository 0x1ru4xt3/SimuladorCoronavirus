# Simulador COVID-19

## 1. Descripción del sistema
El objetivo del sistema es simular las interacciones que se producen entre una población de individuos.
En este caso, aunque se podría utilizar para diferentes aplicaciones, se va a simular la evolución y los efectos
de un virus biológico sobre una población que se desplaza por un escenario compuesto por dos dimensiones.

Debido a que no es posible hacer una representación completamente realista de ningún sistema físico, por
la cantidad de variables que habría que controlar, se utilizarán modelos que representan ciertas características
de los componentes que se quiere modelar. Estos modelos están parametrizados, es decir, dependiendo de los
valores con los que se inicialicen se comportarán de diferente forma. En particular en esta práctica se va a
modelar el comportamiento del virus COVID-19.

## 2. Modelado de los componentes
El sistema está compuesto por tres elementos principales:
 - Persona/Virus
 - Poblacion
 - Escenario

Para cada uno de estos elementos habrá que construir un modelo el cual represente su comportamiento y
las interacciones que pueda tener con otros elementos del sistema.
### 2.1. Persona/Virus
Debido a que un virus tiene que estar asociado a una persona para poder existir, el modelado de estos dos
componentes se hará de manera conjunta. En particular, el modelado de cada persona se tiene que realizar
en base a dos conjuntos de caracterısticas. El primer conjunto sirve para modelar la infección:
 - Edad: Número entre 0 y 100.
 - Estado: Número entre 0 y 3: (0) si el individuo esta sano, (1) si está infectado pero no tiene síntomas,
(2) si esta infectado y tiene síntomas y (3) si se ha recuperado.
 - Probabilidad de morir una vez que se ha infectado.

Por su parte, el segundo conjunto sirve para modelar la interaccion entre los individuos y el escenario:
 - Posicion: Vector de coordenadas p = {px, py} que representa la posicion de un individuo en el escenario.
 - Velocidad: Vector de velocidad v = {vx, vy} que representa la direccion y la velocidad a la que el
individuo se esta moviendo.
### 2.2. Poblacion
La poblacion va a ser modelada utilizando un conjunto de personas cuyo tamaño puede variar en el
tiempo. La interaccion entre las personas se va a producir en un escenario 2D sobre el cual se pueden mover
libremente atendiendo a una serie de parametros con los que se inicializa el sistema. La poblacion hay que
representarla de manera dinamica, es decir, las personas que fallecen deben de ser eliminadas de la lista.

Por el momento, una poblacion se va a definir utilizando los siguientes parametros:
Practica MPI Sistemas de Computo Paralelo
 - Tamaño de la poblacion: Indica el numero maximo de individuos que tiene la poblacion.
 - Media de edad de los individuos: Este parametro permite definir distintos tipos de poblaciones en
funcion de la edad.
 - Radio de contagio: Los individuos contagiados (presenten o no sıntomas) pueden contagiar a otros que
se encuentren en un radio menor o igual a este parametro.
 - Probabilidad de contagio: Los individuos que se encuentren dentro del radio de contagio pueden ser o
no contagiados en funcion de este parametro. Sirve para modelar si se toman medidas preventivas o no.
 - Periodo de incubacion: Tiempo que debe pasar desde que un individuo se contagia hasta que presenta
sıntomas.
 - Periodo de recuperacion: Tiempo que debe pasar desde que un individuo presenta sıntomas hasta que
se recupera. Durante este periodo de tiempo un individuo puede morir en funcion de la edad.
 - Probabilidad de cambio de velocidad y direccion: Los individuos se mueven libremente por el escenario
y se podra cambiar su velocidad y direccion de manera aleatoria.
### 2.3. Escenario
   El escenario representa el mundo sobre el que los individuos se mueven e interactuan. En una primera
aproximacion, vamos a representarlo como una malla (superficie) 2D de tamaño variable. Este tamaño es un
parametro del sistema y no se puede modificar durante la simulacion. En este escenario, cada individuo sera
representado como un punto, es decir, si pensamos el escenario como si fuera una pantalla, cada individuo se
representara con un pixel.
	Se puede realizar una representacion del estado del sistema en la que coexisten individuos
sanos (verde), contagiados asintomaticos (naranja), contagiados sintomaticos (rojo) y recuperados (azul) en
un escenario en dos dimensiones.

## 3. Simulacion del sistema
   La simulacion del sistema sera de tipo time-driven. Esto quiere decir que en cada unidad de tiempo hay 
que calcular todas la interacciones que se producen entre los individuos de la poblacion, es decir, hay que
actualizar todos los componentes del sistema y recoger y almacenar las metricas necesarias..
### 3.1. Inicializacion del sistema
   En esta etapa hay que inicializar todos los parametros de la simulacion. En particular hay que definir el
tamaño del escenario 2D, el numero de individuos de la simulacion y los parametros asociados con cada individuo. Ası mismo, hay que definir la duracion de la simulacion en unidades de tiempo. Todas las simulaciones
comenzaran eligiendo de manera aleatoria al paciente 0.
### 3.2. Calculo de las interacciones
   Las interacciones que se producen entre los diferentes individuos y el escenario se pueden clasificar en tres
tipos:
 - Estado de los individuos: Hay que controlar para cada individuo el estado en el que se encuentra
teniendo en cuenta el tiempo transcurrido entre los diferentes estados. En particular, hay que controlar
si un individuo esta infectado pero asintomatico, cuando se convierte en sintomatico, si un individuo
esta infectado y sintomatico, cuando se recupera o, en su caso, cuando muere (usando la probabilidad
de muerte por edad).
 - Propagacion: Para cada individuo hay que controlar su estado y el radio de contagio. En caso de que
este contagiado y haya individuos sanos dentro de su radio de accion, hay que determinar si estos
individuos se contagiaran usando la probabilidad de contagio.
 - Movimiento: Hay que calcular la nueva posicion de todos los individuos en funcion del vector posicion
y el vector velocidad. La direccion y la velocidad en la que se mueven los individuos puede ser alterada
de manera aleatoria. Hay que tener en cuenta que las dimensiones de nuestro escenario 2D son finitas
por lo que habra que controlar que los individuos no se salgan del escenario.
### 3.3. Recogida de metricas
   Dentro de la informacion que el sistema debe guardar se encuentran las metricas que utilizaremos para
registrar la evolucion del sistema. El objetivo de la simulacion es medir la evolucion del numero de personas
sanas, contagiadas (asintomaticas y sintomaticas) y recuperadas. Ası mismo, nos interesa conocer la posicion
en el escenario 2D de cada persona, ası como una metrica utilizada en epidemiologıa, llamada numero reproductivo basico (R0), la cual indica la capacidad de contagio, es decir, el numero de personas que es capaz de
contagiar un paciente infectado.
	Debido a que la cantidad de informacion que se requiere registrar en simulaciones de larga duracion puede
ser demasiado grande, se debera realizar una simulacion por batches, es decir, se seleccionaran periodos de
tiempo y solo se guardaran las medias de los valores de las metricas en esos intervalos. Si el valor del batch
es 1, se guardara toda la informacion.

## Apendices
### A. Uso de distribuciones de probabilidad
   La definicion de varias caracterısticas del sistema depende de numeros aleatorios lo que nos va a permitir
modelar diferentes escenarios. El modelado de estos valores se realiza utilizando distribuciones de probabilidad. Normalmente para generar numeros aleatorios utilizamos la distribucion uniforme (rand()) pero en este
caso, para modelar algunos escenarios mas complejos, deberıamos utilizar otras. Por ejemplo, para modelar
la probabilidad de muerte de un individuo podrıamos implementar una funcion que calculara la probabilidad
en base a los siguientes datos:
 - < 50 años: 0.004
 - ≥ 50 y < 60 años: 0.013
 - ≥ 60 y < 70 años: 0.036
 - ≥ 70 y < 80 años: 0.08
 - ≥ 80 años: 0.148
	En este ejemplo hemos utilizado una distribucion ad-hoc pero podrıamos utilizar otras como por ejemplo
una distribucion normal para modelar la edad de la poblacion. Otro ejemplo podrıa ser el modelado del
perıodo de incubacion del virus que varıa entre 1 y 14 dıas siendo la media 5 o 6.

### B. Puesta en marcha del programa
   Para ejecutar el programa se le pasan como parámetro tiempo que se va a simular, tamaño del escenario (ancho y alto), el radio de contagio, la probabilidad de contagio dentro del radio y el número de personas que se tendrán en cuenta.
	>./main <tiempoASimular> <tamanoAncho> <tamanoAlto> <radio> <probRadio> <poblacion>
	
   El programa utiliza el paquete [GSL](https://www.gnu.org/software/gsl/doc/html/), por lo que es necesario que el paquete esté instalado en el sistema. Para compilar con gcc:
   	>gcc main.c -o main -lgsl
	

Desarrollado con @iBiri99 
