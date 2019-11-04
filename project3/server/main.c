//Dominic Taraska
//Tug76525
//tug76525@temple.edu
//CIS 3207 Project 3

#include "server.h"

//driver function
int main(int argc, char *argv[]) {

    printf("\n Hello, welcome to my networked spell checker.\n");
    printf("To connect as a client, you will need to use 'telnet localhost [SERVER'S PORT #]'\n");

    if (argc == 1) { //uses the default socket number and dictionary file
        if (!(DICTIONARY_FILE = fopen(DICTIONARY_DEFAULT, "r"))) {
            perror("Could not find dictionary");
            exit(EXIT_FAILURE);
        }
        lPort = PORT_DEFAULT;
    }
    else if (argc == 2) { //argument handling for dictionary and port number
        char *finish;
        if (!(lPort = (int) strtol(argv[1], &finish, 10))) {  //if the 1st argument is not a number, it's a dictionary
            if (!(DICTIONARY_FILE = fopen(argv[1], "r"))) {//error handling for file DNE
                perror("Could not find dictionary");
                exit(EXIT_FAILURE);
            }
            lPort = PORT_DEFAULT;
        }
    }
    else { //specified parameters
        if (!(DICTIONARY_FILE = fopen(argv[1], "r"))) {
            perror("Could not find dictionary");
            exit(EXIT_FAILURE);
        }
        char *finish;
        if (!(lPort = (int) strtol(argv[2], &finish, 10))) {
            perror("Could not make port");
            exit(EXIT_FAILURE);
        }
        //if the port isn't within range
        if (lPort < 1024 || lPort > 65535) {
            perror("Port must be between 1025-65534");
            exit(EXIT_FAILURE);
        }
    }

    int lSocket; //uses the default socket number and dictionary file
    if ((lSocket = open_listenfd(lPort)) < 0) {
        perror("Can't open listening socket");
        exit(EXIT_FAILURE);
    }

    server *serv = malloc(sizeof(*serv)); //creates server struct that contains all mutex locks and other important vars
    createServer(serv);//init server vars

    pthread_t workers[BUFFER_MAX]; //make thread source

    for (int i = 0; i < BUFFER_MAX; i++) { //add threads to source
        pthread_create(&workers[i], NULL, workerProcedure, (void *)serv);
    }
    if ((LOG_FILE = fopen("log.txt", "w+")) == NULL) { //open logging file
        perror("Could not create log file");
        exit(EXIT_FAILURE);
    }
    pthread_t logThread; //makes thread for logging
    pthread_create(&logThread, NULL, loggingProcedure, (void *) serv);
    int clientSocket; //make socket for client connections
    char *helloPrompt = "Greetings, type a word and I will let you know if it's spelt correctly.\n";
    while (1) { //begin taking clients
        if ((clientSocket = accept(lSocket, NULL, NULL)) < 1) {
            perror("Unable to connect to client");
            break;
        }
        puts("Client has successfully connected");
        send(clientSocket, helloPrompt, strlen(helloPrompt), 0); //give hello prompt to clients
        pthread_mutex_lock(&serv->clientMTX); //get lock for client queue
        while (serv->clientRead == serv->clientWrite && serv->clientCounter == BUFFER_MAX) { //check if the queue is full
            pthread_cond_wait(&serv->client_Empty, &serv->clientMTX);
        }

        addClient(serv, clientSocket); //give socket from client into queue

        pthread_cond_signal(&serv->client_notEmpty); //signal to the workers
        pthread_mutex_unlock(&serv->clientMTX);  //unlock the queue
    }
    close(lSocket);
    close(clientSocket);
    fclose(LOG_FILE);
    return 0;
}

