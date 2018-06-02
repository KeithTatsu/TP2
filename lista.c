#include <stdlib.h>
#include "lista.h"

typedef struct nodo{
	void* dato;
	struct nodo* proximo;
}nodo_t;

typedef struct lista{
	nodo_t* primero;
	nodo_t* ultimo;
	size_t largo;
}lista_t;

typedef struct lista_iter{
	nodo_t* anterior;
	nodo_t* actual;
	lista_t* lista;
}lista_iter_t;

/* ******************************************************************
 *                    PRIMITIVAS DE LA LISTA
 * *****************************************************************/

nodo_t* crear_nodo(void* dato){

	nodo_t* nodo_nuevo = malloc(sizeof(nodo_t));

	if(!nodo_nuevo) return NULL;

	nodo_nuevo->dato = dato;
	nodo_nuevo->proximo = NULL;

	return nodo_nuevo;
}

lista_t *lista_crear(void){

	lista_t* lista = malloc(sizeof(lista_t));

	if(!lista) return NULL;

	lista->primero = NULL;
	lista->ultimo = NULL;
	lista->largo = 0;

	return lista;
}

bool lista_esta_vacia(const lista_t *lista){

	return (!lista->primero && !lista->ultimo && lista->largo == 0);
}

bool lista_insertar_primero(lista_t *lista, void *dato){

	nodo_t* nodo_nuevo = crear_nodo(dato);

	if(!nodo_nuevo) return false;

	if(lista_esta_vacia(lista)){
		lista->ultimo = nodo_nuevo;
		lista->ultimo->proximo = NULL;
	}

	nodo_nuevo->proximo = lista->primero;
	lista->primero = nodo_nuevo;

	(lista->largo)++;

	return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato){

	nodo_t* nodo_nuevo = crear_nodo(dato);

	if(!nodo_nuevo) return false;

	if(lista_esta_vacia(lista)){
		lista->primero = nodo_nuevo;
	}else{
		lista->ultimo->proximo = nodo_nuevo;
	}

	lista->ultimo = nodo_nuevo;
	lista->ultimo->proximo = NULL;

	(lista->largo)++;

	return true;
}

void *lista_borrar_primero(lista_t *lista){

	if(lista_esta_vacia(lista)) return NULL;

	nodo_t* nodo_aux = lista->primero;
	void* dato = nodo_aux->dato;

	lista->primero = lista->primero->proximo;

	free(nodo_aux);

	(lista->largo)--;

	if(lista->largo == 0){
		lista->ultimo = lista->primero = NULL;
	}

	return dato;
}

void *lista_ver_primero(const lista_t *lista){

	if(lista_esta_vacia(lista)) return NULL;

	return lista->primero->dato;
}

void *lista_ver_ultimo(const lista_t* lista){

	if(lista_esta_vacia(lista)) return NULL;

	return lista->ultimo->dato;
}

size_t lista_largo(const lista_t *lista){

	return lista->largo;
}

void lista_destruir(lista_t *lista, void destruir_dato(void *)){

	void* dato;

	while(!lista_esta_vacia(lista)){
		dato = lista_borrar_primero(lista);
		if(destruir_dato){
			destruir_dato(dato);
		}
	}

	free(lista);
}
/* ******************************************************************
 *                    PRIMITIVA DEL ITERADOR INTERNO
 * *****************************************************************/

void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra){

	nodo_t* actual = lista->primero;

	while(actual){
		if(!visitar(actual->dato, extra)){
			return;
		}
		actual = actual->proximo;
	}
}

/* ******************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

lista_iter_t *lista_iter_crear(lista_t *lista){

	lista_iter_t* iter = malloc(sizeof(lista_iter_t));

	if(!iter) return NULL;

	iter->anterior = NULL;
	iter->actual = lista->primero;
	iter->lista = lista;

	return iter;
}

bool lista_iter_avanzar(lista_iter_t *iter){

	if(lista_iter_al_final(iter)) return false;

	iter->anterior = iter->actual;
	iter->actual = iter->actual->proximo;

	return true;
}

void *lista_iter_ver_actual(const lista_iter_t *iter){

	if(lista_iter_al_final(iter)) return NULL;

	return iter->actual->dato;
}

bool lista_iter_al_final(const lista_iter_t *iter){

	return (iter->actual == NULL && iter->anterior == iter->lista->ultimo);
}

void lista_iter_destruir(lista_iter_t *iter){

	free(iter);
}

bool lista_iter_insertar(lista_iter_t *iter, void *dato){

	if(lista_iter_al_final(iter)){
		lista_insertar_ultimo(iter->lista, dato);
		iter->actual = iter->lista->ultimo;
		return true;
	}else if(iter->actual == iter->lista->primero){
		lista_insertar_primero(iter->lista, dato);
		iter->actual = iter->lista->primero;
		return true;
	}

	nodo_t* nodo_nuevo = crear_nodo(dato);

	if(!nodo_nuevo) return false;

	iter->anterior->proximo = nodo_nuevo;
	nodo_nuevo->proximo = iter->actual;
	iter->actual = nodo_nuevo;
	
	(iter->lista->largo)++;

	return true;
}

void *lista_iter_borrar(lista_iter_t *iter){

	if(lista_iter_al_final(iter)) return NULL;

	if(iter->anterior == NULL){
		iter->actual = iter->actual->proximo;
		return lista_borrar_primero(iter->lista);
	}

	iter->anterior->proximo = iter->actual->proximo;

	nodo_t* nodo_aux = iter->actual;
	void* dato = nodo_aux->dato;
	iter->actual = iter->anterior->proximo;

	if(iter->actual == NULL){
		iter->lista->ultimo = iter->anterior;
	}

	(iter->lista->largo)--;

	free(nodo_aux);

	return dato;
}