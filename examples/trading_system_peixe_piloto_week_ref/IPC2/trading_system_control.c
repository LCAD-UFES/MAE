/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "trading_system_control.h"

FL_FORM *Trading_System;

FL_OBJECT
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

void create_form_Trading_System(void)
{
  FL_OBJECT *obj;

  if (Trading_System)
     return;

  Trading_System = fl_bgn_form(FL_NO_BOX,650,470);
  obj = fl_add_box(FL_UP_BOX,0,0,650,470,"");
  obj = fl_add_text(FL_NORMAL_TEXT,20,40,180,20,"best_stock_name");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  best_stock_name_val = obj = fl_add_text(FL_NORMAL_TEXT,210,40,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,70,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,70,180,20,"price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,180,20,"current_state");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  state_val = obj = fl_add_text(FL_NORMAL_TEXT,210,10,200,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  moving_average_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,100,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,100,180,20,"moving_average_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,430,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,430,180,20,"order_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  quantity_val = obj = fl_add_text(FL_NORMAL_TEXT,320,430,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,430,150,20,"quantity");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  maximum_buy_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,150,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,150,180,20,"maximum_buy_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  previous_minimum_intraday_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,180,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,180,180,20,"previous_minimum_intraday_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  possible_buy_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,310,210,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,210,280,20,"The system send a buy order if moving_average_price > ");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  minimum_sell_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,290,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,290,180,20,"minimum_sell_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  previous_maximum_intraday_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,320,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,320,180,20,"previous_maximum_intraday_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  possible_sell_order_price_val = obj = fl_add_text(FL_NORMAL_TEXT,310,350,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,350,280,20,"The system send a sell order if moving_average_price < ");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  buy_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,240,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,240,180,20,"buy_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  sell_price_val = obj = fl_add_text(FL_NORMAL_TEXT,210,380,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,380,180,20,"sell_price");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,40,180,20,"predicted_return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  predicted_return_val = obj = fl_add_text(FL_NORMAL_TEXT,320,40,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,70,180,20,"current_return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  current_return_val = obj = fl_add_text(FL_NORMAL_TEXT,320,70,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,420,380,180,20,"final_current_return");
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  final_return_val = obj = fl_add_text(FL_NORMAL_TEXT,320,380,90,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  date_and_time_val = obj = fl_add_text(FL_NORMAL_TEXT,420,10,180,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_box(FL_FRAME_BOX,420,100,180,130,"");
  buy_button = obj = fl_add_button(FL_NORMAL_BUTTON,480,140,100,20,"Buy");
    fl_set_object_callback(obj,buy_button_callback,0);
  order_buy_price_val = obj = fl_add_input(FL_NORMAL_INPUT,480,170,100,20,"Price");
    fl_set_object_callback(obj,order_buy_price_callback,0);
  order_buy_quantity_val = obj = fl_add_input(FL_NORMAL_INPUT,480,200,100,20,"Quantity");
    fl_set_object_callback(obj,order_buy_quantity_callback,0);
  fast_buy_button = obj = fl_add_button(FL_NORMAL_BUTTON,480,110,100,20,"Fast Buy");
    fl_set_object_callback(obj,fast_buy_button_callback,0);
  obj = fl_add_box(FL_FRAME_BOX,420,240,180,130,"");
  sell_button = obj = fl_add_button(FL_NORMAL_BUTTON,480,280,100,20,"Sell");
    fl_set_object_callback(obj,sell_button_callback,0);
  order_sell_price_val = obj = fl_add_input(FL_NORMAL_INPUT,480,310,100,20,"Price");
    fl_set_object_callback(obj,order_sell_price_callback,0);
  order_sell_quantity_val = obj = fl_add_input(FL_NORMAL_INPUT,480,340,100,20,"Quantity");
    fl_set_object_callback(obj,order_sell_quantity_callback,0);
  fast_sell_button = obj = fl_add_button(FL_NORMAL_BUTTON,480,250,100,20,"Fast Sell");
    fl_set_object_callback(obj,fast_sell_button_callback,0);
  plot_graph_button = obj = fl_add_button(FL_NORMAL_BUTTON,580,430,60,20,"Plot Graph");
    fl_set_object_callback(obj,plot_graph_button_callback,0);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_Trading_System();
}

