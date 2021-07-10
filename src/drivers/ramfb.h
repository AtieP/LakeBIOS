#ifndef __DRIVERS_RAMFB_H__
#define __DRIVERS_RAMFB_H__

int ramfb_detect();
void *ramfb_get_framebuffer();
void ramfb_set_resolution(int width, int height, int bpp);

#endif
