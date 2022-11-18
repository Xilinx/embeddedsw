APP = axidma-test
LIBSOURCES=main.c
OUTS = ./main.o
INCLUDES =  -I"../src/"
INCLUDES += -I"../../../../../embeddedsw/lib/bsp/standalone/src/arm/cortexr5/"
INCLUDES += -I"../../../../../embeddedsw/lib/bsp/standalone/src/arm/common/gcc/"
BOARD_FLAG =
OBJS = ./main.c

all: $(RFDC_OBJS)
	make -C ../src
	$(CC) ${LDFLAGS} $(INCLUDES) $(OBJS) $(RFDC_OBJS) -o $(OUTS) -lmetal -L ../src -laxidma

clean:
	rm -rf $(OUTS) *.o
	make -C ../src clean
