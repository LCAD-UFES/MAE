CC := gcc -g
CONFILE := $(shell ls *.con)
EXAMPLE := $(CONFILE:%.con=%)

build:
	netcompiler $(EXAMPLE)

clean:
	rm -f *.o $(EXAMPLE)_user_functions/*.o
	rm -f $(EXAMPLE)
	rm -f $(EXAMPLE).[c,h]
	rm -f *.out
	rm -f $(EXAMPLE).sh.*
	
test:
	netcompiler $(EXAMPLE)
	@echo "Testing $(EXAMPLE)..."
	@{ time ./$(EXAMPLE) $(EXAMPLE).cml ; } > $(EXAMPLE).test 2> times.out
	@if cmp -s $(EXAMPLE).test $(EXAMPLE).default ; \
	then echo "$(EXAMPLE) is OK" ; \
	else echo "$(EXAMPLE) error" ; \
	fi
 
default:
	netcompiler $(EXAMPLE)
	@echo "Testing $(EXAMPLE)..."
	@{ time ./$(EXAMPLE) $(EXAMPLE).cml ; } > $(EXAMPLE).default 2> times.out
	

