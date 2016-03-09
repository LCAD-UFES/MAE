set term postscript eps enhanced color "Times-Roman" 24
set encoding iso_8859_1

set xrange [1:]
set yrange [1:100]
set logscale x
set grid lt 9
set xtics ("2x2" 4, "4x4" 16, "8x8" 64, "16x16" 256, "32x32" 1024, "64x64" 4096, "128x128" 16384)
set xtics rotate by 90
set key bottom
set xlabel "Number of neurons"
set ylabel "Recognition rate (%)"

set output "face_recog_without_filters_gaussian_results.eps"
plot    "< awk '{if($2==32)print$0}' face_recog_without_filters_gaussian_results.txt" using ($1*$1):3 title "Number of synapses=32" with linespoints,\
	"< awk '{if($2==64)print$0}' face_recog_without_filters_gaussian_results.txt" using ($1*$1):3 title "Number of synapses=64" with linespoints,\
        "< awk '{if($2==128)print$0}' face_recog_without_filters_gaussian_results.txt" using ($1*$1):3 title "Number of synapses=128" with linespoints,\
        "< awk '{if($2==256)print$0}' face_recog_without_filters_gaussian_results.txt" using ($1*$1):3 title "Number of synapses=256" with linespoints

set output "face_recog_without_filters_random_results.eps"
plot    "< awk '{if($2==32)print$0}' face_recog_without_filters_random_results.txt" using ($1*$1):3 title "Number of synapses=32" with linespoints,\
	"< awk '{if($2==64)print$0}' face_recog_without_filters_random_results.txt" using ($1*$1):3 title "Number of synapses=64" with linespoints,\
        "< awk '{if($2==128)print$0}' face_recog_without_filters_random_results.txt" using ($1*$1):3 title "Number of synapses=128" with linespoints,\
        "< awk '{if($2==256)print$0}' face_recog_without_filters_random_results.txt" using ($1*$1):3 title "Number of synapses=256" with linespoints

