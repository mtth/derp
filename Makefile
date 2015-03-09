CFLAGS := -m64 -std=c99 -g -MMD -Wall -Wextra $(OPTFLAGS)

sources = $(wildcard src/*.c)
objects = $(patsubst %.c, %.o, $(sources))

.PHONY: clean

all: bin/derp bin/herp

bin/%: $(objects) | bin
	$(LINK.c) $(filter-out src/$*.o, $^) -o $@

bin:
	mkdir bin

clean:
	rm -rf bin
	rm -f src/*.[do]

-include $(patsubst %.c, %.d, $(sources))
