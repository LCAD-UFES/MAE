set term postscript eps enhanced color "Times-Roman" 28
set encoding iso_8859_1

set yrange[0.5:1]
set ytics 0.05
set logscale x
set xtics ("4x4" 16, "8x8" 64, "32x32" 1024)
set xlabel "Number of neurons"
set key right bottom

set output "Precision_neu.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Precision"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):4 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):4 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):4 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):4 title "Number of synapses=1024" with linespoints lw 5

set output "Recall_neu.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Recall"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):5 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):5 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):5 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):5 title "Number of synapses=1024" with linespoints lw 5

set output "False_positive_rate_neu.eps"
#set key right top
set yrange[0:1]
set ylabel "False_positive_rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):6 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):6 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):6 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):6 title "Number of synapses=1024" with linespoints lw 5
	
set output "False_negative_rate_neu.eps"
#set key right top
set yrange[0:1]
set ylabel "False Negative Rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):7 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):7 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):7 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):7 title "Number of synapses=1024" with linespoints lw 5

set output "Precision_min.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Precision"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):8 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):8 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):8 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):8 title "Number of synapses=1024" with linespoints lw 5
	
set output "Recall_min.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Recall"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):9 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):9 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):9 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):9 title "Number of synapses=1024" with linespoints lw 5
	
set output "False_positive_rate_min.eps"
#set key right top
set yrange[0:1]
set ylabel "False_positive_rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):10 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):10 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):10 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):10 title "Number of synapses=1024" with linespoints lw 5
	
set output "False_negative_rate_min.eps"
#set key right top
set yrange[0:1]
set ylabel "False Negative Rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):11 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):11 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):11 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):11 title "Number of synapses=1024" with linespoints lw 5
	
set output "Precision_max.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Precision"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):12 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):12 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):12 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):12 title "Number of synapses=1024" with linespoints lw 5
	
set output "Recall_max.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Recall"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):13 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):13 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):13 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):13 title "Number of synapses=1024" with linespoints lw 5
set output "False_positive_rate_max.eps"
#set key right top
set yrange[0:1]
set ylabel "False_positive_rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):14 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):14 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):14 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):14 title "Number of synapses=1024" with linespoints lw 5
	
set output "False_negative_rate_max.eps"
#set key right top
set yrange[0:1]
set ylabel "False Negative Rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):15 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):15 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):15 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):15 title "Number of synapses=1024" with linespoints lw 5

set output "Precision_avg.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Precision"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):16 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):16 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):16 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):16 title "Number of synapses=1024" with linespoints lw 5
	
set output "Recall_avg.eps"
#set key right bottom
set yrange[0:1]
set ylabel "Recall"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):17 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):17 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):17 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):17 title "Number of synapses=1024" with linespoints lw 5
	
set output "False_positive_rate_avg.eps"
#set key right top
set yrange[0:1]
set ylabel "False_positive_rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):18 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):18 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):18 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):18 title "Number of synapses=1024" with linespoints lw 5
set output "False_negative_rate_avg.eps"
#set key right top
set yrange[0:1]
set ylabel "False Negative Rate"
plot   	"<  awk '{if ($3==128)print$0}' tuning_results.dat" using ($1*$2):19 title "Number of synapses=128" with linespoints lw 5,\
	"<  awk '{if ($3==256)print$0}' tuning_results.dat" using ($1*$2):19 title "Number of synapses=256" with linespoints lw 5, \
	"<  awk '{if ($3==512)print$0}' tuning_results.dat" using ($1*$2):19 title "Number of synapses=512" with linespoints lw 5, \
	"<  awk '{if ($3==1024)print$0}' tuning_results.dat" using ($1*$2):19 title "Number of synapses=1024" with linespoints lw 5
