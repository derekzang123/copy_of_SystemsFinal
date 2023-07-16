#ifndef CLIENT_H
#define CLIENT_H

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

#define PORT 2023
#define MAXLINE 2048

union semun
{
    int val;               // used for SETVAL
    struct semid_ds *buf;  // used for IPC_STAT and IPC_SET
    unsigned short *array; // used for SETALL
    struct seminfo *__buf;
};

char check_winner(char board[3][3]);

void draw_board(char board[3][3]);

void update_board(char board[3][3], char position, char player);

int get_input(char board[3][3], char player, int fd);

#endif