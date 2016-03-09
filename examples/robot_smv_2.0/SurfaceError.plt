set logscale x 2
set logscale y
set xlabel "Working distance [cm]"
set key left top
set xtics 12.5,2,6400
set mytics 5
set grid xtics ytics mxtics mytics lt 9, lt 0
show grid
set title "X uncertainty for one pixel vergence error"
set ylabel "X uncertainty [cm]"
plot "SpatialResolution.dat" every :::1::1 using ($1/10):($3/10) title "Baseline 12.5cm" with lines, \
     "SpatialResolution.dat" every :::2::2 using ($1/10):($3/10) title "Baseline 25cm" with lines, \
     "SpatialResolution.dat" every :::3::3 using ($1/10):($3/10) title "Baseline 50cm" with lines
pause -1
set title "Y uncertainty for one pixel vergence error"
set ylabel "Y uncertainty [cm]"
plot "SpatialResolution.dat" every :::1::1 using ($1/10):($4/10) title "Baseline 12.5cm" with lines, \
     "SpatialResolution.dat" every :::2::2 using ($1/10):($4/10) title "Baseline 25cm" with lines, \
     "SpatialResolution.dat" every :::3::3 using ($1/10):($4/10) title "Baseline 50cm" with lines
pause -1
set title "Z uncertainty for one pixel vergence error"
set ylabel "Z uncertainty [cm]"
plot "SpatialResolution.dat" every :::1::1 using ($1/10):($5/10) title "Baseline 12.5cm" with lines, \
     "SpatialResolution.dat" every :::2::2 using ($1/10):($5/10) title "Baseline 25cm" with lines, \
     "SpatialResolution.dat" every :::3::3 using ($1/10):($5/10) title "Baseline 50cm" with lines
pause -1
set title "Vectorial uncertainty for one pixel vergence error"
set ylabel "Vectorial uncertainty [cm]"
plot "SpatialResolution.dat" every :::1::1 using ($1/10):(sqrt($3*$3+$4*$4+$5*$5)/10) title "Baseline 12.5cm" with lines, \
     "SpatialResolution.dat" every :::2::2 using ($1/10):(sqrt($3*$3+$4*$4+$5*$5)/10) title "Baseline 25cm" with lines, \
     "SpatialResolution.dat" every :::3::3 using ($1/10):(sqrt($3*$3+$4*$4+$5*$5)/10) title "Baseline 50cm" with lines
pause -1
