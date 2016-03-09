find ~stiven/feret/data/smaller/* -name "*" -print | sort > caco
grep ppm caco > files.txt
rm caco 
