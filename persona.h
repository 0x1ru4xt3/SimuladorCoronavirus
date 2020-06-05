/*
 * persona.h
 * Fichero recogiendo las funciones relacionadas con personas.
 * Abril de 2020
 */

 // OBJETO PERSONA
 struct persona {
 	int edad;
 	int estado;
 	int diasContaminado;
 	float probMuerte;
 	int pos[2];
 	int vel[2];
 };

 typedef struct persona Persona;

 // CREAR PERSONA
 struct persona crearPersona(int edadMedia, int escAncho, int escAlto,int dev,int posX, int posY){ 

 // MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
 // (Par: struct persona)
 void moverPersona(Persona *pers, int escAncho, int escAlto, int cordX, int cordY, int limiteAncho, int limiteAlto){

 // DECISION DE INFECTAR UNA PERSONA por RADIO DE CONTAGIADO
 // (Par: struct persona, ints radio del infectado)
 int infecPersona(Persona *per, int rangox, int rangoy, int radio, float probRadio);

 // DECISION DE MUERTE DE UNA PERSONA
 // (Par: struct persona)
 int matarPersona(Persona *per);
