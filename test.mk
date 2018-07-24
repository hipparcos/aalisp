# Config: files & dirs.
tests:=lval_test.c lbuiltin_test.c lisp_test.c llexer_test.c lparser_test.c
test_build_dir:=$(build_dir)

# Config: options.
DO_MEMCHECK=true # 0 == true

testobjects:=$(tests:%.c=%.o)
testobjects_built:=$(addprefix $(test_build_dir)/,$(testobjects))
testcases:=$(tests:%.c=%)
testcases_built:=$(addprefix $(test_build_dir)/,$(testcases))

TEST_CFLAGS:=-DSNOW_ENABLED -g

test: $(testcases)

$(testcases): %: $(test_build_dir)/%

$(testcases_built): % : %.o $(filter-out $(build_dir)/$(PROGNAME).o,$(objects))
# $(test_build_dir)/%_test: $(test_build_dir)/%_test.o $(filter-out $(build_dir)/$(PROGNAME).o,$(objects))
	@$(CC) $(LDFLAGS) $(LDLIBS) $^ $(TEST_CFLAGS) -o $@
	@if [ $$($(DO_MEMCHECK); echo $$?) -eq 0 ]; then valgrind $(VGFLAGS) ./$@; \
		else ./$@; fi

$(test_build_dir)/%_test.o: %_test.c $$(@D)/.f
	@$(CC) $(CFLAGS) $(TEST_CFLAGS) -c -o $@ $<

clean::
	rm -f $(testobjects_built) $(testcases_built)

# Include dependancies makefiles.
include $(tests:%.c=$(build_dir)/%.d)

.PHONY: test $(testcases) $(testcases_built)