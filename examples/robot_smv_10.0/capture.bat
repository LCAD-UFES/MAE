del img_left.jpg
del img_right.jpg

comline -get "http://172.26.20.241/__live.jpg?&&&" -o img_left.jpg
comline -get "http://172.26.20.242/__live.jpg?&&&" -o img_right.jpg

convert img_left.jpg -compress None img_left.pnm
convert img_right.jpg -compress None img_right.pnm

del img_left.jpg
del img_right.jpg

