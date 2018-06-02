#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutil.h"
#include "cola.h"

char* _clave_copiar(const char* clave){
	
	char* clave_aux = malloc((strlen(clave)+1)*sizeof(char));
	if(!clave_aux) return NULL;
	strcpy(clave_aux, clave);
	return clave_aux;
}
void interfaz(){

	char* linea;
	size_t cant = 0;
	ssize_t leidos;
	cola_t* cola_comandos = cola_crear();
	while((leidos = getline(&linea, &cant, stdin) > 0)){
		if(strcmp(linea,"\n") == 0){
			free(linea);
			return 0;//ACA ejecutamos los comandos 
		_clave_copiar
		linea[strlen(linea)-1] = '\0'//elimino el salto de linea
		printf("%s",linea);
		char** arreglo_comandos = split(leidos,' ')

		//ordenar_archivo <nombre_archivo> <nombre_salida>
		//agregar_archivo <nombre_archivo>
		//ver_visitantes <desde> <hasta>
		//if 'error' fprintf(stderr, "Error en comando <comando>", <comando>)
	}
}

int main(){
	
	interfaz();

	return 0;
}