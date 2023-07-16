#include "server.h"

void initSC(struct ServerContext *sc)
{
  for (int i = 0; i < MaxGames; ++i)
  {
    struct Game *game = &(sc->games[i]);
    game->fds[0] = game->fds[1] = -1;
    game->gameId = i;
  }
}

struct Game *findGame(struct ServerContext *sc, int fd)
{
  for (int i = 0; i < MaxGames; ++i)
  {
    struct Game *game = &(sc->games[i]);
    if (game->fds[0] == fd || game->fds[1] == fd)
    {
      return game;
    }
  }
  return NULL;
}

struct Game *findFreeGame(struct ServerContext *sc, int fd)
{
  for (int i = 0; i < MaxGames; ++i)
  {
    struct Game *game = &(sc->games[i]);
    if (game->fds[0] == -1 && game->fds[1] == -1)
    {
      game->fds[0] = fd;
      return game;
    }
  }
  return NULL;
}

struct Game *findMatch(struct ServerContext *sc, int fd)
{
  for (int i = 0; i < MaxGames; ++i)
  {
    struct Game *game = &(sc->games[i]);
    if (game->fds[0] >= 0 && game->fds[1] == -1)
    {
      game->fds[1] = fd;
      return game;
    }
  }
  return NULL;
}

void destroyUser(struct Game *game, int fd, fd_set *master)
{
  close(fd);
  FD_CLR(fd, master);
  int other = game->fds[0] == fd ? game->fds[1] : game->fds[0];
  if (other >= 0)
  {
    write(other, "Q", 1);
    printf("sent Q command to [%d]\n", other);
    FD_CLR(other, master);
    close(other);
    printf("destroyed user [%d] and [%d]\n", fd, other);
  }
  game->fds[0] = game->fds[1] = -1;
}

struct Game *addUser(struct ServerContext *sc, int fd)
{
  // find matching game
  struct Game *game = findMatch(sc, fd);
  if (game)
  {
    printf("created a new game[%d]: [%d] vs [%d]\n", game->gameId,
           game->fds[0], game->fds[1]);
    return game;
  }

  game = findFreeGame(sc, fd);
  if (game)
  {
    printf("assigned [%d] to game [%d], waiting for other player\n",
           fd, game->gameId);
    return game;
  }

  printf("run out of resource\n");
  return NULL;
}

void handleCommand(struct ServerContext *sc, int fd, char position)
{
  printf("received position [%d] from player [%d]\n", (int)position, fd);

  struct Game *game = findGame(sc, fd);

  if (game == NULL)
  {
    printf("cannot find game for player [%d]\n", fd);
    return;
  }

  printf("game [%d] found\n", game->gameId);

  if (game->fds[0] == fd)
  {
    printf("forward position [%d]: [%d] -> [%d]\n", position,
           fd, game->fds[1]);
    write(game->fds[1], &position, 1);
  }
  else
  {
    printf("forward position [%d]: [%d] -> [%d]\n", position,
           fd, game->fds[0]);
    write(game->fds[0], &position, 1);
  }
}

struct ServerContext sc;

int main(int argc, char *argv[])
{
  initSC(&sc);

  fd_set master;
  fd_set read_fds;

  struct sockaddr_in serveraddr;
  struct sockaddr_in clientaddr;

  int fdmax;
  int listener;
  int newfd;

  socklen_t addrlen;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Server socket error");
    exit(1);
  }

  printf("Server-socket() Worked\n");

  int yes = 1;
  if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
  {
    perror("Server-setsockopt() error");
    exit(1);
  }
  printf("Server-setsockopt() Worked\n");

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(PORT);
  memset(&(serveraddr.sin_zero), 0, sizeof(serveraddr.sin_zero));

  if (bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
  {
    perror("Server-bind() error");
    exit(1);
  }
  printf("Server-bind() Worked\n");

  if (listen(listener, 10) == -1)
  {
    perror("Server-listen() error\n");
    exit(1);
  }
  printf("Server-listen() Worked\n");

  FD_SET(listener, &master);
  fdmax = listener;
  while (1)
  {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
    {
      perror("Server-select() error");
      continue;
    }
    printf("Server-select() Worked\n");

    for (int i = 0; i <= fdmax; i++)
    {
      if (!FD_ISSET(i, &read_fds))
      {
        continue;
      }
      // accept new connection
      if (i == listener)
      {
        addrlen = sizeof(clientaddr);
        if ((newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
        {
          perror("Server-accept() error");
        }
        else
        {
          printf("Server-accept() Worked\n");
          FD_SET(newfd, &master);
          if (newfd > fdmax)
          {
            fdmax = newfd;
          }
          printf("%s: New connection on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
          struct Game *game = addUser(&sc, newfd);
          if (game && game->fds[0] >= 0 && game->fds[1] >= 0)
          {
            char c = 'S';
            int firstPlayer = game->fds[rand() % 2];
            printf("Starting Game, Player %d will go first\n", firstPlayer);
            // whoever receives 'S' will play first
            write(firstPlayer, &c, 1);
          }
        }
        continue;
      }

      char position;
      int nbytes = read(i, &position, 1);
      // read error
      if (nbytes <= 0)
      {
        if (nbytes == 0)
        {
          printf("%s: socket %d disconnected\n", argv[0], i);
          struct Game *game = findGame(&sc, i);
          if (game)
          {
            destroyUser(game, i, &master);
          }
        }
        else
        {
          perror("read() error");
        }
        close(i);
        FD_CLR(i, &master);
        continue;
      }

      // process datax
      handleCommand(&sc, i, position);
    }
  }
  return 0;
}