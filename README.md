# SimuladorCoronavirus

## 1. Descripci´on del sistema
   El objetivo del sistema es simular las interacciones que se producen entre una poblaci´on de individuos.
En este caso, aunque se podr´ıa utilizar para diferentes aplicaciones, se va a simular la evoluci´on y los efectos
de un virus biol´ogico sobre una poblaci´on que se desplaza por un escenario compuesto por dos dimensiones.
	Debido a que no es posible hacer una representaci´on completamente realista de ning´un sistema f´ısico, por
la cantidad de variables que habr´ıa que controlar, se utilizar´an modelos que representan ciertas caracter´ısticas
de los componentes que se quiere modelar. Estos modelos est´an parametrizados, es decir, dependiendo de los
valores con los que se inicialicen se comportar´an de diferente forma. En particular en esta pr´actica se va a
modelar el comportamiento del virus COVID-19.

## 2. Modelado de los componentes
   El sistema est´a compuesto por tres elementos principales:
 - Persona/Virus
 - Poblaci´on
 - Escenario
	Para cada uno de estos elementos habr´a que construir un modelo el cual represente su comportamiento y
las interacciones que pueda tener con otros elementos del sistema.
### 2.1. Persona/Virus
   Debido a que un virus tiene que estar asociado a una persona para poder existir, el modelado de estos dos
componentes se har´a de manera conjunta. En particular, el modelado de cada persona se tiene que realizar
en base a dos conjuntos de caracter´ısticas. El primer conjunto sirve para modelar la infecci´on:
 - Edad: N´umero entre 0 y 100.
 - Estado: N´umero entre 0 y 3: (0) si el individuo esta sano, (1) si est´a infectado pero no tiene s´ıntomas,
(2) si est´a infectado y tiene s´ıntomas y (3) si se ha recuperado.
 - Probabilidad de morir una vez que se ha infectado.
	Por su parte, el segundo conjunto sirve para modelar la interacci´on entre los individuos y el escenario:
 - Posici´on: Vector de coordenadas p = {px, py} que representa la posici´on de un individuo en el escenario.
 - Velocidad: Vector de velocidad v = {vx, vy} que representa la direcci´on y la velocidad a la que el
individuo se est´a moviendo.
### 2.2. Poblaci´on
   La poblaci´on va a ser modelada utilizando un conjunto de personas cuyo tama˜no puede variar en el
tiempo. La interacci´on entre las personas se va a producir en un escenario 2D sobre el cual se pueden mover
libremente atendiendo a una serie de par´ametros con los que se inicializa el sistema. La poblaci´on hay que
representarla de manera din´amica, es decir, las personas que fallecen deben de ser eliminadas de la lista.
	Por el momento, una poblaci´on se va a definir utilizando los siguientes par´ametros:
Pr´actica MPI Sistemas de C´omputo Paralelo
 - Tama˜no de la poblaci´on: Indica el n´umero m´aximo de individuos que tiene la poblaci´on.
 - Media de edad de los individuos: Este par´ametro permite definir distintos tipos de poblaciones en
funci´on de la edad.
 - Radio de contagio: Los individuos contagiados (presenten o no s´ıntomas) pueden contagiar a otros que
se encuentren en un radio menor o igual a este par´ametro.
 - Probabilidad de contagio: Los individuos que se encuentren dentro del radio de contagio pueden ser o
no contagiados en funci´on de este par´ametro. Sirve para modelar si se toman medidas preventivas o no.
 - Periodo de incubaci´on: Tiempo que debe pasar desde que un individuo se contagia hasta que presenta
s´ıntomas.
 - Periodo de recuperaci´on: Tiempo que debe pasar desde que un individuo presenta s´ıntomas hasta que
se recupera. Durante este periodo de tiempo un individuo puede morir en funci´on de la edad.
 - Probabilidad de cambio de velocidad y direcci´on: Los individuos se mueven libremente por el escenario
y se podr´a cambiar su velocidad y direcci´on de manera aleatoria.
### 2.3. Escenario
   El escenario representa el mundo sobre el que los individuos se mueven e interact´uan. En una primera
aproximaci´on, vamos a representarlo como una malla (superficie) 2D de tama˜no variable. Este tama˜no es un
par´ametro del sistema y no se puede modificar durante la simulaci´on. En este escenario, cada individuo sera
representado como un punto, es decir, si pensamos el escenario como si fuera una pantalla, cada individuo se
representar´a con un pixel.
	En la Figura 1 se puede ver una representaci´on del estado del sistema en la que coexisten individuos
sanos (verde), contagiados asintom´aticos (naranja), contagiados sintom´aticos (rojo) y recuperados (azul) en
un escenario en dos dimensiones.

## 3. Simulaci´on del sistema
   La simulaci´on del sistema ser´a de tipo time-driven. Esto quiere decir que en cada unidad de tiempo hay ´
que calcular todas la interacciones que se producen entre los individuos de la poblaci´on, es decir, hay que
actualizar todos los componentes del sistema y recoger y almacenar las m´etricas necesarias..
### 3.1. Inicializaci´on del sistema
   En esta etapa hay que inicializar todos los par´ametros de la simulaci´on. En particular hay que definir el
tama˜no del escenario 2D, el n´umero de individuos de la simulaci´on y los par´ametros asociados con cada individuo. As´ı mismo, hay que definir la duraci´on de la simulaci´on en unidades de tiempo. Todas las simulaciones
comenzar´an eligiendo de manera aleatoria al paciente 0.
### 3.2. Calcul´o de las interacciones
   Las interacciones que se producen entre los diferentes individuos y el escenario se pueden clasificar en tres
tipos:
 - Estado de los individuos: Hay que controlar para cada individuo el estado en el que se encuentra
teniendo en cuenta el tiempo transcurrido entre los diferentes estados. En particular, hay que controlar
si un individuo est´a infectado pero asintom´atico, cuando se convierte en sintom´atico, si un individuo
esta infectado y sintom´atico, cuando se recupera o, en su caso, cuando muere (usando la probabilidad
de muerte por edad).
 - Propagaci´on: Para cada individuo hay que controlar su estado y el radio de contagio. En caso de que
est´e contagiado y haya individuos sanos dentro de su radio de acci´on, hay que determinar si estos
individuos se contagiar´an usando la probabilidad de contagio.
 - Movimiento: Hay que calcular la nueva posici´on de todos los individuos en funci´on del vector posici´on
y el vector velocidad. La direcci´on y la velocidad en la que se mueven los individuos puede ser alterada
de manera aleatoria. Hay que tener en cuenta que las dimensiones de nuestro escenario 2D son finitas
por lo que habr´a que controlar que los individuos no se salgan del escenario.
### 3.3. Recogida de m´etricas
   Dentro de la informaci´on que el sistema debe guardar se encuentran las m´etricas que utilizaremos para
registrar la evoluci´on del sistema. El objetivo de la simulaci´on es medir la evoluci´on del n´umero de personas
sanas, contagiadas (asintom´aticas y sintom´aticas) y recuperadas. As´ı mismo, nos interesa conocer la posici´on
en el escenario 2D de cada persona, as´ı como una m´etrica utilizada en epidemiolog´ıa, llamada n´umero reproductivo b´asico (R0), la cual indica la capacidad de contagio, es decir, el n´umero de personas que es capaz de
contagiar un paciente infectado.
	Debido a que la cantidad de informaci´on que se requiere registrar en simulaciones de larga duraci´on puede
ser demasiado grande, se deber´a realizar una simulaci´on por batches, es decir, se seleccionar´an periodos de
tiempo y s´olo se guardar´an las medias de los valores de las m´etricas en esos intervalos. Si el valor del batch
es 1, se guardar´a toda la informaci´on.

## Apendices
### A. Uso de distribuciones de probabilidad
   La definici´on de varias caracter´ısticas del sistema depende de n´umeros aleatorios lo que nos va a permitir
modelar diferentes escenarios. El modelado de estos valores se realiza utilizando distribuciones de probabilidad. Normalmente para generar n´umeros aleatorios utilizamos la distribuci´on uniforme (rand()) pero en este
caso, para modelar algunos escenarios mas complejos, deber´ıamos utilizar otras. Por ejemplo, para modelar
la probabilidad de muerte de un individuo podr´ıamos implementar una funci´on que calculara la probabilidad
en base a los siguientes datos:
 - < 50 a˜nos: 0.004
 - ≥ 50 y < 60 a˜nos: 0.013
 - ≥ 60 y < 70 a˜nos: 0.036
 - ≥ 70 y < 80 a˜nos: 0.08
 - ≥ 80 a˜nos: 0.148
	En este ejemplo hemos utilizado una distribuci´on ad-hoc pero podr´ıamos utilizar otras como por ejemplo
una distribuci´on normal para modelar la edad de la poblaci´on. Otro ejemplo podr´ıa ser el modelado del
per´ıodo de incubaci´on del virus que var´ıa entre 1 y 14 d´ıas siendo la media 5 o 6.
