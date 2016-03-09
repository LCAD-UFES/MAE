BEGIN { 
	average_target_return_buy_sell = 0.0; 
	average_wnn_error_buy_sell = 0.0; 

	standard_deviation_target_return_buy_sell = 0.0;
	standard_deviation_wnn_error_buy_sell = 0.0;

	hr_up_buy_sell = 0.0; 
	hr_dn_buy_sell = 0.0; 
	hr_buy_sell = 0.0;

	average_target_return_sell_buy = 0.0; 
	average_wnn_error_sell_buy = 0.0; 

	standard_deviation_target_return_sell_buy = 0.0;
	standard_deviation_wnn_error_sell_buy = 0.0;

	hr_up_sell_buy = 0.0; 
	hr_dn_sell_buy = 0.0; 
	hr_sell_buy = 0.0;

	count = 0; 
}

/average_target_return_buy_sell/ { 
	average_target_return_buy_sell += $4; 
	average_wnn_error_buy_sell += $6; 
	count++ 
}

/standard_deviation_target_return_buy_sell/ { 
	standard_deviation_target_return_buy_sell += $4; 
	standard_deviation_wnn_error_buy_sell += $6; 
}

/hr_up_buy_sell/ { 
	hr_up_buy_sell += $4; 
	hr_dn_buy_sell += $6; 
	hr_buy_sell += $8; 
}



/average_target_return_sell_buy/ { 
	average_target_return_sell_buy += $4; 
	average_wnn_error_sell_buy += $6; 
}

/standard_deviation_target_return_sell_buy/ { 
	standard_deviation_target_return_sell_buy += $4; 
	standard_deviation_wnn_error_sell_buy += $6; 
}

/hr_up_sell_buy/ { 
	hr_up_sell_buy += $4; 
	hr_dn_sell_buy += $6; 
	hr_sell_buy += $8; 
}

END { 
	print 	"target_return;  wnn_error; " \
		"sd_target_return_buy_sell; sd_wnn_error_buy_sell; "\
	      	" hr_up_buy_sell; hr_dn_buy_sell; hr_buy_sell"
	print 	"  " average_target_return_buy_sell/count "; " average_wnn_error_buy_sell/count ";       "\
		standard_deviation_target_return_buy_sell/count ";                " standard_deviation_wnn_error_buy_sell/count ";             "\
		hr_up_buy_sell/count "%;       " hr_dn_buy_sell/count "%;      " hr_buy_sell/count "%"

	print 	"target_return;  wnn_error; " \
		"sd_target_return_sell_buy; sd_wnn_error_sell_buy; "\
	      	" hr_up_sell_buy; hr_dn_sell_buy; hr_sell_buy"
	print 	"  " average_target_return_sell_buy/count "; " average_wnn_error_sell_buy/count ";       "\
		standard_deviation_target_return_sell_buy/count ";                " standard_deviation_wnn_error_sell_buy/count ";             "\
		hr_up_sell_buy/count "%;       " hr_dn_sell_buy/count "%;      " hr_sell_buy/count "%"
}


#  ****  average_target_return_buy_sell:  0.002448, average_wnn_error_buy_sell:  0.000667
#  ****  standard_deviation_target_return_buy_sell:  0.009583, standard_deviation_wnn_error_buy_sell:  0.012621
#  ****  hr_up_buy_sell: 71.57, hr_dn_buy_sell: 27.78, hr_buy_sell: 56.41
#
#  ****  average_target_return_sell_buy:  0.001524, average_wnn_error_sell_buy: -0.000648
#  ****  standard_deviation_target_return_sell_buy:  0.009291, standard_deviation_wnn_error_sell_buy:  0.011106
#  ****  hr_up_sell_buy: 62.96, hr_dn_sell_buy: 35.42, hr_sell_buy: 54.49
