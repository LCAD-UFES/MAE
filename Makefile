#DIRS_EXAMPLES := $(shell  ls -d examples/*)
DIRS_EXAMPLES := examples/circle examples/face_recog examples/robot examples/visual_search
.PHONY: example $(DIRS_EXAMPLES)

build:
	$(MAKE) -C src
	
clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C docs

install:
	./configure
	$(MAKE) -C src
	
documentation:
	$(MAKE) documentation -C docs

cleanauto:
	rm -f *.log *.status confdefs.h *.cache configure *~ src/Makefile bin/netcompiler

test:	
	$(MAKE) clean -C src
	$(MAKE) -C src
	@rm -f times.out
	@for dir in $(DIRS_EXAMPLES) ; do \
		$(MAKE) clean -C $$dir ; \
		$(MAKE) test -C $$dir ; \
		echo "$$dir" >> times.out ; \
		grep "user" $$dir/times.out >> times.out ; \
	done
	@echo "Test finished. Check the execution times archive. (./times.out)"

default:	
	$(MAKE) clean -C src
	$(MAKE) -C src
	@rm -f times.out
	@for dir in $(DIRS_EXAMPLES) ; do \
		$(MAKE) clean -C $$dir ; \
		$(MAKE) default -C $$dir ; \
		echo "$$dir" >> times.out ; \
		grep "user" $$dir/times.out >> times.out ; \
	done
	@echo "Test finished. Check the execution times archive. (./times.out)"
