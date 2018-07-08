out=alisp
sources=alisp.c
objects=$(sources:%.c=%.o)

DEBUG?=-g -O0
CFLAGS=-Wall -c -std=c99 $(DEBUG)
LDFLAGS=-Wall

all: build

build: $(out)

clean:
	rm -f *.o *.d tags $(out)

rebuild: clean build

$(out): $(objects)

# Generate C source files dependancies.
%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

# Include dependancies makefiles.
include $(sources:%.c=%.d)

tags:
	ctags -R .

# List of all special targets (always out-of-date).
.PHONY: all build clean rebuild
