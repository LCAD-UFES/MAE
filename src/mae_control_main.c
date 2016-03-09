#include "forms.h"
#include "mae_control.h"

int main(int argc, char *argv[])
{

   fl_initialize(&argc, argv, 0, 0, 0);

   create_the_forms();

   /* fill-in form initialization code */

   /* show the first form */
   fl_show_form(mae_control,FL_PLACE_CENTER,FL_FULLBORDER,"mae_control");
   fl_do_forms();
   return 0;
}
