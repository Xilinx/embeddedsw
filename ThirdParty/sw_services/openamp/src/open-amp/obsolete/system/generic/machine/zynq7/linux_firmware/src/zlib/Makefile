# Make file to create ipc stack library.

# Include commons make file to get platform and tool chain specific variables.
include Makefile.commons

LIB := libzlib.a

SRCFILES += \
$(wildcard *.c)

OBJFILES := $(patsubst %.c, %.o, $(SRCFILES))

DEPFILES := $(patsubst %.c, %.d, $(SRCFILES))

all: $(LIB)

$(LIB): $(OBJFILES)
	@echo AR $@
	@$(AR) -r $@ $(OBJFILES)

%.o:%.c $(HEADERS)
	@echo CC $(<:.c=.o)
	@$(CC) $(CFLAGS) $(ARCH_CFLAGS) $(INCLUDE) -c $< -o $@

clean:
	-$(RM) $(LIB) $(OBJFILES) $(DEPFILES)

PHONY: all clean
