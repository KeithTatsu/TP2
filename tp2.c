#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "strutil.h"
#include "heap.h"
#include "lista.h"
#include "hash.h"
#include "abb.h"

#define TIME_FORMAT "%FT%T%z"
#define ORDENAR_ARCHIVO "ordenar_archivo"
#define AGREGAR_ARCHIVO "agregar_archivo"
#define VER_VISITANTES "ver_visitantes"
#define ERROR_EN_COMANDO "Error en comando"
#define CANT_MAX_LOGS 30 //Cantidad de lineas que se mantendrán en memoria
#define POSIBLE_DOS 5

typedef struct arch_procedente{
	char* linea;
	size_t archivo;
}arch_proce_t;

time_t iso8601_to_time(const char* iso8601){

	struct tm bktime = { 0 };
	strptime(iso8601, TIME_FORMAT, &bktime);

	return mktime(&bktime);
}
/* nombre: nombre del prefijo del archivo, que puede ser el nombre del archivo original
   particion: numero de particion a crear
   Por ejemplo: crear_nombre_particion("access001.log", 7) --> "access001.log_07"
*/
char* crear_nombre_particion(const char* nombre, size_t particion){

	char* nombre_archivo = malloc(sizeof(char)*(strlen(nombre) + 5));

	if(!nombre_archivo) return NULL;

	sprintf(nombre_archivo, "%s_%02zu", nombre, particion);

	return nombre_archivo;
}

FILE* crear_archivo_particion(const char* nombre, size_t particion, char* modo){

	char* nombre_archivo = crear_nombre_particion(nombre, particion);
	FILE* salida_actual = fopen(nombre_archivo, modo);

	if(!salida_actual) return NULL;

	free(nombre_archivo);
	return salida_actual;
}

int comparar_ip(char** ip1, char** ip2, size_t pos){

	if(pos == 4) return 0;

	size_t len_ip1 = strlen(ip1[pos]);
	size_t len_ip2 = strlen(ip2[pos]);

	if(len_ip1 == len_ip2){
		if(strcmp(ip1[pos], ip2[pos]) == 0){
			return comparar_ip(ip1, ip2, pos+1);
		}
		return strcmp(ip2[pos], ip1[pos]);
	}

	if(len_ip1 > len_ip2) return -1;

	return 1;
}

int ip_cmp(const char* IP1, const char* IP2){

	char** ip1 = split(IP1, '.');
	char** ip2 = split(IP2, '.');

	int cmp = comparar_ip(ip2, ip1, 0);

	free_strv(ip1);
	free_strv(ip2);

	return cmp;
}

int tiempo_cmp(void* LINEA_1, void* LINEA_2){

	char* linea1 = LINEA_1;
	char* linea2 = LINEA_2;

	char** linea_1 = split(linea1, '\t');
	char** linea_2 = split(linea2, '\t');

	time_t tiempo1 = iso8601_to_time(linea_1[1]);
	time_t tiempo2 = iso8601_to_time(linea_2[1]);

	int diferencia = (int)difftime(tiempo2, tiempo1);

	if(diferencia == 0){
		diferencia = ip_cmp(linea_2[0], linea_1[0]);

		if(diferencia == 0){
			diferencia = strcmp(linea_2[3], linea_1[3]);
		}
	}
	
	free_strv(linea_1);
	free_strv(linea_2);

	return diferencia;
}

int struct_cmp(void* IP1, void* IP2){

	arch_proce_t* linea_1 = IP1;
	arch_proce_t* linea_2 = IP2;

	int cmp = tiempo_cmp(linea_1->linea, linea_2->linea);

	return cmp;
}

bool imprimir_error(char* comando){

	fprintf(stderr, "%s %s\n", ERROR_EN_COMANDO, comando);

	return false;
}

void pasar_lista_a_heap(lista_t* lista_logs, heap_t* heap_logs){

	while(!lista_esta_vacia(lista_logs)){
		char* log = lista_borrar_primero(lista_logs);
		heap_encolar(heap_logs, log);
	}
}

void ordenar_particiones(FILE* archivo_desordenado, size_t* cant_particiones, size_t tam_limite, heap_t* heap_logs, lista_t* lista_logs){

	size_t cant = 0, tam = 0;
	char* linea = NULL;
	ssize_t leidos;

	for(size_t i = 0; i < CANT_MAX_LOGS; i++){
		if((leidos = getline(&linea, &cant, archivo_desordenado)) > 0){
			char* linea_actual = strdup(linea);
			heap_encolar(heap_logs, linea_actual);
		}
	}

	FILE* archivo_particionado = crear_archivo_particion("particion", (*cant_particiones), "w");

	while((leidos = getline(&linea, &cant, archivo_desordenado) > 0)){
		char* linea_actual = strdup(linea);

		if(tam > (tam_limite*1000) || lista_largo(lista_logs) == CANT_MAX_LOGS){
			fclose(archivo_particionado);
			(*cant_particiones)++;
			tam = 0;
			archivo_particionado = crear_archivo_particion("particion", (*cant_particiones), "w");
			if(lista_largo(lista_logs) == CANT_MAX_LOGS){
				pasar_lista_a_heap(lista_logs, heap_logs);
			}
		}
		if(!heap_esta_vacio(heap_logs)){
			char* minimo_actual = heap_desencolar(heap_logs);
			int cmp = tiempo_cmp(minimo_actual, linea_actual);
			if(cmp > 0){
				heap_encolar(heap_logs, linea_actual);
			}else if(cmp <= 0){
				lista_insertar_ultimo(lista_logs, linea_actual);
			}
			fprintf(archivo_particionado, "%s", minimo_actual);
			free(minimo_actual);
		}
		tam+= cant;
	}

	pasar_lista_a_heap(lista_logs, heap_logs);
	while(!heap_esta_vacio(heap_logs)){
		if(tam > (tam_limite*1000)){
			fclose(archivo_particionado);
			(*cant_particiones)++;
			archivo_particionado = crear_archivo_particion("particion", (*cant_particiones), "w");
		}
		char* linea_actual = heap_desencolar(heap_logs);
		fprintf(archivo_particionado, "%s", linea_actual);
		free(linea_actual);
	}

	fclose(archivo_particionado);
	free(linea);
}

void procesar_particiones_desordenadas(FILE* archivo_desordenado, size_t* cant_particiones, size_t tam_limite){

	heap_t* heap_logs = heap_crear(tiempo_cmp);
	if(!heap_logs) return;

	lista_t* lista_logs = lista_crear();
	if(!lista_logs){
		free(heap_logs);
		return;
	}

	ordenar_particiones(archivo_desordenado, cant_particiones, tam_limite, heap_logs, lista_logs);

	heap_destruir(heap_logs, NULL);
	lista_destruir(lista_logs, NULL);
}

arch_proce_t* pasar_linea(char* linea, size_t pos){

	arch_proce_t* linea_nueva = malloc(sizeof(arch_proce_t));

	if(!linea_nueva) return NULL;

	linea_nueva->linea = strdup(linea);
	linea_nueva->archivo = pos;

	return linea_nueva;
}

void destruir_linea(arch_proce_t* linea_actual){

	free(linea_actual->linea);
	free(linea_actual);
}

void k_merge(char* nombre_archivo_ordenado, void** particiones, size_t cant_particiones){

	heap_t* heap_lineas = heap_crear(struct_cmp);

	if(!heap_lineas) return;

	nombre_archivo_ordenado[strlen(nombre_archivo_ordenado)-1] = '\0';

	FILE* archivo_final = fopen(nombre_archivo_ordenado, "w");

	if(!archivo_final){
		heap_destruir(heap_lineas, NULL);
		return;
	}

	size_t cant = 0;
	char* linea = NULL;
	ssize_t leidos;

	for(size_t i = 0; i < cant_particiones; i++){
		if((leidos = getline(&linea, &cant, particiones[i])) > 0){
			arch_proce_t* linea_actual = pasar_linea(linea, i);
			heap_encolar(heap_lineas, linea_actual);
		}
	}

	while(!heap_esta_vacio(heap_lineas)){
		arch_proce_t* linea_actual = heap_desencolar(heap_lineas);
		fprintf(archivo_final, "%s", linea_actual->linea);
		size_t pos = linea_actual->archivo;
		FILE* particion_actual = particiones[pos];
		if((leidos = getline(&linea, &cant, particion_actual)) > 0){
			arch_proce_t* linea_a_agregar = pasar_linea(linea, pos);
			heap_encolar(heap_lineas, linea_a_agregar);
		}
		destruir_linea(linea_actual);
	}

	heap_destruir(heap_lineas, NULL);
	free(linea);
	fclose(archivo_final);
}

void procesar_particiones_ordenadas(char* nombre_archivo_ordenado, size_t cant_particiones){

	void** particiones = malloc(cant_particiones*sizeof(void*));

	if(!particiones) return;

	for(size_t i = 0; i < cant_particiones; i++){
		particiones[i] = crear_archivo_particion("particion", i+1, "r");
	}

	k_merge(nombre_archivo_ordenado, particiones, cant_particiones);

	for(size_t i = 0; i < cant_particiones; i++){
		FILE* archivo_actual = particiones[i];
		fclose(archivo_actual);
		char* nombre_archivo = crear_nombre_particion("particion", i+1);
		remove(nombre_archivo);
		free(nombre_archivo);
	}

	free(particiones);
}
bool ordenar_archivo(char* nombre_archivo, char* nombre_archivo_ordenado, int tam_limite){
 	/*ojo que tam_limite es en kilobyte y leidos en byte, 1 kb = 1000 b*/
	FILE* archivo_desordenado = fopen(nombre_archivo, "r");
	if(!archivo_desordenado) return imprimir_error(nombre_archivo);

	size_t cant_particiones = 1;

	procesar_particiones_desordenadas(archivo_desordenado, &cant_particiones, tam_limite);
	procesar_particiones_ordenadas(nombre_archivo_ordenado, cant_particiones);

	fclose(archivo_desordenado);

	return true;
}

void verificar_ataque_dos(abb_t* abb_dos, lista_t* lista_tiempos, char* ip_actual){

	lista_iter_t* iter1 = lista_iter_crear(lista_tiempos);
	if(!iter1) return;
	lista_iter_t* iter2 = lista_iter_crear(lista_tiempos);
	if(!iter2){
		lista_iter_destruir(iter1);
		return;
	}

	size_t pos = 1;

	while(pos < POSIBLE_DOS){
		lista_iter_avanzar(iter2);
		pos++;
	}

	while(!lista_iter_al_final(iter1) && !lista_iter_al_final(iter2)){
		char* tiempo1 = lista_iter_ver_actual(iter1);
		char* tiempo2 = lista_iter_ver_actual(iter2);

		int diferencia = tiempo_cmp(tiempo1, tiempo2);

		if(diferencia < 2 && diferencia > -2){
			abb_guardar(abb_dos, ip_actual, NULL);
			lista_iter_destruir(iter1);
			lista_iter_destruir(iter2);
			return;
		}
		lista_iter_avanzar(iter1);
		lista_iter_avanzar(iter2);
	}

	lista_iter_destruir(iter1);
	lista_iter_destruir(iter2);
}

void imprimir_atacantes_dos(abb_t* abb_dos){

	abb_iter_t* iter = abb_iter_in_crear(abb_dos);

	if(!iter) return;

	while(!abb_iter_in_al_final(iter)){
		const char* ip_actual = abb_iter_in_ver_actual(iter);
		fprintf(stdout, "%s: %s\n", "DoS", ip_actual);
		abb_iter_in_avanzar(iter);
	}

	abb_iter_in_destruir(iter);
}

void buscar_atacantes_dos(FILE* archivo_actual, hash_t* hash_logs, abb_t* visitantes, abb_t* abb_dos){

	char* linea = NULL;
	size_t cant = 0;
	ssize_t leidos;

	while((leidos = getline(&linea, &cant, archivo_actual)) > 0){
		char** linea_actual = split(linea, '\t');
		char* linea_aux = strdup(linea);
		lista_t* lista_tiempos = hash_obtener(hash_logs, linea_actual[0]);
		abb_guardar(visitantes, linea_actual[0], NULL);

		if(!lista_tiempos){
			lista_tiempos = lista_crear();
			if(!lista_tiempos){
				free(linea);
				return;
			}
		}
		lista_insertar_ultimo(lista_tiempos, linea_aux);
		if(lista_largo(lista_tiempos) >= POSIBLE_DOS){
			verificar_ataque_dos(abb_dos, lista_tiempos, linea_actual[0]);
		}
		hash_guardar(hash_logs, linea_actual[0], lista_tiempos);
		free_strv(linea_actual);
	}

	free(linea);
}

bool agregar_archivo(char* nombre_archivo, abb_t* visitantes){

	size_t largo = strlen(nombre_archivo);
	nombre_archivo[largo-1] = '\0';

	FILE* archivo_actual = fopen(nombre_archivo, "r");
	if(!archivo_actual) return imprimir_error(nombre_archivo);

	hash_t* hash_logs = hash_crear(NULL);
	if(!hash_logs){
		fclose(archivo_actual);
		return false;
	}

	abb_t* abb_dos = abb_crear(ip_cmp, NULL);
	if(!abb_dos){
		free(hash_logs);
		fclose(archivo_actual);
		return false;
	}

	buscar_atacantes_dos(archivo_actual, hash_logs, visitantes, abb_dos);

	fclose(archivo_actual);
	hash_destruir(hash_logs, lista_destruir);

	imprimir_atacantes_dos(abb_dos);

	abb_destruir(abb_dos);

	return true;
}

bool imprimir_visitantes(const char* clave, char* desde, char* hasta){

	int desde_clave = ip_cmp(desde, clave);
	int hasta_clave = ip_cmp(hasta, clave);

	if(desde_clave <= 0 && hasta_clave >= 0){
		fprintf(stdout, "\t%s\n", clave);
		return true;
	}
	if(desde_clave > 0) return true;

	return false;
}

bool ver_visitantes(char* desde, char* hasta, abb_t* visitantes){

	fprintf(stdout, "%s:\n", "Visitantes");
	abb_in_order(visitantes, imprimir_visitantes, desde, hasta);

	return true;
}
bool comparar_comando(char** comando, int tam_limite, abb_t* visitantes){

	if(strcmp(comando[0], ORDENAR_ARCHIVO) == 0){
		if(!comando[2] || !comando[1]) return imprimir_error(comando[0]);
		else{
			if(ordenar_archivo(comando[1], comando[2], tam_limite)){
				fprintf(stdout, "%s\n", "OK");
				return true;
			}
		}
	}else if(strcmp(comando[0], AGREGAR_ARCHIVO) == 0){
		if(!comando[1]) return imprimir_error(comando[0]);
		else{
			if(agregar_archivo(comando[1], visitantes)){
				fprintf(stdout, "%s\n", "OK");
				return true;
			}
		}
	}else if(strcmp(comando[0], VER_VISITANTES) == 0){
		if(!comando[1] || !comando[2]) return imprimir_error(comando[0]);
		else{
			if(ver_visitantes(comando[1], comando[2], visitantes)){
				fprintf(stdout, "%s\n", "OK");
				return true;
			}
		}
	}

	return imprimir_error(comando[0]);
}

void interfaz(int tam_limite){

	char* linea = NULL;
	size_t cant = 0;
	ssize_t leidos;
	abb_t* visitantes = abb_crear(ip_cmp, NULL);

	if(!visitantes) return;

	while((leidos = getline(&linea, &cant, stdin) > 0)){

		char** comando = split(linea, ' ');
		if(comando[0]){
			if(!comparar_comando(comando, tam_limite, visitantes)){
				abb_destruir(visitantes);
				free(linea);
				free_strv(comando);
				return;
			}
		}else{
			imprimir_error(linea);
			abb_destruir(visitantes);
			free(linea);
			free_strv(comando);
			return;
		}
		free_strv(comando);
	}
	free(linea);
	abb_destruir(visitantes);
}

int main(int argc, char* argv[]){

	if(argc < 2){
		imprimir_error(argv[0]);
		return -1;
	}

	int tam_limite = atoi(argv[1]);

	if(tam_limite == 0){
		imprimir_error(argv[1]);
		return -1;
	}

	interfaz(tam_limite);

	return 0;
}

		//ordenar_archivo <nombre_archivo> <nombre_salida>
		//agregar_archivo <nombre_archivo>
		//ver_visitantes <desde> <hasta>
		//if 'error' fprintf(stderr, "Error en comando <comando>", <comando>)