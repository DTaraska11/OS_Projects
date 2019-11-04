//
// Created by domin on 11/2/2019.
//
#include "server.h"

//Creates server
void createServer(server *serv) {
    serv->clientCounter = 0;
    serv->loggCounter = 0;

    serv->clientRead = 0;
    serv->clientWrite = 0;
    serv->logRead = 0;
    serv->logWrite = 0;

    for(int i = 0; i < BUFFER_MAX; i++) {
        clients[i] = (int) calloc(1, sizeof(int));
        logs[i] = (char *) calloc(1, sizeof(char *));
    }
}
//adds client
void addClient(server *serv, int socket) {

    clients[serv->clientWrite] = socket;//add to client buffer
    serv->clientWrite = (++serv->clientWrite) % BUFFER_MAX; //increment write ptr
    ++serv->clientCounter; //increment total
}

//Function that implements worker functionality
void *workerProcedure(void* args) {
    server *serv = args; //
    char *mError = "Could not receive message ";
    int bytesGiven;
    char *res; //
    char *word; //
    char *prompt = ">"; //

    while(1) {
        pthread_mutex_lock(&serv->clientMTX); //lock the client queue
        while (serv->clientRead == serv->clientWrite && serv->clientCounter == 0) { //while the client queue is empty
            pthread_cond_wait(&serv->client_notEmpty, &serv->clientMTX);
        }

        int socket = removeClient(serv); //get socket from client queue
        pthread_cond_signal(&serv->client_Empty); //send signal that queue is empty
        pthread_mutex_unlock(&serv->clientMTX); //unlock mutex

        while(1) { //keep taking input words until the client disconnects
            send(socket, prompt, strlen(prompt), 0);
            word = calloc(DICTIONARY_BUFFER, 1); //initialize word
            bytesGiven = (int) recv(socket, word, DICTIONARY_BUFFER, 0); //receive word
            if (bytesGiven < 0) { //error handling for message reception
                send(socket, mError, strlen(mError), 0);
                continue;
            }
            if(word[0] == 27) { //client disconnects by pressing esc
                send(socket, "Goodbye.\n", strlen("Goodbye.\n"), 0);
                break;
            }
            if(strcmp(word, "EXIT\r\n") == 0) {
                exit(0);
            }
            int correct = wSearch(word); //looks for word, sets result var
            correct ? (res = "VALID\n") : (res = "MISSPELLED\n");
            send(socket, res, strlen(res), 0); //give result to client
            pthread_mutex_lock(&serv->logMTX); //get lock and push result to log queue
            while(serv->logWrite == serv->logRead && serv->loggCounter == BUFFER_MAX) { //check if buffer is full
                pthread_cond_wait(&serv->log_Empty, &serv->logMTX);
            }
            addLog(serv, word, correct); //write to log queue
            pthread_cond_signal(&serv->log_notEmpty); //signal that log queue is full
            pthread_mutex_unlock(&serv->logMTX); //unlock mutex lock
        }
        close(socket);
    }
}

//Dequeue client
int removeClient(server *serv) {
    int socket = clients[serv->clientRead];
    clients[serv->clientRead] = 0; //clears array index
    serv->clientRead = (++serv->clientRead) % BUFFER_MAX;  //increment index, and if end of buffer go back to zero
    --serv->clientCounter; //decrement #of clients in buffer

    return socket;
}
//Add log
void addLog(server *serv, char *word, int correct) {

    char logVar[DICTIONARY_BUFFER]; //var to hold entirety of logging text
    clients[serv->clientRead] = (int) calloc(1, sizeof(int)); //clear index
    size_t length = strlen(word); //remove '\n' char
    word[length - 1] = '\0';
    char * answer;
    correct ? (answer = "VALID") : (answer = "MISSPELLED"); //set that var accordingly with test of argument passed
    strcpy(logVar, "\"");
    strcat(logVar, word);
    strcat(logVar, "\" is ");
    strcat(logVar, answer);

    strcpy(logs[serv->logWrite], logVar); //push string to the queue
    serv->logWrite = (++serv->logWrite) % BUFFER_MAX; //increment the index, and optionally loop back to 0 if we reach end of buffer
    ++serv->loggCounter; //increment the amount of logs in the buffer
}
//Function that implements log keeping
void *loggingProcedure(void *args){
    server *serv = args;

    while(1) {

        pthread_mutex_lock(&serv->logMTX); //locks log queue
        while (serv->logRead == serv->logWrite && serv->loggCounter == 0) { //if empty
            pthread_cond_wait(&serv->log_notEmpty, &serv->logMTX);
        }

        /* TEST */

        for(int i = 0; i < BUFFER_MAX; i++) {
            printf("before removing log[%d]:\t%d\n", i, logs[i]);
        }
        printf("log write pointer: \t%d\n", serv->logWrite);
        printf("log #: \t%d\n", serv->loggCounter);
        //
        char *result = removeLog(serv); //gets results

        fprintf(LOG_FILE, "%s\n", result); //write to log
        fflush(LOG_FILE);
        pthread_cond_signal(&serv->log_Empty); //signals that queue is empty
        pthread_mutex_unlock(&serv->logMTX); //unlocks queue
    }
}
//remove log from buffer
char *removeLog(server *serv) {
    char *result = logs[serv->logRead];
    logs[serv->logRead] = (char *) calloc(1, sizeof(char *)); //clear index
    serv->logRead = (++serv->logRead) % BUFFER_MAX; //increment the read pointer
    --serv->loggCounter; //decrement the total log count
    return result;
}
//function that implements searching of word in dictionary
_Bool wSearch(char *word) {
    char word_buffer[DICTIONARY_BUFFER]; //holds dictionary word
    _Bool found = 0;
    size_t length = strlen(word); //get rid of '\n'
    word[length - 2] = '\n';
    word[length - 1] = '\0';
    while(fgets(word_buffer, DICTIONARY_BUFFER, DICTIONARY_FILE) != NULL) {

        if(strcmp(word_buffer, word) != 0) { //word isn't a match, get next word
            continue;
        }
        else {

            rewind(DICTIONARY_FILE);
            return ++found;
        }
    }
    rewind(DICTIONARY_FILE);
    return found;
}