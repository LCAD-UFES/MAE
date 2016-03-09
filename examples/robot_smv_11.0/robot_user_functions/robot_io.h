#ifndef __ROBOT_IO_H
#define __ROBOT_IO_H

#ifdef __cplusplus
extern "C" {
#endif

int load_camera_pair (char *name, int camera_pair, char *format);

int load_current_camera_pair (char *name, char *format);

int get_current_camera_pair (void);

#ifdef __cplusplus
}
#endif

#endif
