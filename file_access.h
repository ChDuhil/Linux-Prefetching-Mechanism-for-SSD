
// -------------------------------------------------------------------
// Define the data structure of file_access.c used to store the transition history frequencies between file pages.
// 
// Auteurs : Thibaud Hulin
// Auteurs : Christophe Duhil
// 
// creation : 2016-12-12 
// définition de transition_list
//
//--------------------------------------------------------------------

#include <linux/list.h> 

#define PATH_STR_LEN 512

/* @ChD : 	Define the data structure*/
struct transition_file
{
	char file_name [PATH_STR_LEN];  
	int current_page;
	struct list_head file_list;
	int *freq_array; // array of transition frequency.
	struct transition_list **list_array; // array of transition list.
};

/* @ChD : 	Node definition of the transition list.
* 			contain the page number, the number of occurence and kernel list structure.
* 			the structure uses the kernel list define in : include/linux/list.h. 
*/
struct transition_list
{
	int page_number; /*@ChD : the number of the page read after the current_page >> next page number*/
	int occurence_number; /* @ChD : Number of the occurence of the transition. */
	struct list_head list; 
};

/* @ThH: struct used to store data needed to show transitions in userspace*/
struct mmap_data
{
	char file_name [PATH_STR_LEN]; 
	int current_page;
	int next_page;
	int occurence;
	int total_occurence;
};


int init_transition_file(const char *name, int nb_of_pages);
int add_transition_file(const char *name, int nb_of_pages, struct transition_file** file_node);
int init_file_access(struct transition_file** file, const char *name, int nb_of_pages);
int init_transition_list(struct transition_list **current_list, int page);
int add_new_occurence (struct transition_file **file, int next_page);
int get_page_node(struct transition_list **t_list, int page, struct transition_list** element);
int get_file_node(struct transition_file **t_file, const char *f_file_name, struct transition_file** element);