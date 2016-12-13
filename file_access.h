
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

// @ChD : 	Define the data structure
struct transition_file
{
	char *file_name;  
	int current_page;
	int freq_array [] ; // array of transition frequency.
	struct transition_list *list_array []; // array of transition list.
};

// @ChD : 	Node definition of the transition list.
// 			contain the page number, the number of occurence and kernel list tructure.
// 			the strucute uses the kernel list define in : include/linux/list.h. 
struct transition_list
{
	int page_number; //@ChD : number of the page read after the current_page 
	int occurence_number; // @ChD : Number of the occurence of the transition.
	struct list_head list;
};


int init_transition_file(const char *name, int pg_numb);
void add_transition_file(const char *name, int pg_numb);
int init_file_access(struct transition_file* file, const char *name, int pg_numb);