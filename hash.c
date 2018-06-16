#include <stdlib.h>
#include <string.h>
#include "lista.h"
#include "hash.h"
#include "lista.h"

#define TAM_INICIAL 103
#define TAM_AGRANDAR 2
#define TAM_ACHICAR 4
#define PORCENTAJE_AGRANDAR 70
#define PORCENTAJE_ACHICAR 20
#define RELACION_LISTAS_TAM 20
#define POS_INICIAL 0

typedef struct campo{
	char* clave;
	void* dato;
}campo_t;

struct hash{
	lista_t** tabla;
	size_t tamanio;
	size_t ocupados;
	hash_destruir_dato_t destruir;
};

struct hash_iter{
	size_t pos;
	const hash_t* hash;
	lista_iter_t* iter_actual;
};

size_t hash_f(const char *clave, size_t tam){

	size_t hash_v = tam, i = 0;
	int c;

	while(clave[i] != '\0'){
		c = clave[i];
		hash_v = ((hash_v << 5) + hash_v) + c; // hash * 33 + c 
		i++;
	}

	return hash_v%tam;
}

size_t calcular_porcentaje(size_t ocupados, size_t tamanio){

	return (ocupados/tamanio*RELACION_LISTAS_TAM)*100;
}

campo_t* crear_campo(const char* clave, void* dato){

	campo_t* campo_nuevo = malloc(sizeof(campo_t));

	if(!campo_nuevo) return NULL;

	char* clave_aux = malloc((strlen(clave)+1)*sizeof(char));

	if(!clave_aux){
		free(campo_nuevo);
		return NULL;
	}

	strcpy(clave_aux, clave);
	campo_nuevo->clave = clave_aux;
	campo_nuevo->dato = dato;

	return campo_nuevo;
}

campo_t* buscar_clave(lista_t* tabla, const char* clave){

	lista_iter_t* iter_lista = lista_iter_crear(tabla);

	if(!iter_lista) return NULL;

	while(!lista_iter_al_final(iter_lista)){
		campo_t* campo_actual = lista_iter_ver_actual(iter_lista);
		if(strcmp(campo_actual->clave, clave) == 0){
			lista_iter_destruir(iter_lista);
			return campo_actual;
		}
		lista_iter_avanzar(iter_lista);
	}
	lista_iter_destruir(iter_lista);	

	return NULL;
}

bool insertar_existente(lista_t* tabla, const char* clave, void* dato, hash_destruir_dato_t destruir){

	campo_t* campo = buscar_clave(tabla, clave);

	if(destruir){
		void* dato_aux = campo->dato;
		destruir(dato_aux);
	}

	campo->dato = dato;

	return true;
}

bool insertar_en_tabla(lista_t* tabla, const char* clave, void* dato){

	campo_t* campo = crear_campo(clave, dato);

	if(!campo) return false;

	return lista_insertar_ultimo(tabla, campo);
}

bool pasar_datos(hash_t* hash, lista_t** tabla_nueva, size_t tam_nuevo){

	for(size_t i = 0; i < hash->tamanio; i++){
		while(!lista_esta_vacia(hash->tabla[i])){
			campo_t* campo = lista_borrar_primero(hash->tabla[i]);
			size_t pos = hash_f(campo->clave, tam_nuevo);
			if(!insertar_en_tabla(tabla_nueva[pos], campo->clave, campo->dato)){
				for(size_t j = 0; j < tam_nuevo; j++){
					if(tabla_nueva[j] != NULL){
						lista_destruir(tabla_nueva[j], hash->destruir);
					}
				}
				return false;
			}
			free(campo->clave);
			free(campo);
		}
		lista_destruir(hash->tabla[i], NULL);
	}

	return true;
}

bool inicializar_tabla(lista_t** tabla, size_t tam){

	for(size_t i = 0; i < tam; i++){
		tabla[i] = lista_crear();
		if(!tabla[i]){
			for(size_t j = 0; j < i; j++){
				lista_destruir(tabla[j], NULL);
			}
			return false;
		}
	}

	return true;
}

bool redimensionar_hash(hash_t* hash, size_t tam_nuevo){

	lista_t** tabla_nueva = malloc(tam_nuevo*sizeof(lista_t*));

	if(!tabla_nueva) return false;

	if(!inicializar_tabla(tabla_nueva, tam_nuevo)){
		free(tabla_nueva);
		return false;
	}

	if(!pasar_datos(hash, tabla_nueva, tam_nuevo)){
		free(tabla_nueva);
		return false;
	}

	free(hash->tabla);

	hash->tamanio = tam_nuevo;
	hash->tabla = tabla_nueva;

	return true;
}

campo_t* _borrar_elemento(lista_t* tabla, const char* clave){

	lista_iter_t* iter_lista = lista_iter_crear(tabla);

	if(!iter_lista) return NULL;

	while(!lista_iter_al_final(iter_lista)){
		campo_t* campo_actual = lista_iter_ver_actual(iter_lista);
		if(strcmp(campo_actual->clave, clave) == 0){
			campo_actual = lista_iter_borrar(iter_lista);
			lista_iter_destruir(iter_lista);

			return campo_actual;
		}
		lista_iter_avanzar(iter_lista);
	}

	lista_iter_destruir(iter_lista);

	return NULL;
}

hash_t *hash_crear(hash_destruir_dato_t destruir_dato){

	hash_t* hash_nuevo = malloc(sizeof(hash_t));

	if(!hash_nuevo) return NULL;

	hash_nuevo->tabla = malloc(TAM_INICIAL*sizeof(lista_t*));

	if(!hash_nuevo->tabla){
		free(hash_nuevo);
		return NULL;
	}

	if(!inicializar_tabla(hash_nuevo->tabla, TAM_INICIAL)){
		free(hash_nuevo);
		return NULL;
	}

	hash_nuevo->tamanio = TAM_INICIAL;
	hash_nuevo->ocupados = 0;
	hash_nuevo->destruir = destruir_dato;

	return hash_nuevo;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

	if(calcular_porcentaje(hash->ocupados, hash->tamanio) >= PORCENTAJE_AGRANDAR){
		size_t tam_nuevo = hash->tamanio*TAM_AGRANDAR;
		if(!redimensionar_hash(hash, tam_nuevo)) return false;
	}

	size_t pos = hash_f(clave, hash->tamanio);

	if(hash_pertenece(hash, clave)){
		insertar_existente(hash->tabla[pos], clave, dato, hash->destruir);
		return true;
	}

	if(!insertar_en_tabla(hash->tabla[pos], clave, dato)) return false;

	hash->ocupados++;

	return true;
}

void *hash_borrar(hash_t *hash, const char *clave){

	if(hash->ocupados == 0) return NULL;

	if(calcular_porcentaje(hash->ocupados, hash->tamanio) <= PORCENTAJE_ACHICAR){
		if(hash->tamanio > TAM_INICIAL){
			size_t tam_nuevo = hash->tamanio/TAM_ACHICAR;
			redimensionar_hash(hash, tam_nuevo);
		}
	}

	size_t pos = hash_f(clave, hash->tamanio);

	campo_t* campo_actual = _borrar_elemento(hash->tabla[pos], clave);

	if(!campo_actual) return NULL;

	void* dato = campo_actual->dato;
	hash->ocupados--;
	free(campo_actual->clave);
	free(campo_actual);

	return dato;
}

void *hash_obtener(const hash_t *hash, const char *clave){

	if(hash->ocupados == 0)	return NULL;

	size_t pos = hash_f(clave, hash->tamanio);

	campo_t* campo = buscar_clave(hash->tabla[pos], clave);

	if(!campo) return NULL;

	return campo->dato;
}

bool hash_pertenece(const hash_t *hash, const char *clave){

	if(hash->ocupados == 0)	return false;

	size_t pos = hash_f(clave, hash->tamanio);

	campo_t* campo = buscar_clave(hash->tabla[pos], clave);

	return (campo != NULL);
}

size_t hash_cantidad(const hash_t *hash){

	return hash->ocupados;
}

void hash_destruir(hash_t *hash, void lista_destruir(lista_t*, void destruir_dato(void *))){

	for(size_t i = 0; i < hash->tamanio; i++){
		while(!lista_esta_vacia(hash->tabla[i])){
			campo_t* campo = lista_borrar_primero(hash->tabla[i]);
			if(lista_destruir != NULL){
				lista_destruir(campo->dato, free);
			}
			free(campo->clave);
			free(campo);
		}
		lista_destruir(hash->tabla[i], NULL);
	}
	free(hash->tabla);
	free(hash);
}

/*ITERADOR*/

size_t _encontrar_lista_no_vacia(lista_t** tabla,size_t pos,size_t tamanio){

	size_t nueva_pos = pos;
	while(nueva_pos < tamanio){
		++nueva_pos;
		if(nueva_pos == tamanio) return nueva_pos-1;// pos seria la ubicacion de la ultima lista, la misma, estoy al final
		if(!lista_esta_vacia(tabla[nueva_pos])) return nueva_pos;
	}
	return nueva_pos;
}

hash_iter_t *hash_iter_crear(const hash_t* hash){
	
	hash_iter_t* hash_iter = malloc(sizeof(hash_iter_t));
	if (!hash_iter) return NULL;
	hash_iter->hash = hash;

	size_t i = _encontrar_lista_no_vacia(hash->tabla,POS_INICIAL,hash->tamanio);
	hash_iter->pos = i;

	hash_iter->iter_actual = lista_iter_crear(hash->tabla[i]);	
	if (!hash_iter->iter_actual){
		free(hash_iter);
		return NULL;
	}
	return hash_iter;
}

bool hash_iter_avanzar(hash_iter_t* iter){
	if(hash_iter_al_final(iter)) return false;

	lista_iter_avanzar(iter->iter_actual);

	if(lista_iter_al_final(iter->iter_actual)){
		size_t nueva_pos = _encontrar_lista_no_vacia(iter->hash->tabla,iter->pos,iter->hash->tamanio);

		if(nueva_pos >= iter->hash->tamanio) return false;

		lista_iter_t* lista_iter = lista_iter_crear(iter->hash->tabla[nueva_pos]);

		if(!lista_iter) return false;

		lista_iter_destruir(iter->iter_actual);
	
		iter->iter_actual = lista_iter;
		iter->pos = nueva_pos;
	}

	return true;
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){

	if(hash_iter_al_final(iter)) return NULL;

	campo_t* campo =lista_iter_ver_actual(iter->iter_actual);
	if(!campo) return NULL;
	return campo->clave;
}

bool hash_iter_al_final(const hash_iter_t* iter){
	
	return (iter->pos == iter->hash->tamanio-1);
}

void hash_iter_destruir(hash_iter_t* iter){
	lista_iter_destruir(iter->iter_actual);
	free(iter);
}