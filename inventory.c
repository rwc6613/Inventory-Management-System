/*
 * @author: Ryan Chen, rwc6613@rit.edu
 */

// inventory.c file
#include "inventory.h"

#define MAX_LINE_LENGTH 256

inventory_t inv = {.part_list = NULL, .part_count = 0, .assembly_list = NULL, .assembly_count = 0};

void add_part(inventory_t * invp, char * id){
        struct part * current = invp->part_list;

        // checking for invalid ID
        if (id[0] != 'P'){
                fprintf(stderr, "!!! %s: part ID must start with 'P'\n", id);
                return;
        }
        if (strlen(id) > ID_MAX){
                fprintf(stderr, "!!! %s: part ID too long\n", id);
                return;
        }

        // checking for duplicate ID
        part_t * part_lookup_pointer = lookup_part(inv.part_list, id);
        if (part_lookup_pointer != NULL){
                fprintf(stderr, "!!! %s: duplicate part ID\n", id);
                return;
        }

        // creating new part
        struct part * new_part = (struct part *)malloc(sizeof(struct part));
        // checking for allocation
        if (new_part == NULL){
                fprintf(stderr, "!!! Memory allocation failed\n");
                return;
        }
        strcpy(new_part->id, id);
        new_part->id[ID_MAX] = '\0';
        new_part->next = NULL;

        if (invp->part_list == NULL){
                invp->part_list = new_part;
        }
        else {
                current = invp->part_list;
                while (current->next != NULL){
                    current = current->next;
                }
                current->next = new_part;
        }

        invp->part_count++;
}

void add_assembly(inventory_t * invp, char * id, int capacity, items_needed_t * items){
        assembly_t * current = invp->assembly_list;

        // checking for invalid ID
        if (id[0] != 'A'){
                fprintf(stderr, "!!! %s: assembly ID must start with 'A'\n", id);
                free(items);
                return;
        }
        if (strlen(id) > ID_MAX){
                fprintf(stderr, "!!! %s: assembly ID too long\n", id);
                free(items);
                return;
        }
        if (capacity < 0){
                fprintf(stderr, "!!! %d: illegal capacity for ID %s\n", capacity, id);
                free(items);
                return;
        }

        // checking for duplicate ID
        assembly_t * assembly_lookup_pointer = lookup_assembly(inv.assembly_list, id);
        if (assembly_lookup_pointer != NULL){
                fprintf(stderr, "!!! %s: duplicate assembly ID\n", id);
                free(items);
                return;
        }

        // creating new assembly
        struct assembly * new_assembly = (struct assembly *)malloc(sizeof(struct assembly));

        // checking for allocation
        if (new_assembly == NULL){
                fprintf(stderr, "!!! Memory allocation failed\n");
                return;
        }
        strcpy(new_assembly->id, id);
        new_assembly->id[ID_MAX] = '\0';
        new_assembly->capacity = capacity;
        new_assembly->on_hand = 0;
        new_assembly->items = items;
        new_assembly->next = NULL;

        // adding the assembly itself to the inventory
        if (invp->assembly_list == NULL){
                invp->assembly_list = new_assembly;
        }
        else {
                current = invp->assembly_list;
                while (current->next != NULL){
                        current = current->next;
                }
                current->next = new_assembly;
        }

        invp->assembly_count++;
}

void add_item(items_needed_t * items, char * id, int quantity){
        // looking for matching item IDs
        item_t * item_lookup_pointer = lookup_item(items->item_list, id);

        // if the item was found, aka != NULL, add quantity, otherwise make the new item
        if (item_lookup_pointer != NULL){
                item_lookup_pointer->quantity += quantity;
        }
        else{
                // making the new item
                struct item * new_item = (struct item *)malloc(sizeof(struct item));
                strcpy(new_item->id, id);
                new_item->quantity = quantity;
                new_item->next = NULL;

                // if-else statement is redundant, can reduce it
                // adding it to the list
                if (items->item_list == NULL){
                        items->item_list = new_item;
                }
                else{
                        struct item * current_item = items->item_list;
                        while (current_item->next != NULL){
                                current_item = current_item->next;
                        }
                        current_item->next = new_item;
                }
                items->item_count += 1;
        }
}

void fulfillOrder(char * order){
        items_needed_t * items = calloc(1, sizeof(struct items_needed)); // why calloc calloc is pain

        // main loop for parsing and adding items to item list
        char * token;
        token = strtok(order, " ");
        while (token != NULL){
                char * ID = token;
                char * string_quantity = strtok(NULL, " ");

                // checking for valid inputs before continuing
                if (ID == NULL || string_quantity == NULL){
                        fprintf(stderr, "!!! Invalid input\n");
                        free(items);
                        return;
                }
                int quantity = atoi(string_quantity);

                // checking for valid inputs starting with 'A' and valid quantity number, as well as whether the assembly requested exists
                if (ID[0] != 'A'){
                        fprintf(stderr, "!!! %s: assembly ID is not in the inventory -- order canceled\n", ID);
                        free(items);
                        return;
                }

                assembly_t * assembly_lookup_pointer = lookup_assembly(inv.assembly_list, ID);
                if (assembly_lookup_pointer == NULL){
                        fprintf(stderr, "!!! %s: assembly ID is not in the inventory -- order canceled\n", ID);
                        free(items);
                        return;
                }

                if (quantity <= 0){
                        fprintf(stderr, "!!! %d: illegal order quantity for ID %s -- order canceled\n", quantity, ID);
                        free(items);
                        return;
                }

                add_item(items, ID, quantity);

                token = strtok(NULL, " ");
        }

        struct items_needed * parts = calloc(1, sizeof(struct items_needed));

        // getting and maintaining list for parts requested
        struct item * current_item = items->item_list;
        while (current_item != NULL){
                get(&inv, current_item->id, current_item->quantity, parts);
                current_item = current_item->next;
        }

        // freeing 'items'
        current_item = items->item_list;
        while (current_item != NULL){
                struct item * temp = current_item;
                current_item = current_item->next;
                free(temp);
        }
        free(items);

        // printing 'parts'
        if (parts->item_count > 0){
                printf("Parts needed:\n");
                printf("-------------\n");
                printf("Part ID     quantity\n");
                printf("=========== ========\n");
                item_t ** item_array = to_item_array(parts->item_count, parts->item_list);
                qsort(item_array, parts->item_count, sizeof(item_t *), item_compare); // make the item_compare function

                for (int i = 0; i < parts->item_count; i++){
                        printf("%-11s %8d\n", item_array[i]->id, item_array[i]->quantity);
                }

                free(item_array);
        }
        // freeing 'parts'
        current_item = parts->item_list;
        while (current_item != NULL){
                struct item * temp = current_item;
                current_item = current_item->next;
                free(temp);
        }
        free(parts);
}

void stock(inventory_t * invp, char *id, int n){
        // checks
        if (n <= 0){
                fprintf(stderr, "!!! %d: illegal quantity for ID %s\n", n, id);
        }

        assembly_t * current_assembly = lookup_assembly(inv.assembly_list, id);

        // checking for valid id
        if (current_assembly == NULL){
                fprintf(stderr, "!!! %s: assembly ID is not in the inventory\n", id);
                return;
        }

        // a parts needed list
        items_needed_t * parts = calloc(1, sizeof(struct items_needed));

        char * current_id = current_assembly->id;
        int capacity = current_assembly->capacity;
        int on_hand = current_assembly->on_hand;

        // checking for exceeding maximum capacity; if requested amount results in over capacity, only make enough to capacity
        int amt_needed = n;
        if (on_hand + n > capacity){
                amt_needed = capacity - on_hand;
        }

        make(invp, current_id, amt_needed, parts);
        current_assembly->on_hand += amt_needed;

        // printing out the parts needed
        if (parts->item_count > 0){
                printf("Parts needed:\n");
                printf("-------------\n");
                printf("Part ID     quantity\n");
                printf("=========== ========\n");
                item_t ** item_array = to_item_array(parts->item_count, parts->item_list);
                qsort(item_array, parts->item_count, sizeof(item_t *), item_compare); // make the item_compare function

                for (int i = 0; i < parts->item_count; i++){
                        printf("%-11s %8d\n", item_array[i]->id, item_array[i]->quantity);
                }
        }

        // freeing parts needed list
        item_t * current_item = parts->item_list;
        while (current_item != NULL){
                struct item * temp = current_item;
                current_item = current_item->next;
                free(temp);
        }


        free(parts);
}

void restock(inventory_t * invp, char *id){
        // a parts needed list
        items_needed_t * parts = calloc(1, sizeof(struct items_needed));

        if (id == NULL){
                // turning my list into an array to work from LIFO (last in, first out)
                assembly_t ** assembly_array = to_assembly_array(inv.assembly_count, inv.assembly_list);

                // iterating through each item LIFO
                for (int i = inv.assembly_count - 1; i >= 0; i--){
                        assembly_t * current_assembly;
                        // checking to ensure pointer isn't NULL
                        if (assembly_array[i] != NULL){
                                current_assembly = assembly_array[i];
                        }
                        else{
                                continue;
                        }
                        char * current_id = current_assembly->id;
                        int capacity = current_assembly->capacity;
                        int on_hand = current_assembly->on_hand;

                        // if there aren't enough of a certain assembly, make it and all it's parts
                        if (on_hand < capacity / 2 + 1){
                                int amt_needed = capacity - on_hand;
                                fprintf(stdout, ">>> restocking assembly %s with %d items\n", current_id, amt_needed);
                                make(invp, current_id, amt_needed, parts);
                                current_assembly->on_hand += amt_needed;
                        }
                }

                free(assembly_array);
        }
        else{
                assembly_t * current_assembly = lookup_assembly(inv.assembly_list, id);
                if (current_assembly == NULL){
                        fprintf(stderr, "!!! %s: assembly ID is not in the inventory\n", id);
                        free(parts);
                        return;
                }
                char * current_id = current_assembly->id;
                int capacity = current_assembly->capacity;
                int on_hand = current_assembly->on_hand;

                if (on_hand < capacity / 2 + 1){
                        int amt_needed = capacity - on_hand;
                        make(invp, current_id, amt_needed, parts);
                        current_assembly->on_hand += amt_needed;
                        fprintf(stdout, ">>> restocking assembly %s with %d items\n", current_id, amt_needed);
                }
        }

        // printing out the parts needed
        if (parts->item_count > 0){
                fprintf(stdout, "Parts needed:\n");
                fprintf(stdout, "-------------\n");
                fprintf(stdout, "Part ID     quantity\n");
                fprintf(stdout, "=========== ========\n");
                item_t ** item_array = to_item_array(parts->item_count, parts->item_list);
                qsort(item_array, parts->item_count, sizeof(item_t *), item_compare); // make the item_compare function

                for (int i = 0; i < parts->item_count; i++){
                        printf("%-11s %8d\n", item_array[i]->id, item_array[i]->quantity);
                }

                free(item_array);
        }
        // freeing parts needed list
        item_t * current_item = parts->item_list;
        while (current_item != NULL){
                struct item * temp = current_item;
                current_item = current_item->next;
                free(temp);
        }
        free(parts);
}

void empty(char *id){
        if (id[0] != 'A'){
                fprintf(stderr, "!!! %s: ID not an assembly\n", id);
                return;
        }
        assembly_t * assembly_lookup_pointer = lookup_assembly(inv.assembly_list, id);
        if (assembly_lookup_pointer == NULL){
                fprintf(stderr, "!!! %s: assembly ID is not in the inventory\n", id);
                return;
        }

        assembly_lookup_pointer->on_hand = 0;
}

void inventory(char *id){
        if (id == NULL){
                fprintf(stdout, "Assembly inventory:\n");
                fprintf(stdout, "-------------------\n");

                if (inv.assembly_list == NULL){
                        fprintf(stdout, "EMPTY INVENTORY\n");
                }
                else{
                        fprintf(stdout, "Assembly ID Capacity On Hand\n");
                        fprintf(stdout, "=========== ======== =======\n");

                        assembly_t ** assembly_array = to_assembly_array(inv.assembly_count, inv.assembly_list);
                        qsort(assembly_array, inv.assembly_count, sizeof(assembly_t *), assembly_compare);

                        for (int i = 0; i < inv.assembly_count; i++){
                                fprintf(stdout, "%-11s %8d %7d", assembly_array[i]->id, assembly_array[i]->capacity, assembly_array[i]->on_hand);
                                if (assembly_array[i]->on_hand < (assembly_array[i]->capacity / 2) + 1){
                                        fprintf(stdout, "*\n");
                                }
                                else{
                                        fprintf(stdout, "\n");
                                }
                        }
                        free(assembly_array);
                }
        }
         else{
                // checking for assembly id existing
                assembly_t * assembly = lookup_assembly(inv.assembly_list, id);

                if (assembly == NULL){
                        fprintf(stderr, "!!! %s: part/assembly ID is not in the inventory\n", id);
                        return;
                }

                fprintf(stdout, "Assembly ID:  %s\n", id);
                fprintf(stdout, "bin capacity: %d\n", assembly->capacity);
                fprintf(stdout, "on-hand:      %d\n", assembly->on_hand);

                item_t * items = assembly->items->item_list;
                int item_count = assembly->items->item_count;

                if (items != NULL){
                        item_t ** item_array = to_item_array(item_count, items);
                        // checking to ensure item list isn't empty before outputting

                        qsort(item_array, item_count, sizeof(item_t*), item_compare);
                        fprintf(stdout, "Parts list:\n");
                        fprintf(stdout, "-----------\n");
                        fprintf(stdout, "Part ID     quantity\n");
                        fprintf(stdout, "=========== ========\n");
                        for (int i = 0; i < item_count; i++){
                                fprintf(stdout, "%-15s %4d\n", item_array[i]->id, item_array[i]->quantity);
                        }
                        free(item_array);
                }
        }
}

void parts(){
        // simply printing out what parts we have
        fprintf(stdout, "Part inventory:\n");
        fprintf(stdout, "---------------\n");
        if (inv.part_count == 0){
                fprintf(stdout, "NO PARTS\n");
        }
        else {
                part_t ** part_array = to_part_array(inv.part_count, inv.part_list);
                qsort(part_array, inv.part_count, sizeof(part_t *), part_compare);
                fprintf(stdout, "Part ID\n");
                fprintf(stdout, "===========\n");
                for (int i = 0; i < inv.part_count; i++){
                        fprintf(stdout, "%s\n", part_array[i]->id);
                }
                free(part_array);
        }
}

void help(){
        // copied and pasted from website, all commands
        fprintf(stdout, "Requests:\n");
        fprintf(stdout, "    addPart\n");
        fprintf(stdout, "    addAssembly ID capacity [x1 n1 [x2 n2 ...]]\n");
        fprintf(stdout, "    fulfillOrder [x1 n1 [x2 n2 ...]]\n");
        fprintf(stdout, "    stock ID n\n");
        fprintf(stdout, "    restock [ID]\n");
        fprintf(stdout, "    empty ID\n");
        fprintf(stdout, "    inventory [ID]\n");
        fprintf(stdout, "    parts\n");
        fprintf(stdout, "    help\n");
        fprintf(stdout, "    clear\n");
        fprintf(stdout, "    quit\n");
}

void clear(){
        // clearing parts and resetting count
        part_t * current_part = inv.part_list;
        while (current_part != NULL){
                part_t * temp = current_part;
                current_part = current_part->next;
                free(temp);
        }
        inv.part_count = 0;

        // clearing assemblies and resetting count
        assembly_t * current_assembly = inv.assembly_list;
        while (current_assembly != NULL){
                assembly_t * temp_assembly = current_assembly;
                current_assembly = current_assembly->next;

                // freeing items needed list
                items_needed_t * current_items_needed = temp_assembly->items;
                item_t * current_item = current_items_needed->item_list;
                while (current_item != NULL){
                        item_t * temp_item = current_item;
                        current_item = current_item->next;
                        free(temp_item);
                }
                free(current_items_needed);
                free(temp_assembly);
        }
        inv.assembly_count = 0;
}

void quit(){
        clear();
        exit(EXIT_SUCCESS);
}

// things related to manufacturing
void make(inventory_t * invp, char * id, int n, items_needed_t * parts){
        // basic checks
        if (id[0] != 'A'){
                fprintf(stderr, "!!! %s: assembly ID must start with 'A'\n", id);
                return;
        }
        if (n <= 0){
                return;
        }

        // lookup
        assembly_t * assembly = lookup_assembly(invp->assembly_list, id);
        fprintf(stdout, ">>> make %d units of assembly %s\n", n, id);

        // looping through the item's required items
        item_t ** item_array = to_item_array(assembly->items->item_count, assembly->items->item_list);

        for (int i = assembly->items->item_count - 1; i >= 0; i--){
                item_t * current_item = item_array[i];
                char * current_id = current_item->id; // ID of the current item we are looking through
                int quantity = current_item->quantity; // quantity of the item we need

                if (current_id[0] == 'P'){
                        add_item(parts, current_id, n * quantity);
                }
                else{
                        get(invp, current_id, n * quantity, parts);
                }
                current_item = current_item->next;
        }

        free(item_array);
}

void get(inventory_t * invp, char * id, int n, items_needed_t * parts){
        // basic checks
        if (id[0] != 'A'){
                fprintf(stderr, "!!! %s: assembly ID must start with 'A'\n", id);
                return;
        }
        if (n <= 0){
                return;
        }

        // lookup
        assembly_t * assembly = lookup_assembly(invp->assembly_list, id);
        if (assembly->on_hand >= n){
                assembly->on_hand -= n;
        }
        else{
                int remaining_quantity = n - assembly->on_hand;
                make(invp, id, remaining quantity, parts);
                assembly->on_hand = 0;
        }
}

// things related to parts
part_t ** to_part_array(int count, part_t * part_list){
        part_t ** part_array = (part_t **)malloc(count * sizeof(part_t *));
        if (part_array == NULL){
                fprintf(stderr, "!!! Memory allocation failed\n");
                return NULL;
        }

        part_t * current = part_list;
        for (int i = 0; i < count; i++){
                part_array[i] = current;
                current = current->next;
        }
        return part_array;
}

int part_compare(const void * a, const void * b){
        // need to type-cast the provided parameters to be part_t
        // need to dereference once to point to the object itself
        const part_t * p1 = *(const part_t**)a;
        const part_t * p2 = *(const part_t**)b;
        return strcmp(p1->id, p2->id);
}

// things related to assemblies
assembly_t ** to_assembly_array(int count, assembly_t * assembly_list){
        assembly_t ** assembly_array = (assembly_t **)malloc(count * sizeof(assembly_t *));
        if (assembly_array == NULL){
                fprintf(stderr, "!!! Memory allocation failed\n");
                return NULL;
        }

        assembly_t * current = assembly_list;
        for (int i = 0; i < count; i++){
                assembly_array[i] = current;
                current = current->next;
        }
        return assembly_array;
}

int assembly_compare(const void * a, const void * b){
        const assembly_t * a1 = *(const assembly_t**)a;
        const assembly_t * a2 = *(const assembly_t**)b;
        return strcmp(a1->id, a2->id);
}

// things related to items_needed_t
item_t ** to_item_array(int count, item_t * item_list){
        item_t ** item_array = malloc(count * sizeof(item_t *));
        if (item_array == NULL){
                fprintf(stderr, "!!! Memory allocation failed\n");
                return NULL;
        }

        item_t * current = item_list;
        for (int i = 0; i < count; i++){
                item_array[i] = current;
                current = current->next;
        }
        return item_array;
}

int item_compare(const void * a, const void * b){
        const item_t * i1 = *(const item_t**)a;
        const item_t * i2 = *(const item_t**)b;
        return strcmp(i1->id, i2->id);
}

// lookup functions
part_t * lookup_part(part_t * pp, char * id){
        part_t * pointer;
        while (pp != NULL){
                if (strcmp(pp->id, id) == 0){
                        pointer = pp;
                        return pointer;
                }
                pp = pp->next;
        }
        return NULL;
}

assembly_t * lookup_assembly(assembly_t * ap, char * id){
        assembly_t * pointer;
        while (ap != NULL){
                if (strcmp(ap->id, id) == 0){
                        pointer = ap;
                        return pointer;
                }
                ap = ap->next;
        }
        return NULL;
}

item_t * lookup_item(item_t * ip, char * id){
        item_t * pointer;
        while (ip != NULL){
                if (strcmp(ip->id, id) == 0){
                        pointer = ip;
                        return pointer;
                }
                ip = ip->next;
        }
        return NULL;
}

// main function
int main(int argc, char *argv[]){
        // checking for correct command line size
        if (argc > 2){
                perror("Too many command-line arguments");
                return EXIT_FAILURE;
        }

        // file creation, and determining whether program is reading file or standard input
        FILE *fp;

        if (argc == 2){
                fp = fopen(argv[1], "r");
                if(fp == NULL){
                        perror("Failed to open file");
                        return EXIT_FAILURE;
                }
        }
        else{
                fp = stdin;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), fp) != NULL){

                // handling in case there is an in-line comment
                char * comment_pos = strchr(line, '#'); // finding index of comment
                if (comment_pos != NULL){
                    *comment_pos = '\0';
                }
                char * trimmed_line = trim(line);

                // checking for empty lines or comment lines
                if (strlen(trimmed_line) == 0){
                        continue;
                }
                if (trimmed_line[0] == '#'){
                        continue;
                }

                // echo back the request
                fprintf(stdout, "+ %s\n", trimmed_line);

                // creating token to read requests
                char * token;
                int line_length = strlen(trimmed_line);
                token = strtok(trimmed_line, " ");

                // checking for requests
                if (strcmp(token, "quit") == 0){
                        fclose(fp);
                        quit();
                }
                else if (strcmp(token, "addPart") == 0){
                        char * ID = strtok(NULL, " ");

                        add_part(&inv, ID);
                }
                else if (strcmp(token, "addAssembly") == 0){
                        // assists in checking invalid input
                        int errorChecker = 1;

                        char * ID = strtok(NULL, " ");
                        if (ID == NULL){
                                fprintf(stderr, "!!! Invalid input\n");
                                continue;
                        }

                        char * capacityString = strtok(NULL, " ");
                        if (capacityString == NULL){
                                fprintf(stderr, "!!! Invalid input\n");
                                continue;
                        }
                        int capacity = atoi(capacityString);

                        // creating items list
                        items_needed_t * items = malloc(sizeof(items_needed_t));
                        if (items == NULL){
                                fprintf(stderr, "!!! Memory allocation failed\n");
                                free(items);
                        }

                        items->item_list = NULL;
                        items->item_count = 0;

                        // keeping track of the last item added
                        token = strtok(NULL, " ");
                        while (token != NULL){
                                char itemName[ID_MAX + 1];
                                int quantity;

                                strcpy(itemName, token);
                                token = strtok(NULL, " ");

                                // checking for mismatched ITEM-ID pairing
                                if (token == NULL){
                                        errorChecker = -1;
                                        fprintf(stderr, "!!! Invalid input\n");
                                        item_t * current_item = items->item_list;
                                        while (current_item != NULL){
                                                item_t * temp = current_item;
                                                current_item = current_item->next;
                                                free(temp);
                                        }
                                        free(items);
                                        break;
                                }

                                quantity = atoi(token);

                                // remaining checks
                                if (lookup_part(inv.part_list, itemName) == NULL && lookup_assembly(inv.assembly_list, itemName) == NULL){
                                        fprintf(stderr, "!!! %s: part/assembly ID is not in the inventory\n", itemName);
                                        errorChecker = -1;
                                        item_t * current_item = items->item_list;
                                        while (current_item != NULL){
                                                item_t * temp = current_item;
                                                current_item = current_item->next;
                                                free(temp);
                                        }
                                        free(items);
                                        break;
                                }

                                if (quantity <= 0){
                                        fprintf(stderr, "!!! %d: illegal quantity for ID %s\n", quantity, itemName);
                                        errorChecker = -1;
                                        item_t * current_item = items->item_list;
                                        while (current_item != NULL){
                                                item_t * temp = current_item;
                                                current_item = current_item->next;
                                                free(temp);
                                        }
                                        free(items);
                                        break;
                                }

                                token = strtok(NULL, " ");

                                add_item(items, itemName, quantity);
                        }

                        if (errorChecker == -1){
                                continue;
                        }

                        add_assembly(&inv, ID, capacity, items);
                }
                else if (strcmp(token, "fulfillOrder") == 0){
                        char * order = malloc(line_length * sizeof(char) + 1);
                        memset(order, 0, line_length * sizeof(char) + 1);
                        // getting the rest of the line into one string
                        token = strtok(NULL, " ");
                        while (token != NULL){
                                strcat(order, token);
                                strcat(order, " ");
                                // printf("%s\n", token);
                                token = strtok(NULL, " ");
                        }
                        // capping off the string
                        strcat(order, "\0");

                        fulfillOrder(order);
                        free(order);
                }
                else if (strcmp(token, "stock") == 0){
                        char * ID = strtok(NULL, " ");
                        char * quantityString = strtok(NULL, " ");
                        int quantity = atoi(quantityString);
                        stock(&inv, ID, quantity);
                }
                else if (strcmp(token, "restock") == 0){
                        char * ID = strtok(NULL, " ");
                        restock(&inv, ID);
                }
                else if (strcmp(token, "empty") == 0){
                        char * ID = strtok(NULL, " ");
                        empty(ID);
                }
                else if (strcmp(token, "inventory") == 0){
                        char * ID = strtok(NULL, " ");
                        inventory(ID);
                }
                else if (strcmp(token, "parts") == 0){
                        parts();
                }
                else if (strcmp(token, "help") == 0){
                        help();
                }
                else if (strcmp(token, "clear") == 0){
                        clear();
                }
                else {
                    fprintf(stderr, "!!! %s: unknown command\n", token);
                }
        }
        clear();
        fclose(fp);
        return EXIT_SUCCESS;
}
