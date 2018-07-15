tests:=lbuiltin_test.c
test_build_dir:=$(build_dir)

testcases:=$(addprefix $(test_build_dir)/,$(tests:%.c=%))
testobjects:=$(addprefix $(test_build_dir)/,$(tests:%.c=%.o))

test: $(testcases)

$(testcases): % : %.o $(filter-out $(build_dir)/$(PROGNAME).o,$(objects))
	@echo "---- "$@ \
		&& $(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ \
		&& valgrind $(VGFLAGS) ./$@ \
		&& echo "----"

clean::
	rm -f $(testobjects) $(testcases)

# Include dependancies makefiles.
include $(tests:%.c=$(build_dir)/%.d)

.PHONY: test $(testcases)
