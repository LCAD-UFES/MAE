set mouse labels
plot	'ini.out' using 1:2 title 'Start Point' with impulses,\
	'verg.out' using 1:2 title 'Vergence' with impulses,\
	'minus.out' using 1:2 title 'Response on minus' with lines
pause -1  '"press return"'
