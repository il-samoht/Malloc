#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


void printFreeList();
void *new_malloc(size_t size);
void new_free(void * ptr);
struct node_metadata *findFreelist(size_t size);
void printlist(struct node_metadata *ptrfreelist);
unsigned long countMemory(struct node_metadata *ptrfreelist);
void coalesce(struct node_metadata *ptrfreelist);
struct node_metadata *coalesce_list(struct node_metadata *ptrfreelistCPY, struct node_metadata *current);

typedef struct node_metadata{
    struct node_metadata *prev;
    struct node_metadata *next;
    unsigned long prev_length;
    unsigned long next_length;
    int free;
};

//#define META_SIZE sizeof(struct node_metadata);

struct node_metadata meta;
struct node_metadata freelist;
struct node_metadata* metaptr = &meta;
struct node_metadata* freelistptr = &freelist;

struct node_metadata freelist32;
struct node_metadata freelist64;
struct node_metadata freelist128;
struct node_metadata freelist256;
struct node_metadata freelist512;
struct node_metadata freelist1024;
struct node_metadata freelist2048;
struct node_metadata freelist4096;
struct node_metadata freelist8192;

struct node_metadata *ptrfreelist32 = &freelist32;
struct node_metadata *ptrfreelist64 = &freelist64;
struct node_metadata *ptrfreelist128 = &freelist128;
struct node_metadata *ptrfreelist256 = &freelist256;
struct node_metadata *ptrfreelist512 = &freelist512;
struct node_metadata *ptrfreelist1024 = &freelist1024;
struct node_metadata *ptrfreelist2048 = &freelist2048;
struct node_metadata *ptrfreelist4096 = &freelist4096;
struct node_metadata *ptrfreelist8192 = &freelist8192;


int main(){  
    /*
    printf("%p\n", new_malloc(0));
    printf("%p\n", new_malloc(1024));
    printf("%p\n", new_malloc(1024));
    printf("%p\n", (metaptr->next + sizeof(struct node_metadata)) );
    new_free((void*) &meta);
    printf("%p\n", new_malloc(1024));
    */
    while(1){
        char func;
        size_t size = 0;
        scanf("%c", &func);
        scanf("%li", &size);
        if(func == 'A'){
            new_malloc(size);
            
        }else if(func == 'F'){
            new_free((void*) size);
        }
        printf("\n");
    }
}

void *new_malloc(size_t size){

    if(size > 0){
        void *adress;

        struct node_metadata *previous;
        previous = metaptr;
        
        //find current metadata
        while(previous->next != NULL){
            previous = previous->next;
        }
        //find space in freelist
        struct node_metadata *lastFree;
        lastFree = findFreelist(8192 - size);
        int freelist_used = 0;
        //printf("test***************\n");
        while(lastFree != NULL){
            //add to meta list
            if(lastFree->next_length != 0){
                //printf("test see if any free memory has been checked\n");
                //if(size <= lastFree->next_length){
                if((size + sizeof(struct node_metadata)) <= lastFree->next_length){
                    //printf("free list used\n");
                    freelist_used = 1;
                    
                    //remove from freelist
                    if(lastFree->prev == NULL){ //first of list
                        freelistptr->next->prev = NULL;
                        freelistptr = freelistptr->next; 
                    }else if(lastFree->next == NULL){//last of list
                        lastFree->prev->next = NULL;
                    }else{//anywhere else of list
                        lastFree->prev->next = lastFree->next;
                        lastFree->next->prev = lastFree->prev;
                    }
                    
                    size_t remainder = lastFree->next_length - size;
                    size_t remainder_next_length = lastFree->next_length - size - sizeof(struct node_metadata);

                    previous->next = lastFree;
                    lastFree->prev = previous;
                    lastFree->next = NULL;
                    lastFree->prev_length = previous->next_length;
                    lastFree->next_length = size;
                    
                    //remainder
                    previous = lastFree;
                    if(remainder > sizeof(struct node_metadata)){
                        lastFree = NULL;
                        lastFree = findFreelist(remainder_next_length);
                        while(lastFree->next != NULL){
                            lastFree = lastFree->next;
                        }
                        struct node_metadata *restFree = (((void*)previous) + sizeof(struct node_metadata) + size);
                    
                        lastFree->next = restFree;
                        restFree->prev = lastFree;
                        //restFree->next_length = (lastFree->next_length - (size + sizeof(struct node_metadata))); //COME BACK TO THIS LINEEEEEEE IT IS CURRENTLY CALCULATING THE REMAINING SIZE WRONG
                        restFree->next_length = remainder_next_length;
                        restFree->prev_length = lastFree->next_length;
                        restFree->free = 1;                   
                    }
                    


                    previous->free = 0;
                    printFreeList();
                    return lastFree + sizeof(struct node_metadata);
                }   
            }
            lastFree = lastFree->next;
        } 

        if(freelist_used == 0){
            struct node_metadata *new_meta;
            if((size + sizeof(struct node_metadata)) <=8192){
                //find last free
                lastFree = NULL;
                lastFree = findFreelist(8192 - (size + sizeof(struct node_metadata)));
                while(lastFree->next != NULL){
                    lastFree = lastFree->next;
                }

                int remaining_size = (8192 - (size + sizeof(struct node_metadata)));
                new_meta = sbrk(8192);
                struct node_metadata *restFree = (((void*)new_meta) + sizeof(struct node_metadata) + size);

                if((8192 - (size + sizeof(struct node_metadata))) > sizeof(struct node_metadata)){
                    lastFree->next = restFree;
                    restFree->prev = lastFree;
                    restFree->next_length = (8192 - (size + sizeof(struct node_metadata)));
                    restFree->prev_length = lastFree->next_length;
                    restFree->free = 1;
                }
                

            }else{
                //mmap
            }
            
            

            //struct node_metadata *new_meta;
            //new_meta = sbrk(size + sizeof(struct node_metadata));
            previous->next = new_meta;
            new_meta->prev = previous;
            new_meta->prev_length = previous->next_length;
            new_meta->next_length = size;
            new_meta->free = 0;

            printFreeList();
            return new_meta + sizeof(struct node_metadata);
        }
    }else{
        printFreeList();
        return sbrk(0);
    }
    
    
    
}

void new_free(void * ptr){
    void* findptr = ptr - sizeof(struct node_metadata);
    //printf("%p\n", findptr);
    struct node_metadata *current;
    /*
    current = metaptr;
    while(current->next != NULL){
        if(current == findptr){
            break;
        }
        current = current->next;
    }
    */
    current = findptr;

    struct node_metadata *last_free;
    last_free = findFreelist(current->next_length);
    while(last_free->next != NULL){
        last_free = last_free->next;
    }

    if(current->prev->next_length == 0){
        //metaptr = current->next;
        metaptr->next = current->next;
    }else if(current->next == NULL){
        current->prev->next = NULL;
    }else{
        current->prev->next = current->next;
        current->next->prev = current->prev;
    }
    last_free->next = current;
    current->free = 1;
    current->next = NULL;
    current->prev = last_free;
    
    coalesce(current);

    printFreeList();
    //printf("%p\n", current->next);
}

void coalesce(struct node_metadata *ptrfreelist){
    struct node_metadata *current = ptrfreelist;
    struct node_metadata *result = NULL;
    size_t currentLength = current->next_length;
    //ptrfreelist32
    result = coalesce_list(ptrfreelist32, current);
    if(currentLength != result->next_length){
        //printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }
    
    //ptrfreelist64
    result = coalesce_list(ptrfreelist64, current);
    if(currentLength != result->next_length){
        //printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }
    
    //ptrfreelist128
    result = coalesce_list(ptrfreelist128, current);
    if(currentLength != result->next_length){
        //printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }
    
    //ptrfreelist256
    result = coalesce_list(ptrfreelist256, current);
    if(currentLength != result->next_length){
        //printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }

    //ptrfreelist512
    result = coalesce_list(ptrfreelist512, current);
    if(currentLength != result->next_length){
        // printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }

    //ptrfreelist1024
    result = coalesce_list(ptrfreelist1024, current);
    if(currentLength != result->next_length){
        // printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }

    //ptrfreelist2048
    result = coalesce_list(ptrfreelist2048, current);
    if(currentLength != result->next_length){
        // printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }

    //ptrfreelist4096
    result = coalesce_list(ptrfreelist4096, current);
    if(currentLength != result->next_length){
        // printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }

    //ptrfreelist8192
    result = coalesce_list(ptrfreelist8192, current);
    if(currentLength != result->next_length){
        // printf("coalesced\n");
        struct node_metadata *last_free = findFreelist(result->next_length + sizeof(struct node_metadata));
        while(last_free->next != NULL){
            last_free = last_free->next;
        }
        last_free->next = result;
        result->next = NULL;
    }
    
}

// this would return a node_metadata pointer no matter it has or hasnt been changed and will be re-allocated in the free list if its not the same as before
struct node_metadata *coalesce_list(struct node_metadata *ptrfreelistCPY, struct node_metadata *current){
    struct node_metadata *ptrfreelist = ptrfreelistCPY;
    //printf("current = %p\n", current);
    while(ptrfreelist != NULL){
        if(ptrfreelist->next_length != 0){
            int modified = 0;
            //current and behind block
            struct node_metadata *calculatednextPos = (void*)current + current->next_length + sizeof(struct node_metadata);
            printf("            behind: %p\n", ptrfreelist->next);
            printf("current           : %p\n\n", calculatednextPos);
            if(calculatednextPos == ptrfreelist->next){
                current->next_length += sizeof(struct node_metadata) + ptrfreelist->next->next_length;
                if(ptrfreelist->next->next == NULL){
                    ptrfreelist->next = NULL;
                }else{
                    ptrfreelist->next->next->prev = ptrfreelist;
                    ptrfreelist->next = ptrfreelist->next->next;
                }
                printf("*************************************\n");
                //test
                printFreeList();
                return coalesce_list(ptrfreelist, current);
            }
            
            
            //front and current block
            calculatednextPos = (void*)ptrfreelist + ptrfreelist->next_length + sizeof(struct node_metadata);
            printf("front                  : %p\n", calculatednextPos);
            printf("          current block: %p\n\n", current);
            if(calculatednextPos == (void*)current){
                
                ptrfreelist->next_length += sizeof(struct node_metadata) + current->next_length;
                //printf("front length += struct size + next length :    %li\n", ptrfreelist->next_length);
                
                if(ptrfreelist->prev->next_length != 0){
                    if(ptrfreelist->next != NULL){
                        ptrfreelist->prev->next = ptrfreelist->next;
                        ptrfreelist->next->prev = ptrfreelist->prev;
                        printf("ptrfreelist->next is not null\n");
                    }else{
                        printf("ptrfreelist->prev->next = %p\n", (void*)ptrfreelist->prev->next);
                        ptrfreelist->prev->next = NULL;
                        printf("ptrfreelist->next is null\n");
                        printf("ptrfreelist->prev->next = %p\n", ptrfreelist->prev->next);
                    }
                }else{
                    ptrfreelistCPY->next = NULL;
                }
                
                
                return coalesce_list(ptrfreelistCPY, ptrfreelist);
                //return ptrfreelist;
            }
            

            
            
        }
        ptrfreelist = ptrfreelist->next;
        
    }
    return current;
}


void printFreeList(){
    //// count total free memory
    unsigned long total_memory = countMemory(ptrfreelist32) + countMemory(ptrfreelist64) + countMemory(ptrfreelist128) + countMemory(ptrfreelist256) + countMemory(ptrfreelist512) + countMemory(ptrfreelist1024) + countMemory(ptrfreelist2048) + countMemory(ptrfreelist4096) + countMemory(ptrfreelist8192);

    printf("Total Free Memory: %li\n", total_memory);

    printf("32-63         : ");
    printlist(ptrfreelist32);
    printf("64-127        : ");
    printlist(ptrfreelist64);
    printf("128-255       : ");
    printlist(ptrfreelist128);
    printf("256-511       : ");
    printlist(ptrfreelist256);
    printf("512-1023      : ");
    printlist(ptrfreelist512);
    printf("1024-2047     : ");
    printlist(ptrfreelist1024);
    printf("2048-4095     : ");
    printlist(ptrfreelist2048);
    printf("4096-8191     : ");
    printlist(ptrfreelist4096);
    printf("8192-Infinity : ");
    printlist(ptrfreelist8192);

    printf("\nAddresslist : ");
    printlist(metaptr);
}

unsigned long countMemory(struct node_metadata *ptrfreelist){
    unsigned long total = 0;
    while(ptrfreelist != NULL){
        total += ptrfreelist->next_length;
        ptrfreelist = ptrfreelist->next;
    }
    return total;
}

void printlist(struct node_metadata *ptrfreelist){
    
    while(ptrfreelist != NULL){
        if(ptrfreelist->next_length != 0){
            printf("%p - %li | ", ((void*) ptrfreelist + sizeof(struct node_metadata)), ptrfreelist->next_length);
        }
        ptrfreelist = ptrfreelist->next;
    }
    
    //printf("")
    printf("\n");
}
struct node_metadata *findFreelist(size_t size){
    size_t totalsize = size + sizeof(struct node_metadata);

    if(totalsize < 64){
        return ptrfreelist32;
    }else if(totalsize < 128){
        return ptrfreelist64;
    }else if(totalsize < 256){
        return ptrfreelist128;        
    }else if(totalsize < 512){
        return ptrfreelist256;
    }else if(totalsize < 1024){
        return ptrfreelist512;
    }else if(totalsize < 2048){
        return ptrfreelist1024;        
    }else if(totalsize < 4096){
        return ptrfreelist2048;        
    }else if(totalsize < 8192){
        return ptrfreelist4096;        
    }else{
        return ptrfreelist8192;
    }

}
