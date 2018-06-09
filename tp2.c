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

time_t iso8601_to_time(const char* iso8601){

	struct tm bktime = { 0 };
	strptime(iso8601, TIME_FORMAT, &bktime);

	return mktime(&bktime);
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

bool ordenar_archivo(char* nombre_archivo, char* nombre_arhivo_ordenado){

	FILE* archivo_desordenado = fopen(nombre_archivo, "r");

	if(!archivo_desordenado) return imprimir_error(nombre_archivo);

	//SE ORDENA EL ARCHIVO

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

bool comparar_comando(char** comando){

	if(strcmp(comando[0], ORDENAR_ARCHIVO) == 0){
		if(!comando[2] || !comando[1]) return imprimir_error(comando[0]);
		else{
			if(ordenar_archivo(comando[1], comando[2])){
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

void interfaz(){

	char* linea = NULL;
	size_t cant = 0;
	ssize_t leidos;
//	cola_t* cola_comandos = cola_crear();
	while((leidos = getline(&linea, &cant, stdin) > 0)){

		linea[cant-1] = '\0';
		char** comando = split(linea, ' ');
		if(comando[0]){
			if(!comparar_comando(comando)){
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

int main(){
	
	interfaz();

	return 0;
}

		//ordenar_archivo <nombre_archivo> <nombre_salida>
		//agregar_archivo <nombre_archivo>
		//ver_visitantes <desde> <hasta>
		//if 'error' fprintf(stderr, "Error en comando <comando>", <comando>)