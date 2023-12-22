TESTFILES := $(wildcard *.dm)
TESTS := $(TESTFILES:.dm=.test)
LIBLINUX := -dynamic-linker /lib64/ld-linux-x86-64.so.2

.PHONY: all

all: 
	$(TESTS)

%.dm:
	@echo "TEST $*"
	@../dmc $*.dm -o $*.s ;\
	COMP_EXIT_CODE=$$?;
	@as -o $*.o $*.s
	@ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 \
		/usr/lib/x86_64-linux-gnu/crt1.o \
		/usr/lib/x86_64-linux-gnu/crti.o \
		-lc \
		this.o \
		stddrewno_mars.o \
		/usr/lib/x86_64-linux-gnu/crtn.o \
		-o  this.prog
	@./this.prog < p7_tests/write4.in > testFile.out; \
	diff -B --ignore-all-space $*.out $*.out.expected;\
	RUN_DIFF_EXIT=$$?;\
	exit $$RUN_DIFF_EXIT

clean:
	rm -f *.3ac *.out *.err *.o *.s *.prog
