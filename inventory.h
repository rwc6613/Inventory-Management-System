/*
 * @auther: Ryan Chen, rwc6613@rit.edu
 */

#ifndef INVENTORY_H
#define INVENTORY_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "trimit.h"

extern char * trim(char *);
extern int getline(char **, size_t *, FILE *);

#define ID_MAX 11

/*
 * Struct for a "part"
 * @param id - the id associated with a given part, used for identification
 * @param next - pointer to the next part added, in the form of a linked list
 */
struct part {
    char id[ID_MAX+1];        // ID_MAX plus NUL
    struct part * next; // the next part in the list of parts
};

/*
 * Struct for an "assembly"
 * @param id - the id associated with a given assembly, used for identification
 * @param capacity - the maximum number of a given "assembly" can have on-hand
 * @param on_hand - the current amount of the given "assembly" that is available
 * @param items - the "recipe" for the assembly, consisting of "parts"/"assemblies" needed to make it
 * @param next - pointer to the next assembly, in the form of a linked list
 */
struct assembly {
    char id[ID_MAX+1];
    int capacity;
    int on_hand;
    struct items_needed * items; // parts/sub-assemblies needed for this ID
    struct assembly * next;      // the next assembly in the inventory list
};

/*
 * Struct for an "item", which can either be a "part" or an "assembly"
 * @param id - the id associated with a given item, used for identification
 * @param quantity - the amount of the item that is currently available and/or is needed
 * @param next - pointer to the next item, in the form of a linked list
 */
struct item {
    char id[ID_MAX+1];           // ID_MAX plus NUL
    int quantity;
    struct item * next; // next item in the part/assembly list
};

/*
 * Struct for an "inventory", which consists of a list of "parts" and "assemblies"
 * @param part_list - pointer to the first element of the list of parts
 * @param part_count - the amount of parts in the inventory
 * @param assembly_list - pointer to the first element of the list of assemblies
 * @param assembly_count - the amount of assemblies in the inventory
 */
struct inventory {
   struct part * part_list;         // list of parts by ID
    int part_count;                  // number of distinct parts
    struct assembly * assembly_list; // list of assemblies by ID
    int assembly_count;              // number of distinct assemblies
};

/*
 * Struct of an "items_needed" list, which is a list of items needed to make a given "assembly"
 * @param item_list - pointer to the first element of the list of parts
 * @param item_count - the amount of items in "item_list"
 */
struct items_needed {
    struct item * item_list;
    int item_count;
};

// NOTE: pre-provided "request" struct has been removed; not used

// Type-defs for the various structs indicated above
typedef struct inventory inventory_t;
typedef struct items_needed items_needed_t;
typedef struct item item_t;
typedef struct part part_t;
typedef struct assembly assembly_t;

/*
 * FUNCTIONS TO BE IMPLEMENTED
 */

/*
 * Looks up a part with the same ID as the given parameter "id"
 * @param pp - part pointer, points to a list of "parts"
 * @param id - a string ID that we want to lookup
 * @return - returns a pointer whose ID matches given parameter "id"
 *           if such a part doesn't exist, returns NULL
 */
part_t * lookup_part(part_t * pp, char * id);

/*
 * Looks up an assembly with the same ID as the given parameter "id"
 * @param ap - assembly pointer, points to a list of "assemblies"
 * @param id - a string ID that we want to lookup
 * @return - returns a pointer whose ID matches given parameter "id"
 *           if such an assembly doesn't exist, returns NULL
 */
assembly_t * lookup_assembly(assembly_t * ap, char * id);

/*
 * Looks up an item with the same ID as the given parameter "id"
 * @param ip - item pointer, points to a list of "items"
 * @param id - a string ID that we want to lookup
 * @return - returns a pointer whose ID matches given parameter "id"
 *           if such an item doesn't exist, returns NULL
 */
item_t * lookup_item(item_t * ip, char * id);

/*
 * Adds a part to the inventory's parts list
 * @param invp - inventory pointer to the inventory we want to add a part to
 * @param id - a string for the part's name
 */
void add_part(inventory_t * invp, char * id);

/*
 * Adds an assembly to the inventory's assembly list
 * @param invp - inventory pointer to the inventory we want to add an assembly to
 * @param id - a string for the assembly's name
 * @param capacity - an integer that determines how many of this assembly the inventory can hold
 * @param items - a list of the items needed to create the assembly
 */
void add_assembly(inventory_t * invp,
                  char * id,
                  int capacity,
                  items_needed_t * items);

/*
 * Adds an item to the given items_needed list parameter "items"
 * @param items - the items_needed list to add an item to
 * @param id - a string for the item's name
 * @param quantity - the amount of the item that should be added to the items_needed list
 */
void add_item(items_needed_t * items, char * id, int quantity);

/*
 * FUNCTIONS FOR THE INDIVIDAUL REQUESTS
 */

/*
 * Fulfills an order given by the user, and will make more items to fulfill the order if necessary
 * @param order - a string with the user's order request, in the form of [xi ni [xi2 ni2 ...]]
 */
void fulfillOrder(char * order);

/*
 * Stocks the inventory with an assembly with the given parameter "id" by the given paramenter amount "n"; will not stock more than the capacity of the assembly in the inventory
 * @param invp - inventory pointer to the inventory we want to stock to
 * @param id - a string for the assembly's name
 * @param n - the number of assemblies to add to the inventory
 */
void stock(inventory_t * invp, char * id, int n);

/*
 * Restocks either a certain assembly in the inventory, or all assemblies within the inventory
 * @param invp - inventory pointer to the inventory we want to restock
 * @param id - an "optional" parameter; if it is provided, restock the assembly with the given ID, if it is NOT provided, restock all assemblies within the inventory
 */
void restock(inventory_t * invp, char * id);

/*
 * Empties out an entire assembly from the inventory, setting its on_hand to 0
 * @param id - the ID of the assembly we want to empty out
 */
void empty(char * id);

/*
 * Displays the content of the inventory, along with their capacities and amount on hand. If an ID is provided, will instead display contents specifically about the provided parameter "id", along with the components needed to make the assembly with the provided ID
 * @param id - an "optional" parameter; if it is provided, provides specific information about an assembly with that ID, if it is NOT provided, instead displays all assemblies within the inventory
 */
void inventory(char * id);

/*
 * Displays all parts of the inventory
 */
void parts();

/*
 * Displays a list of all possible requests and commands
 */
void help();

/*
 * Completely clears out the inventory, individually clearing all parts, assemblies, and assembly "recipes", then setting part count and assembly count back to 0
 */
void clear();

/*
 * Calls clear() to clear all the inventory, then terminates the program
 */
void quit();


/*
 * THESE ARE USED FOR SORTING PURPOSES
 */
part_t ** to_part_array(int count, part_t * part_list);
assembly_t ** to_assembly_array(int count, assembly_t * assembly_list);
item_t ** to_item_array(int count, item_t * item_list);
int part_compare(const void *, const void *);
int assembly_compare(const void *, const void *);
int item_compare(const void *, const void *);

/*
 * Responsible for making more copies of an assembly
 * @param invp - inventory pointer of the inventory we want to access
 * @param id - the ID of the assembly we want to make
 * @param n - the number of copies of the assembly we want to make
 * @param parts - an items_needed list of the parts we will need to make "n" copies of assembly "id"
 */
void make(inventory_t * invp, char * id, int n, items_needed_t * parts);

/*
 * Gets copies of assemblies that we need to fulfill orders; if there are already enough copies of the ordered assemblies in the inventory, will take from the inventory before making more
 * @param invp - inventory pointer of the inventory we want to access
 * @param id - the ID of the assembly we want to get
 * @param n - the number of copies of the assembly we want to get
 * @param parts - an items_needed list of the parts we will need for "n" copies of assembly "id"
 */
void get(inventory_t * invp, char * id, int n, items_needed_t * parts);

// Note: pre-provided "print" functions and "free_inventory()" function were removed; functionality was either directly implemented into other functions, or were renamed to something else

#endif // INVENTORY_H