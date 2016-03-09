rm -f img_left.jpg
rm -f img_right.jpg


./capture "http://10.50.2.129/__live.jpg?&&&" img_left.jpg > caco
./capture "http://10.50.2.130/__live.jpg?&&&" img_right.jpg > caco

convert img_left.jpg  img_left.pnm
convert img_right.jpg img_right.pnm
rm -f img_left.jpg
rm -f img_right.jpg
rm -f caco

