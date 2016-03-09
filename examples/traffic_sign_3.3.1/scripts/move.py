from os.path import join
import os

training_folders = [join("../Final_Training", "%05d"%(i)) for i in range(0,43)]
csv_info_train = [join(join(join("../Final_Training","Images"), "%05d"%(i)),"GT-%05d.csv"%(i)) for i in range(0,43)]

testing_folder = join("..","Final_Test")
csv_info_test = join(testing_folder,"GT-final_test.csv")


def move_imgs_test():
	csv = open(csv_info_test)
	lines = csv.readlines()[1:]
	for line in lines:
		Filename, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = line.split(';')
		Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = int(Width), int(Height), int(Roi_X1), int(Roi_Y1), int(Roi_X2), int(Roi_Y2), int(ClassId)
		
		file_path = "../Final_Test/Images/" + Filename
		Filename2 = ("%05d_" % ClassId) + Filename[:-4] + "_00000" + ".ppm"
		file_path_out = "../_scripts/testing_set_raw/" + Filename2
		
		command = "cp -f %s %s" % (file_path, file_path_out)
		os.system(command)
		
		print Filename2, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId


def move_imgs_train():
	
	for seqn in range(43):
		csv = open(csv_info_train[seqn])
		lines = csv.readlines()[1:] #ignore header
		for line in lines:
			Filename, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = line.split(';')
			Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = int(Width), int(Height), int(Roi_X1), int(Roi_Y1), int(Roi_X2), int(Roi_Y2), int(ClassId)
			
			file_path = "../Final_Training/Images/"+"%05d/"%(seqn)+Filename

			Filename2 = ("%05d_" % ClassId) + Filename
			file_path_out = "../_scripts/training_set_raw/" + Filename2
			
			command = "cp -f %s %s" % (file_path, file_path_out)
			os.system(command)
			print Filename2, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId
			

#move_imgs_train()
#move_imgs_test()
