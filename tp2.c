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

typedef struct logs{
	char* ip;
	time_t tiempo;
	char* metodo;
	char* url;
}logs_t; //NOMBRE SUJETO A CAMBIO

time_t iso8601_to_time(const char* iso8601){

	struct tm bktime = { 0 };
	strptime(iso8601, TIME_FORMAT, &bktime);

	return mktime(&bktime);
}
/* nombre: nombre del prefijo del archivo, que puede ser el nombre del archivo original
   particion: numero de particion a crear
   Por ejemplo: crear_nombre_particion("access001.log", 7) --> "access001.log_07"
*/
char* crear_nombre_particion(const char* nombre, size_t particion) {
    char* nombre_archivo = malloc(sizeof(char) * (strlen(nombre) + 5));
    sprintf(nombre_archivo, "%s_%02zu", nombre, particion);
    return nombre_archivo;
}

/* Crea el FILE* para la particion, en el modo que sea necesario */ 
FILE* crear_archivo_particion(const char* nombre, size_t particion, char* modo) {
    char* nombre_archivo = crear_nombre_particion(nombre, particion);
    FILE* salida_actual = fopen(nombre_archivo, modo);
    free(nombre_archivo);
    return salida_actual;
}

int comparar_ip(char** IP1, char** IP2, size_t pos){

	if(pos == 4) return 0;

	size_t len_ip1 = strlen(IP1[pos]);
	size_t len_ip2 = strlen(IP2[pos]);

	if(len_ip1 == len_ip2){
		if(strcmp(IP1[pos], IP2[pos]) == 0){
			return comparar_ip(IP1, IP2, pos+1);
		}
		return strcmp(IP1[pos], IP2[pos]);
	}

	if(len_ip1 < len_ip2) return -1;

	return 1;
}

int ip_cmp(const char* ip1, const char* ip2){

	char** IP1 = split(ip1, '.');
	char** IP2 = split(ip2, '.');

	return comparar_ip(IP1, IP2, 0);
}

bool imprimir_error(char* comando){

	fprintf(stderr, "%s %s\n", ERROR_EN_COMANDO, comando);

	return false;
}
/*heap_t* heap_ordenar = heap_crear(ip_cmp);*/
size_t particionar_archivo(char* nombre_archivo, char* nombre_arhivo_ordenado, size_t tam_limite){
 	/*ojo que tam_limite es en kilobyte y leidos en byte*/
	FILE* archivo_desordenado = fopen(nombre_archivo, "r");
	if(!archivo_desordenado){
		imprimir_error(nombre_archivo);
		return 0;
	}
	size_t tam_limite_byte = tam_limite*1000;
	size_t cant = 0,tam = 0,cant_particiones = 1;
	char* linea = NULL;
	ssize_t leidos;
	FILE* archivo_particionado = crear_archivo_particion(nombre_arhivo_ordenado,cant_particiones,"w");
	if(!archivo_particionado){
		imprimir_error(nombre_archivo);
		return 0;
	}
	while((leidos = getline(&linea, &cant, archivo_desordenado) > 0)){
		if (tam > tam_limite_byte){
			fclose(archivo_particionado);
			++cant_particiones;
			tam = 0;
			archivo_particionado = crear_archivo_particion(nombre_arhivo_ordenado,cant_particiones,"w");

		}
		fprintf(archivo_particionado, "%s", linea);
		tam += leidos;
	}
	free(linea);
	fclose(archivo_desordenado);
	return cant_particiones;
}

bool agregar_archivo(char* nombre_archivo){

	//hash y lista

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
			if(agregar_archivo(comando[1])){
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

		linea[cant-1] = '\0';
		char** comando = split(linea, ' ');
		if(comando[0]){
			if(!comparar_comando(comando, tam_limite, visitantes)){
				free(linea);
				free_strv(comando);
				return;
			}
		}else{
			imprimir_error(linea);
			free(linea);
			free_strv(comando);
			return;
		}
		free_strv(comando);
	}
	free(linea);
}

int main(int argc, char* argv[]){

	if(argc < 2){
		fprintf(stderr, "",); //CONSULTAR
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