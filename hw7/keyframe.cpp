#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "parser.h"

// Vars
static animation anim;
static GLUquadric *gluquad;

// Parser function
void parse_file(std::istream &is, animation *output);
void print_animation(animation *anim);

void render_ibar();

/** PROTOTYPES **/
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

    render_ibar();

    glutSwapBuffers();
}

void render_ibar()
{
    gluCylinder(gluquad, 10, 10, 10, 10, 10);
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

void mousefunc(int button, int state, int x, int y)
{
}

void motionfunc(int x, int y)
{
}

/**
 * Set up OpenGL state.  This does everything so when we draw we only need to
 * actually draw the sphere, and OpenGL remembers all of our other settings.
 */
void initGL()
{
    // Enable back-face culling:
    /*
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    */

    // Enable depth-buffer test.
    glEnable(GL_DEPTH_TEST);

    // Set up projection and modelview matrices ("camera" settings) 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1, 10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Camera transform
    glTranslatef(0, 0, -5);

    gluquad = gluNewQuadric();
    assert(gluquad);
}

void print_animation(animation *anim)
{
    std::cout << "Total Frames: " << anim->nframes << '\n';
    std::cout << "Keyframes:\n";
    for (int i = 0; i < anim->keyframes.size(); i++)
    {
        const keyframe &kf = anim->keyframes[i];
        std::cout << "Frame Number: " << kf.framenum
            << "  Translation: " << kf.translation[0] << ' ' << kf.translation[1] << ' ' << kf.translation[2]
            << "  Rotation: " << kf.rotation[0] << ' ' << kf.rotation[1] << ' ' << kf.rotation[2] << ' ' << kf.rotation[3]
            << "  Scale: " << kf.scale[0] << ' ' << kf.scale[1] << ' ' << kf.scale[2] << '\n';
    }
}

/**
 * Main entrance point, obviously.
 * Sets up some stuff then passes control to glutMainLoop() which never
 * returns.
 */
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "usage: " << argv[0] << " <script-file-name>\n";
        exit(1);
    }
    std::string filename = argv[1];
    std::ifstream file(filename.c_str());
    if (!file)
    {
        std::cout << "Unable to open file " << filename << " for reading\n";
        exit(1);
    }

    parse_file(file, &anim);
    print_animation(&anim);

    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(400, 400);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS171 HW7 - Interpolating Keyframes - Zack Gomez");
    
    initGL();


    // set up GLUT callbacks.
    glutDisplayFunc(redraw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyfunc);
    glutMouseFunc(mousefunc);
    glutMotionFunc(motionfunc);

    // From here on, GLUT has control,
    glutMainLoop();

    // so we should never get to this point.
    return 1;
}

