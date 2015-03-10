CFLAGS := -m64 -std=c99 -g -MMD -Wall -Wextra $(OPTFLAGS)

sources = $(wildcard src/*.c)
others = $(filter-out src/herp.c src/derp.c, $(sources))

.PHONY: clean

all: bin/derp bin/herp

bin/%: src/%.o $(others) | bin
	$(LINK.c) $^ -o $@

bin:
	mkdir bin

clean:
	rm -rf bin
	rm -f src/*.[do]

-include $(patsubst %.c, %.d, $(sources))
