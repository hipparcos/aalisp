tests:=lval_test.c #lbuiltin_test.c
test_build_dir:=$(build_dir)

testcases:=$(addprefix $(test_build_dir)/,$(tests:%.c=%))
testobjects:=$(addprefix $(test_build_dir)/,$(tests:%.c=%.o))

TEST_CFLAGS:=-DSNOW_ENABLED -g

test: $(testcases)

# $(testcases): % : %.o $(filter-out $(build_dir)/$(PROGNAME).o,$(objects))
build/lval_test: $(addprefix $(build_dir)/,lval_test.o lval.o vendor/mini-gmp/mini-gmp.o)
	@$(CC) $(CFLAGS) $(TEST_CFLAGS) $^ $(LDFLAGS) -o $@
	@valgrind $(VGFLAGS) ./$@

clean::
	rm -f $(testobjects) $(testcases)

# Include dependancies makefiles.
include $(tests:%.c=$(build_dir)/%.d)

.PHONY: test $(testcases)
