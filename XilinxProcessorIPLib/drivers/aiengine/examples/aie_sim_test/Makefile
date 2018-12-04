COMPILER=gcc
CC_FLAGS=-DXAIE_DEBUG -D__AIESIM__ -D__AIESIM_SOCK__

SRCDIR=../../src
EXTDIR=./ext
OBJDIR=./obj

INCLUDES=-I$(SRCDIR)/global -I$(SRCDIR)/dma -I$(SRCDIR)/tile -I$(SRCDIR)/lib -I$(EXTDIR)/sockcli -I$(EXTDIR)/sockserv -I$(EXTDIR)/top

VPATH:=$(SRCDIR)/tile:$(SRCDIR)/dma:$(SRCDIR)/global:$(SRCDIR)/lib:$(EXTDIR)/sockcli:$(EXTDIR)/sockserv:$(EXTDIR)/top

CLIENT_SOURCES = $(wildcard $(SRCDIR)/*/*.c) $(wildcard $(EXTDIR)/sockcli/*.c) $(wildcard $(EXTDIR)/top/*.c)
SERVER_SOURCES = $(wildcard $(EXTDIR)/sockserv/*.c)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)

all: create_dir client_object server_object clean

create_dir: 
	mkdir -p obj
	
client_object: $(CLIENT_OBJECTS)
	@echo making client
	$(COMPILER) -o $(OBJDIR)/client.out $^
	
server_object: $(SERVER_OBJECTS)
	@echo making server
	$(COMPILER) -o $(OBJDIR)/server.out $^	

%.o: %.c
	@echo compiling $<
	$(COMPILER) $(CC_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(CLIENT_OBJECTS) $(SERVER_OBJECTS)

