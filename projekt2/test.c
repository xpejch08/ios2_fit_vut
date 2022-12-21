#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MMAP(pointer) {(pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);}
#define UNMAP(pointer) {munmap( (pointer), sizeof((pointer)) );}

/**
 * initializnig all global variables
 */
FILE *file;

sem_t *hydrogenSem = NULL;
sem_t *oxygenSem = NULL;
sem_t *mutex = NULL;
sem_t *turnstile1 = NULL;
sem_t *barrierMutex = NULL;
sem_t *turnstile2 = NULL;
sem_t *printMutex = NULL;
sem_t *endMutex = NULL;

int *hydrogenCount = NULL;
int *oxygenCount = NULL;
int *barrierCount = NULL;
int *lineCounter = NULL;
int *NODec = NULL;
int *NHDec = NULL;
int *moleculesCount = NULL;
int *counter = NULL;

/**
 * @brief function that checks if array of characters is a number or not
 * @param num array of characters that we want co check
 * @return true if num is a number false if not
 */
bool numberCheck(char *num){
    if(num[0] == '-'){
        return false;
    }
    int i = 0;
    while(num[i] != '\0'){
        if(isdigit(num[i]) == 0){
            return false;
        }
        i++;
    }
    return true;
}

/**
 * function that builds all variables
 * @return
 */
int buildAll(){
    MMAP(hydrogenCount);
    MMAP(oxygenCount);
    MMAP(barrierCount);
    MMAP(lineCounter);
    MMAP(NHDec);
    MMAP(NODec);
    MMAP(moleculesCount);
    MMAP(counter);
    if ((hydrogenSem = sem_open("/xpejch08.hydrogenSem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    if ((endMutex = sem_open("/xpejch08.endMutex", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    if ((printMutex = sem_open("/xpejch08.printMutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return 1;
    if ((turnstile1 = sem_open("/xpejch08.turnstile1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    if ((turnstile2 = sem_open("/xpejch08.turnstile2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    if ((oxygenSem = sem_open("/xpejch08.oxygenSem", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) return 1;
    if ((mutex = sem_open("/xpejch08.mutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return 1;
    if ((barrierMutex = sem_open("/xpejch08.barrierMutex", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return 1;
    return 0;
}


/**
 * function that destroys all initialized variables
 * @return
 */

int destroy(){
    UNMAP(hydrogenCount);
    UNMAP(oxygenCount);
    UNMAP(barrierCount);
    UNMAP(lineCounter);
    UNMAP(NODec);
    UNMAP(NHDec);
    UNMAP(moleculesCount);
    UNMAP(counter);
    sem_close(hydrogenSem);
    sem_close(oxygenSem);
    sem_close(mutex);
    sem_close(turnstile1);
    sem_close(turnstile2);
    sem_close(barrierMutex);
    sem_close(printMutex);
    sem_close(endMutex);
    sem_unlink("/xpejch08.turnstile1");
    sem_unlink("/xpejch08.endMutex");
    sem_unlink("/xpejch08.printMutex");
    sem_unlink("/xpejch08.turnstile2");
    sem_unlink("/xpejch08.barrierMutex");
    sem_unlink("/xpejch08.hydrogenSem");
    sem_unlink("/xpejch08.oxygenSem");
    sem_unlink("/xpejch08.mutex");
    return 0;

}

/**
 * function that counts how many molecules will be created overall
 * @param NO amount of all oxygens
 * @param NH amount of all hydrogens
 * @return returns number of all created molecules
 */
int moleculesCreate(int NO, int NH){
    int NHMod = NH/2;
    if(NO > NHMod){
        return NHMod;
    }
    else{
        return NO;
    }
}

/**
 * function that makes the program sleep for a random amount of time in the range TI
 * @param TI TI is the maximum time the numbers can be asleep for in miliseconds
 */
void randSleep(int TI){
    usleep((rand()%(TI+1)));
}

/**
 * process hydrogen
 * @param idH id of the specific hydrogen used in the process
 * @param TI maximum time for how long the sleep can last before going into queue, used in randSleep function
 * @param TB maximum time for how long the sleep can last for creating a molecule, used in randSleep function
 * @return
 */
int hydrogenProcess(int idH, int TI, int TB){

    sem_wait(mutex);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: H %d: started\n", *lineCounter , idH);
    sem_post(printMutex);

    (*hydrogenCount)++;

    (*NHDec)++;
    if(((*NHDec) > ((*counter)*2) || (*counter) == 0)) {
        (*lineCounter)++;
        fprintf(file, "%d: H %d: not enough O or H\n", (*lineCounter), idH);
        sem_post(mutex);
        exit(0);
    }
    if ((*hydrogenCount) >= 2 && (*oxygenCount) >= 1) {
        sem_post(hydrogenSem);
        sem_post(hydrogenSem);
        (*hydrogenCount) -= 2;

        sem_post(oxygenSem);
        (*oxygenCount) -= 1;
    }
    else {
        sem_post(mutex);
    }


    randSleep(TI);
    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: H %d: going to queue\n", *lineCounter, idH);
    sem_post(printMutex);

    sem_wait(hydrogenSem);

    randSleep(TB);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: H %d: creating molecule %d\n", *lineCounter, idH, (*moleculesCount));
    sem_post(printMutex);

    sem_wait(barrierMutex);
    (*barrierCount)++;
    if((*barrierCount)==3){
        sem_post(turnstile1);
        sem_post(turnstile1);
        sem_post(turnstile1);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile1);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: H %d: molecule %d created\n", *lineCounter, idH, (*moleculesCount));
    sem_post(printMutex);

    sem_wait(barrierMutex);
    (*barrierCount)--;
    if((*barrierCount)==0){
        sem_post(turnstile2);
        sem_post(turnstile2);
        sem_post(turnstile2);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile2);

    return 0;
}

/**
 * process hydrogen
 * @param idHO id of the specific oxygen used in the process
 * @param TI maximum time for how long the sleep can last before going into queue, used in randSleep function
 * @param TB maximum time for how long the sleep can last for creating a molecule, used in randSleep function
 * @return
 */
void oxygenProcess(int idO, int TI, int TB){

    sem_wait(mutex);
    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file,"%d: O %d: started\n", *lineCounter, idO);
    sem_post(printMutex);

    (*oxygenCount) += 1;

    if((*counter) == 0){
        sem_post(endMutex);
    }


    if((*hydrogenCount) >= 2){
        sem_post(hydrogenSem);
        sem_post(hydrogenSem);
        (*hydrogenCount) -= 2;

        sem_post(oxygenSem);
        (*oxygenCount) -= 1;
    }
    else{
        sem_post(mutex);
    }

    randSleep(TI);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file,"%d: O %d: going to queue\n", *lineCounter, idO);
    sem_post(printMutex);

    (*NODec)++;
    if(( (*NODec) > (*counter)) || ((*counter) == 0)) {
        sem_wait(printMutex);
        (*lineCounter)++;
        fprintf(file, "%d: O %d: not enough H\n", (*lineCounter), idO);
        sem_post(printMutex);
        exit(0);
    }

    randSleep(TB);
    sem_wait(oxygenSem);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: O %d: creating molecule %d\n", *lineCounter, idO, (*moleculesCount));
    sem_post(printMutex);

    sem_wait(barrierMutex);
    (*barrierCount)++;
    if((*barrierCount)==3){
        sem_post(turnstile1);
        sem_post(turnstile1);
        sem_post(turnstile1);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile1);

    sem_wait(printMutex);
    (*lineCounter)++;
    fprintf(file, "%d: O %d: molecule %d created\n", *lineCounter, idO, (*moleculesCount));
    sem_post(printMutex);

    sem_wait(barrierMutex);
    (*barrierCount)--;
    if((*barrierCount)==0){
        sem_post(turnstile2);
        sem_post(turnstile2);
        sem_post(turnstile2);
    }
    sem_post(barrierMutex);

    sem_wait(turnstile2);

    (*moleculesCount)++;

    sem_post(mutex);

}

/**
 * function that cerates all oxygen and hydrogen processes
 * @param NH number of hydrogen's
 * @param NO number of oxygen's
 * @param TI maximum time for how long the sleep can last before going into queue, used as parameter
 * in oxygen and hydrogen function call
 * @param TB maximum time for how long the sleep can last for creating a molecule, used as parameter
 * in oxygen and hydrogen function call
 */
int Queue(int NH, int NO, int TI, int TB){
    pid_t oxygenId;
    for(int i = 1; i<=NO; i++){
        oxygenId = fork();
        if(oxygenId==0){
            oxygenProcess(i, TI, TB);
            fclose(file);
            exit(0);

        }

        else if(oxygenId < 0){
            fprintf(stderr, "ERROR - UNABLE TO FORK()\n");
            return 1;
        }
    }

    pid_t hydrogenId;
    for(int i = 1; i<=NH; i++){
        hydrogenId = fork();
        if(hydrogenId==0){
            hydrogenProcess(i, TI, TB);
            fclose(file);
            exit(0);
        }

        else if (hydrogenId < 0){
            fprintf(stderr, "ERROR - UNABLE TO FORK()\n");
            return 1;
        }
    }
    return 0;
}



int main(int argc, char *argv[]) {
    destroy();
    /**
     * opening output file
     */
    if((file = fopen("proj2.out", "w")) == NULL){
        fprintf(stderr, "ERROR CANNOT OPEN FILE\n");
        return 1;
    }
    /**
     * setting buffer to have correct output into the file
     */
    setbuf(file, NULL);
    setbuf(stderr, NULL);

    /**
     * checking number of arguments
     */
    if(argc != 5){
        fprintf(stderr, "INCORRECT ARGUMENT COUNT\n");
        return 1;
    }

    int NO, NH, TI, TB;

    /**
     * checking if parameters are numbers, calling numberCheck function
     */
    for(int i = 1; i < argc; i++){
        if(!(numberCheck(argv[i]))){
            fprintf(stderr, "ARGUMENT %d IS NOT A NUMBER\n", i);
            return 1;
        }
    }
    /**
     * initialazing value of variables and checking if variable TI abd TB are from the correct interval
     */
    NO = atoi(argv[1]);

    NH = atoi(argv[2]);

    if(NO == 0 || NH == 0){
        fprintf(stderr, "ERROR - NOT ENOUGH ATOMS FOR MOLECULE\n");
    }

    TI = atoi(argv[3]);
    if(TI<0 || TI>=1001){
        fprintf(stderr, "NUMBER TI ISN'T FROM INTERVAL 0-1000\n");
        return 1;
    }

    TB = atoi(argv[4]);
    if(TB<0 || TB>=1001){
        fprintf(stderr, "NUMBER TB ISN'T FROM INTERVAL 0-1000\n");
        return 1;
    }

    /**
     * calling build function to build all variables global
     */
    if (buildAll() == 1) {
        destroy();
        fclose(file);
        fprintf(stderr,"AN ERROR OCCURRED WHILE BUILDING\n");
        return 1;
    }
    (*NODec) = 0; // assigning oxygen atom used counter
    (*NHDec) = 0; // assigning hydrogen atom used counter
    // assigning amount of all molecules that will be created
    (*counter) = moleculesCreate(NO,NH); // assigning amount of all molecules that will be created
    (*moleculesCount) = 1; // assigning counter of molecules variable
    Queue(NH, NO, TI, TB); // calling queue function to call all processes
    while(wait(NULL) > 0); //waiting for all processes to end before ending main
    destroy(); // destroying all global variables
    fclose(file); // closing output file
    return 0;
}
