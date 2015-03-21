CFLAGS := -m64 -std=c99 -g -MMD -Wall -Wextra -D_BSD_SOURCE $(OPTFLAGS)

sources = $(wildcard src/*.c)
objects = $(patsubst %.c, %.o, $(filter-out src/herp.c src/derp.c, $(sources)))

.PHONY: all clean test

all: bin/derp bin/herp

test: bin/test
	./$<

bin/test: $(patsubst %.c, %.o, $(wildcard test/*.c)) $(objects) | bin
	$(LINK.c) $^ -o $@

bin/%: src/%.o $(objects) | bin
	$(LINK.c) $^ -o $@

bin:
	mkdir bin

clean:
	rm -rf bin
	rm -f src/*.[do]

-include $(patsubst %.c, %.d, $(sources))
