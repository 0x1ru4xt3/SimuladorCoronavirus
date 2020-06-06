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

 struct envio{
 	int capacidad;
 	struct persona *personas;
 };

 typedef struct persona Persona;
 typedef struct envio Envio;

 // CREAR PERSONA
 struct persona crearPersona(int edadMedia, int escAncho, int escAlto,int dev,int posX, int posY);

 // MOVER PERSONA y CAMBIAR VELOCIDAD PARA LA SIGUIENTE RONDA
 // (Par: struct persona)
 int moverPersona(Persona *pers, int escAncho, int escAlto, int cordX, int cordY, int limiteAncho, int limiteAlto);

 // DECISION DE INFECTAR UNA PERSONA por RADIO DE CONTAGIADO
 // (Par: struct persona, ints radio del infectado)
 int infecPersona(Persona *per, int rangox, int rangoy, int radio, float probRadio);

 // DECISION DE MUERTE DE UNA PERSONA
 // (Par: struct persona)
 int matarPersona(Persona *per);

// CREAR DATATYPE PARA MANDAR
void crearTipoPersona(Persona *pers, MPI_Datatype *MPI_DATOS);

// CREAR DATATYPE PARA MANDAR
void crearTipoEnvio(Envio *envio, MPI_Datatype *MPI_DATOS, MPI_Datatype *persona);
