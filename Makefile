CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wshadow
CPPFLAGS = -MMD -MF $*.d
LDLIBS =

all: main

-include *.d

main: $(OBJS)

%.d: ;

%.o: %.d

clean:
	@$(RM) -v $(OBJS) $(OBJS:.o=.d) main

.PHONY: all clean
.PRECIOUS: *.d *.o

