#ifndef ABB_H
#define ABB_H

#include <stdbool.h>
#include <stdlib.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE ESTRUCTURA
 * *****************************************************************/

typedef struct abb abb_t;

typedef int (*abb_comparar_clave_t) (const char *, const char *);
typedef void (*abb_destruir_dato_t) (void *);

typedef struct abb_iter abb_iter_t;

/* ******************************************************************
 *                    PRIMITIVAS DEL ABB
 * *****************************************************************/

//Crea el abb
abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato);
//Devuelve true si pudo guardar clave y dato, false en caso de algún error.
//Pre: El arbol fue creado.
//Pos: Clave y dato guardadas.
bool abb_guardar(abb_t *arbol, const char *clave, void *dato);
//Borra la clave junto al dato asociado del arbol, devuelve el dato. Devuelve NULL en caso de error.
//Pre: El arbol fue creado.
//Pos: Devuelve el dato asociado a la clave borrada.
void *abb_borrar(abb_t *arbol, const char *clave);
//Devuelve el dato asociado a la clave, NULL en caso de que no exista.
//Pre: El arbol fue creado.
//Pos: Devuelve el dato asociado a la clave, NULL en caso de que la clave no exista.
void *abb_obtener(const abb_t *arbol, const char *clave);
//Devuelve true si la clave pertenece al arbol, false en caso contrario.
//Pre: El arbol fue creado.
//Pos: Devuelve true si la clave pertenece, false en caso contrario.
bool abb_pertenece(const abb_t *arbol, const char *clave);
//Devuelve la cantidad de claves/datos en el arbol.
//Pre: El arbol fue creado.
//Pos: Devuelve la cantidad de claves/datos en el arbol.
size_t abb_cantidad(abb_t *arbol);
//Destruye el arbol.
//Pre: El arbol fue creado.
//Pos: El arbol fue destruido.
void abb_destruir(abb_t *arbol);

/********************************************************************
*					PRIMITIVA DEL ITERADOR INTERNO
* ******************************************************************/
//Aplica visitar() a las claves y datos del arbol.
void abb_in_order(abb_t *arbol, bool visitar(const char*, char*, char*), char* desde, char* hasta);

/* ******************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/
//Crea el iterador.
abb_iter_t *abb_iter_in_crear(const abb_t *arbol);
//Devuelve true en caso de poder avanzar, false en caso contrario.
bool abb_iter_in_avanzar(abb_iter_t *iter);
//Devuelve la clave actual.
const char *abb_iter_in_ver_actual(const abb_iter_t *iter);
//Devuelve true si está al final, false en caso contrario.
bool abb_iter_in_al_final(const abb_iter_t *iter);
//Destruye el iterador.
void abb_iter_in_destruir(abb_iter_t* iter);

#endif