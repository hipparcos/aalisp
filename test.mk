tests:=polish_test.c
testcases:=$(tests:%.c=%)

polish_test: vendor/mpc/mpc.o vendor/mini-gmp/mini-gmp.o lval.o operator.o

test: $(testcases)

$(testcases): % : %.o
	@echo "---- "$@ \
		&& $(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@ \
		&& valgrind $(VGFLAGS) ./$@ \
		&& echo "----"

clean::
	rm -f $(testcases)

# Include dependancies makefiles.
include $(tests:%.c=%.d)

.PHONY: test $(testcases)
