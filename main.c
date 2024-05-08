#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "linkedlist.h"

#define MAXCHAR 100
#define PAGE_SIZE 4
#define MEMORY_SIZE 2048
#define MAX_PAGES 512
#define MIN_PAGES_REQUIRED 4

typedef struct Process{
    int timeArrived;
    char *processName;
    int timeRequired;
    int timeRemaining;
    int memRequired;
    int memAllocated;
    int memLocations[MAX_PAGES+1];
    int lastUsed;
    int allocatedPages;
}Process;

Process *newProcess(int timeArrived, char *processName, int timeRequired, int memRequired, char *mode);
void freeProcess(Process *p, char *mode, int *memory, int *unusedFrames);
void printProcess(Process *p);
Linklist *readFile(char *filename, char *mode);
void executeProcess(Process *process, int quantum, char *mode, int *memory, char *prevProcessName, int simTime, int pleft, int unusedFrames);

void roundRobin(Linklist *notArrive_list, int quantum, char* mode);
int rr_Step3(Process * Process, int quantum, char *mode, int *memory, Linklist *arrivedList, int *unusedFrames, int simulationTime);
int mem_UsageCalc(int *memory, char *mode, int unusedFrames);
Process *find_lruProcess(Linklist *arrivedList, Process *process,int simTime);
void arraySortPrint(int *frames);
void stats(int turnTime[],int requiredTime[],int size,int simTime);
void arraySort(int *frames);

typedef struct Node{
    void *content;
    Node *next;
}Node;

typedef struct Linklist{
    Node *head;
    Node *tail;
}Linklist;

int main(int argc, char *argv[]){
    char *filename=NULL;
    char *mode =NULL;
    int quantum;
    int DEBUG = 1;
    
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-f") == 0){filename = argv[i+1];}
        else if(strcmp(argv[i], "-m") == 0){mode = argv[i+1];}
        else if (strcmp(argv[i], "-q") == 0){quantum = atoi(argv[i+1]);}
    }
    if(0 && DEBUG) printf("%s %s %d\n", filename, mode, quantum);

    Linklist *notArrive_list = readFile(filename, mode);
    roundRobin(notArrive_list, quantum, mode);
    return 0;
}

struct Process *newProcess(int timeArrived, char *processName, int timeRequired, int memRequired, char *mode){
    Process *p = (Process *)malloc(sizeof(struct Process));
    p -> timeArrived = timeArrived;
    p -> processName = processName;
    p -> timeRequired = timeRequired;
    p -> timeRemaining = timeRequired;
    p -> memRequired = memRequired;
    p -> memAllocated = 0;
    p -> memLocations[0] = -1;
    p -> lastUsed = -1;
    p -> allocatedPages = 0;

    //Sets number of pages in process
    if(strcmp(mode, "paged") == 0){
        int pagenumber = (int)ceil(memRequired/(float)PAGE_SIZE);
        p -> memLocations[pagenumber] = -1;
        
        for(int i=0; i < pagenumber; i++){
            p -> memLocations[i] = MAX_PAGES+1;
        }
    }
    if(strcmp(mode, "virtual") == 0){
        int pagenumber = (int)ceil(memRequired/(float)PAGE_SIZE);
        p -> memLocations[pagenumber] = -1;
        for(int i = 0; i < pagenumber; i++){
            p -> memLocations[i] = MAX_PAGES+1;
        }
    }



    return p;
}

void freeProcess(struct Process *p, char *mode, int *memory, int *unusedFrames){
    int DEBUG = 0;
    if(DEBUG && 1) printf("free called\n");
    if(strcmp(mode, "first-fit") == 0){
        int p_start = p -> memLocations[0];
        int p_end = p -> memLocations[1];
        int i = 0;
        int temp;
        while(memory[i] < p_start && memory[i] != -1){
            i += 2;
        }
        //New hole is in middle of memory, add hole and shift everything rightwards
        while(memory[i] != -1){
            i += 2;
            temp = memory[i-2];
            memory[i-2] = p_start;
            p_start = temp;
            temp = memory[i-1];
            memory[i-1] = p_end;
            p_end = temp;
        }
        memory[i] = p_start;
        memory[i+1] = p_end;
        memory[i+2] = -1;
        //Joins any holes together(remove any duplicates) 
        i = 0;
        int j = 0;
        while(memory[i] != -1){
            if(memory[i] != memory[i+1]){
                memory[j] = memory[i];
                j++;
                i++;
            }
            else{
                i += 2;
            }
        }
        memory[j] = -1;

        i = 0;
        while(memory[i] != -1){
            i++;
        }
    }
    else if(strcmp(mode, "paged") == 0){
        int i = 0;
        while(p -> memLocations[i] != -1){
            memory[i] = 0;
            (*unusedFrames)++;
            i++;
        }
    }

    else if(strcmp(mode, "virtual") == 0){
        int i = 0;
        while(p -> memLocations[i] != -1){
            memory[i] = 0;
            if(p->memLocations[i]!=MAX_PAGES+1){
                (*unusedFrames)++;
            }
            i++;
        }
    }


    free(p -> processName);
    free(p);
}

void printProcess(struct Process *p){
    printf("Process name %s: %d, %d, %d\n", p -> processName, p -> timeArrived, p -> timeRequired, p->memRequired);
}

/*Reads provided file, returns linked list of process structs*/
Linklist *readFile(char *filename, char *mode){
    int DEBUG = 0;
    FILE *fp;
    char row[MAXCHAR];
    char *token;
    Linklist *notArrive_list = newLinklist();
    char *fullpath = realpath(filename,NULL);
    //printf("File Path:%s\n",filename);
    fp = fopen(fullpath, "r");
    //printf("File Path:%s\n",fullpath);
    assert(fp != NULL);
    
    while(fgets(row, MAXCHAR, fp) != NULL){
        row[strcspn(row, "\n")] = '\0';
        if(1 && DEBUG == 1 ) printf("Row: %s\n", row);

        int timeArrived;
        char *processName;
        int timeRequired;
        int memRequired;

        for(int i = 0; i < 4; i++){
            if(i == 0){
                token = strtok(row," ");
                timeArrived = atoi(token);
            }
            else token = strtok(NULL, " ");

            if(i == 1){
                processName = (char *)malloc(strlen(token) + 1);
                strcpy(processName, token);
            }
            if(i == 2) timeRequired = atoi(token);
            if(i == 3) memRequired = atoi(token);

            if(1 && DEBUG == 1) printf("Token: %s\n", token);
        }
        
        //Inserts process into struct
        Process *process = newProcess(timeArrived, processName, timeRequired, memRequired, mode);
        //Inserts struct into linked list
        insert_Linklist(notArrive_list, process);
    }
    free(fullpath);
    fclose(fp);
    return notArrive_list;
}



// Function to execute the running process for one quantum
void executeProcess(Process *process, int quantum, char *mode, int *memory, char *prevProcessName, int simTime, int pleft, int unusedFrames) {

    process->lastUsed=simTime;
    
    if(prevProcessName == NULL) prevProcessName = "empty";
    if (process->timeRemaining >= 0 && (strcmp(process -> processName, prevProcessName) != 0 || (pleft-1)>0)) {
        printf("%d,RUNNING,process-name=%s,remaining-time=%d", simTime, process -> processName, process->timeRemaining);

        if(strcmp(mode, "first-fit") == 0) printf(",mem-usage=%d%%,allocated-at=%d", mem_UsageCalc(memory, mode, unusedFrames), process -> memLocations[0]);

        if(strcmp(mode, "paged") == 0 || strcmp(mode, "virtual") == 0){
            printf(",mem-usage=%d%%,mem-frames=", mem_UsageCalc(memory, mode, unusedFrames));
            arraySortPrint(process -> memLocations);
        } 

        printf("\n");
    }

    process->timeRemaining -= quantum;
    if (process->timeRemaining <= 0) {
        process->timeRemaining = 0;
        if(strcmp(mode, "paged") == 0 || strcmp(mode, "virtual") == 0){
            printf("%d,EVICTED,evicted-frames=", simTime + quantum);
            arraySortPrint(process->memLocations);
            printf("\n");
        }
    }
}

void roundRobin(Linklist *notArrive_list, int quantum, char *mode) {
    int DEBUG = 0;
    int simulationTime = 0;
    int remainingProcesses = 0;
    int mem_flag =0;
    Process *prevProcess = NULL;
    char *prevProcessName = NULL;
    Linklist *arrived_list = newLinklist();
    int turnTime[MEMORY_SIZE];
    int requiredtime[MEMORY_SIZE];
    int size = 0;
    //Declares memory space
    int memory[MEMORY_SIZE];
    if(strcmp(mode, "first-fit") == 0){
        memory[0] = 0;
        memory[1] = MEMORY_SIZE;
        memory[2] = -1;
    }
    if(strcmp(mode, "paged") == 0){
        memory[MAX_PAGES] = -1;
        for(int i = 0; i < MAX_PAGES;i++) memory[i] = 0;
    }
    if(strcmp(mode, "virtual") == 0){
        memory[MAX_PAGES] = -1;
        for(int i = 0; i < MAX_PAGES;i++) memory[i] = 0;
    }
    int unusedFrames = MAX_PAGES;
    
    while (!is_empty(notArrive_list) || !is_empty(arrived_list) || prevProcess != NULL) {
        if(DEBUG && 1) printf("Simulation time: %d\n", simulationTime);

        // Move processes from notArrive_list to arrived_list based on arrival time
        while (!is_empty(notArrive_list)) {
            Process *process = (Process *)check_head(notArrive_list);
            if (process->timeArrived <= simulationTime) {
                // Move the process to arrived_list
                if(DEBUG && 1)printf("inserting %s\n", ((Process *)check_head(notArrive_list)) -> processName);
                insert_Linklist(arrived_list, pop_Linklist(notArrive_list));
                remainingProcesses++;
            } else {
                // No more processes to move for this simulation time
                break;
            }
        }

        //Insert process from previous quanta
        if(prevProcess != NULL){
            if(DEBUG && 1) printf("inserting here %s, remaining time:%d \n", prevProcess -> processName,prevProcess->timeRemaining);
            if (prevProcess->timeRemaining == 0) {
                    //Process completed running, free from simulated and real memory
                    remainingProcesses--;
                    printf("%d,FINISHED,process-name=%s,proc-remaining=%d\n", simulationTime, prevProcess -> processName, remainingProcesses);
                    freeProcess(prevProcess, mode, memory, &unusedFrames);
                    prevProcess = NULL;
            } 
            else{
                insert_Linklist(arrived_list, prevProcess);
                prevProcess = NULL;
            }
        }

        if(DEBUG && 0){
            Node *currNode = arrived_list -> head;
            Process *currProcess = currNode -> content;
            printf("RoundRobin - iterating through queue: ");
            printf("%s ", currProcess -> processName);
            while(currNode -> next != NULL){
                //Updates currProcess
                currNode = currNode -> next;
                currProcess = currNode -> content;
                printf("%s ", currProcess -> processName);
                
            }
            printf("\n");
        }

        // Step 2: Check if the currently running process (if any) has completed its execution
        Process *runningProcess = (Process *)check_head(arrived_list);
        if (runningProcess != NULL && runningProcess->timeRemaining == 0) {
            if(DEBUG && 0) printf("Something has been freed!\n");
            // Process has completed its execution, update its state and deallocate memory
            freeProcess(runningProcess, mode, memory, &unusedFrames);
            pop_Linklist(arrived_list); 
            remainingProcesses--;
            continue;
        }

        // Step 3: Determine the process that runs in this cycle
        runningProcess = (Process *)pop_Linklist(arrived_list);
        if(mem_flag==1 && strcmp(mode, "first-fit") == 0 && 0==1 ){
            printf("%d,RUNNING,process-name=%s,remaining-time=%d", simulationTime, runningProcess -> processName, runningProcess->timeRemaining);
            printf(",mem-usage=%d%%,allocated-at=%d\n", mem_UsageCalc(memory, mode, unusedFrames), runningProcess -> memLocations[0]);
            mem_flag=0;
        }
        if (runningProcess != NULL) {
            if(DEBUG && 0 && arrived_list -> head != NULL){
                Node *currNode = arrived_list -> head;
                Process *currProcess = currNode -> content;
                
                printf("RoundRobin - iterating through queue: ");
                printf("%s ", currProcess -> processName);
                while(currNode -> next != NULL){
                    //Updates currProcess
                    currNode = currNode -> next;
                    currProcess = currNode -> content;
                    printf("%s ", currProcess -> processName);
                    
                }
                printf("\n");
            }
            if(rr_Step3(runningProcess, quantum, mode, memory, arrived_list, &unusedFrames, simulationTime) == 1){
                //Memory has been allocated, execute the current process
                if(runningProcess->timeRemaining <= quantum){
                    turnTime[size]=(quantum+simulationTime) - runningProcess->timeArrived;
                    requiredtime[size] = runningProcess->timeRequired;
                    size++;
                }
                executeProcess(runningProcess, quantum, mode, memory, prevProcessName, simulationTime, remainingProcesses, unusedFrames);
                prevProcessName = runningProcess -> processName;
                    // Store the process to be inserted at the next quantum
                prevProcess = runningProcess;
            }
            else{
                //Memory cannot be allocated, move current process to end of the queue and check next process
                insert_Linklist(arrived_list, runningProcess);
                mem_flag=1;
                continue;
            }
        }

        // Step 4: Increment the simulation time by quantum
        simulationTime += quantum;
    }
    stats(turnTime,requiredtime,size,simulationTime-quantum);
    // Free memory allocated for processes and process queues
    free_list(notArrive_list);
    free_list(arrived_list);
}

int rr_Step3(Process * process, int quantum, char *mode, int *memory, Linklist *arrivedList, int *unusedFrames, int simulationTime){
    int DEBUG = 0;
    int evictedPages[MAX_PAGES];
    int evictedPageCount = 0;
    if(DEBUG && 1) printf("rr_Step3 called\n");
    //Memory has already been allocated
    if(process -> memAllocated == 1){
        return 1;
    }
    if(strcmp(mode, "first-fit") == 0){
        //Task 2
        int i = 0;
        if(DEBUG && 0) printf("%d %d %d ", memory[0], memory[1], memory[2]);
        while(memory[i] != -1){
            
            //Iterates through memory, finding the first available hole
            if(memory[i+1] - memory[i] >= process -> memRequired){
                if(DEBUG && 1) printf("%d %d \n", memory[i+1] - memory[i], process -> memRequired);
                //Hole found, allocate memory there
                process -> memLocations[0] = memory[i];
                process -> memLocations[1] = memory[i] + process -> memRequired;
                process -> memLocations[2] = -1;
                memory[i] += process -> memRequired;
                //Removes hole if hole is now length 0
                if(memory[i] == memory[i+1]){
                    while(memory[i+2] != -1){
                        memory[i] = memory[i+2];
                        memory[i+1] = memory[i+3];
                        i += 2;
                    }
                    memory[i] = -1;
                }
                //Memory allocated, process ready to run
                process -> memAllocated = 1;
                return 1;
            }
            i += 2; 
        }
        //Memory NOT allocated, process cannot run
        process -> memAllocated = 0;
        return 0;
    }
    else if(strcmp(mode, "paged") == 0){
        //Checks amount of frames needed
        int i = 0;
        int emptyPageNum = 0;
        while(process -> memLocations[i] != -1){
            if(process -> memLocations[i] == MAX_PAGES+1) emptyPageNum++;
            i++;
        }
        if(DEBUG && 1) printf("emptyPageNum is %d initallocatedpage is %d \n", emptyPageNum,process->allocatedPages);
        //Finds least recently used process
        //Allocates to all unused frames in memory
        i = 0;
        arraySort(process -> memLocations);
        
        //Allocates unused frames to process
        while(*unusedFrames > 0 && i < MAX_PAGES && emptyPageNum > 0){
            if(memory[i] == 0){
                memory[i] = 1;
                process -> memLocations[process -> allocatedPages] = i;
                process -> allocatedPages++;
                (*unusedFrames)--;
                emptyPageNum--;
            }
            i++;
        }

        while(emptyPageNum > 0){
            i = 0;
            Process *lruProcess = find_lruProcess(arrivedList, process,simulationTime);
            if(DEBUG && 1) printf("lru process found\n");
            while(lruProcess->memLocations[i] != -1){
                if(emptyPageNum == 0) break;
                if(lruProcess -> memLocations[i] != (MAX_PAGES+1)){
                    //Allocate page to current process
                    process -> memLocations[process -> allocatedPages] = lruProcess -> memLocations[i];
                    process -> allocatedPages++;
                    emptyPageNum--;
                    //Adds page to list of evicted pages
                    evictedPages[evictedPageCount] = lruProcess -> memLocations[i];
                    evictedPageCount++;
                    //Deallocate page from previous process
                    lruProcess -> memLocations[i] = MAX_PAGES+1;
                    lruProcess -> allocatedPages--;
                    lruProcess -> memAllocated = 0;
                }
                i++;
            }
        }
        //Prints the EVICT statment
        if(evictedPageCount >= 1){
            evictedPages[evictedPageCount] = -1;
            printf("%d,EVICTED, evicted-frames=", simulationTime);
            arraySortPrint(evictedPages);
            printf("\n");
        }
        process -> memAllocated = 1;
        return 1;
    }
    else if(strcmp(mode,"virtual")==0){
        //variables to track frames required and frames assigned
        int requiredFrames = 0;
        int i = 0;
        while(process->memLocations[i]!=-1){
            if(process -> memLocations[i] == MAX_PAGES+1){
                requiredFrames++;
                
            } 
            i++;
        }

        arraySort(process -> memLocations);
        i=0;
        while(*unusedFrames>0 && i<MAX_PAGES && requiredFrames >0){
            if(memory[i] == 0){
                memory[i] = 1;
                process->memLocations[process->allocatedPages]=i;
                process->allocatedPages++;
                (*unusedFrames)--;
                requiredFrames--;
            }
            i++;
        }

        if(requiredFrames==0 || process->allocatedPages>=MIN_PAGES_REQUIRED){
            process->memAllocated =1;
        }
        else{
            while(process->allocatedPages < MIN_PAGES_REQUIRED && requiredFrames > 0){
                i=0;
                Process *lruProcess = find_lruProcess(arrivedList,process,simulationTime);
                //lruProcess->memLocations[i] != -1
                while(lruProcess->memLocations[i] != -1){
                    //requiredFrames==0||process->allocatedPages==4
                    if(requiredFrames==0||process->allocatedPages == MIN_PAGES_REQUIRED){
                        break;
                    }
                    if(lruProcess->memLocations[i] != MAX_PAGES+1){
                        process -> memLocations[process -> allocatedPages] = lruProcess -> memLocations[i];
                        process -> allocatedPages++;
                        requiredFrames--;
                        //Adds page to list of evicted pages
                        evictedPages[evictedPageCount] = lruProcess -> memLocations[i];
                        evictedPageCount++;
                        //Deallocate page from previous process
                        lruProcess -> memLocations[i] = MAX_PAGES+1;
                        lruProcess -> allocatedPages--;
                        lruProcess -> memAllocated = 0;
                           
                    }
                    i++;
                }
            }
        }
        if (evictedPageCount >= 1) {
            evictedPages[evictedPageCount] = -1;
            printf("%d,EVICTED, evicted-frames=", simulationTime);
            arraySortPrint(evictedPages);
            printf("\n");
        }
        // Memory allocated, process ready to run
        process->memAllocated = 1;
        return 1;
    }
    else{
        //Task 1 
        process -> memAllocated = 1;
        return 1;
    }
}

int mem_UsageCalc(int *memory, char *mode, int unusedFrames){
    if(strcmp(mode, "first-fit") == 0){
        int i = 0;
        int sum = 0;
        while(memory[i] != -1){
            sum += memory[i+1] - memory[i];
            i += 2;
        }
        return((int)ceil(100*((float)MEMORY_SIZE-sum)/(float)MEMORY_SIZE));
    }
    else if(strcmp(mode, "paged") == 0 || strcmp(mode, "virtual") == 0){
        return((int)ceil(100*(MAX_PAGES - unusedFrames)/(float)MAX_PAGES));
    }
    return 0;
}

Process *find_lruProcess(Linklist *arrivedList, Process *process,int simTime){
    //Finds least recently used process
    int DEBUG = 0;
    //Gets the head node
    Node *currNode = arrivedList -> head;
    Process *currProcess = currNode -> content;
    Process *lruProcess = currProcess;
    int lruTime = simTime+1;
    //LRU time must be lastused of head or smaller
    if(DEBUG && 1)printf("LRU - evicting memory for %s\n", process -> processName);
    if(arrivedList -> head != NULL){
        Process *currProcess = currNode -> content;
        if(DEBUG && 1) printf("%s:%d:%d ", currProcess -> processName, currProcess -> allocatedPages, currProcess -> lastUsed);
        if(lruProcess->allocatedPages>0)lruTime=lruProcess->lastUsed;
        while(currNode -> next != NULL){
            if(DEBUG && 1)printf("Am in LRU Loop\n");
            //Updates currProcess
            currNode = currNode -> next;
            currProcess = currNode -> content;
            if(DEBUG && 1) printf("Name: %s Pages Allocated:%d Last use Time:%d \n", currProcess -> processName, currProcess -> allocatedPages, currProcess -> lastUsed);
            
            if((currProcess -> allocatedPages != 0) && (currProcess -> lastUsed>=0 && currProcess -> lastUsed < lruTime)){
                //First page with allocated pages, or new LRU page found
                lruTime = currProcess -> lastUsed;
                lruProcess = currProcess;
            }
        }
    }
    if(DEBUG && 1) printf("\n");

    if(DEBUG && 1)printf("find_lruProcess - Returning %s:%d\n", lruProcess -> processName, lruProcess -> allocatedPages);
    assert(lruProcess -> allocatedPages != 0);

    return lruProcess;
}

void arraySortPrint(int *frames){
    int i, j, key;
    int length = 0;
    while(frames[length] != -1){
        length++;
    }
    for (i = 1; i < length; i++) {
        key = frames[i];
        j = i - 1;
        
        while (j >= 0 && frames[j] > key) {
            frames[j + 1] = frames[j];
            j--;
        }
        frames[j + 1] = key;
    }  
    i=0;
    printf("[");
    while(frames[i+1]!=(MAX_PAGES+1) && frames[i+1]!=-1){
        printf("%d,",frames[i]);
        i++;
    }
    printf("%d]",frames[i]);

}
void arraySort(int *frames){
    int i, j, key;
    int length = 0;
    while(frames[length] != -1){
        length++;
    }
    

    for (i = 1; i < length; i++) {
        key = frames[i];
        j = i - 1;
        
        while (j >= 0 && frames[j] > key) {
            frames[j + 1] = frames[j];
            j--;
        }
        frames[j + 1] = key;
    }
}

void stats(int turnTime[],int requiredTime[],int size,int simTime){
    float maxOverhead=0.0;
    float overheadTime=0.0;
    float averageTurnaround=0;
    for(int i=0;i<size;i++){
        averageTurnaround+=turnTime[i];
        overheadTime+=(float)turnTime[i]/requiredTime[i];
        if((float)turnTime[i]/requiredTime[i]>maxOverhead){
            maxOverhead=(float)turnTime[i]/requiredTime[i];
        }
    }
    averageTurnaround=averageTurnaround/size;
    overheadTime=overheadTime/size;
    
    printf("Turnaround time %d\n",(int)ceil(averageTurnaround));
    printf("Time overhead %.2f %.2f\n",maxOverhead,overheadTime);
    printf("Makespan %d\n",simTime);

}