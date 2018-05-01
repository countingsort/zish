CC ?= gcc
CPPFLAGS=-D_POSIX_C_SOURCE=200809L
CFLAGS=-std=c11
ifeq ($(CC), clang)
	CFLAGS += -Weverything -pedantic -Wno-vla
else
	CFLAGS += -Wall -Wextra -pedantic
endif
CPPFLAGS += -MMD -MF $*.d
LDLIBS=-lreadline
OBJS=zish.o builtins.o execute.o interrupt_handler.o

all: zish

-include *.d

zish: $(OBJS)

%.d: ;

%.o: %.d

clean:
	@$(RM) -v $(OBJS) $(OBJS:.o=.d) zish

.PHONY: all clean
.PRECIOUS: *.d *.o

