
#include <stdio.h>
#include <stdlib.h>
#include <linux/list.h> 
#include "file_access.h"

struct transition_file* file_head;
// initialion

void init_transition_file(const char *name, int pg_numb){
	init_file_access(file_head, name, pg_numb); // TODO:gerer erreur
	//initialiser file_list
	INIT_LIST_HEAD(&(file_head->file_list));
}

void add_transition_file(const char *name, int pg_numb){
	struct transition_file element;
	//TODO: verifier que file_head existe
	init_file_access(element, name, pg_numb); //TODO: gerer erreur
	//ajouter
	list_add(&(element->file_list), &(file_head->file_list));
}

void init_file_access(struct transition_file* file, const char *name, int pg_numb){
	// copier name dans une stucture allouée
	void* tmp = kmalloc(sizeof(struct transition_file), NULL);

	if(!tmp) {
		//error
		return;
	}
	file = tmp;



	tmp = kmalloc(pg_numb*sizeof(struct transition_list*),NULL);
	if(!tmp) {
		//error
		return;
	}
	file->list_array = tmp;

	file->current_page = pg_numb;

	tmp = kmalloc(pg_numb*sizeof(int),NULL);
	if(!tmp) {
		//error
		return;
	}
	file->freq_array = tmp;
}

 


// create a new node


// function is_in_list
bool is_in_list(struct transition_list *t_list, int page) {





	return false;
}