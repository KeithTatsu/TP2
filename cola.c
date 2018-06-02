#include "cola.h"
#include <stdlib.h>
#include <stdbool.h>

/* Definición del struct pila proporcionado por la cátedra.
 */

typedef struct nodo {
    void* datos;
    struct nodo* ant;
}nodo_t;

struct cola{
	nodo_t* prim;
	nodo_t* ult;
};
/* *****************************************************************
 *                    PRIMITIVAS DEL NODO
 * *****************************************************************/
nodo_t* nodo_crear(void){

	nodo_t* nodo = malloc(sizeof(nodo_t));

	if (nodo == NULL) {
		return NULL;
    }

    return nodo;
}
/* *****************************************************************
 *                    PRIMITIVAS DE LA COLA
 * *****************************************************************/
cola_t* cola_crear(void){

	cola_t* cola = malloc(sizeof(cola_t));

	if (cola == NULL){
        return NULL;
    }

    cola->prim = NULL;
    cola->ult = NULL;

    return cola;
}

void cola_destruir(cola_t *cola, void destruir_dato(void*)){
	
	void* auxiliar;	
	while(!cola_esta_vacia(cola)){
		auxiliar=cola_desencolar(cola);
		if(destruir_dato != NULL){
			destruir_dato(auxiliar);
		}
	}
	free(cola);
}

bool cola_esta_vacia(const cola_t *cola){
	return cola->prim == NULL;
}

bool cola_encolar(cola_t *cola, void* valor){

	nodo_t* nodo_nuevo=nodo_crear();
	
	if (nodo_nuevo == NULL){
		return false;
	}

	nodo_nuevo->datos=valor;
	nodo_nuevo->ant=NULL;

	if (cola->prim == NULL){

		cola->prim=nodo_nuevo;
	}
	else{
		cola->ult->ant=nodo_nuevo;
	}

	cola->ult=nodo_nuevo;
	return true;
}

void* cola_ver_primero(const cola_t *cola){

	if(cola->prim == NULL){
		return NULL;
	}
	return cola->prim->datos;
}

void* cola_desencolar(cola_t *cola){

	if (cola_esta_vacia(cola)){
		return NULL;
	}
	nodo_t* puntero_de_nodo;
	void* puntero_de_datos;
	
	puntero_de_nodo=cola->prim;	
	cola->prim=cola->prim->ant;
	
	if(cola_esta_vacia(cola)){
		cola->ult = NULL;
	}
	puntero_de_datos=puntero_de_nodo->datos;

	free(puntero_de_nodo);
	return puntero_de_datos;
}