# Gnuplot command file for Aspirin NN generated by pwb_pred <freitas@computer.org>
# Generated at 2005Apr11-16:27:53 

set datafile separator ";"
set style data linespoints
#
# --- Targets X Outputs graphics
#
set title ""
# - Plot command
plot \
"caco" u 2 title "TGT" lt 3 pt 4,\
"caco" u 3 title "PRD" lt 1 pt 7
#
# EOF
