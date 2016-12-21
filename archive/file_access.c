/*
* Auteur : Ch. Duhil
* Auteur :Ti Hulin
* Date de Création : 13 décembre 2016
*
* implementation fonctions  for a learning machin  on transition page.
* The gaol of this fontions is 
*/
#include <linux/slab.h>
#include <linux/list.h> 
#include "file_access.h"

struct transition_file* file_head = NULL;
// initialion

//TODO: pg_numb doit strictement supérieure à la valeur max de current_page
int init_transition_file(const char *name, int pg_numb){
	if(init_file_access(file_head, name, pg_numb)) {
		return -1;
	}
	//initialiser file_list
	INIT_LIST_HEAD(&(file_head->file_list));

	return 0;
}

void add_transition_file(const char *name, int pg_numb){

	struct transition_file element;
	if (!file_head) {
		return init_transition_file(name, pg_numb);
	}
	
	if(init_file_access(element, name, pg_numb)) {
		return;
	}
	//ajouter
	list_add(&(element->file_list), &(file_head->file_list));
}

int init_file_access(struct transition_file* file, const char *name, int pg_numb){
	// copier name dans une stucture allouée
	void* tmp = kmalloc(sizeof(struct transition_file), NULL);

	if(!tmp) {
		printk(KERN_WARNING "Cannot allocate transition struct");
		return -1;
	}
	file = tmp;



	tmp = kmalloc(pg_numb*sizeof(struct transition_list*), NULL);
	if(!tmp) {
		printk(KERN_WARNING "Cannot allocate transition page array");
		return -1;
	}
	file->list_array = tmp;

	file->current_page = -1;

	tmp = kmalloc(pg_numb*sizeof(int), NULL);
	if(!tmp) {
		printk(KERN_WARNING "Cannot allocate occurencies array");
		return -1;
	}
	file->freq_array = tmp;

	return 0;
}

// rechercher un fichier dans la liste des fichiers
 

//TODO: qu'est-ce qu'il se passe pour la première page
/*
// create a new node in  transition_list
int add_new_occurence (struct transition_file *file, int next_page){

	if(!file){
		printk(KERN_WARNING "invalide transition_file");
		return -1;
	}


	struct transition_list* current_list = file->list_array[file->current_page];
	//si la liste n'a jamais été crée
	if (!current_list) {
		// créer le premier élément
		current_list = kmalloc(sizeof(struct transition_list));
		if (!current_list) {
			return -1;
		}
		current_list->page_number = next_page;
		current_list->occurence_number = 1;

		//initialiser la liste
		INIT_LIST_HEAD(current_list->list);
		return 0;
	}

	struct transition_list* next_node = get_page_node(current_list, next_page);
	//s'il y a déjà un élément dans la liste
	if (next_node) {
		next_node->occurence_number ++;
		file->freq_array[file->current_page] ++;

	}
	//si l'élément n'existe pas
	else {
		next_node = kmalloc(sizeof(struct transition_list));
		if (!next_node) {
			return -1;
		}
		next_node->page_number = next_page;
		next_node->occurence_number = 1;
		list_add(&(next_node->list), &(current_list->list));
	}
	
}


// function is_in_list
(struct transition_list)* get_page_node(struct transition_list *t_list, int page) {
	struct transition_list *tmp;
	struct list_head *pos;

	list_for_each(pos, &(t_list->list)){

		//
		 tmp = list_entry(pos, struct transition_list, list);

		 //
		 if (tmp->page_number == page) 
		 	return tmp;

		 printk(KERN_INFO "page_number = %d\n", tmp->page_number);

	}



	return NULL;
}
*/



// TODO: faire la fonction pour kfree()
