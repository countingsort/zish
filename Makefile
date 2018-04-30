CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow -D_POSIX_C_SOURCE=200809L
CPPFLAGS = -MMD -MF $*.d
LDLIBS = -lreadline
OBJS = builtins.o execute.o interrupt_handler.o

all: zish

-include *.d

zish: $(OBJS)

%.d: ;

%.o: %.d

clean:
	@$(RM) -v $(OBJS) $(OBJS:.o=.d) zish

.PHONY: all clean
.PRECIOUS: *.d *.o
