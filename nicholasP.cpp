#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "log.h"
#include "fonts.h"
#include "nicholasP.h"

Object::Object()
{
        pos[0] = 0;
        pos[1] = 0;
        shape.width = 0.0;
        shape.height = 0.0;
        shape.base = 0.0;
        shape.radius = 0.0;
        shape.center[0] = 0.0;
        shape.center[1] = 0.0;
}

Object::~Object()
{

}

// ---------- //

MovableObject::MovableObject()
{
        vel[0] = 0;
        vel[1] = 0;
}

// ---------- //

Ship2::Ship2()
{
        // Cockpit
        collidables[0].width = 25;
        collidables[0].height = 25;

        // Left wing
        collidables[1].width = 20;
        collidables[1].height = 20;

        // Right wing
        collidables[2].width = 20;
        collidables[2].height = 20;

        // Nose
        collidables[3].width = 20;
        collidables[3].height = 20;

        // Nose Cone
        collidables[4].base = 20;
        collidables[4].height = 20;

        shape.width = collidables[0].width + collidables[1].width + collidables[2].width;
        shape.height = collidables[0].height + collidables[3].height + collidables[4].height;
        // Figure out how to set positions

        fuel = 100;
}

void Ship2::move()
{
        //printf("The ship is moving!\n");
}

bool Ship2::collidesWith(Object ob)
{
        if (ob.getWidth() > 0) {			// rectangle
                // Rectangle collide with rectangle
                if (pos[0] < ob.getPosX() + ob.getWidth()	&&
                                pos[0] + shape.width > ob.getPosX()		&&
                                pos[1] < ob.getPosY() + ob.getHeight()	&&
                                shape.height + pos[1] > ob.getPosY()) {

                        printf("Colliding with rectangle!\n");
                }
        } else if (ob.getHeight() > 0) { 		// triangle
                printf("Colliding with triangle!\n");
        } else if (ob.getRadius() > 0) { 		// circle
                printf("Colliding with circle!\n");
        } else {
                printf("The object has no shape\n");
        }

        return true;
}

void Ship2::draw()
{
        //draw ship box
        glColor3ub(0,0,255);
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

        //printf("Drawing the ship!\n");
}

void Ship2::drawRect()
{
        //draw platform
        glColor3ub(255,165,0);
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

void Ship2::drawTri()
        //void Ship2::drawTri(Shape s)
{
        glColor3ub(255,0,0);
        glPushMatrix();
        glTranslatef(200, 200, 0);
        glBegin(GL_TRIANGLES);

        glVertex2i(0,0);
        glVertex2i(50, 0);
        glVertex2i(25, 50);

        //    glVertex2i(0,0);
        //    glVertex2i(shape.base, 0);
        //    glVertex2i(shape.base\2, shape.height);

        glEnd();
        glPopMatrix();
}

void Ship2::enableBooster1()
{
        enabledBooster2 = false;
        enabledBooster3 = false;
        enabledBooster1 = true;
}

void Ship2::enableBooster2()
{
        enabledBooster1 = false;
        enabledBooster3 = false;
        enabledBooster2 = true;
}

void Ship2::enableBooster3()
{
        enabledBooster1 = false;
        enabledBooster2 = false;
        enabledBooster3 = true;
}

// ---------- //

Enemy::Enemy()
{
        health = 100;
}

void Enemy::move() {
        printf("The enemy is moving!\n");
}

void Enemy::draw() {
        printf("Drawing the enemy!\n");
}

// ---------- //

//Chaser::Chaser(Ship * s)
//{
//    ship = s;
//}

void Chaser::move() {
        printf("The chaser is moving!\n");
}

void Chaser::draw() {
        printf("Drawing the chaser!\n");
}

// ---------- //

//Circler::Circler(Ship * s)
//{
//    ship = s;
//}

void Circler::move() 
{
        printf("The circler is moving!\n");
}

void Circler::draw() 
{
        printf("Drawing the circler!\n");
}

// ---------- //

void Asteroid2::move()
{
        printf("The asteroid is moving!\n");
}

void Asteroid2::draw()
{
        printf("Drawing the asteroid!\n");
}

// ---------- //

void Platform::draw()
{
        //draw platform
        glColor3ub(255,165,0);
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

void drawNicholasMenu(int xres, int yres, Rect r)
{   

        // Test Rectangle collision
        Platform plat;
        plat.setPosX(100);
        plat.setPosY(100);
        plat.setWidth(50);
        plat.setHeight(10);
        plat.draw();
        //
        //    // Test circle collision
        //    Platform plat2;
        //    plat2.setHeight(10);
        //
        //    // Test circle collision
        //    Platform plat3;
        //    plat3.setRadius(10);

        Ship2 ship;
        ship.move();
        ship.collidesWith(plat);
        ship.draw();
        ship.drawTri();

        //Ship ship;
        //Enemy enemy(&ship);
        //enemy.move();
        //enemy.draw();

        //Ship ship;
        //Chaser e1(&ship);
        //e1.move();
        //e1.draw();

        //Ship ship;
        //Circler e2(&ship);
        //e2.move();
        //e2.draw();

        //Asteroid ast;
        //ast.move();
        //ast.draw();

        glColor3f(1.0, 0.0, 0.0);
        int cx = xres/2;
        int cy = yres/2;
        glBegin(GL_QUADS);
        glVertex2f(cx-100, cy+100);
        glVertex2f(cx+100, cy+100);
        glVertex2f(cx+100, cy-100);
        glVertex2f(cx-100, cy-100);
        glEnd();
        glEnable(GL_TEXTURE_2D);
        r.bot = cy + 20;
        r.left = cx;
        r.center = 1;
        ggprint8b(&r, 16, 0x00ffff00, "This is Nick's menu");
        ggprint8b(&r, 16, 0x00ffff00, "Debugging!");
}
