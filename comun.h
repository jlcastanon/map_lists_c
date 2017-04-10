/*
   Incluya en este fichero todas las definiciones que pueden
   necesitar compartir todos los módulos (editor, subscriptor y
   proceso intermediario), si es que las hubiera.
*/

/**
 * @file
 * Simple hash map implementation.
 */

#ifndef HASH_MAP_H
#define HASH_MAP_H



#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef TEST
extern int __malloc_counter;
#endif


#define OP_EVENT 1
#define OP_ALTA 2
#define OP_BAJA 3

/**
 * Allocate memory while exiting on failure. Also used to count
 * references during testing. All code could call this instead of
 * `malloc`.
 * @param size amount of memory to allocate
 * @return pointer to allocated memory
 */
void *safe_malloc(size_t size);

/**
 * Pass-thru call to free. Also used to count references during
 * testing. All code should call this instead of `free`.
 * @param ptr pointer to de-allocate
 */
void safe_free(void *ptr);

/**
 * Function to deallocate data pointers. For automatically allocated
 * memory, pass in `NULL` to call nothing. For memory allocated with
 * `malloc`, pass in `free`.
 */
typedef void (*linked_list_destructor)(const void *data);

/**
 * Linked list node structure
 */
typedef struct _linked_list_node {
	/** Pointer to the next node */
	struct _linked_list_node *next;
	/** Pointer to data */
	void *data;
} linked_list_node;

/**
 * Linked list object
 */
typedef struct {
	/** Pointer to sentinel node */
	linked_list_node *head;
	/** Function used to free data */
	linked_list_destructor free_data;
	/** Size of the linked_list */
	size_t size;
} linked_list;

/**
 * Initialize a linked list.
 * @param list linked list structure
 * @param comparator data comparator function
 * @param free_list data de-allocation function
 */
void linked_list_init(linked_list *list, linked_list_destructor free_list);

/**
 * Get the first linked list node.
 * @param list linked list structure
 * @return first linked list node
 */
linked_list_node *linked_list_head(linked_list *list);

/**
 * Append data to the list.
 * @param list linked list structure
 * @param data data to append
 */
int linked_list_append(linked_list *list, void *data);

/**
 * Prepend data to the list.
 * @param list linked list structure
 * @param data data to prepend
 */
void linked_list_prepend(linked_list *list, void *data);

/**
 * Free the linked list and all its nodes and data. Uses @ref
 * linked_list_destructor function passed to @ref linked_list_init
 * @param list linked list structure
 */
void linked_list_free(linked_list *list);

/**
 * Returns size of the linked list.
 * @param list linked list structure
 * @return size of list
 */
size_t linked_list_size(linked_list *list);

/**
 * Comparator function to determine whether `*l` and `*r` are equal.
 * @return Negative if `*l` is less than `*r`; zero if `*l` is equal to `*r`; positive if `*l` is greater than `*r`.
 */
typedef int (*hash_map_comparator)(const void *l, const void *r);

/**
 * Hash function
 * @param key key to hash
 * @param capacity maximum size of the map
 * @return an offset within the range `[0, capacity)`
 */
typedef size_t (*hash_map_hash_func)(const void *key, size_t capacity);

/**
 * Hash map object
 */
typedef struct {
	/** Maximum size of hash table */
	size_t capacity;
	/** Size of hash table */
	size_t size;
	/** Hash table */
	linked_list **table;
	/** Key comparator function */
	hash_map_comparator comparator;
	/** Key hash function */
	hash_map_hash_func hash_func;
	/** Keys */
	linked_list *keys;
} hash_map;

/**
 * Key/value pair
 */
typedef struct {
	/** Key */
	void *key;
	/** Value */
	void *value;
} hash_map_pair;



typedef struct{
	int op;
	char tema[100];
	char valor[1024];
	
	struct sockaddr_in info;

}flyingpacket;

typedef struct{
	int con;
	struct sockaddr_in c;
	void (*notif_evento)(const char *, const char *);
}threadargs;

void print_flyingpacket(flyingpacket f);

/**
 * Initialize the hash map.
 * @param map hash map structure
 * @param capacity maximum size of the hash map
 * @param comparator key comparator function
 * @param hash_func key hash function
 */
void hash_map_init(hash_map *map, size_t capacity, hash_map_comparator comparator, hash_map_hash_func hash_func);

/**
 * Free the hash map.
 * This function will also free the table of internal linked lists.
 * @param map hash map structure
 */
void hash_map_free(hash_map *map);

/**
 * Get the value for the given key.
 * @param map hash map structure
 * @param key key for value to fetch
 * @return pointer to the value
 */
void *hash_map_get(hash_map *map, void *key);

/**
 * Insert the value into the map.
 * @param map hash map structure
 * @param key key associated with value
 * @param value value associated with key
 */
int hash_map_put(hash_map *map, void *key, void *value);

/**
 * Remove the mapping from the map if this key exists. Calling this on
 * a key for which there is no mapping does nothing (does not error).
 * @param map hash map structure
 * @param key key for mapping to remove
 */
void hash_map_remove(hash_map *map, void *key);

/**
 * Returns number of key-value pairs in the map
 * @param map hash map structure
 * @return size of the hash map
 */
size_t hash_map_size(hash_map *map);

/**
 * Returns a linked list that contains all keys in the map
 * @param map hash map structure
 * @return a linked list containing all keys
 */
linked_list *hash_map_keys(hash_map *map);

/**
 * Removes all key/value pairs from the map
 * @param map hash map structure
 */
void hash_map_clear(hash_map *map);

/**
 * Check if the map contains the given key
 * @param map hash map structure
 * @param key the key to check
 * @return true if map contains key
 int = 1 false no contiene
 int = 0 true contiene
 */
int hash_map_contains_key(hash_map *map, void *key);


//aux functions 
/**
esta funcion imporime los elementos 
de una lista siempre y cuando
sean char * o char[]
es decir strings
*/

void print_list(linked_list *l);

hash_map * mapinit(char * txt,int tam);

linked_list * creatorlists();


int linked_list_has_sub(linked_list *l/*,subscriber s*/);

void * start_thd(void *a);

int linked_list_has_port(linked_list *l, int port);

void print_list_int(linked_list *l);

int linked_list_remove(linked_list *list,void *data);

int introduceTema(char * fich,char * tema,FILE * f);


#endif

