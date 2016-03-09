CC := gcc296
FILES := $(shell ls *.con)
COMPILE := $(FILES:%.con=%)

all: stereo.con
	netcompiler $(COMPILE)
	gcc -o convert_image convert_image.c `pkg-config --cflags opencv` `pkg-config --libs opencv`

run_teddy:
	./stereo stereo_teddy.cml

run_teddy_evaluation:
	./convert_image teddy_disparity_map.ppm teddy.pgm exp_teddy.txt exp_teddy_all.txt teddy
	cp teddy.pgm StereoEvaluation/teddy/results/
	cp exp_teddy.txt exp_teddy_all.txt StereoEvaluation/Scripts
	cd StereoEvaluation/StereoMatch/
	make
	./StereoMatch script ../exp_teddy_all.txt
	cd -
clean:
	rm -f *.o stereo_user_functions/*.o
	rm -f stereo stereo.c stereo.h
	rm -f *.out *.txt *.ppm *.pgm convert_image *~
