rm -rf face.pnm
cp -rf $1 .
bunzip2 *.bz2
mv *.ppm face.pnm
