#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h> /* pour les jprobes */
#include <linux/dcache.h> /* pour struct dentry */
#include <linux/mm.h>
#include <linux/fs.h> // pour struct file
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/list.h> 
#include <linux/string.h> //memset et strncpy
//ecriture dans /proc
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "file_access.h"

#define BUF_SIZE            256
#define PROC_FILE           "mmap_transitions"

/* instantiation de la liste de fichier*/
static struct transition_file* file_head = NULL; 

/* Buffer des transitions à afficher dans /proc/mmap_transitions*/
static struct mmap_data data[BUF_SIZE];
/* Indice de lecture */
static int rpos = 0;
/* Indice d'écriture */
static int wpos = 0;


/**
 * Fonction utilitaire qui retourne l'indice suivant (en tenant compte de
 * la circularité du buffer.
 * @param pos un indice
 */
static int get_next_pos(int pos)
{
	/* test du paramètre en entrée */
	if(pos < 0 || pos >= BUF_SIZE) 
		return 0;

	return (pos == BUF_SIZE - 1) ? 0 : pos + 1;
}

//=============================================================================
//
// TRANSITION_FILE OPS
//
//=============================================================================
/*
* init_transition_file 
* initialise la liste des fichiers contenant les données de transition transition_list
* @param *name : nom du premier fichier
* @param nb_of_pages : nombre de pages de ce fichier. 
* @return : retourne 0 si l'initialisation est bonne retourne -1 si erreur
*/
int init_transition_file(const char *name, int nb_of_pages) {
	// initialisation
	if(init_file_access(&file_head, name, nb_of_pages)) {
		printk(KERN_WARNING "[M2LSE] init_transition_file : Error when initializing transition_file\n");
		return -1;
	}
	
	//initialiser file_list
	INIT_LIST_HEAD(&(file_head->file_list));

	return 0;
}


/**
* add_transition_file
* ajout d'un fichier dans la liste transition_file
* @param *name : nom du fichier
* @param  nb_of_page : nombre de pages du fichier
* @param file_node: adresse du noeud ajouté à la liste
*/
int add_transition_file(const char *name, int nb_of_pages, struct transition_file** file_node) {

	struct transition_file* element; // déclaration d'une variable element
	element = NULL;
	
	if (!file_head) {
		printk(KERN_WARNING "[M2LSE] add_transition_file : no head created\n");
		return -1;
	}
	
	if (init_file_access(&element, name, nb_of_pages)) {
		printk(KERN_WARNING "[M2LSE] add_transition_file : Error when initializing transition_file\n");
		return -1;
	}

	list_add(&(element->file_list), &(file_head->file_list));
	*file_node = element;
	
	return 0;
}
/**
* init_file_access 
* initialise un nouveau noeud dans transition_file
* @param *name : nom du premier fichier
* @param nb_of_pages : nombre de pages de ce fichier. 
* @return : retourne 0 si l'initialisation est bonne retourne -1 si erreur
*/
int init_file_access(struct transition_file** file, const char *name, int nb_of_pages) {
	// copier name dans une stucture allouée
	*file = kmalloc(sizeof(struct transition_file), 0);

	if(!(*file)) {
		printk(KERN_WARNING "[M2LSE] init_file_access : Cannot allocate transition struct\n");
		return -1;
	}
	memset((*file), 0, sizeof(struct transition_file));


	(*file)->list_array = kmalloc(nb_of_pages*sizeof(struct transition_list*), 0);
	if(!(*file)->list_array) {
		printk(KERN_WARNING "[M2LSE] init_file_access : Cannot allocate transition page array\n");
		return -1;
	}
	//On nettoie la zone mémoire allouée
	memset((*file)->list_array, 0, nb_of_pages*sizeof(struct transition_list*));

	strncpy((*file)->file_name, name, PATH_STR_LEN);

	(*file)->current_page = -1;

	(*file)->freq_array = kmalloc(nb_of_pages*sizeof(int), 0);
	if(!(*file)->freq_array) {
		printk(KERN_WARNING "[M2LSE] init_file_access : Cannot allocate occurencies array\n");
		return -1;
	}
	memset((*file)->freq_array, 0, nb_of_pages*sizeof(int));

	return 0;
}

/**
* init_transition_list 
* initialise un nouveau noeud dans transition_list
* @param current_list : adresse de la liste à initialiser
* @param page : page du premier noeud. 
* @return : retourne 0 si l'initialisation est bonne retourne -1 si erreur
*/
int init_transition_list(struct transition_list **current_list, int page) {
	(*current_list) = kmalloc(sizeof(struct transition_list), 0);
	if (!(*current_list)) {
		printk(KERN_WARNING "[M2LSE] add_new_occurence : cannot allocate transition_list\n");
		return -1;
	}
	(*current_list)->page_number = page;
	(*current_list)->occurence_number = 1;
	//printk(KERN_INFO "[M2LSE] add_new_occurence : page %d, create list\n", (*current_list)->page_number);

	return 0;
}
 

/** 
* add_new_occurence
* create a new node in  transition_list
* @param file adresse du noeud du fichier auquel ajouter la transition
* @param next_page la dernière page chargée pour ce fichier
*/
int add_new_occurence(struct transition_file **file, int next_page) {
	struct transition_list* current_list;
	struct transition_list* next_node;
	struct mmap_data* current_data;

	next_node = NULL;
	current_list = NULL;
	current_data = NULL;

	if (!(*file)) {
		printk(KERN_WARNING "[M2LSE] add_new_occurence : invalid transition_file\n");
		return -1;
	}

	//cas de la toute première page
	if ((*file)->current_page == -1) {
		(*file)->current_page = next_page; // on définit la première page avant de définir la liste
		return 0;
	}

	//si la liste n'a jamais été créée pour cette page
	current_list = (*file)->list_array[(*file)->current_page];
	if (!current_list) {
		// créer la tete: elle n'est pas considérée comme un noeud par get_page_node
		if (init_transition_list(&current_list, -1)) {
			printk(KERN_WARNING "[M2LSE] add_new_occurence : cannot create transition_list\n");
			return -1;
		}
		//initialiser la liste
		INIT_LIST_HEAD(&(current_list->list));

		(*file)->list_array[(*file)->current_page] = current_list;
		(*file)->freq_array[(*file)->current_page] = 0;
	}

	if (get_page_node(&current_list, next_page, &next_node)) {
		printk(KERN_WARNING "[M2LSE] add_new_occurence : cannot iterate on list\n");
		return -1;
	}

	//si l'élément n'existe pas
	if (!next_node) {
		if (init_transition_list(&next_node, next_page)) {
			printk(KERN_WARNING "[M2LSE] add_new_occurence : cannot create new node\n");
			return -1;
		}
		list_add(&(next_node->list), &(current_list->list));
	}
	//s'il y a déjà un élément dans la liste
	else {
		next_node->occurence_number ++;
	}

	(*file)->freq_array[(*file)->current_page] ++;

	//écrire les données pour /proc
	current_data = &(data[wpos]);
	strncpy(current_data->file_name, (*file)->file_name, PATH_STR_LEN);
	current_data->current_page = (*file)->current_page;
	current_data->next_page = next_page;
	current_data->occurence = (next_node)? next_node->occurence_number : 1;
	current_data->total_occurence = (*file)->freq_array[(*file)->current_page];

	// incrémentation de l'indice d'écriture
	wpos = get_next_pos(wpos);

	// test de l'overflow
	if (rpos == get_next_pos(wpos))
	{
		// attention overflow des données écrites
		rpos = wpos;
	}


	(*file)->current_page = next_page;
	return 0;
}


/**
* get_page_node
* place le pointeur de transition_list sur le neud recherché
* @param *t_list : l'instance de la transition_list
* @param page : numero de la page recherche 
*/
int get_page_node(struct transition_list **t_list, int page, struct transition_list** element) {
	struct transition_list *item; // variable temporaire 
	struct list_head *pos; // Pointeur sur la position dans le list placé sur la tete.
	
	if(!(t_list) || !(*t_list)) {
		printk(KERN_WARNING "[M2LSE] get_page_node : Liste invalide\n");
		return -1;
	}

	list_for_each(pos, &((*t_list)->list)){  
		//On recupere la structure contenant la list_head
		item = list_entry(pos, struct transition_list, list);

		//on verifie le numero de la page
		if (item->page_number == page) {
			//printk(KERN_INFO "[M2LSE] get_page_node %d(%d)\n", item->page_number, item->occurence_number);
			*element = item;
			return 0;
		}
		 
	}
	// if (!(*element)){
	// 	printk(KERN_INFO "[M2LSE] get_page_node : la page %d n'existe pas \n", page);
	// }

	return 0;
}

/* get_file_node
* place le pointeur de transition_file sur le noeud recherché
* @param *t_file : l'instance de la transition_file.
* @param *f_file_name : nom du fichier recherché.
* @param *element = NULL .. recupération du pointeur sur le noeud recherché 
* @return -1 si la tête de liste est invalide, 0 autrement
*/

int get_file_node(struct transition_file **t_file, const char *f_file_name, struct transition_file** element) {
	struct transition_file *item;
	struct list_head *pos;

	if(!(t_file) || !(*t_file)) {
		printk(KERN_WARNING "[M2LSE] get_file_node : Invalid transition_file list\n");
		return -1;
	}

	list_for_each(pos, &((*t_file)->file_list)){
		item = list_entry(pos, struct transition_file, file_list);
		// test sur le nom du fichier 
		if (strlen(item->file_name) == strlen(f_file_name)){
			if (!strncmp(item->file_name, f_file_name, strlen(item->file_name))) {
				//printk(KERN_INFO "[M2LSE] get_file_node : fichier  = %s\n", item->file_name);
				*element = item;
				return 0;
			}
		}
	}
	
	//printk(KERN_INFO "[M2LSE] get_file_node : le fichier %s n'existe pas \n", f_file_name);
	
	return 0;
}

static pgoff_t nb_pages = 0;

//=============================================================================
//
// JPROBE HANDLER
//
//=============================================================================


int filemap_fault_handler(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	const char* file_name;
	struct transition_file* element = NULL;
  
	if (!vma->vm_file) {
		jprobe_return();return 0;
	}

	file_name = vma->vm_file->f_dentry->d_name.name;
	nb_pages = (vma->vm_end - vma->vm_start) >> PAGE_CACHE_SHIFT;

	//recuperation du transition_file
	if(!file_head) {
		//creation de la tete de liste
		init_transition_file("", 1);
		if (add_transition_file(file_name, nb_pages, &element) < 0) {
			printk(KERN_WARNING "[M2LSE] : can't create the first file in transition list\n");
			jprobe_return();return 0;
		}
	}
	else {
		// on crée le fichier s'il n'existe pas 
		if (get_file_node(&file_head, file_name, &element) < 0) {
			printk(KERN_WARNING "[M2LSE] : invalid head\n");
			jprobe_return();return 0;
		}
		//si l'élément n'a pas été trouvé, on ajoute
		if (!element && add_transition_file(file_name, nb_pages, &element) < 0) {
			printk(KERN_WARNING "[M2LSE] : can't add the file in transition list\n");
			jprobe_return();return 0;
		}
	}

	// on ajoute une nouvelle occurence
	if (add_new_occurence(&element, vmf->pgoff) < 0) {
		printk("[M2LSE] : add new occurence failed\n");

		jprobe_return();return 0;
	}

	jprobe_return(); return 0;
}


static struct jprobe mmap_probe = {
	. entry = filemap_fault_handler,
	. kp = {
		. symbol_name = "filemap_fault",
	},
};

//=============================================================================
//
// SEQ OPS
//
//=============================================================================


static void *data_seq_start(struct seq_file *s, loff_t *pos)
{
	struct mmap_data *transition = NULL;

	if (rpos == wpos) {
		/* il n'y a pas de transition à lire*/
		//printk(KERN_DEBUG "End reading transitions (%d)\n", rpos);
		return transition;
	}

	transition = &(data[rpos]);

	rpos = get_next_pos(rpos);

	return transition;
}

static void *data_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct mmap_data *transition = NULL;

	if(rpos == wpos)
	{
		/* toutes les transitions ont été lues */
		return transition;
	}

	// Envoie le fev_t suivant
	transition = &(data[rpos]);

	rpos = get_next_pos(rpos);

	return transition;
}

static void data_seq_stop(struct seq_file *m, void *v)
{
}

static int data_seq_show(struct seq_file *m, void *v)
{
	struct mmap_data *transition = (struct mmap_data*) v;
	//on affiche pas d'élément de buffer vide
	if (transition && transition->total_occurence > 0) {
		seq_printf(m, "%s\t%d > %d\t%d/%d\n", transition->file_name, 
			transition->current_page,
			transition->next_page,
			transition->occurence,
			transition->total_occurence);
	}
	return 0;
}

static struct seq_operations data_seq_ops =
{
	.start = data_seq_start,
	.next  = data_seq_next,
	.stop  = data_seq_stop,
	.show  = data_seq_show,
};


//=============================================================================
//
// PROC OPS
//
//=============================================================================

static int mmap_data_proc_open(struct inode *inode, struct  file *file)
{
	printk(KERN_INFO "mmap_data_monitor : open proc\n");
	return seq_open(file, &data_seq_ops);
}

static ssize_t mmap_data_proc_write(struct file* file, const char* buf, size_t len, loff_t* offset)
{
	char c = buf[0];

	printk(KERN_INFO "mmap_data_monitor : write on proc %c\n",c);
	/* TODO: Insérer ici le chargement des données apprises */

	return len;
}


static const struct file_operations mmap_data_proc_fops =
{
	.owner   = THIS_MODULE,
	.open    = mmap_data_proc_open,
	.read    = seq_read,
	.write   = mmap_data_proc_write,
	.llseek  = seq_lseek,
	.release = seq_release,
};

//=============================================================================
//
// MODULE DEFINITION
//
//=============================================================================

static int __init mmap_monitor_init(void)
{
	int res;
	/* pose de la sonde */
	res = register_jprobe(&mmap_probe); 
	if (res < 0) {
		printk(KERN_ALERT "[M2LSE] Erreur pose jprobe ...\n ");
		return -1;
	}

	/* initalisation du point d'entrée dans /proc */
	if(proc_create(PROC_FILE, S_IWUGO | S_IRUGO, NULL, &mmap_data_proc_fops) == NULL) {
		printk("ERROR - proc_create() failed\n");
		return -1;
	}

	printk(KERN_ALERT "[M2LSE] Module chargee\n");
	return 0;
}

static void __exit mmap_monitor_exit(void)
{
	/* libération du point d'entrée de /proc */
	remove_proc_entry(PROC_FILE, NULL);
	
	unregister_jprobe(&mmap_probe);
	printk(KERN_ALERT "[M2LSE] Module dechargee \n ");
}

module_init(mmap_monitor_init);
module_exit(mmap_monitor_exit);

MODULE_AUTHOR("LSE - GROUPE 2");
MODULE_DESCRIPTION("Traceur des acces par mmap au systeme de stockage flash");
MODULE_LICENSE("GPL");