BEGIN { 
	average_target_return = 0.0; 
	average_wnn_error = 0.0; 

	standard_deviation_target_return = 0.0;
	standard_deviation_wnn_error = 0.0;

	wnn_same_up = 0.0; 
	wnn_same_down = 0.0; 
	count = 0 
}

/average_target_return/ { 
	average_target_return += $4; 
	average_wnn_error += $6; 
	count++ 
}

/standard_deviation_target_return/ { 
	standard_deviation_target_return += $4; 
	standard_deviation_wnn_error += $6; 
}

/wnn_same_up/ { 
	wnn_same_up += $4; 
	wnn_same_down += $6; 
	wnn_same_signal += $8; 
}

END { 
	print 	"num_stocks; " "target_return; wnn_error; " \
		"standard_deviation_target_return; standard_deviation_wnn_error; "\
	      	"wnn_same_up; wnn_same_down; wnn_same_signal"
	print 	count "; " average_target_return/count "; " average_wnn_error/count "; "\
		standard_deviation_target_return/count "; " standard_deviation_wnn_error/count "; "\
		wnn_same_up/count "; " wnn_same_down/count "; "	wnn_same_signal/count
}


# **** average_target_return: -0.004267, average_wnn_error: -0.013357, average_rna_error:  0.003470
# **** standard_deviation_target_return:  0.047842, standard_deviation_wnn_error:  0.054220, standard_deviation_rna_error:  0.062345
# **** wnn_same_up: 66.67, wnn_same_down: 40.54, rna_same_up: 41.67, rna_same_down: 59.46
# **** wnn_same_signal: 50.82, rna_same_signal: 52.46
