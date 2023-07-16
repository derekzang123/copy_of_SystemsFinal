#include "client.h"

char check_winner(char board[3][3])
{
    // check rows
    for (int i = 0; i < 3; i++)
    {
        if (board[i][0] == board[i][1] && board[i][1] == board[i][2])
        {
            return board[i][0];
        }
    }
    // check columns
    for (int i = 0; i < 3; i++)
    {
        if (board[0][i] == board[1][i] && board[1][i] == board[2][i])
        {
            return board[0][i];
        }
    }
    // check diagonals
    if (board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        return board[0][0];
    }
    if (board[2][0] == board[1][1] && board[1][1] == board[0][2])
    {
        return board[2][0];
    }
    return ' ';
}

void draw_board(char board[3][3])
{
    printf("\n");
    printf(" %c | %c | %c \n", board[0][0], board[0][1], board[0][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[1][0], board[1][1], board[1][2]);
    printf("---+---+---\n");
    printf(" %c | %c | %c \n", board[2][0], board[2][1], board[2][2]);
    printf("\n");
}

void update_board(char board[3][3], char player, char position)
{
    int tile = position;
    /*
    if (tile < 1 || tile > 9)
    {
        printf("Invalid move!\n");
        return;
    }
    */
    int row = tile / 3;
    int col = tile % 3;
    board[row][col] = player;
}

int get_input(char board[3][3], char player, int fd)
{
    int tile;
    printf("Enter a tile number:\n");
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    FD_SET(fd, &fds);

    if (select(fd + 1, &fds, NULL, NULL, NULL) < 0)
    {
        perror("select error");
        return -1;
    }
    if (FD_ISSET(fd, &fds))
    {
        char c = 0;
        read(fd, &c, 1);
        if (c == 'Q')
        {
            printf("The other player disconnected, you win!\n");
        }
        return -1;
    }
    // printf("select returned, read stdin...\n");
    scanf("%d", &tile);
    printf("Entered %d\n", tile);
    tile--;
    int row = tile / 3;
    int col = tile % 3;
    if (row < 3 && row >= 0 &&
        col < 3 && col >= 0 &&
        board[row][col] != 'D' && board[row][col] != 'V')
    {
        board[row][col] = player;
        printf("Marking board\n");
        return tile;
    }
    else
    {
        printf("Invalid move!\n");
        return get_input(board, player, fd);
    }
}

int main()
{
    printf("Welcome to DerekVincentMania. Start with a 3x3 grid and place down V's and D's\n");
    char mine = 'V';
    char other = 'D';
    char board[3][3] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'}};

    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket creation failed");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
    }

    printf("Connected to server, waiting for other player...\n");

    int turn = 0;

    while (1)
    {
        char position;
        int rc = read(sockfd, &position, 1);
        if (rc <= 0)
        {
            if (rc == 0)
            {
                printf("Server closed connection\n");
                close(sockfd);
                return 0;
            }
            else
            {
                perror("read() error");
            }
        }
        if (position == 'Q')
        {
            printf("Other player disconnected, you win!\n");
            close(sockfd);
            return 0;
        }
        if (position == 'S')
        {
            printf("Player Connected, starting game. You are 'D'\n");
            mine = 'D';
            other = 'V';
        }
        if (position != 'S' && turn == 0)
        {
            printf("Player Connected, starting game. You are 'V'\n");
        }

        if (position != 'S')
        {
            update_board(board, other, position);
        }
        draw_board(board);
        char winner = check_winner(board);
        if (winner != ' ')
        {
            printf("I lose\n");
            close(sockfd);
            return 0;
        }

        position = get_input(board, mine, sockfd);
        if (position == -1)
        {
            close(sockfd);
            printf("Server closed connection\n");
            return 0;
        }
        turn++;
        update_board(board, mine, position);
        draw_board(board);
        write(sockfd, &position, 1);
        winner = check_winner(board);
        if (winner != ' ')
        {
            printf("I win\n");
            close(sockfd);
            return 0;
        }
    }
}
