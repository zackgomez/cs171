#include <iostream>
#include "GL/gl.h"
#include "GL/glut.h"
#include "parser.h"

void parse_file(std::istream &input, Scene *output);

// Our scene
Scene scene;

/** PROTOTYPES **/
void initLights();
void initMaterial(const Material &mat);
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

    glPushMatrix();
    // TODO apply mouse transformation now
    for (unsigned i = 0; i < scene.separators.size(); i++)
    {
        const Separator &sep = scene.separators[i];
        initMaterial(sep.material);
        glPushMatrix();

        glBegin(GL_POLYGON);
        for (unsigned j = 0; j < sep.indices.size(); j++)
        {
            int idx = sep.indices[j];
            if (idx == -1)
            {
                glEnd();
                glBegin(GL_POLYGON);
                continue;
            }

            Vector3 pt = sep.points[idx];
            glVertex3f(pt(0), pt(1), pt(2));
        }
        glEnd();

        glPopMatrix();
    }
    glPopMatrix();

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
    // TODO check for f == flat | g == gourad | w == toggle wireframe
    // use glShadeMode for f/g
}

/** Utility functions **/

/**
 * Sets up an OpenGL light.  This only needs to be called once
 * and the light will be used during all renders.
 */
void initLights() {
    for (unsigned i = 0; i < scene.lights.size(); i++)
    {
        const Light &light = scene.lights[i];
        GLfloat amb[] = { 0.0f, 0.0f, 0.0f };
        GLfloat diff[]= { light.color(0), light.color(1), light.color(2) };
        GLfloat spec[]= { light.color(0), light.color(1), light.color(2) };
        GLfloat lightpos[]= { light.position(0), light.position(1), light.position(2) };
        // not specified in .iv file, just make it 1.0 
        GLfloat shiny = 1.0f; 

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);
        glLightfv(GL_LIGHT0 + i, GL_AMBIENT, amb);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, diff);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, spec);
        glLightfv(GL_LIGHT0 + i, GL_POSITION, lightpos);
        glLightf(GL_LIGHT0 + i, GL_SHININESS, shiny);
        glEnable(GL_LIGHT0 + i);
    }

    // Turn on lighting.  You can turn it off with a similar call to
    // glDisable().
    glEnable(GL_LIGHTING);
}

/**
 * Sets the OpenGL material state.  This is remembered so we only need to
 * do this once.  If you want to use different materials, you'd need to do this
 * before every different one you wanted to use.
 */
void initMaterial(const Material& mat) {
    GLfloat emit[] = {0.0, 0.0, 0.0, 1.0};
    GLfloat  amb[] = {mat.ambientColor(0), mat.ambientColor(1), mat.ambientColor(2)};
    GLfloat diff[] = {mat.diffuseColor(0), mat.diffuseColor(1), mat.diffuseColor(2)};
    GLfloat spec[] = {mat.specularColor(0), mat.specularColor(1), mat.specularColor(2)};
    GLfloat shiny = mat.shininess;

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
    // Tell openGL to use gouraud shading:
    glShadeModel(GL_SMOOTH);
    
    // Enable back-face culling:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable depth-buffer test.
    glEnable(GL_DEPTH_TEST);

    const Camera& cam = scene.camera;
    
    float mat[16];
    // Set up projection and modelview matrices ("camera" settings) 
    // Look up these functions to see what they're doing.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // take these params from scene.camera
    glFrustum(cam.left, cam.right, cam.bottom, cam.top, cam.nearDistance, cam.farDistance);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // change this to a transform for the camera
    glRotatef(-cam.orientation(3) * 180.0 / M_PI, cam.orientation(0), cam.orientation(1), cam.orientation(2));
    glTranslatef(-cam.position(0), -cam.position(1), -cam.position(2));
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);

    for (int i = 0; i < 16; i++)
    {
        if (i % 4 == 0 && i != 0)
            std::cout << '\n';
        std::cout << mat[i] << ' ';
    }
    std::cout << '\n';

    // set light parameters
    initLights();
}

/**
 * Main entrance point, obviously.
 * Sets up some stuff then passes control to glutMainLoop() which never
 * returns.
 */
int main(int argc, char* argv[])
{
    // TODO read this from args (also xdim ydim)
    parse_file(std::cin, &scene);
    
    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(800, 800);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS171 HW4 - Zack Gomez");
    
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

