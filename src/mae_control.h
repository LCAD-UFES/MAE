/** Header file generated with fdesign on Thu Feb  1 10:14:07 2007.**/

#ifndef FD_mae_control_h_
#define FD_mae_control_h_

/** Callbacks, globals and object handlers **/
extern void callback_build_button(FL_OBJECT *, long);
extern void callback_paste_button(FL_OBJECT *, long);
extern void callback_run_button(FL_OBJECT *, long);
extern void callback_speed_slider(FL_OBJECT *, long);
extern void callback_file(FL_OBJECT *, long);
extern void callback_copy_button(FL_OBJECT *, long);
extern void callback_timer(FL_OBJECT *, long);
extern void callback_train_button(FL_OBJECT *, long);
extern void callback_move_button(FL_OBJECT *, long);
extern void callback_draw_button(FL_OBJECT *, long);
extern void callback_undo_button(FL_OBJECT *, long);
extern void callback_steps_input(FL_OBJECT *, long);
extern void callback_clear_button(FL_OBJECT *, long);
extern void callback_rand_button(FL_OBJECT *, long);
extern void callback_command_input(FL_OBJECT *, long);
extern void callback_exec_button(FL_OBJECT *, long);


/**** Forms and Objects ****/
extern FL_FORM *mae_control;

extern FL_OBJECT
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


/**** Creation Routine ****/
extern void create_the_forms(void);

#endif /* FD_mae_control_h_ */
