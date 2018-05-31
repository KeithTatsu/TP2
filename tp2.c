#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strutil.h"

void interfaz(){

	char* linea;
	size_t cant = 0;
	ssize_t leidos;

	while((leidos = getline(&linea, &cant, stdin) > 0)){
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