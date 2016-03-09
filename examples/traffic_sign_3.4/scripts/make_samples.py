import cv
from random import random

d_pos = range(-2,2)
d_scale = [0.9, 0.95, 1.05, 1.1]
d_degrees = range(-10,10)


	

def rotation_mat(img):
	center = img.width/2, img.height/2
	angle = d_degrees[int(random()*len(d_degrees))]
	scale = 1.0
	m = cv.CreateMat(2,3,cv.CV_32F)
	cv.GetRotationMatrix2D(center, angle, scale, m)
	return m
	

def jitter_show():
	folder = "./training_set_raw/"
	desc = open("./training_set_raw/desc_training_set")
	ls = desc.readlines()
	for l in ls:
		Filename, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2 = l.split()
		Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2 =  int(Width), int(Height), int(Roi_X1), int(Roi_Y1), int(Roi_X2), int(Roi_Y2)
		img = cv.LoadImage(folder+Filename)
		warped = cv.CreateImage(cv.GetSize(img),8,3)
		cv.WarpAffine(img, warped, rotation_mat(img))
		cv.ShowImage("",warped)
		cv.WaitKey()

def newname(name, mod):
	c,i,f = name[:-4].split("_")
	c,i,f = int(c),int(i),int(f)
	n = "%05d_%05d_%05d.ppm"%(c,i,f+mod)
	return n


def jitter_pos():
	
	folder = "./training_set_raw/"
	desc = open("./training_set_raw/desc_training_set")
	
	newfolder = "./JITTER/training_set_jittered/"
	newdesc = open(newfolder+"desc_training_set_JITTER_pos","w")
	
	ls = desc.readlines()
	for l in ls:
		Filename, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = l.split()
		Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId =  int(Width), int(Height), int(Roi_X1), int(Roi_Y1), int(Roi_X2), int(Roi_Y2), int(ClassId)
		
		img = cv.LoadImage(folder+Filename)
		
		dx = d_pos[int(random()*len(d_pos))]
		dy = d_pos[int(random()*len(d_pos))]
		
		cv.SaveImage(newfolder+newname(Filename,100),img)
		
		newdesc.write("%s %d %d %d %d %d %d %d\n" % (newname(Filename,100), Width, Height, Roi_X1+dx, Roi_Y1+dy, Roi_X2+dx, Roi_Y2+dy, ClassId))
		cv.ResetImageROI(img)
		
def jitter_angle():

	folder = "./training_set_raw/"
	desc = open("./training_set_raw/desc_training_set")
	
	newfolder = "./JITTER/training_set_jittered/"
	newdesc = open(newfolder+"desc_training_set_JITTER_angle","w")
	
	ls = desc.readlines()
	for l in ls:
		Filename, Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId = l.split()
		Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId =  int(Width), int(Height), int(Roi_X1), int(Roi_Y1), int(Roi_X2), int(Roi_Y2), int(ClassId)
		
		img = cv.LoadImage(folder+Filename)
		
		warped = cv.CreateImage(cv.GetSize(img),8,3)
		cv.WarpAffine(img, warped, rotation_mat(img))
		
		cv.SaveImage(newfolder+newname(Filename,200),warped)
		newdesc.write("%s %d %d %d %d %d %d %d\n" % (newname(Filename,200), Width, Height, Roi_X1, Roi_Y1, Roi_X2, Roi_Y2, ClassId))


#jitter_angle()
#jitter_pos()
#jitter_show()

















