/*
Name: Nick Polach
Project Rocket Lander

Description:
My part of the project deals with the ship and the simple objects that
it interacts with (platforms/goal). In the main file, my ship object is
provided with other objects in the level and it checks if it collides
with those objects. My file also provides functions to draw UI items
that give info about the ship such as the fuel gauge.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctime>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "ppm.h"
#include "nicholasP.h"
#include "ramonR.h" // Temp

#define PI 3.14159265
int tempcount = 0;

extern double timeDiff(struct timespec *start, struct timespec *end);
extern void timeCopy(struct timespec *dest, struct timespec *source);
const double oobillion = 1.0 / 1e9;

Ppmimage * image;
GLuint texture;
GLuint silhouette;

timespec gStartTime, gCurTime;
timespec fStartTime, fCurTime;

// Object with basic properties of shape
Object::Object()
{
    pos[0] = 0;
    pos[1] = 0;
    shape.width = 0.0;
    shape.height = 0.0;
    shape.base = 0.0;
    shape.radius = 0.0;
}

Object::~Object()
{

}

// ---------- //

// Object with basic properties for movable shape
MovableObject::MovableObject()
{
    vel[0] = 0;
    vel[1] = 0;
}

// ---------- //

Ship::Ship()
{
    // Cockpit
    collidables[0].width = 26;
    collidables[0].height = 26;

    // Left wing
    //collidables[1].base = 40;
    collidables[1].base = 26;
    collidables[1].height = 26;

    // Right wing
    //collidables[2].base = 40;
    collidables[2].base = 26;
    collidables[2].height = 26;

    // Nose
    collidables[3].width = 20;
    collidables[3].height = 20;

    // Nose Cone
    collidables[4].base = 26;
    collidables[4].height = 20;

    // Overall ship size
    shape.width = collidables[0].width +
        collidables[1].base*.5 + collidables[2].base*.5;
    shape.height = collidables[0].height +
        collidables[3].height + collidables[4].height;

    fuel = 100;
    fuelMax = 100;


    watchGoal = false;
    watchFuel = false;
    exploded = false;

}

void Ship::move()
{
    pos[0] += vel[0];
    pos[1] += vel[1];

}

// Checks if goal was triggered
bool Ship::goalTriggered(Goal goal)
{
    // Timing has already started...
    if (gStartTime.tv_nsec > 0) {
        clock_gettime(CLOCK_REALTIME, &gCurTime);
    }
    // Start timing...
    else {
        clock_gettime(CLOCK_REALTIME, &gStartTime);
    }

    // Check to make sure the ship is upright and still
    if (collidesWith(goal.getTrigger()) &&
            vel[0] == 0 &&
            vel[1] == 0 &&
            rot == 0 ) {
        // Reset timers and start countdown to goal
        if (!watchGoal) {
            gStartTime.tv_sec = 0;
            gStartTime.tv_nsec = 0;
            gCurTime.tv_sec = 0;
            gCurTime.tv_nsec = 0;
            watchGoal = true;
            // After 2 second countdown trigger goal
        } else if (timeDiff(&gStartTime, &gCurTime) > 2.0) {
            return true;
        }
    } else {
        watchGoal = false;
    }
    return false;
}

// Checks if fueler was triggered
bool Ship::fuelerTriggered(Fueler fueler)
{
    // Timing has already started...
    if (fStartTime.tv_nsec > 0) {
        clock_gettime(CLOCK_REALTIME, &fCurTime);
    }
    // Start timing...
    else {
        clock_gettime(CLOCK_REALTIME, &fStartTime);
    }

    // Check to make sure the ship is upright and still
    if (collidesWith(fueler.getTrigger()) &&
            vel[0] == 0 &&
            vel[1] == 0 &&
            rot == 0 ) {
        // Reset timers and start countdown to refuel
        if (!watchFuel) {
            fStartTime.tv_sec = 0;
            fStartTime.tv_nsec = 0;
            fCurTime.tv_sec = 0;
            fCurTime.tv_nsec = 0;
            watchFuel = true;
            // After 1 second countdown start refueling
        } else if (timeDiff(&fStartTime, &fCurTime) > 1.0) {
            return true;
        }
    } else {
        watchFuel = false;
    }
    return false;
}

bool Ship::collidesWith(Object ob)
{
    // Use line-line collision testing for all shapes except cirlce
    if (triCollidesWith(collidables[1], ob,
                pos[0],
                pos[1]) || // left wing
            rectCollidesWith(collidables[0], ob,
                pos[0]+(collidables[1].base*.5),
                pos[1]) || //cockpit
            triCollidesWith(collidables[2], ob,
                pos[0]+collidables[1].base,
                pos[1])) { // right wing
        // Ship explodes if going to fast and hits object
        if (vel[0] < -1.5 || vel[0] > 1.5 || 
                vel[1] < -1.5 || vel[1] > 1.5) {
            exploded = true;
        }

        // Place ship above platform
        if (pos[1] < ob.getPosY() + ob.getHeight()) {
            pos[1] = ob.getPosY() + ob.getHeight();
            vel[0] = 0; // Temp
            vel[1] = 0;
            if (rot > 45)
                rot += 2;
            else if (rot < -45)
                rot -= 2;
            else if (rot > 0)
                rot -= 2;
            else if (rot < 0)
                rot += 2;

            // Prevents rot from going over float limit
            if (rot > 360)
                rot -= 360;
            else if (rot < -360)
                rot += 360;
        }
        return true;
        // Ship explodes if it nose cone or nose box are hit
    } else if (rectCollidesWith(collidables[3], ob,
                pos[0]+(collidables[1].base*.5)+3,
                pos[1]+collidables[0].height) || // nose box
            triCollidesWith(collidables[4], ob,
                pos[0]+(collidables[1].base*.5),
                pos[1]+collidables[0].height+collidables[3].height)) {
        exploded = true;
        return true;
    } else {
        return false;
    }
}

// Checks whether to given lines intersect
bool Ship::linesIntersect(Line l1, Line l2)
{
    if (l1.p1.x <= l2.p2.x && 
            l1.p2.x >= l2.p1.x &&
            l1.p1.y <= l2.p2.y &&
            l1.p2.y >= l2.p1.y) {
        return true;
    }
    return false;
}

// Generates array of points that create the square with the given parameters
Point * Ship::getRectPointArray(float x, float y,
        float rot, float width, float height)
{
    // p2 -> p3 
    // ^     |
    // |     V
    // p1 <- p4

    Point * pts = new Point[4];

    pts[0].x = x;
    pts[0].y = y;

    pts[1].x = x;
    pts[1].y = y+height;

    pts[2].x = x+width;
    pts[2].y = y+height;

    pts[3].x = x+width;
    pts[3].y = y;

    // Convert rotation from degrees to radians
    float rad = rot * (PI/180);

    // Pivot points
    float pivx = pos[0]+(shape.width*.5);
    float pivy = pos[1]+(shape.height*.5);

    // Rotate points
    float xnew;
    float ynew;
    for (int i=0; i<4; i++) {
        pts[i].x -= pivx;
        pts[i].y -= pivy;

        xnew = pts[i].x * cos(rad) - pts[i].y * sin(rad);
        ynew = pts[i].x * sin(rad) + pts[i].y * cos(rad);

        pts[i].x = xnew + pivx;
        pts[i].y = ynew + pivy;
    }

    return pts;
}


// Generates array of points that create the triangle with the given parameters
Point * Ship::getTriPointArray(float x, float y,
        float rot, float base, float height)
{
    //    p2  
    // ^     |
    // |     V
    // p1 <- p3

    Point * pts = new Point[3];

    pts[0].x = x;
    pts[0].y = y;

    pts[1].x = x+(base*.5);
    pts[1].y = y+height;

    pts[2].x = x+base;
    pts[2].y = y;

    // Convert rotation from degrees to radians
    float rad = rot * (PI/180);

    // Pivot points
    float pivx = pos[0]+(shape.width*.5);
    float pivy = pos[1]+(shape.height*.5);

    // Rotate points
    float xnew;
    float ynew;
    for (int i=0; i<3; i++) {
        pts[i].x -= pivx;
        pts[i].y -= pivy;

        xnew = pts[i].x * cos(rad) - pts[i].y * sin(rad);
        ynew = pts[i].x * sin(rad) + pts[i].y * cos(rad);

        pts[i].x = xnew + pivx;
        pts[i].y = ynew + pivy;
    }

    return pts;
}

// Checks if rectangular collidables of the ship collide with the given object
bool Ship::rectCollidesWith(Shape collidable, Object ob, float x, float y)
{
    bool collides = false;
    if (ob.getWidth() > 0) {			// rectangle
        Point * pts1 = getRectPointArray(x, y,
                rot, collidable.width, collidable.height);
        Point * pts2 = getRectPointArray(ob.getPosX(), ob.getPosY(),
                ob.getRot(), ob.getWidth(), ob.getHeight());
        Line l1;
        Line l2;
        // Check each line in ship against 4 lines in rectangle 
        for (int i=0; i<4; i++) {
            l1.p1 = pts1[i];
            l1.p2 = pts1[(i+1)%4];
            for (int i2=0; i2<4; i2++) {
                l2.p1 = pts2[i2];
                l2.p2 = pts2[(i2+1)%4];
                if (linesIntersect(l1, l2)) {
                    collides = true;
                    break;
                }
            }
        }
        delete [] pts1;
        delete [] pts2;
    }
    return collides;
}

// Checks if triangular collidables of the ship collide with the given object
bool Ship::triCollidesWith(Shape collidable, Object ob, float x, float y)
{
    bool collides = false;
    if (ob.getWidth() > 0) {			// rectangle
        Point * pts1 = getTriPointArray(x, y,
                rot, collidable.base, collidable.height);
        Point * pts2 = getRectPointArray(ob.getPosX(), ob.getPosY(),
                ob.getRot(), ob.getWidth(), ob.getHeight());
        Line l1;
        Line l2;
        // Check each line in ship against 3 lines in triangle
        for (int i=0; i<3; i++) {
            l1.p1 = pts1[i];
            l1.p2 = pts1[(i+1)%3];
            for (int i2=0; i2<4; i2++) {
                l2.p1 = pts2[i2];
                l2.p2 = pts2[(i2+1)%4];
                if (linesIntersect(l1, l2)) {
                    collides = true;
                    break;
                }
            }
        }
        delete [] pts1;
        delete [] pts2;
    } 
    return collides;
}

// Draw the collidable pieces of the ship to the screen
// Used mainly for debugging
void Ship::draw_debug()
{
    Point * pts;
    // The area for the ship image
    pts = getRectPointArray(pos[0], pos[1], rot, shape.width, shape.height);
    glColor3ub(111,111,111);
    glPushMatrix();
    glBegin(GL_QUADS);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);
    glVertex2i(pts[3].x, pts[3].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
    ////////////////////////
    // Visualization of the collision shapes
    //draw cockpit box
    pts = getRectPointArray(pos[0]+(collidables[1].base*.5), pos[1],
            rot, collidables[0].width, collidables[0].height);
    glColor3ub(255,0,0);
    glPushMatrix();
    glBegin(GL_QUADS);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);
    glVertex2i(pts[3].x, pts[3].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
    ////////////////////////
    // draw left wing
    pts = getTriPointArray(pos[0], pos[1],
            rot, collidables[1].base, collidables[1].height);
    glColor3ub(0,0,255);
    glPushMatrix();
    glBegin(GL_TRIANGLES);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
    ////////////////////////
    // draw right wing
    pts = getTriPointArray(pos[0]+collidables[1].base, pos[1],
            rot, collidables[2].base, collidables[2].height);
    glColor3ub(0,0,255);
    glPushMatrix();
    glBegin(GL_TRIANGLES);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
    ////////////////////////
    //draw nose box
    pts = getRectPointArray(pos[0]+(collidables[1].base*.5)+3,
            pos[1]+collidables[0].height,
            rot, collidables[3].width, collidables[3].height);
    glColor3ub(255,0,0);
    glPushMatrix();
    glBegin(GL_QUADS);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);
    glVertex2i(pts[3].x, pts[3].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
    ////////////////////////
    // draw nose cone
    pts = getTriPointArray(pos[0]+(collidables[1].base*.5),
            pos[1]+collidables[0].height+collidables[3].height,
            rot, collidables[4].base, collidables[4].height);
    glColor3ub(0,0,255);
    glPushMatrix();
    glBegin(GL_TRIANGLES);

    glVertex2i(pts[0].x, pts[0].y);
    glVertex2i(pts[1].x, pts[1].y);
    glVertex2i(pts[2].x, pts[2].y);

    glEnd();
    glPopMatrix();
    delete [] pts;
    pts = NULL;
}

void Ship::draw()
{
    Point * pts;
    pts = getRectPointArray(pos[0], pos[1], rot, shape.width, shape.height);
    // Ship image
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, silhouette);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f); //Alpha
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(pts[0].x, pts[0].y);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(pts[1].x, pts[1].y); 
    glTexCoord2f(1.0f, 0.0f); glVertex2i(pts[2].x, pts[2].y);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(pts[3].x, pts[3].y);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);

    delete [] pts;
    pts = NULL;
}

///// Swaps enabled booster /////
void Ship::enableBooster1()
{
    enabledBooster2 = false;
    enabledBooster3 = false;
    enabledBooster1 = true;
}

void Ship::enableBooster2()
{
    enabledBooster1 = false;
    enabledBooster3 = false;
    enabledBooster2 = true;
}

void Ship::enableBooster3()
{
    enabledBooster1 = false;
    enabledBooster2 = false;
    enabledBooster3 = true;
}
///////////////////////////////

// Changes velocity to move ship in direction that it's pointing
void Ship::accelerate()
{
    if (fuel > 0) {
        float rad = (rot+90.0) / 360.0f * PI * 2.0;
        float xdir = cos(rad);
        float ydir = sin(rad);
        if (enabledBooster1) {
            vel[0] += xdir * 0.02f;
            vel[1] += ydir * 0.02f;
        } else if (enabledBooster2) {
            vel[0] += xdir * 0.025f;
            vel[1] += ydir * 0.025f;
        } else if (enabledBooster3) {
            vel[0] += xdir * 0.03f;
            vel[1] += ydir * 0.03f;
        }
        fuel -= .2;
    } else {
        fuel = 0;
    }
}

// Add gravity to move ship to the bottom of the window
void Ship::addGravity(float grav)
{
    //printf("VelY: %f\n", vel[1]);
    vel[1] -= grav;
    // Prevent infinite downward acceleration
    if (vel[1] < -2.0)
        vel[1] = -2.0;

    // Prevent infinite upward acceleration
    if (vel[1] > 4.0)
        vel[1] = 4.0;
}

void Ship::rotateLeft()
{
    rot += 4.0;

    // Prevents rot from going over float limit
    if (rot > 360)
        rot -= 360;
}

void Ship::rotateRight()
{
    rot -= 4.0;
    // Prevents rot from going under float limit
    if (rot < -360)
        rot += 360;
}

void Ship::addFuel()
{
    fuel += 1;
}

void Ship::resetFuel()
{
    fuel = 100;
}

// ---------- //

// Basic platform
void Platform::draw()
{
    // Platform image
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(pos[0], pos[1], 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, shape.height); 
    glTexCoord2f(1.0f, 0.0f); glVertex2i(shape.width, shape.height);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Platform::draw_debug()
{
    //draw platform
    //glColor3ub(255,165,0);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glBegin(GL_QUADS);

    // Coords using (0,0) pos
    glVertex2i(0,0);
    glVertex2i(0, shape.height);
    glVertex2i(shape.width, shape.height);
    glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
}

// ---------- //

// Platform with goal trigger
void Goal::setPosX(float x)
{
    pos[0]=x;
    trigger.setPosX(x);
}

void Goal::setPosY(float y)
{
    pos[1]=y;
    trigger.setPosY(y);
}

void Goal::setWidth(float w)
{
    shape.width=w;
    trigger.setWidth(w);
}

void Goal::setHeight(float h)
{
    shape.height=h;
    trigger.setHeight(h+5);
}

void Goal::draw()
{
    // Platform image
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(pos[0], pos[1], 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, shape.height+5); 
    glTexCoord2f(1.0f, 0.0f); glVertex2i(shape.width, shape.height+5);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Goal::draw_debug()
{
    //draw trigger 
    glColor3ub(255,111,111);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glBegin(GL_QUADS);

    // Coords using (0,0) pos
    glVertex2i(0,0);
    glVertex2i(0, trigger.getHeight());
    glVertex2i(trigger.getWidth(), trigger.getHeight());
    glVertex2i(trigger.getWidth(), 0);

    glEnd();
    glPopMatrix();

    //draw platform
    glColor3ub(111,111,111);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glBegin(GL_QUADS);

    // Coords using (0,0) pos
    glVertex2i(0,0);
    glVertex2i(0, shape.height);
    glVertex2i(shape.width, shape.height);
    glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
}

// ---------- //

Fueler::Fueler()
{
    fuel = 100;
}

void Fueler::setPosX(float x)
{
    pos[0]=x;
    trigger.setPosX(x);
}

void Fueler::setPosY(float y)
{
    pos[1]=y;
    trigger.setPosY(y);
}

void Fueler::setWidth(float w)
{
    shape.width=w;
    trigger.setWidth(w);
}

void Fueler::setHeight(float h)
{
    shape.height=h;
    trigger.setHeight(h+5);
}

void Fueler::removeFuel()
{
    fuel -= 1;
}

void Fueler::draw()
{
    // Fueler image
    glColor4f(1.0, 1.0, 1.0, 1.0); // reset gl color
    glPushMatrix();
    glTranslated(pos[0], pos[1], 0);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D, silhouette);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.0f); //Alpha
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex2i(0, 0);
    glTexCoord2f(0.0f, 0.0f); glVertex2i(0, shape.height+5); 
    glTexCoord2f(1.0f, 0.0f); glVertex2i(shape.width, shape.height+5);
    glTexCoord2f(1.0f, 1.0f); glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_ALPHA_TEST);
}

void Fueler::draw_debug()
{
    //draw trigger
    glColor3ub(255,255,255);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glBegin(GL_QUADS);

    // Coords using (0,0) pos
    glVertex2i(0,0);
    glVertex2i(0, trigger.getHeight());
    glVertex2i(trigger.getWidth(), trigger.getHeight());
    glVertex2i(trigger.getWidth(), 0);

    glEnd();
    glPopMatrix();

    //draw platform
    glColor3ub(111,111,111);
    glPushMatrix();
    glTranslatef(pos[0], pos[1], 0);
    glBegin(GL_QUADS);

    // Coords using (0,0) pos
    glVertex2i(0,0);
    glVertex2i(0, shape.height);
    glVertex2i(shape.width, shape.height);
    glVertex2i(shape.width, 0);

    glEnd();
    glPopMatrix();
}

// ---------- //

void drawFuelGauge(float fuelLeft, float fuelMax, float x, float y)
{
    float width = 155;
    float height = 25;
    float fuelWidth = ((fuelLeft)/fuelMax)*width-8;
    //outer outline platform
    glColor3ub(255,0,0);
    glPushMatrix();
    glTranslatef(x-(width*.5), y, 0);
    glBegin(GL_QUADS);

    glVertex2i(0,0);
    glVertex2i(0, height);
    glVertex2i(width, height);
    glVertex2i(width, 0);

    glEnd();
    glPopMatrix();

    // inner outline
    glColor3ub(0,0,0);
    glPushMatrix();
    glTranslatef(x-(width*.5)+2, y+2, 0);
    glBegin(GL_QUADS);

    glVertex2i(0,0);
    glVertex2i(0, height-4);
    glVertex2i(width-4, height-4);
    glVertex2i(width-4, 0);

    glEnd();
    glPopMatrix();

    if (fuelWidth > 0) {
        // fuel gauge
        glColor3ub(0,255,0);
        glPushMatrix();
        glTranslatef(x-(width*.5)+4, y+4, 0);
        glBegin(GL_QUADS);

        glVertex2i(0,0);
        glVertex2i(0, height-8);
        glVertex2i(fuelWidth, height-8);
        glVertex2i(fuelWidth, 0);

        glEnd();
        glPopMatrix();
    }
}

void init_image(char * imagePath, Ppmimage * image, GLuint * texture)
{
    image = ppm6GetImage(imagePath);
    glGenTextures(1, texture);

    // Image
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
            image->width, image->height,
            0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
}

void init_alpha_image(char * imagePath, Ppmimage * image,
        GLuint * texture, GLuint * silhouette)
{
    image = ppm6GetImage(imagePath);
    glGenTextures(1, silhouette);
    glGenTextures(1, texture);

    // Image
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
            image->width, image->height,
            0, GL_RGB, GL_UNSIGNED_BYTE, image->data);

    // Alpha
    glBindTexture(GL_TEXTURE_2D,*silhouette);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    unsigned char *silhouetteData = buildAlphaData(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            image->width, image->height,
            0, GL_RGBA, GL_UNSIGNED_BYTE, silhouetteData);
    free(silhouetteData);
}
