#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>
#include <stdlib.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

struct lista;
typedef struct lista lista_t;

/* ******************************************************************
 *                    PRIMITIVAS DE LA LISTA
 * *****************************************************************/

//Crea una lista.
//Pos: Devuelve una lista vacía.
lista_t *lista_crear(void);

//Devuelve true si la lista está vacía, false en caso contrario.
//Pre: La lista fue creada.
bool lista_esta_vacia(const lista_t *lista);

//Inserta un elemento en la primera posición de la lista, devuelve true
//si se pudo insertar, false en caso de algún error.
//Pre: La lista fue creada.
//Pos: Se insertó un nuevo elemento en la primera posición.
bool lista_insertar_primero(lista_t *lista, void *dato);

//Inserta un elemento en la última posición de la lista, devuelve true
//si se pudo insertar, false en caso de algún error.
//Pre: La lista fue creada.
//Pos: Se insertó un nuevo elemento en la última posición.
bool lista_insertar_ultimo(lista_t *lista, void *dato);

//Elimina el primer elemento de la lista y devuelve su valor, si está
//vacía devuelve NULL.
//Pre: Lista fue creada.
//Pos: Devuelve el valor del elemento borrado en caso de que la lista
//no esté vacía.
void *lista_borrar_primero(lista_t *lista);

//Obtiene el valor del primer elemento. Si la lista no tiene elementos
//se devuelve NULL.
//Pre: Lista fue creada.
//Pos: Se devolvió el primer elemento de la lista, si no está vacía.
void *lista_ver_primero(const lista_t *lista);

//Obtiene el valor del último elemento. Si la lista no tiene elementos
//se devuelve NULL.
//Pre: Lista fue creada.
//Pos: Se devolvió el último elemento de la lista, si no está vacía.
void *lista_ver_ultimo(const lista_t* lista);

//Obtiene el valor del largo de la lista.
//Pre: Lista fue creada.
size_t lista_largo(const lista_t *lista);

//Se destruye la lista. Si se recibe la función destruir_dato por
//parámetro, para cada uno de los elementos de la lista llama a
//destruir_dato.
//Pre: Lista fue creada. destuir_dato es una función capaz de destruir
//los datos de la lista, o NULL en caso de que no se la necesite.
//Pos: Se eliminaron todos los elementos de la lista.
void lista_destruir(lista_t *lista, void destruir_dato(void *));

/* ******************************************************************
 *                    PRIMITIVA DEL ITERADOR INTERNO
 * *****************************************************************/

//Se aplica la función visitar a los elementos de la lista.
//Pre: Lista creada.
//Pos: Se aplicó 'visitar()' a los elementos de la lista.
void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra);

/* ******************************************************************
 *                    PRIMITIVAS DEL ITERADOR EXTERNO
 * *****************************************************************/

struct lista_iter;
typedef struct lista_iter lista_iter_t;

//Crea un iterador de la lista.
//Pos: Devuelve un iterador.
lista_iter_t *lista_iter_crear(lista_t *lista);

//Avanza una posición en la lista, devuelve true si avanza a una posición válida,
//false en caso de algún error.
//Pre: Lista e iterador fueron creados.
//Pos: Avanzó una posición en la lista, false en caso de algún error.
bool lista_iter_avanzar(lista_iter_t *iter);

//Devuelve el valor del elemento actual si la lista no está vacía y el iterador no
//está al final.
//Pre: Lista e iterador creados.
//Pos: Devuelve el valor del elemento actual, NULL en caso de algún error.
void *lista_iter_ver_actual(const lista_iter_t *iter);

//Devuelve true o false depenediendo de si está al final o no.
//Pre: Lista e iterador creados.
//Pos: Devuelve true si llegó al final, false en caso contrario.
bool lista_iter_al_final(const lista_iter_t *iter);

//Destruye el iterador.
//Pre: Iterador fue creado.
//Pos: Iterador destruido.
void lista_iter_destruir(lista_iter_t *iter);

//Inserta un nuevo elemento en la posición actual, devuelve false en caso de error.
//Pre: Lista e iterador creados.
//Pos: Devuelve true si pudo insertar el elemento satisfactoriamente, false en caso de algún error.
bool lista_iter_insertar(lista_iter_t *iter, void *dato);

//Borra el elemento en la posición actual y se devuelve su valor. Devuelve NULL en caso
//de que la lista esté vacía.
//Pre: Lista e iterador creados.
//Pos: Devuelve el valor del elemento borrado, si la lista está vacía devuelve NULL.
void *lista_iter_borrar(lista_iter_t *iter);

/* *****************************************************************
 *                      PRUEBAS UNITARIAS
 * *****************************************************************/
void pruebas_lista_alumno(void);

#endif // LISTA_H