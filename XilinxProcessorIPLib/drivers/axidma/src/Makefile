APP = axidma

AR=ar
LIBSOURCES=*.c
OUTS = *.o
NAME := axidma
MAJOR = 1
MINOR = 1
VERSION = $(MAJOR).$(MINOR)

INCLUDES = -I"../../../../../embeddedsw/lib/bsp/standalone/src/arm/cortexr5/"
INCLUDES += -I"../../../../../embeddedsw/lib/bsp/standalone/src/arm/common/gcc/"
INCLUDES += -I"./."

#all: lib$(NAME).so
all: lib$(NAME).a

#lib$(NAME).so.$(VERSION): $(OUTS)
#	$(CC) $(LDFLAGS) $(OUTS) -shared -Wl,-soname,lib$(NAME).so.$(MAJOR) -o lib$(NAME).so.$(VERSION) -lmetal

lib$(NAME).a: $(OUTS)
	$(AR) -rcs lib$(NAME).a $(LDFLAGS) $(OUTS)


#lib$(NAME).so: lib$(NAME).so.$(VERSION)
#	rm -f lib$(NAME).so.$(MAJOR) lib$(NAME).so
#	ln -s lib$(NAME).so.$(VERSION) lib$(NAME).so.$(MAJOR)
#	ln -s lib$(NAME).so.$(MAJOR) lib$(NAME).so

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $(LIBSOURCES)

clean:
	rm -rf *.o *.so *.so.* *.a*