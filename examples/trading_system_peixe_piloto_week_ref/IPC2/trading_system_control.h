/** Header file generated with fdesign on Wed Jul  6 23:19:31 2011.**/

#ifndef FD_Trading_System_h_
#define FD_Trading_System_h_

/** Callbacks, globals and object handlers **/
extern void buy_button_callback(FL_OBJECT *, long);
extern void order_buy_price_callback(FL_OBJECT *, long);
extern void order_buy_quantity_callback(FL_OBJECT *, long);
extern void fast_buy_button_callback(FL_OBJECT *, long);
extern void sell_button_callback(FL_OBJECT *, long);
extern void order_sell_price_callback(FL_OBJECT *, long);
extern void order_sell_quantity_callback(FL_OBJECT *, long);
extern void fast_sell_button_callback(FL_OBJECT *, long);
extern void plot_graph_button_callback(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *Trading_System;

extern FL_OBJECT
        *best_stock_name_val,
        *price_val,
        *state_val,
        *moving_average_price_val,
        *order_price_val,
        *quantity_val,
        *maximum_buy_price_val,
        *previous_minimum_intraday_price_val,
        *possible_buy_order_price_val,
        *minimum_sell_price_val,
        *previous_maximum_intraday_price_val,
        *possible_sell_order_price_val,
        *buy_price_val,
        *sell_price_val,
        *predicted_return_val,
        *current_return_val,
        *final_return_val,
        *date_and_time_val,
        *buy_button,
        *order_buy_price_val,
        *order_buy_quantity_val,
        *fast_buy_button,
        *sell_button,
        *order_sell_price_val,
        *order_sell_quantity_val,
        *fast_sell_button,
        *plot_graph_button;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_Trading_System_h_ */
