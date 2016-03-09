/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "trading_system_control.h"

FL_FORM *Trading_System;

FL_OBJECT
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

void create_form_Trading_System(void)
{
  FL_OBJECT *obj;

  if (Trading_System)
     return;

  Trading_System = fl_bgn_form(FL_NO_BOX,630,600);
  obj = fl_add_box(FL_UP_BOX,1,0,630,600,"");
  obj = fl_add_text(FL_NORMAL_TEXT,20,110,180,20,"best_stock_name");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_name_val = obj = fl_add_text(FL_NORMAL_TEXT,210,110,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  ref_stock_name_val = obj = fl_add_text(FL_NORMAL_TEXT,210,140,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,140,180,20,"ref_stock_name");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,180,20,"current_state");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  state_val = obj = fl_add_text(FL_NORMAL_TEXT,210,10,200,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  moving_average_of_delta_val = obj = fl_add_text(FL_NORMAL_TEXT,320,180,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,180,280,20,"moving_average_of_delta");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  first_orders_trigger_moving_average_of_delta_val = obj = fl_add_text(FL_NORMAL_TEXT,320,220,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,220,280,20,"Trigger first_orders if moving_average_of_delta > ");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_first_order_quantity_val = obj = fl_add_text(FL_NORMAL_TEXT,320,280,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,280,180,20,"best_stock_first_order_quantity");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_first_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,250,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,250,180,20,"best_stock_first_order_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,40,180,20,"predicted week gross return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  predicted_week_return_val = obj = fl_add_text(FL_NORMAL_TEXT,210,40,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,70,180,20,"current_week_return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  current_week_return_val = obj = fl_add_text(FL_NORMAL_TEXT,320,70,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,560,180,20,"final_week_current_return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  final_week_return_val = obj = fl_add_text(FL_NORMAL_TEXT,210,560,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  date_and_time_val = obj = fl_add_text(FL_NORMAL_TEXT,420,10,180,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_box(FL_FRAME_BOX,420,220,180,310,"");
  send_first_orders_button = obj = fl_add_button(FL_NORMAL_BUTTON,440,280,140,20,"Send First Orders!");
    fl_set_object_callback(obj,send_first_orders_button_callback,0);
  plot_graph_button = obj = fl_add_button(FL_NORMAL_BUTTON,520,560,70,20,"Plot Graph");
    fl_set_object_callback(obj,plot_graph_button_callback,0);
  send_second_orders_button = obj = fl_add_button(FL_NORMAL_BUTTON,440,450,140,20,"Send Second Orders!");
    fl_set_object_callback(obj,send_second_orders_button_callback,0);
  obj = fl_add_text(FL_NORMAL_TEXT,20,70,180,20,"trade_order");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  trade_order_val = obj = fl_add_text(FL_NORMAL_TEXT,210,70,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  second_orders_trigger_moving_average_of_delta_val = obj = fl_add_text(FL_NORMAL_TEXT,320,390,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,390,280,20,"Trigger second_orders if moving_average_of_delta < ");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ref_stock_first_order_quantity_val = obj = fl_add_text(FL_NORMAL_TEXT,320,340,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,340,180,20,"ref_stock_first_order_quantity");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ref_stock_first_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,310,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,310,180,20,"ref_stock_first_order_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_name_val2 = obj = fl_add_text(FL_NORMAL_TEXT,20,250,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  ref_stock_name_val2 = obj = fl_add_text(FL_NORMAL_TEXT,20,310,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  best_stock_second_order_quantity_val = obj = fl_add_text(FL_NORMAL_TEXT,320,450,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,450,180,20,"best_stock_second_order_quantity");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_second_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,420,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,420,180,20,"best_stock_second_order_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ref_stock_second_order_quantity_val = obj = fl_add_text(FL_NORMAL_TEXT,320,510,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,510,180,20,"ref_stock_second_order_quantity");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  ref_stock_second_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,480,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,120,480,180,20,"ref_stock_second_order_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_name_val3 = obj = fl_add_text(FL_NORMAL_TEXT,20,420,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  ref_stock_name_val3 = obj = fl_add_text(FL_NORMAL_TEXT,20,480,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  best_stock_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,110,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  ref_stock_price_val = obj = fl_add_text(FL_NORMAL_TEXT,320,140,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,110,180,20,"best_stock_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,140,180,20,"ref_stock_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,40,180,20,"current week gross return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  current_week_gross_return_val = obj = fl_add_text(FL_NORMAL_TEXT,320,40,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_Trading_System();
}

