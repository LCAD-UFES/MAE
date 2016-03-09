#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_SUBJECTS 136
#define NUM_MEN  76
#define NUM_WOMEN  60

#define SECTION_SIZE 13

#define AR_FACE_DATABASE_DIR "/home/hallysson/img/ARDataBase/ARPhotoDataBase/"

#define NUM_SUBJECTS_TR_VA 50

#define FACES_TR	"faces_tr.txt"
#define FACES_VA	"faces_va.txt"

#define FACES_TV "faces_tv.txt"
#define FACES_TE_SMILE "faces_te_smile.txt"
#define FACES_TE_ANGER "faces_te_anger.txt"
#define FACES_TE_SCREAM "faces_te_scream.txt"
#define FACES_TE_LEFT_LIGHT "faces_te_left_light.txt"
#define FACES_TE_RIGHT_LIGHT "faces_te_right_light.txt"
#define FACES_TE_ALL_SIDE_LIGHTS "faces_te_all_side_lights.txt"
#define FACES_TE_GLASSES "faces_te_glasses.txt"
#define FACES_TE_SCARF "faces_te_scarf.txt"

#define NEUTRAL 1
#define SMILE 2
#define ANGER 3
#define SCREAM 4
#define LEFT_LIGHT 5
#define RIGHT_LIGHT 6
#define ALL_SIDE_LIGHTS 7
#define GLASSES 8
#define SCARF 11

int images_men[NUM_MEN+1];
int images_women[NUM_WOMEN+1];

void 	
faces_tv()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;
	
  sprintf(file_name,"%s",FACES_TV);
	
  file = fopen(file_name,"w");
	
  for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, NEUTRAL);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, NEUTRAL+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, NEUTRAL);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, NEUTRAL+SECTION_SIZE);
	}
    }
  
  fclose(file);	
}

void 
faces_te_smile()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;
	
  sprintf(file_name,"%s",FACES_TE_SMILE);
	
  file = fopen(file_name,"w");

  for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SMILE);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SMILE+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SMILE);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SMILE+SECTION_SIZE);
	}
    }

  fclose(file);	
}

void 
faces_te_anger()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_ANGER);

  file = fopen(file_name,"w");

   for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, ANGER);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, ANGER+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, ANGER);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, ANGER+SECTION_SIZE);
	}
    }

  fclose(file);	
}

void 
faces_te_scream()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_SCREAM);

  file = fopen(file_name,"w");

 for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SCREAM);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SCREAM+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SCREAM);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SCREAM+SECTION_SIZE);
	}
    }
	
  fclose(file);
}

void 
faces_te_left_light()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_LEFT_LIGHT);

  file = fopen(file_name,"w");

   for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, LEFT_LIGHT);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, LEFT_LIGHT+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, LEFT_LIGHT);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, LEFT_LIGHT+SECTION_SIZE);
	}
    }

  fclose(file);	
}

void 
faces_te_right_light()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_RIGHT_LIGHT);

  file = fopen(file_name,"w");

  for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, RIGHT_LIGHT);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, RIGHT_LIGHT+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, RIGHT_LIGHT);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, RIGHT_LIGHT+SECTION_SIZE);
	}
    }

  fclose(file);	
}	

void 
faces_te_all_side_lights()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_ALL_SIDE_LIGHTS);

  file = fopen(file_name,"w");

  for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, ALL_SIDE_LIGHTS);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, ALL_SIDE_LIGHTS+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, ALL_SIDE_LIGHTS);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, ALL_SIDE_LIGHTS+SECTION_SIZE);
	}
    }

  fclose(file);	
}

void 
faces_te_glasses()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_GLASSES);

  file = fopen(file_name,"w");

  for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, GLASSES);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, GLASSES+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, GLASSES);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, GLASSES+SECTION_SIZE);
	}
    }
  
  fclose(file);	
}	

void 
faces_te_scarf()
{
  int exist;
  FILE *file;
  char file_name[256];
  int i;

  sprintf(file_name,"%s",FACES_TE_SCARF);

  file = fopen(file_name,"w");

 for (i=0;i<NUM_MEN;i++)
    {
      if (images_men[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SCARF);
	  fprintf(file,"%i %s %i %i\n", i+1, "m", i+1, SCARF+SECTION_SIZE);
	}
    }
	
  for (i=0;i<NUM_WOMEN;i++)
    {
      if (images_women[i+1])
	{
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SCARF);
	  fprintf(file,"%i %s %i %i\n", NUM_MEN+i+1, "w", i+1, SCARF+SECTION_SIZE);
	}
    }

  fclose(file);	
}

int
is_repeated(int person_id, int *subjects, int num_subjects)
{
  int j;
  int repeated;

  repeated=0;
  for (j=0;j<num_subjects;j++)
    {
      if (person_id == subjects[j])
	{
	  repeated = 1;
	}
    }

  return(repeated);
}

void
print_faces_tr_va(int *subjects)
{
  FILE *file_tr;
  char file_tr_name[256];
  FILE *file_va;
  char file_va_name[256];
  int i;
  int man_id;
  int woman_id;

  sprintf(file_tr_name,"%s",FACES_TR);
  sprintf(file_va_name,"%s",FACES_VA);
  file_tr = fopen(file_tr_name,"w");
  file_va = fopen(file_va_name,"w");

  for (i=0;i<NUM_SUBJECTS_TR_VA;i++)
    {
      if (subjects[i] <= NUM_MEN)
	{	
	  man_id =  subjects[i];

	  fprintf(file_tr,"%i %s %i %i\n", i+1, "m", man_id, 1);
	  fprintf(file_tr,"%i %s %i %i\n", i+1, "m", man_id, 14);

	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 2);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 3);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 8);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 11);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 15);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 16);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 21);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "m", man_id, 24);		
	}
      else
	{
	  woman_id = subjects[i]-NUM_MEN;

	  fprintf(file_tr,"%i %s %i %i\n", i+1, "w", woman_id, 1);
	  fprintf(file_tr,"%i %s %i %i\n", i+1, "w", woman_id, 14);

	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 2);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 3);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 8);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 11);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 15);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 16);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 21);
	  fprintf(file_va,"%i %s %i %i\n", i+1, "w", woman_id, 24);		
	}
    }
	
  fclose(file_tr);
  fclose(file_va);
}

int
does_exist(int ar_person_id, char *gender, int expression)
{	
  char command[256];
  int exist;
  int man_id;
  int woman_id;
   
  exist=0;
  if (ar_person_id<=9)
    {
      sprintf(command, "ls %s%s-00%i-%i.txt",AR_FACE_DATABASE_DIR,gender,ar_person_id,expression);
    }
  else
    {
      sprintf(command, "ls %s%s-0%i-%i.txt",AR_FACE_DATABASE_DIR,gender,ar_person_id,expression);
    }
  if (system(command)==0)
    {
	
      exist=1;
    }
   
  return(exist);
}

void 
images()
{
  int i;
  int j;
  int man_id;
  int woman_id;

  for (i=0;i<NUM_MEN;i++)
    {
      man_id = i+1;
      if (does_exist(man_id,"m",1)&&does_exist(man_id,"m",2)&&does_exist(man_id,"m",3)&&does_exist(man_id,"m",4)&&does_exist(man_id,"m",5)&&does_exist(man_id,"m",6)&&does_exist(man_id,"m",7)&&does_exist(man_id,"m",8)&&does_exist(man_id,"m",11)&&does_exist(man_id,"m",14)&&does_exist(man_id,"m",15)&&does_exist(man_id,"m",16)&&does_exist(man_id,"m",17)&&does_exist(man_id,"m",18)&&does_exist(man_id,"m",19)&&does_exist(man_id,"m",20)&&does_exist(man_id,"m",21)&&does_exist(man_id,"m",24))
	{
	  images_men[man_id]=1;
	}
    }	  

  for (i=0;i<NUM_WOMEN;i++)
    { 
      woman_id=i+1;
      if (does_exist(woman_id,"w",1)&&does_exist(woman_id,"w",2)&&does_exist(woman_id,"w",3)&&does_exist(woman_id,"w",4)&&does_exist(woman_id,"w",5)&&does_exist(woman_id,"w",6)&&does_exist(woman_id,"w",7)&&does_exist(woman_id,"w",8)&&does_exist(woman_id,"w",11)&&does_exist(woman_id,"w",14)&&does_exist(woman_id,"w",15)&&does_exist(woman_id,"w",16)&&does_exist(woman_id,"w",17)&&does_exist(woman_id,"w",18)&&does_exist(woman_id,"w",19)&&does_exist(woman_id,"w",20)&&does_exist(woman_id,"w",21)&&does_exist(woman_id,"w",24))
	{
	  images_women[woman_id]=1;
	}
    }
}

void 
faces_tr_va()
{
  int exist;
  int i;
  int man_id;
  int person_id;
  int repeated;
  int subjects[NUM_SUBJECTS_TR_VA];
  int woman_id;

  //time_t srand(time(&t));

  for (i=0;i<NUM_SUBJECTS_TR_VA;i++)
    {			
      person_id=0;
      repeated=1;
      exist=0;
      while ((person_id == 0) || (repeated==1) || (exist==0) )
	{
	  person_id = rand()%(NUM_SUBJECTS + 1);

	  repeated=is_repeated(person_id, subjects, i);
		
	  if (person_id<= NUM_MEN)
	    {
	      exist = images_men[person_id];
	    }
	  else
	    {
	      exist = images_women[person_id-NUM_MEN];
	    } 
	}
      subjects[i] = person_id;
    }	
  print_faces_tr_va(subjects);
}

int 
main ()
{
  int i;

  for (i=0;i<NUM_MEN;i++)
    {
      images_men[i+1]=0;
    }
  for (i=0;i<NUM_WOMEN;i++)
    {
      images_women[i+1]=0;
    }

  images();

  faces_tr_va();

  faces_tv();
  faces_te_smile();	
  faces_te_anger();	
  faces_te_scream();
  faces_te_left_light();
  faces_te_right_light();	
  faces_te_all_side_lights();	
  faces_te_glasses();	
  faces_te_scarf();

  return(0);
}
