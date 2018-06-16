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
#define CANT_MAX_LOGS 30
#define POSIBLE_DOS 5

time_t iso8601_to_time(const char* iso8601){

	struct tm bktime = { 0 };
	strptime(iso8601, TIME_FORMAT, &bktime);

	return mktime(&bktime);
}
/* nombre: nombre del prefijo del archivo, que puede ser el nombre del archivo original
   particion: numero de particion a crear
   Por ejemplo: crear_nombre_particion("access001.log", 7) --> "access001.log_07"
*/
char* crear_nombre_particion(const char* nombre, size_t* particion){

	char* nombre_archivo = malloc(sizeof(char)*(strlen(nombre) + 5));

	if(!nombre_archivo) return NULL;

	sprintf(nombre_archivo, "%s_%02zu", nombre, (*particion));

	return nombre_archivo;
}

/* Crea el FILE* para la particion, en el modo que sea necesario */ 
FILE* crear_archivo_particion(const char* nombre, size_t* particion, char* modo){

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
		return strcmp(ip1[pos], ip2[pos]);
	}

	if(len_ip1 < len_ip2) return -1;

	return 1;
}

int ip_cmp(const char* IP1, const char* IP2){

	char** ip1 = split(IP1, '.');
	char** ip2 = split(IP2, '.');

	int cmp = comparar_ip(ip1, ip2, 0);

	free_strv(ip1);
	free_strv(ip2);

	return cmp;
}
int linea_cmp(char** IP1, char** IP2){

	return ip_cmp(IP2[0], IP1[0]);
}

int tiempo_cmp(char** LINEA1, char** LINEA2){

	time_t tiempo1 = iso8601_to_time(LINEA1[1]);
	time_t tiempo2 = iso8601_to_time(LINEA2[1]);

	int diferencia = (int)difftime(tiempo1, tiempo2);

//	if(diferencia != 0) diferencia = diferencia*(-1);	DEJAR ESTA LINEA por las dudas

	if(diferencia == 0){
		diferencia = ip_cmp(LINEA1[0], LINEA2[0]);

		if(diferencia == 0){
			diferencia = strcmp(LINEA1[3], LINEA2[3]);
		}
	}
	
	return diferencia;
}

bool imprimir_error(char* comando){

	fprintf(stderr, "%s %s\n", ERROR_EN_COMANDO, comando);

	return false;
}

void pasar_lista_a_heap(lista_t* lista_logs, heap_t* heap_logs){

	while(!lista_esta_vacia(lista_logs)){
		char** log = lista_borrar_primero(lista_logs);
		heap_encolar(heap_logs, log);
	}
}

void ordenar_particiones(FILE* archivo_desordenado, size_t* cant_particiones, size_t tam_limite){

	heap_t* heap_logs = heap_crear(tiempo_cmp);
	if(!heap_logs) return;

	lista_t* lista_logs = lista_crear();
	if(!lista_logs){
		free(heap_logs);
		return;
	}

	size_t cant = 0, tam = 0;
	char* linea = NULL;
	ssize_t leidos;

	for(size_t i = 0; i < CANT_MAX_LOGS; i++){
		if((leidos = getline(&linea, &cant, archivo_desordenado)) > 0){
			char** linea_actual = split(linea, '\t');
			heap_encolar(heap_logs, linea_actual);
		}
	}

	FILE* archivo_particionado = crear_archivo_particion("particion", cant_particiones, "w");

	while((leidos = getline(&linea, &cant, archivo_desordenado) > 0)){
		char** linea_actual = split(linea, '\t');

		if(tam > (tam_limite*1000)){
			fclose(archivo_particionado);
			(*cant_particiones)++;
			tam = 0;
			archivo_particionado = crear_archivo_particion("particion", cant_particiones, "w");
		}
		if(!heap_esta_vacio(heap_logs)){
			char** minimo_actual = heap_desencolar(heap_logs);
			int cmp = tiempo_cmp(minimo_actual, linea_actual);
			if(cmp <= 0){
				heap_encolar(heap_logs, linea_actual);
			}else if(cmp > 0){
				lista_insertar_ultimo(lista_logs, linea_actual);
			}
			char* linea_a_escribir = join(minimo_actual, '\t');
			fprintf(archivo_particionado, "%s", linea_a_escribir);
			free_strv(minimo_actual);
			free(linea_a_escribir);
		}else if(lista_largo(lista_logs) == CANT_MAX_LOGS){
			fclose(archivo_particionado);
			(*cant_particiones)++;
			tam = 0;
			archivo_particionado = crear_archivo_particion("particion", cant_particiones, "w");
			pasar_lista_a_heap(lista_logs, heap_logs);
		}
		tam+= leidos;
	}

	pasar_lista_a_heap(lista_logs, heap_logs);
	while(!heap_esta_vacio(heap_logs)){
		char** linea_actual = heap_desencolar(heap_logs);
		char* linea_a_escribir = join(linea_actual, '\t');
		fprintf(archivo_particionado, "%s", linea_a_escribir);
		free_strv(linea_actual);
		free(linea_a_escribir);
	}


	fclose(archivo_particionado);
	free(linea);
	heap_destruir(heap_logs, NULL);
	lista_destruir(lista_logs, NULL);
}

void k_merge(char* nombre_archivo_ordenado, size_t cant_particiones){

}

bool ordenar_archivo(char* nombre_archivo, char* nombre_archivo_ordenado, int tam_limite){
 	/*ojo que tam_limite es en kilobyte y leidos en byte, 1 kb = 1000 b*/
	FILE* archivo_desordenado = fopen(nombre_archivo, "r");
	if(!archivo_desordenado) return imprimir_error(nombre_archivo);

	size_t cant_particiones = 1;

	ordenar_particiones(archivo_desordenado, &cant_particiones, tam_limite);
	k_merge(nombre_archivo_ordenado, cant_particiones);

	fclose(archivo_desordenado);

	return true;
}

void verificar_ataque_dos(heap_t* heap_dos, lista_t* lista_tiempos, char** linea_actual){

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
		time_t tiempo_1 = iso8601_to_time(tiempo1);
		time_t tiempo_2 = iso8601_to_time(tiempo2);

		double diferencia = difftime(tiempo_1, tiempo_2);

		if(diferencia < 2 && diferencia > -2){
			heap_encolar(heap_dos, linea_actual);
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

void imprimir_atacantes_dos(heap_t* heap_dos){

	while(!heap_esta_vacio(heap_dos)){
		char** linea_actual = heap_desencolar(heap_dos);
		fprintf(stdout, "%s: %s\n", "DoS", linea_actual[0]);
		free_strv(linea_actual);
	}
}

void buscar_atacantes_dos(FILE* archivo_actual, hash_t* hash_logs, abb_t* visitantes, heap_t* heap_dos){

	char* linea = NULL;
	size_t cant = 0;
	ssize_t leidos;

	while((leidos = getline(&linea, &cant, archivo_actual)) > 0){
		char** linea_actual = split(linea, '\t');
		lista_t* lista_tiempos = hash_obtener(hash_logs, linea_actual[0]);
		abb_guardar(visitantes, linea_actual[0], NULL);

		if(!lista_tiempos){
			lista_tiempos = lista_crear();
			if(!lista_tiempos){
				free(linea);
				return;
			}
		}
		lista_insertar_ultimo(lista_tiempos, linea_actual[1]);
		if(lista_largo(lista_tiempos) >= POSIBLE_DOS){
			verificar_ataque_dos(heap_dos, lista_tiempos, linea_actual);
		}
		hash_guardar(hash_logs, linea_actual[0], lista_tiempos);
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

	heap_t* heap_dos = heap_crear(linea_cmp);
	if(!heap_dos){
		free(hash_logs);
		fclose(archivo_actual);
		return false;
	}

	buscar_atacantes_dos(archivo_actual, hash_logs, visitantes, heap_dos);

	fclose(archivo_actual);
	hash_destruir(hash_logs, free);

	imprimir_atacantes_dos(heap_dos);

	heap_destruir(heap_dos, NULL);

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

//		linea[cant-1] = '\0';
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
		fprintf(stderr, "%s", "error"); //CONSULTAR
		return -1;
	}

	int tam_limite = atoi(argv[1]);

	if(tam_limite == 0){
		//IGUAL A AGRC < 2
	}

	interfaz(tam_limite);

	return 0;
}

		//ordenar_archivo <nombre_archivo> <nombre_salida>
		//agregar_archivo <nombre_archivo>
		//ver_visitantes <desde> <hasta>
		//if 'error' fprintf(stderr, "Error en comando <comando>", <comando>)