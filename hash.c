#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "hash.h"

/* simple_hash borrowed from http://powerfield-software.com/?p=1240
 * after the license notice the addendum of
 * http://powerfield-software.com/?p=615 
 */
static uint32_t simple_hash(const char * key){
    uint32_t hashval, add;

    static int HASHSZ = 251;
    if (key == NULL) return HASHSZ;

    hashval = 0xdeadbeef, add = 0;
    while (*key != '\0') {
        hashval = (hashval << 3) + add + (uint32_t)(*key);
        add = (add + 1) % 13;
        key++;
    }
    return hashval % HASHSZ;
}

static void locate(HashTable* hashtable, const char* key, uint32_t* index, HashNode** prev, HashNode** node){
    *index = hashtable->fn(key);

    *prev = NULL;
    *node = hashtable->nodes[*index];

    while(*node != NULL){
        if(strcmp((*node)->key, (char *)key) == 0){
            break;
        }
        *prev = *node;
        *node = (*node)->next;
    }
}


static void destroy_node(HashNode *node){
    free(node);
}


HashTable* hashtable_init(uint32_t (*fn) (const char*)){
    uint32_t num_entries, i;
    HashTable* hashtable;
    HashNode** nodes;

    if(fn == NULL){
        fn = simple_hash;
    }

    num_entries = fn(NULL);
    hashtable = (HashTable*) malloc(sizeof(HashTable));
    if (hashtable == NULL){
        return NULL;
    }

    nodes = (HashNode**) malloc(num_entries * sizeof(HashNode*));
    if(nodes == NULL){
        return NULL;
    }

    hashtable->fn = fn;
    for(i=0; i < num_entries; i++){
        nodes[i] = NULL;
    }
    hashtable->nodes = nodes;

    return hashtable;
}

void hashtable_destroy(HashTable* hashtable){
    HashNode* current, *next;
    uint32_t num_entries, i;
    num_entries = hashtable->fn(NULL);
    for(i = 0; i < num_entries; i++){
        current = hashtable->nodes[i];
        while(current!=NULL){
            next = current->next;
            destroy_node(current);
            current = next;
        }
    }
    free(hashtable->nodes);
    free(hashtable);
}


void* hashtable_get(HashTable* hashtable, const char* key){
    HashNode *node, *prev;
    uint32_t index;
    locate(hashtable, key, &index, &prev, &node);
    if (node == NULL) return NULL;
    return node->value;
}


int hashtable_set(HashTable* hashtable, const char* key, void* value){
    HashNode *node, *prev;
    uint32_t index;
    locate(hashtable, key, &index, &prev, &node);
    if(node == NULL){
        node = (HashNode *) malloc(sizeof(HashNode));
        if(node == NULL){
            return 1;
        }
        node->key = (char *) key;
        node->value = value;
        node->next = NULL;
        if(prev == NULL){
            hashtable->nodes[index] = node;
        }
        else{
            prev->next = node;
        }
    }
    else {
        node->value = value;
    }
    return 0;
}


void hashtable_del(HashTable* hashtable, const char* key){
    HashNode *node, *prev;
    uint32_t index;
    locate(hashtable, key, &index, &prev, &node);
    if(node != NULL){
        if(prev == NULL){
            hashtable->nodes[index] = node->next;
        }
        else{
            prev->next = node->next;
        }
        destroy_node(node);
    }
}
