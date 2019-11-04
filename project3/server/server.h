//
// Created by domin on 11/2/2019.
//

#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#endif //SERVER_SERVER_H



#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>

#define DICTIONARY_DEFAULT "dictionary.txt"
#define DICTIONARY_BUFFER 128
#define PORT_DEFAULT 1198
#define BUFFER_MAX 5

FILE *DICTIONARY_FILE;
FILE *LOG_FILE;
int clients[BUFFER_MAX];
char *logs[BUFFER_MAX];
int lPort;

typedef struct server{
    int clientCounter;
    int loggCounter;
    int logRead;
    int logWrite;
    int clientRead;
    int clientWrite;
    pthread_mutex_t clientMTX;
    pthread_mutex_t logMTX;
    pthread_cond_t client_notEmpty;
    pthread_cond_t client_Empty;
    pthread_cond_t log_notEmpty;
    pthread_cond_t log_Empty;
}server;

_Bool wSearch(char *word);
void addLog(server *serv, char *word, int iscorrect);
char *removeLog(server *serv);
int removeClient(server *serv);
void addClient(server *serv, int socket);
void *workerProcedure(void *args);
void *loggingProcedure(void *args);
void createServer(server *serv);
int open_listenfd(int port);