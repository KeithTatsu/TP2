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

#define TIME_FORMAT "%FT%T%z"
#define ORDENAR_ARCHIVO "ordenar_archivo"
#define AGREGAR_ARCHIVO "agregar_archivo"
#define VER_VISITANTES "ver_visitantes"
#define ERROR_EN_COMANDO "Error en comando"
#define MAX_CANT_REGISTRO 20

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

int comparar_ip(const char** IP1, const char** IP2, size_t pos){

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

	const char** IP1 = split(ip1, '.');
	const char** IP2 = split(ip2, '.');

	return comparar_ip(IP1, IP2, 0);
}

bool imprimir_error(char* comando){

	fprintf(stderr, "%s %s\n", ERROR_EN_COMANDO, comando);

	return false;
}

char* _clave_copiar(const char* clave){
	
	char* clave_aux = malloc((strlen(clave)+1)*sizeof(char));
	if(!clave_aux) return NULL;
	strcpy(clave_aux, clave);
	return clave_aux;
}

bool ordenar_archivo(char* nombre_archivo, char* nombre_arhivo_ordenado, size_t tam_limite){

	FILE* archivo_desordenado = fopen(nombre_archivo, "r");

	if(!archivo_desordenado) return imprimir_error(nombre_archivo);

	char* linea = NULL;
	size_t cant = 0, cant_linea = 0, cant_particiones = 1;
	ssize_t leidos;
	heap_t* heap_ordenar = heap_crear();

	FILE* archivo_particionado = fopen(,"w");
	while((leidos = getline(&linea, &cant, archivo_desordenado) > 0)){
		
		if (cant_linea > MAX_CANT_REGISTRO){
			nombre_archivo = // Aca seria cuando concatenamos  y no se como pasar de int a char
			fclose(archivo_particionado);
			archivo_particionado = fopen(nombre_archivo, "w");
		}
		fputs(linea,archivo_particionado);
	}

	fclose(archivo_desordenado);

	return true;
}

bool agregar_archivo(char* nombre_archivo){

	return true;
}

bool ver_visitantes(char* nombre_archivo, char* desde, char* hasta){

	FILE* archivo_visitas = fopen(nombre_archivo, "r");

	if(!archivo_visitas) return imprimir_error(nombre_archivo);

	fclose(archivo_visitas);

	return true;
}
bool comparar_comando(char** comando, int tam_limite){

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
		if(!comando[1] || !comando[2] || !comando[3]) return imprimir_error(comando[0]);
		else{
			if(ver_visitantes(comando[1], comando[2], comando[3])){
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
	abb_t* abb_visitantes = abb_crear(ip_cmp, NULL);

	while((leidos = getline(&linea, &cant, stdin) > 0)){

		linea[cant-1] = '\0';
		char** comando = split(linea, ' ');
		if(comando[0]){
			if(!comparar_comando(comando, tam_limite)){
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

int main(char* argv[], int agrc){

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