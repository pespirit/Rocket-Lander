#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "ppm.h"
unsigned char *buildAlphaData(Ppmimage*);

void drawRamRMenu(int xres, int yres, Rect r);
extern Ppmimage *hitterImage;
extern GLuint silhousetteTexture;
extern GLuint hitterTexture;
void inHitters();
void renderAstro();

