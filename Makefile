out=alisp
sources=alisp.c
objects=$(sources:%.c=%.o)
version_file=version.mk
build_file=buildnumber.mk
version_header=version.h

SHELL=/bin/bash
DEBUG?=-ggdb3 -O0
CFLAGS=-Wall -c -std=c99 $(DEBUG)
LDFLAGS=-Wall

include $(version_file)
include $(build_file)

all: build

build: $(out)

clean:
	rm -f *.o *.d tags $(out)

# Build executable.
$(out): $(objects)

# Generate C source files dependancies.
%.d: %.c
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

# Include dependancies makefiles.
include $(sources:%.c=%.d)

# Generate tags file.
tags:
	ctags -R .

# Increment build number.
$(build_file): $(objects)
	@if ! test -f $@; then echo 0 > $@; fi
	@echo "# Automatically incremented." > $@
	@echo "BUILDNUMBER="$$(( $(BUILDNUMBER)+1 )) >> $@

# version.h contains version informations.
# Don't depends on $(objects) to avoid circular dependancies.
$(version_header): $(sources) $(version_file)
	@echo -e "#ifndef _H_VERSION_" > $(version_header)
	@echo -e "#define _H_VERSION_\n" >> $(version_header)
	@echo -e "#define PROGNAME\t\""$(PROGNAME)"\"" >> $(version_header)
	@echo -e "#define VERSION\t\t\""$(VERSION)"\"" >> $(version_header)
	@echo -e "#define CODENAME\t\""$(CODENAME)"\"" >> $(version_header)
	@echo -e "#define BUILD\t\t"$$(( $(BUILDNUMBER)+1 )) >> $(version_header)
	@echo -e "\n#endif" >> $(version_header)

# List of all special targets (always out-of-date).
.PHONY: all build clean
