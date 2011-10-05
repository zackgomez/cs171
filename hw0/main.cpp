#include <iostream>

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"

using namespace std;

/* 
 * A quadric can be a cylinder, disk, sphere, cone, etc.
 * We just reuse this memory space each time we draw any sort of quad.
 * GLU supplies this funcionality.
 */
GLUquadricObj* quad;

/** PROTOTYPES **/
void initLights();
void initMaterial();
void redraw();
void initGL();
void resize(GLint w, GLint h);
void keyfunc(GLubyte key, GLint x, GLint y);

/** GLUT callback functions **/

/*
 * This function gets called every time the window needs to be updated
 * i.e. after being hidden by another window and brought back into view,
 * or when the window's been resized.
 * You should never call this directly, but use glutPostRedisply() to tell
 * GLUT to do it instead.
 */
void redraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gluSphere(quad, 2.0, 256, 256);

    glutSwapBuffers();
}

/**
 * GLUT calls this function when the window is resized.
 * All we do here is change the OpenGL viewport so it will always draw in the
 * largest square that can fit in our window..
 */
void resize(GLint w, GLint h)
{
    if (h == 0)
        h = 1;

    // ensure that we are always square (even if whole window not used)
    if (w > h)
        w = h;
    else
        h = w;

    // Reset the current viewport and perspective transformation
    glViewport(0, 0, w, h);

    // Tell GLUT to call redraw()
    glutPostRedisplay();
}

/*
 * GLUT calls this function when any key is pressed while our window has
 * focus.  Here, we just quit if any appropriate key is pressed.  You can
 * do a lot more cool stuff with this here.
 */
void keyfunc(GLubyte key, GLint x, GLint y)
{
    // escape or q or Q
    if (key == 27 || key == 'q' || key =='Q')
        exit(0);
}

/** Utility functions **/

/**
 * Sets up an OpenGL light.  This only needs to be called once
 * and the light will be used during all renders.
 */
void initLights() {
    GLfloat amb[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat diff[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat spec[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightpos[]= { 2.0f, 2.0f, 5.0f, 1.0f };
    GLfloat shiny = 4.0f; 

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightf(GL_LIGHT0, GL_SHININESS, shiny);
    glEnable(GL_LIGHT0);

    // Turn on lighting.  You can turn it off with a similar call to
    // glDisable().
    glEnable(GL_LIGHTING);
}

/**
 * Sets the OpenGL material state.  This is remembered so we only need to
 * do this once.  If you want to use different materials, you'd need to do this
 * before every different one you wanted to use.
 */
void initMaterial() {
    GLfloat emit[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat  amb[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat diff[] = {0.0, 0.0, 1.0, 1.0};
    GLfloat spec[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat shiny = 20.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, emit);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shiny);
}

/**
 * Set up OpenGL state.  This does everything so when we draw we only need to
 * actually draw the sphere, and OpenGL remembers all of our other settings.
 */
void initGL()
{
    // Tell openGL to use gauraud shading:
    glShadeModel(GL_SMOOTH);
    
    // Enable back-face culling:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable depth-buffer test.
    glEnable(GL_DEPTH_TEST);
    
    // Set up projection and modelview matrices ("camera" settings) 
    // Look up these functions to see what they're doing.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.5, 0.5, -0.5, 0.5, 1, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5, 5, 5, 0, 0, 0, 1, 0, 0);

    // set light parameters
    initLights();

    // set material parameters
    initMaterial();

    // initialize the "quadric" used by GLU to render high-level objects.
    quad = gluNewQuadric();
    gluQuadricOrientation(quad, GLU_OUTSIDE);
}

/**
 * Main entrance point, obviously.
 * Sets up some stuff then passes control to glutMainLoop() which never
 * returns.
 */
int main(int argc, char* argv[])
{
    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(800, 800);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS171 HW0");
    
    initGL();

    // set up GLUT callbacks.
    glutDisplayFunc(redraw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyfunc);

    // From here on, GLUT has control,
    glutMainLoop();

    // so we should never get to this point.
    return 1;
}

