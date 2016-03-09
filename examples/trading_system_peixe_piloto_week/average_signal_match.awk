BEGIN { 
	average_target_return = 0.0; 
	average_wnn_error = 0.0; 

	standard_deviation_target_return = 0.0;
	standard_deviation_wnn_error = 0.0;

	hr_up = 0.0; 
	hr_dn = 0.0; 
	hr = 0.0;
	count = 0; 
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

/hr_up/ { 
	hr_up += $4; 
	hr_dn += $6; 
	hr += $8; 
}

END { 
	print 	"target_return; wnn_error; " \
		"standard_deviation_target_return; standard_deviation_wnn_error; "\
	      	"hr_up; hr_dn; hr"
	print 	average_target_return/count "; " average_wnn_error/count "; "\
		standard_deviation_target_return/count "; " standard_deviation_wnn_error/count "; "\
		hr_up/count "%; " hr_dn/count "%; " hr/count "%"
}


#  ****  average_target_return:  0.006381, average_wnn_error:  0.002364
#  ****  standard_deviation_target_return:  0.038023, standard_deviation_wnn_error:  0.050875
#  ****  hr_up: 30.19, hr_dn: 12.00, hr: 24.36
