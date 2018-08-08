out=$(PROGNAME)
sources=$(PROGNAME).c vendor/mini-gmp/mini-gmp.c \
		leval.c lval.c lsym.c lbuiltin.c llexer.c lparser.c lmut.c
headers=vendor/mini-gmp/mini-gmp.h \
		leval.h lval.h lsym.h lbuiltin.h llexer.h lparser.h lmut.h

build_dir:=build
version_file:=version.mk
build_file:=buildnumber.mk
test_file:=test.mk
version_header:=version.h

objects=$(addprefix $(build_dir)/,$(sources:%.c=%.o))
deps=$(addprefix $(build_dir)/,$(sources:%.c=%.d))

CC=gcc
SHELL:=/bin/bash
DEBUG?=-ggdb3 -O0
CFLAGS:=-Wall -std=c11 $(DEBUG)
LDFLAGS:=-Wall -lreadline -lm
VGFLAGS?=\
	--quiet --leak-check=full --show-leak-kinds=all \
	--track-origins=yes --error-exitcode=1 --error-limit=no \
	--suppressions=./valgrind-libraryleaks.supp

# Define PROGNAME, VERSION, CODENAME.
include $(version_file)
# Define BUILD.
include $(build_file)

# Use second expansion to create $(build_dir) on demand.
.SECONDEXPANSION:

all: build

build: $(out)

clean::
	rm -f $(objects) $(deps) $(version_header) tags $(out)

# test target.
include $(test_file)

leakcheck: $(out)
	valgrind $(VGFLAGS) ./$^

# Build executable.
$(out): $(objects)
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

# Generate O file in $(build_dir); .f is a directory marker.
$(build_dir)/%.o: %.c $$(@D)/.f
	$(CC) $(CFLAGS) -c -o $@ $<

$(build_dir)/lbuiltin.o: lbuiltin_condition.inc.c lbuiltin_typed_operator.inc.c

# Generate C source files dependancies in $(build_dir); .f is a directory marker.
$(build_dir)/%.d: %.c $(version_header) $$(@D)/.f
	@set -e; rm -f $@; \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

# Include dependancies makefiles.
include $(deps)

# Generate tags file.
tags:
	ctags -R .

# Increment build number & rewrite $(build_file).
$(build_file): $(sources) $(headers)
# Update $(BUILDNUMBER) value in this Makefile.
	$(eval BUILDNUMBER:=$(shell echo $$(( $(BUILDNUMBER)+1 ))))
# Rewrite $(build_file).
	@if ! test -f $@; then touch $@; fi
	@echo "# Automatically incremented." > $@
	@echo "BUILDNUMBER="$(BUILDNUMBER) >> $@

# Rewrite $(version_header) file if required.
$(version_header): $(version_file) $(build_file)
	@if ! test -f $@; then touch $@; fi
	@echo -e "/* Generated by make, do not modify. */\n" > $@
	@echo -e "#ifndef _H_VERSION_" >> $@
	@echo -e "#define _H_VERSION_\n" >> $@
	@echo -e "#define PROGNAME\t\""$(PROGNAME)"\"" >> $@
	@echo -e "#define VERSION\t\t\""$(VERSION)"\"" >> $@
	@echo -e "#define CODENAME\t\""$(CODENAME)"\"" >> $@
	@echo -e "#define BUILD\t\t"$(BUILDNUMBER) >> $@
	@echo -e "\n#endif" >> $@

# Directory marker.
%/.f:
	@mkdir -p $(dir $@)
	@touch $@

.PRECIOUS: %/.f

# List of all special targets (always out-of-date).
.PHONY: all build clean tags
