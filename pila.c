#include "pila.h"

// OBJETO pila
struct pila {
    persona top;
    int capacity;
    persona* array;
};

struct pila* crearpila(int capacity) {
    persona esta;
    esta.edad = 200;
    esta.estado = 5;
    esta.diasContaminado = 200;
    esta.probMuertead = 1;
    esta.pos[0] = 50;
    esta.pos[1] = 50;
    esta.vel[0] = 50;
    esta.vel[1] = 50;

    struct pila* pila = (struct pila*) malloc(sizeof(struct pila));
    pila->capacity = capacity;
    pila->top = esta;
    pila->array = (persona*)malloc(stack->capacity * sizeof(persona));
    return pila;
}

int pilaVacia(struct pila* stack) {
    return stack->top.edad == 200;
}

void push(struct pila* stack, persona item) {
    stack->array[++stack->top] = item;
}

person pop(struct pila* stack) {
    return stack->array[stack->top--];
}
