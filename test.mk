# Config: files & dirs.
tests:=generic/avl_test.c generic/mempool_test.c \
	llexer_test.c lparser_test.c lmutator_test.c \
	lval_test.c lenv_test.c lbuiltin_operator_test.c lbuiltin_func_test.c \
	leval_test.c marker_test.c
test_build_dir:=$(build_dir)

tests_lisp:=test/stdlib_test.lisp

# Config: options.
DO_MEMCHECK=true # 0 == true

testobjects:=$(tests:%.c=%.o)
testobjects_built:=$(addprefix $(test_build_dir)/,$(testobjects))
testcases:=$(tests:%.c=%)
testcases_built:=$(addprefix $(test_build_dir)/,$(testcases))

TEST_CFLAGS:=-DSNOW_ENABLED -g

test: $(testcases) $(tests_lisp)

llexer_test: $(addprefix $(build_dir)/,llexer.o lerr.o)
lparser_test: $(addprefix $(build_dir)/,lparser.o llexer.o lerr.o)
lmutator_test: $(addprefix $(build_dir)/, \
	lmutator.o lparser.o llexer.o lerr.o \
	lval.o lfunc.o lenv.o leval.o \
	lbuiltin.o lbuiltin_condition.o lbuiltin_func.o lbuiltin_operator.o \
	generic/avl.o \
	vendor/mini-gmp/mini-gmp.o \
	)

$(testcases): %: $(test_build_dir)/%.o
	@$(CC) $(LDFLAGS) $(LDLIBS) $^ $(TEST_CFLAGS) -o $(test_build_dir)/$@
	@if [ $$($(DO_MEMCHECK); echo $$?) -eq 0 ]; then valgrind $(VGFLAGS) ./$(test_build_dir)/$@; \
		else ./$(test_build_dir)/$@; fi

$(test_build_dir)/%_test.o: %_test.c $$(@D)/.f
	@$(CC) $(CFLAGS) $(TEST_CFLAGS) -c -o $@ $<

$(tests_lisp): $(PROGNAME)
	./$< -f $@

clean::
	rm -f $(testobjects_built) $(testcases_built)

# Include dependancies makefiles.
include $(tests:%.c=$(build_dir)/%.d)

.PHONY: test $(testcases) $(testcases_built) $(tests_lisp)
