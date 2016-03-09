#include "forms.h"
#include "trading_system_control.h"

int main(int argc, char *argv[])
{

   fl_initialize(&argc, argv, 0, 0, 0);

   create_the_forms();

   /* fill-in form initialization code */

   /* show the first form */
   fl_show_form(Trading_System,FL_PLACE_CENTER,FL_FULLBORDER,"Trading_System");
   fl_do_forms();
   return 0;
}
