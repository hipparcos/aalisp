tests:=polish_test.c
testcases:=$(tests:%.c=%)

polish_test: vendor/mpc/mpc.o lval.o

test: $(testcases)

$(testcases): % : %.o
	@echo "---- "$@ && $(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ && ./$@ && echo "----"

clean::
	rm -f $(testcases)

# Include dependancies makefiles.
include $(tests:%.c=%.d)

.PHONY: test $(testcases)
