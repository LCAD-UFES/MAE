/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "mae_control.h"

FL_FORM *mae_control;

FL_OBJECT
        *main_box,
        *build_buttoon,
        *paste_button,
        *run_button,
        *log_browser,
        *speed_slider,
        *file,
        *copy_button,
        *timer,
        *train_button,
        *status_bar,
        *move_button,
        *draw_button,
        *undo_button,
        *mouse_bar,
        *steps_input,
        *clear_button,
        *rand_button,
        *command_input,
        *exec_button,
        *color_bar;

void create_form_mae_control(void)
{
  FL_OBJECT *obj;

  if (mae_control)
     return;

  mae_control = fl_bgn_form(FL_NO_BOX,490,290);
  main_box = obj = fl_add_box(FL_UP_BOX,0,0,490,290,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  build_buttoon = obj = fl_add_button(FL_NORMAL_BUTTON,80,10,50,20,"Build");
    fl_set_object_callback(obj,callback_build_button,0);
  paste_button = obj = fl_add_button(FL_NORMAL_BUTTON,220,10,50,20,"Paste");
    fl_set_object_callback(obj,callback_paste_button,0);
  run_button = obj = fl_add_button(FL_NORMAL_BUTTON,360,10,50,20,"Run");
    fl_set_object_callback(obj,callback_run_button,0);
  log_browser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,150,470,100,"");
  speed_slider = obj = fl_add_slider(FL_HOR_SLIDER,360,40,120,20,"speed");
    fl_set_object_lsize(obj,FL_DEFAULT_SIZE);
    fl_set_object_callback(obj,callback_speed_slider,0);
  file = obj = fl_add_menu(FL_PULLDOWN_MENU,10,10,50,20,"File");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_callback(obj,callback_file,0);
  copy_button = obj = fl_add_button(FL_NORMAL_BUTTON,150,10,50,20,"Copy");
    fl_set_object_callback(obj,callback_copy_button,0);
  timer = obj = fl_add_timer(FL_HIDDEN_TIMER,10,60,90,20,"Timer");
    fl_set_object_callback(obj,callback_timer,0);
  train_button = obj = fl_add_button(FL_NORMAL_BUTTON,290,10,50,20,"Train");
    fl_set_object_callback(obj,callback_train_button,0);
  status_bar = obj = fl_add_text(FL_NORMAL_TEXT,10,260,470,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  move_button = obj = fl_add_checkbutton(FL_PUSH_BUTTON,140,90,60,20,"Move");
    fl_set_object_callback(obj,callback_move_button,0);
  draw_button = obj = fl_add_checkbutton(FL_PUSH_BUTTON,210,90,60,20,"Draw");
    fl_set_object_callback(obj,callback_draw_button,0);
  undo_button = obj = fl_add_button(FL_NORMAL_BUTTON,290,40,50,20,"Undo");
    fl_set_object_callback(obj,callback_undo_button,0);
  mouse_bar = obj = fl_add_text(FL_NORMAL_TEXT,10,120,120,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  steps_input = obj = fl_add_input(FL_INT_INPUT,430,10,50,20,"st");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,callback_steps_input,0);
  clear_button = obj = fl_add_button(FL_NORMAL_BUTTON,150,40,50,20,"Clear");
    fl_set_object_callback(obj,callback_clear_button,0);
  rand_button = obj = fl_add_button(FL_NORMAL_BUTTON,220,40,50,20,"Rand");
    fl_set_object_callback(obj,callback_rand_button,0);
  command_input = obj = fl_add_input(FL_NORMAL_INPUT,10,70,260,20,"");
    fl_set_object_lsize(obj,FL_TINY_SIZE);
    fl_set_object_callback(obj,callback_command_input,0);
  exec_button = obj = fl_add_button(FL_NORMAL_BUTTON,290,70,50,20,"Exec.");
    fl_set_object_callback(obj,callback_exec_button,0);
  color_bar = obj = fl_add_text(FL_NORMAL_TEXT,140,120,340,20,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_mae_control();
}

