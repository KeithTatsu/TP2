#include "heap.h"
#include <stdlib.h>

#define POSICION_DEL_MAX 0
#define CANTIDAD_INICIAL 0
#define CAPACIDAD_INICIAL 24
#define REDIMENSIONADOR 2
#define TAMANIO_RELATIVO 4

struct heap{
	void** datos;
	cmp_func_t cmp;
	size_t cantidad;
	size_t capacidad;
};

void _swap(void* datos[], size_t pos_1, size_t pos_2){
	void* aux = datos[pos_1];
	datos[pos_1] = datos[pos_2];
	datos[pos_2] = aux;
}
void _upheap(void *datos[], size_t pos, cmp_func_t cmp){
	if(pos == 0) return;
	size_t padre = (pos-1)/2;

	if(cmp(datos[padre], datos[pos]) > 0) return;

	_swap(datos, padre, pos);
	_upheap(datos,padre,cmp);
}
void _downheap(void *datos[],size_t n ,size_t pos ,cmp_func_t cmp){
	if(pos >= n) return;
	size_t  pos_hijo_izq = 2*pos+1;
	size_t  pos_hijo_der = 2*pos+2;
	size_t  pos_max = pos;
	if(pos_hijo_izq < n && cmp(datos[pos_hijo_izq],datos[pos_max])>0){
		pos_max = pos_hijo_izq;
	}
	if(pos_hijo_der < n && cmp(datos[pos_hijo_der],datos[pos_max])>0){
		pos_max = pos_hijo_der;
	}
	if(pos != pos_max){
		_swap(datos, pos, pos_max);
		_downheap(datos,n,pos_max,cmp);
	}
}
heap_t* heap_crear(cmp_func_t cmp){
	heap_t* heap = malloc(sizeof(heap_t));
	if (!heap) return NULL;
	heap->cantidad = CANTIDAD_INICIAL;
	heap->capacidad = CAPACIDAD_INICIAL;
	heap->cmp = cmp;
	heap->datos = malloc((heap->capacidad)*sizeof(void*));
	if(!heap->datos){
        free(heap);
        return NULL;
    }
    return heap;
}
heap_t *heap_crear_arr(void *arreglo[], size_t n, cmp_func_t cmp){
	heap_t* heap = heap_crear(cmp);
	if(!heap) return NULL;
	size_t pos_act = 0;
	while(pos_act < n){
		if(!heap_encolar(heap,arreglo[pos_act])){
			heap_destruir(heap, NULL);
			return NULL;
		}
		pos_act++;
	}
	return heap;
}
bool _redimensionar_heap(heap_t *heap, size_t tam_nuevo){
	void** datos_nuevos = realloc(heap->datos, tam_nuevo*sizeof(void*));
	if(!tam_nuevo) return false;
	heap->datos = datos_nuevos;
	heap->capacidad = tam_nuevo;

	return true;
}
bool heap_encolar(heap_t *heap, void *elem){
	if(heap->cantidad == heap->capacidad){
		size_t tam_nuevo = heap->capacidad*REDIMENSIONADOR;
		if(!_redimensionar_heap(heap,tam_nuevo)){
			return false;
		}
	}
	heap->datos[heap->cantidad] = elem;
	_upheap(heap->datos,heap->cantidad,heap->cmp);
	++heap->cantidad;
	return true;
}
size_t heap_cantidad(const heap_t *heap){
	return(heap->cantidad);
}
bool heap_esta_vacio(const heap_t *heap){
	return (heap_cantidad(heap) == 0);
}
void *heap_ver_max(const heap_t *heap){
	if(heap_esta_vacio(heap)) return NULL;
	return(heap->datos[POSICION_DEL_MAX]);
}
void *heap_desencolar(heap_t *heap){
	if(heap_esta_vacio(heap)) return NULL;
	if(heap->cantidad < heap->capacidad/TAMANIO_RELATIVO && CAPACIDAD_INICIAL < heap->capacidad){
		size_t tam_nuevo = heap->capacidad/REDIMENSIONADOR;
		if(!_redimensionar_heap(heap,tam_nuevo)){
			return false;
		}
	}
	void* elem = heap->datos[POSICION_DEL_MAX];

	_swap(heap->datos, POSICION_DEL_MAX, heap->cantidad-1);
	heap->datos[heap->cantidad-1] = NULL;
	--heap->cantidad;
	_downheap(heap->datos,heap->cantidad,POSICION_DEL_MAX,heap->cmp);
	
	return elem;
}
void heap_destruir(heap_t *heap, void destruir_elemento(void *e)){
	while(!heap_esta_vacio(heap)){
		void* elem = heap_desencolar(heap);
		if(destruir_elemento){
			destruir_elemento(elem);
		}
	}
	free(heap->datos);
    free(heap);
}
void heapify(void* elementos[], size_t cant, cmp_func_t cmp){

	for(size_t i = cant; i > 0; i--){
		_downheap(elementos, cant, i-1, cmp);
	}
}
void heap_sort(void *elementos[], size_t cant, cmp_func_t cmp){

	heapify(elementos, cant, cmp);

	for(size_t i = cant; i > 0; i--){
		_downheap(elementos, i, 0, cmp);
		_swap(elementos, 0, i-1);
	}
}