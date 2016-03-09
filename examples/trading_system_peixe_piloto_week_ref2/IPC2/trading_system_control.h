/** Header file generated with fdesign on Tue Sep 20 09:08:58 2011.**/

#ifndef FD_Trading_System_h_
#define FD_Trading_System_h_

/** Callbacks, globals and object handlers **/
extern void send_first_orders_button_callback(FL_OBJECT *, long);
extern void plot_graph_button_callback(FL_OBJECT *, long);
extern void send_second_orders_button_callback(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *Trading_System;

extern FL_OBJECT
        *best_stock_name_val,
        *ref_stock_name_val,
        *state_val,
        *moving_average_of_delta_val,
        *first_orders_trigger_moving_average_of_delta_val,
        *best_stock_first_order_quantity_val,
        *best_stock_first_order_price_val,
        *predicted_week_return_val,
        *current_week_return_val,
        *final_week_return_val,
        *date_and_time_val,
        *send_first_orders_button,
        *plot_graph_button,
        *send_second_orders_button,
        *trade_order_val,
        *second_orders_trigger_moving_average_of_delta_val,
        *ref_stock_first_order_quantity_val,
        *ref_stock_first_order_price_val,
        *best_stock_name_val2,
        *ref_stock_name_val2,
        *best_stock_second_order_quantity_val,
        *best_stock_second_order_price_val,
        *ref_stock_second_order_quantity_val,
        *ref_stock_second_order_price_val,
        *best_stock_name_val3,
        *ref_stock_name_val3,
        *best_stock_price_val,
        *ref_stock_price_val,
        *current_week_gross_return_val;


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_Trading_System_h_ */
