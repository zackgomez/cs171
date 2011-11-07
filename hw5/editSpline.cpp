#include <iostream>
#include "GL/gl.h"
#include "GL/glut.h"
#include <vector>
#include <glm/glm.hpp>
#include <cassert>

int mouseX, mouseY;
bool dragging;
int dragIdx = -1;
const float cpsize = 0.005;

int winW, winH;

std::vector<float> knotVector;
std::vector<glm::vec2> cps;

/** PROTOTYPES **/
void redraw();
void initGL();
void resize(GLint w, GLint h);
void keyfunc(GLubyte key, GLint x, GLint y);

glm::vec2 splineFunc(float u);
float coxDeBoor(int i, int k, float u);

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

    // Draw each of the cps
    glBegin(GL_QUADS);
    for (int i = 0; i < cps.size(); i++)
    {
        glm::vec2 pt = cps[i];
        // Highlight the dragged cp
        if (i == dragIdx)
            glColor3f(1., 0.2, 0.2);
        else 
            glColor3f(1, 1, 1);

        glVertex3f(pt.x - cpsize, pt.y - cpsize, 0.0);
        glVertex3f(pt.x + cpsize, pt.y - cpsize, 0.0);
        glVertex3f(pt.x + cpsize, pt.y + cpsize, 0.0);
        glVertex3f(pt.x - cpsize, pt.y + cpsize, 0.0);
    }
    glEnd();

    // Draw lines connecting the cps
    assert(cps.size() >= 2);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (size_t i = 0; i < cps.size() - 1; i++)
    {
        glm::vec2 pt = cps[i];
        glm::vec2 pt2 = cps[i+1];

        glVertex3f(pt.x, pt.y, 0.0f);
        glVertex3f(pt2.x, pt2.y, 0.0f);
    }
    glEnd();

    // Now draw the spline!
    glBegin(GL_LINES);
    glColor3f(0.3, 1.0, 0.3);
    for (float u = 0.0f; u < 1.0f-0.01f; u += 0.01)
    {
        glm::vec2 p0 = splineFunc(u);
        glm::vec2 p1 = splineFunc(u + 0.01);

            glVertex3f(p0.x, p0.y, 0.0f);
            glVertex3f(p1.x, p1.y, 0.0f);
    }
    glEnd();

    glutSwapBuffers();
}

/**
 * GLUT calls this function when the window is resized.
 * All we do here is change the OpenGL viewport so it will always draw in the
 * largest square that can fit in our window..
 */
void resize(GLint w, GLint h)
{
    winW = w; winH = h;
    if (h == 0)
        h = 1;

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
    if (button == GLUT_LEFT_BUTTON)
    {
        dragging = (state == GLUT_DOWN);
        if (!dragging) dragIdx = -1;
        mouseX = x; mouseY = y;
        if (dragging)
        {
            dragIdx = -1;
            float xx = 1.0f * x / winW;
            float yy = 1.0f - (1.0f * y / winH);
            // Find the idx of the cp they're dragging, if it exists
            for (size_t i = 0; i < cps.size(); i++)
            {
                glm::vec2 cp = cps[i];
                if (xx > cp.x - cpsize && xx < cp.x + cpsize &&
                    yy > cp.y - cpsize && yy < cp.y + cpsize)
                {
                    dragIdx = i;
                    break;
                }
            }
            // Only dragging if they are clicking on a cp
            dragging = (dragIdx != -1);
        }
    }

    std::cout << "DRAG INDEX: " << dragIdx << '\n';
    glutPostRedisplay();
}

void motionfunc(int x, int y)
{
    float xx = 1.0f * x / winW;
    float yy = 1.0f - (1.0f * y / winH);

    if (dragging)
    {
        assert(dragIdx != -1);
        cps[dragIdx] =  glm::vec2(xx, yy);
    }

    glutPostRedisplay();
}

/** Utility functions **/
/**
 * Set up OpenGL state.  This does everything so when we draw we only need to
 * actually draw the sphere, and OpenGL remembers all of our other settings.
 */
void initGL()
{
    // Set up projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void initNURBS()
{
    float knot0[] = {0, 0, 0, 0, 1, 1, 1, 1};
    knotVector.assign(knot0, knot0+8);

    std::cout << "initial knot vector: ";
    for (size_t i = 0; i < knotVector.size(); i++)
        std::cout << knotVector[i] << ' ';
    std::cout << '\n';

    cps.push_back(glm::vec2(.1, .9));
    cps.push_back(glm::vec2(.1, .1));
    cps.push_back(glm::vec2(.9, .1));
    cps.push_back(glm::vec2(.9, .9));
}

/**
 * Main entrance point, obviously.
 * Sets up some stuff then passes control to glutMainLoop() which never
 * returns.
 */
int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "usage: " << argv[0] << " <xRes> <yRes>\n";
        exit(1);
    }
    int xRes = atoi(argv[1]);
    int yRes = atoi(argv[2]);
    if (xRes == 0 || yRes == 0)
    {
        std::cout << "usage: " << argv[0] << " <xRes> <yRes>\n";
        exit(1);
    }
    
    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(xRes, yRes);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS171 HW5 - Zack Gomez");
    
    initGL();
    initNURBS();

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

glm::vec2 splineFunc(float u)
{
    assert(u >= 0 && u <= 1);
    const int k = 4;
    glm::vec2 result(0.0f);
    for (size_t i = 0; i < cps.size(); i++)
    {
        glm::vec2 cp = cps[i];
        result += cp * coxDeBoor(i, k, u);
    }

    return result;
}

float coxDeBoor(int i, int k, float u)
{
    if (k == 1)
        return (i >= knotVector[i] && u < knotVector[i+1]) ? 1 : 0;

    float adenom = (knotVector[i+k-1] - knotVector[i]);
    float bdenom = (knotVector[i+k] - knotVector[i+1]);

    float a = adenom == 0 ? 0 : ((u - knotVector[i]) * coxDeBoor(i, k-1, u)) / adenom;
    float b = bdenom == 0 ? 0 : ((knotVector[i+k] - u) * coxDeBoor(i+1, k-1, u)) / bdenom;

    return a + b;
}

