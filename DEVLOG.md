# DevLog for Vincent Wang and Derek Zang

## Week of 1/9 - 1/13: 
  
- 1/9: We finished the basic tic-tac-toe `game.c` file that will act as the base code for our project. As of right now, we have a simple, two-person implementation that alternates players until a win, lose, or tie. 
  
- 1/10: We added a basic server and client through a forking server, and created a basic `makefile` to compile the code. We also
  
- 1/11: Established a simple server-client connection program that utilizes sockets
  
- 1/12: Spent whole day brainstorming how to integrate game.c code with Mr. K server-client code
  
- 1/13: Worked on struct implementation. Lot of time spent on OOP-type thinking to plan out file reorganization 

- 1/14 - 1/16: Integrated game.c methods into server.c/client.c structure. Finished simple 2-player version of tic-tac-toe. Broadened to fifo N-client implementation. Included semaphore syncronization to server.c/client.c to make shut off access to memory between turns, ensure board being updated correctly, prevent collision. Fixed bugs. 
  
