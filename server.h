#ifndef SERVER_H
#define SERVER_H

#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

union semun
{
    int val;               // used for SETVAL
    struct semid_ds *buf;  // used for IPC_STAT and IPC_SET
    unsigned short *array; // used for SETALL
    struct seminfo *__buf;
};

#define MaxGames 1024
#define PORT 2023

struct Game
{
    int gameId;
    int fds[2];
    sem_t game_sem;
};

struct ServerContext
{
    struct Game games[MaxGames];
};

void initSC(struct ServerContext *sc);

struct Game *findGame(struct ServerContext *sc, int fd);

struct Game *findMatch(struct ServerContext *sc, int fd);

struct Game *findFreeGame(struct ServerContext *sc, int fd);

void destroyUser(struct Game *game, int fd, fd_set *master);

struct Game *addUser(struct ServerContext *sc, int fd);

void handleCommand(struct ServerContext *sc, int fd, char position);

int main();

#endif