set mouse labels
plot	'ini.gnuplot' using 1:2 title 'Start Point' with impulses,\
	'verg.gnuplot' using 1:2 title 'Vergence' with impulses,\
	'minus.gnuplot' using 1:2 title 'Response on minus' with lines
pause -1  '"press return"'
