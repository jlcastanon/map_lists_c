/*
   Incluya en este fichero todas las implementaciones que pueden
   necesitar compartir todos los módulos (editor, subscriptor y
   proceso intermediario), si es que las hubiera.
*/

#include "comun.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <unistd.h>




#define LINEA 100


static inline int hash_map_default_comparator(const void *l, const void *r) {
	return *((unsigned long *) l) - *((unsigned long *) r);
}

static inline size_t hash_map_default_hash_func(const void *key, size_t capacity) {
	//return *((size_t *) key) % capacity;
	return *((char *) key) % capacity;
}

void hash_map_init(hash_map *map, size_t capacity, hash_map_comparator comparator, hash_map_hash_func hash_func) {
	map->capacity = capacity;
	map->size = 0;

	map->table = (linked_list **) safe_malloc(sizeof(linked_list *) * map->capacity);
	memset(map->table, 0, sizeof(linked_list *) * map->capacity);

	if (comparator) {
		map->comparator = comparator;
	} else {
		map->comparator = hash_map_default_comparator;
	}

	if (hash_func) {
		map->hash_func = hash_func;
	} else {
		map->hash_func = hash_map_default_hash_func;
	}

	map->keys = (linked_list *) safe_malloc(sizeof(linked_list));
	// No free_data func here because keys will be free'd by linked_list_free for **table
	linked_list_init(map->keys, NULL);
}

void hash_map_free(hash_map *map) {
	size_t i;
	for (i = 0; i < map->capacity; i++) {
		if (map->table[i]) {
			linked_list_free(map->table[i]);
		}
	}

	linked_list_free(map->keys);

	safe_free(map->table);

	safe_free(map);
}

void *hash_map_get(hash_map *map, void *key) {
	linked_list *list = map->table[map->hash_func(key, map->capacity)];

	if (!list) {
		return NULL;
	}

	linked_list_node *head = linked_list_head(list);

	while (head) {
		hash_map_pair *pair = (hash_map_pair *) head->data;

		if (map->comparator(pair->key, key) == 0) {
			return pair->value;
		}

		head = head->next;
	}

	return NULL;
}

int hash_map_put(hash_map *map, void *key, void *value) {
	linked_list *list = map->table[map->hash_func(key, map->capacity)];

	if (!list) {
		list = (linked_list *) safe_malloc(sizeof(linked_list));
		linked_list_init(list, (linked_list_destructor) safe_free);
		map->table[map->hash_func(key, map->capacity)] = list;
	}

	linked_list_node *head = linked_list_head(list);

	while (head) {
		hash_map_pair *pair = (hash_map_pair *) head->data;

		// if key already exists, update the value
		if (map->comparator(pair->key, key) == 0) {
			pair->value = value;
			return 0;
		}

		head = head->next;
	}

	// or else insert new one

	hash_map_pair *pair = (hash_map_pair *) safe_malloc(sizeof(hash_map_pair));
	pair->key = key;
	pair->value = value;

	linked_list_prepend(list, pair);

	linked_list_append(map->keys, key);

	map->size++;

	return 0;
}

void hash_map_remove(hash_map *map, void *key) {
	size_t offset = map->hash_func(key, map->capacity);
	linked_list *list = map->table[offset];

	if (!list) {
		return;
	}

	// The variable previous_node is set to the sentinel node, NOT the
	// head item of the list.
	linked_list_node *previous_node = list->head;
	linked_list_node *current_node = previous_node->next;
	while (true) {
		// Is the first node a match?
		if (map->comparator(((hash_map_pair *)current_node->data)->key, key) == 0) {
			// Delete the node and relink.
			previous_node->next = current_node->next;
			if (list->free_data) {
				list->free_data(current_node->data);
			}
			safe_free(current_node);
			// Decrement structure sizes
			list->size--;
			map->size--;
			return;
		}
		// Exit when we are at the end.
		if (current_node->next == NULL) {
			break;
		}
		// Increment
		previous_node = current_node;
		current_node = current_node->next;
	}
}

size_t hash_map_size(hash_map *map) {
	return map->size;
}

linked_list *hash_map_keys(hash_map *map) {
	return map->keys;
}



void hash_map_clear(hash_map *map) {
	size_t i;
	for (i = 0; i < map->capacity; i++) {
		linked_list *list = map->table[i];

		if (list) {
			linked_list_free(list);
			map->table[i] = NULL;
		}
	}

	map->size = 0;
}

int hash_map_contains_key(hash_map *map, void *key) {
	linked_list *list = map->table[map->hash_func(key, map->capacity)];

	if (!list) {
		return 1;
	}

	//printf("entra %p\n", list);

	linked_list_node *head = linked_list_head(list);

	while (head) {
		hash_map_pair *pair = (hash_map_pair *) head->data;
		//printf("CMP(%s,%s)\n",(char *)key,(char *)pair->key);
		if (map->comparator(pair->key, key) == 0) {
			return 0;
		}

		head = head->next;
	}

	return 1;
}

//HASHMAP ----------------------------------------------


void linked_list_init(linked_list *list, linked_list_destructor free_data) {
	// Allocate a sentinel node
	linked_list_node *sentinel = safe_malloc(sizeof(linked_list_node));
	sentinel->next = NULL;
	list->head = sentinel;

	list->free_data = free_data;

	list->size = 0;
}

linked_list_node *linked_list_head(linked_list *list) {
	return list->head->next;
}

int linked_list_append(linked_list *list, void *data) {

	//if(list->size != 0){

		linked_list_node *node = list->head;
	while (node->next) {
		node = node->next;
	}
	linked_list_node *new_node = safe_malloc(sizeof(linked_list_node));
	new_node->data = data;
	new_node->next = NULL;
	node->next = new_node;

	list->size++;

	return 0;

	//}

	//printf("[+]APPEND -> lista vacia");
	//return 1;
	
}

void linked_list_prepend(linked_list *list, void *data) {
	linked_list_node *new_node = safe_malloc(sizeof(linked_list_node));
	new_node->data = data;
	new_node->next = list->head->next;
	list->head->next = new_node;

	list->size++;
}

int linked_list_remove(linked_list *l,void *data){

	linked_list_node *current = linked_list_head(l);
	linked_list_node *prev;
	int f = 0;

	//printf("%d, %d\n",(int)l->head->next->data,(int)current->data);//imprime 0??
	//si el elemento a borrar es el primero
	//printf("%d, %d\n",current->data,data);
	if(current->data == data){
		//printf("entra if");
		l->head->next = current->next;
		l->size--;
		return 0;
	}
	//printf("entra else");
	while(current != NULL && !f){
		if(current->data == data){
			f=1;
			break;
		}
		prev = current;
		current = current->next;
	}
	//printf("printing nodes\n");
	//printf("\nnodo previo -> %d\n",(int)prev->data);
	//printf("\nnodo current -> %d\n",(int)current->data);
	prev->next= current->next;
	l->size--;
	return 0;
}

void linked_list_free(linked_list *list) {
	linked_list_node *previous_node = list->head;
	linked_list_node *current_node = previous_node->next;

	while (current_node != NULL) {
		if (list->free_data != NULL) {
			list->free_data(current_node->data);
		}
		safe_free(previous_node);
		previous_node = current_node;
		current_node = previous_node->next;
	}

	safe_free(previous_node);

	list->head = NULL;

	safe_free(list);
}

int linked_list_has_sub(linked_list *l/*, subscriber s*/){
/*
	int found=1,i=0;
	linked_list_node *node;
	subscriber aux;

	for(i=0, node = linked_list_head(l);
		node != NULL; i++, node = node->next){

		aux=*(subscriber *)node->data;
		if(aux.puerto == s.puerto 
			&& aux.dir == s.dir){
			
			found = 0;
			break;
		}
	}

	

	return found;
	*/
	return 0;
}

int linked_list_has_port(linked_list *l, int port){
	//quitar i no tiene uso

	int found=1,i=0,aux;
	linked_list_node *node;
	

	for(i=0, node = linked_list_head(l);
		node != NULL; i++, node = node->next){

		aux = (int)node->data;
		printf("imprime puerto de comparacion");
		printf("%d %d\n", aux, port);
		if(aux == port){

			found = 0;
			break;
		}
	}

	

	return found;

}

size_t linked_list_size(linked_list *list) {
	return list->size;
}

//linkedlist---------------------------------------------

#ifdef TEST
int __malloc_counter = 0;
#endif

void *safe_malloc(size_t size) {
	void *ptr = malloc(size);

	if (!ptr) {
		fputs("out of memory", stderr);
		exit(1);
	}

#ifdef TEST
	//printf("malloc: %p, counter: %d\n", ptr, __malloc_counter);
	__malloc_counter++;
#endif

	return ptr;
}

void safe_free(void *ptr) {
	free(ptr);
#ifdef TEST
	__malloc_counter--;
	//printf("free: %p, counter: %d\n", ptr, __malloc_counter);
#endif

}

//funcion imprime strings
void print_list(linked_list *l){
	int i=0;
	linked_list_node *node;
	//printf("%zu",linked_list_size(l));
	if(linked_list_size(l) == 0)
		printf("lista vacia");

	//printf("printing list");
	for(i=0,node = linked_list_head(l);
		node != NULL;
		++i,node = node->next){
		printf("%s\n",node->data);
	}
}

void print_list_int(linked_list *l){
	int i=0;
	linked_list_node *node;
	//printf("%zu",linked_list_size(l));
	if(linked_list_size(l) == 0)
		printf("lista vacia");

	//printf("printing list");
	for(i=0,node = linked_list_head(l);
		node != NULL;
		++i,node = node->next){
		printf("%d\n",(int)node->data);
	}
}

void print_flyingpacket(flyingpacket f){
	printf("op: %d\n",f.op);
	printf("tema: %s\n",f.tema);
	printf("valor: %s\n",f.valor);
}



linked_list * creatorlists(){
	linked_list *l = NULL;
	l = safe_malloc(sizeof(linked_list));
	linked_list_init(l,(linked_list_destructor) NULL);
	return l;
}




void * start_thd (void *an){

	threadargs *a = (threadargs *) an;

	flyingpacket f;

	printf("\ninicio del thread\n");
	printf("a.c thread: %d\n",ntohs(a->c.sin_port));

	char buffer[1024];
	socklen_t longc;
	listen(a->con, 3); //Estamos a la escucha
  	printf("A la escucha en el puerto %d\n", htons(a->c.sin_port));
	 
	longc = sizeof(a->c); //Asignamos el tamaño de la estructura a esta variable
	while(1){
		int conexion_cliente = accept(a->con, (struct sockaddr *)&a->c, &longc); //Esperamos una conexion
  if(conexion_cliente<0)
  {
    printf("Error al aceptar trafico\n");
    //close(conexion_servidor);
    break;
  }
  //printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
  if(recv(conexion_cliente, &f, sizeof(f), 0) < 0)
  { //Comenzamos a recibir datos del cliente
    //Si recv() recibe 0 el cliente ha cerrado la conexion. Si es menor que 0 ha habido algún error.
    printf("Error al recibir los datos\n");
    //close(conexion_servidor);
    break;
  }

  //separar tema y valor

  a->notif_evento(f.tema,f.valor);
  printf("%s\n", buffer);
  //bzero((char *)&buffer, sizeof(buffer));
  
    
  
	}
	return NULL;
  	
}


int introduceTema(char * fich,char * tema,FILE * f){
	//f=fopen(fich,"ra");
	f=fopen(fich,"a");
	//while(getline(&line, &linelength,fp) != -1);
	strcat(tema,"\n");
	fputs(tema,f);

	fclose(f);
	return 0;
}






