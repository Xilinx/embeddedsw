Following steps will guide you to run the example test provided in the test directory:

From the current directory run the following cmds:

1) Run make to build client & server executables in obj/:
   make

2) Create TCP server from a terminal with the following cmd:
   ./obj/server.out 4547 > data/dumpserv   

3) Create TCP client from a terminal with the following cmd:
   ./obj/client.out localhost 4547 data/0_0 > data/dumpcli 
