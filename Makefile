lib=allocator.so

# Set the following to '0' to disable log messages:
DEBUG ?= 1

CFLAGS += -Wall -g -pthread -fPIC -shared
LDFLAGS +=

$(lib): allocator.c allocator.h debug.h
	$(CC) $(CFLAGS) $(LDFLAGS) -DDEBUG=$(DEBUG) allocator.c -o $@

docs: Doxyfile
	doxygen

clean:
	rm -f $(lib) $(obj)
	rm -rf docs


# Tests --

test: $(lib) ./tests/run_tests
	./tests/run_tests $(run)

testupdate: testclean test

./tests/run_tests:
	rm -rf ./tests
	git clone https://github.com/usf-cs326-fa19/P3-Tests.git tests
	rm -rf ./tests/.git

testclean:
	rm -rf tests
