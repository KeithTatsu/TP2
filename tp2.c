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
/*
typedef struct logs{
	char* ip;
	char* tiempo;
	char* metodo;
	char* url;
}logs_t; //NOMBRE SUJETO A CAMBIO
*/


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

	return comparar_ip(ip1, ip2, 0);
}

int tiempo_cmp(const char** TIEMPO1, const char** TIEMPO2){

	time_t tiempo1 = iso8601_to_time(TIEMPO1[1]);
	time_t tiempo2 = iso8601_to_time(TIEMPO2[1]);

	int diferencia = (int)difftime(tiempo1, tiempo2);

//	if(diferencia != 0) diferencia = diferencia*(-1);

	if(diferencia == 0){
		diferencia = ip_cmp(TIEMPO1[0], TIEMPO2[0]);

		if(diferencia == 0){
			diferencia = strcmp(TIEMPO1[3], TIEMPO2[3]);
		}
	}
	
	return diferencia;
}

bool imprimir_error(char* comando){

	fprintf(stderr, "%s %s\n", ERROR_EN_COMANDO, comando);

	return false;
}/*
FILE* _ordenar_particio(heap_t* heap,lista_t* lista,){

}
FILE* ordenar_particion(FILE* archivo_particionado, size_t particion){

	heap_t* heap_ordenar = heap_crear(tiempo_cmp);
	if (!heap_ordenar) return false;
	lista_t* lista = lista_crear();
	if(!lista){
		free(heap_ordenar);
		return false;
	}
	
	size_t cant = 0;
	char* linea = NULL;
	ssize_t leidos;
	
	while((leidos = getline(&linea, &cant, archivo_particionado) > 0)){
		if(!heap_encolar(heap_ordenar,linea)){
			//Creo que debemos usar aca la estructura que contenga todos los datos de cada linea
			aux = strdup(linea);//Fijate que en el abb nos dijo el correcto que usemos esto , me olvide de corregirlo
			return NULL;
			//Tuve muchas dudas de como se usa el metodo 2 , asi que perdi mucho tiempo viendo videos de seguimiento de ordenamiento
		}
	}


}*/
/*
logs_t* pasar_datos(char** linea_actual){

	logs_t* log_nuevo = malloc(sizeof(logs_t));

	if(!log_nuevo) return NULL;

	log_nuevo->ip = strdup(linea_actual[0]);
	log_nuevo->tiempo = strdup(linea_actual[1]);
	log_nuevo->metodo = strdup(linea_actual[2]);
	log_nuevo->url = strdup(linea_actual[3]);

	return log_nuevo;
}
*/
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
	fprintf(archivo_particionado, "%s", heap_desencolar(heap_logs));

	while((leidos = getline(&linea, &cant, archivo_desordenado) > 0)){
		char** linea_actual = split(linea, '\t');

		if(tam > (tam_limite/1000)){
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
			fprintf(archivo_particionado, "%s", join(minimo_actual, '\t'));
		}else if(lista_largo(lista_logs) == CANT_MAX_LOGS){
			fclose(archivo_particionado);
			(*cant_particiones)++;
			tam = 0;
			archivo_particionado = crear_archivo_particion("particion", cant_particiones, "w");
		}
		tam+= leidos;
	}
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