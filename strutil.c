#include <string.h>
#include <stdlib.h>
#include "strutil.h"

size_t largo_strv(char** strv){

	size_t total = 0, i = 0;

	while(strv[i] != NULL){
		total += strlen(strv[i]);
		i++;
	}

	total += i;

	return total;
}

char* pasar_str(const char* str, size_t inicio, size_t fin){

	char* array_aux = malloc((fin-inicio+1)*sizeof(char));

	if(!array_aux) return NULL;

	size_t j = 0;

	for(size_t i = inicio; i < fin; i++){
		array_aux[j] = str[i];
		j++;
	}

	array_aux[j] = '\0';

	return array_aux;
}

size_t contar_sep(const char* str, char sep, size_t tam_str){

	size_t contador_sep = 0;

	for(int i = 0; i < tam_str; i++){
		if(str[i] == sep){
			contador_sep++;
		}
	}

	return contador_sep+1;
}

char** split(const char* str, char sep){

	if(sep == '\0') return NULL;

	size_t cant_caracteres = strlen(str)+1;
	size_t tamanio_str_separado = contar_sep(str, sep, cant_caracteres)+1; //+1 para NULL

	char** str_separado = malloc(tamanio_str_separado*sizeof(char*));

	if(!str_separado) return NULL;

	size_t j = 0, inicio = 0;

	for(size_t i = 0; i < cant_caracteres; i++){
		if(str[i] == sep || str[i] == '\0'){
			str_separado[j] = pasar_str(str, inicio, i);
			if(!str_separado[j]){
				free_strv(str_separado);
				return NULL;
			}
			inicio = i+1;
			j++;
		}
	}

	str_separado[j] = NULL;

	return str_separado;
}

char* join(char** strv, char sep){

	size_t tam_strv = largo_strv(strv)+1;

	char* str_completo = malloc(tam_strv*sizeof(char));

	if(!str_completo) return NULL;

	size_t i = 0, j = 0, k = 0;

	while(strv[i] != NULL){
		if(strv[i][j] == '\0'){
			str_completo[k] = sep;
			i++;
			j = 0;
		}else{
			str_completo[k] = strv[i][j];
			j++;
		}
		k++;
	}

	if(k > 0) str_completo[k-1] = '\0';
	else str_completo[k] = '\0';

	return str_completo;
}

void free_strv(char* strv[]){

	size_t i = 0;

	while(strv[i] != NULL){
		free(strv[i]);
		i++;
	}

	free(strv);
}