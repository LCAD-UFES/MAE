FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

run:
	netcompiler $(COMPILE) 
	

clean:
	rm -rf shape_*
	rm -rf *.dat
	rm -rf *.o
	rm -rf disparity_user_functions/*.o
	rm -rf *.c
	rm -rf myPlot
	
