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
int k = 4;
bool editMode = true;

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
void insertKnot(float u);

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

    if (editMode)
    {
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
    }

    // Now draw the spline!
    glBegin(GL_LINES);
    glColor3f(0.3, 1.0, 0.3);
    float dt = 0.01;
    float maxdist = 1.0 / (std::max(winW, winH));
    for (float u = 0.0f; u < 1.0f-dt; u += dt)
    {
        glm::vec2 p0 = splineFunc(u);
        glm::vec2 p1 = splineFunc(u + dt);

        float dist = glm::length(p1 - p0);

        if (dist < maxdist)
        {
            dt *= 1.5;
            p1 = splineFunc(std::min(u + dt, 1.0f));
        }
        else
            while (dist > maxdist)
            {
                dt /= 2;
                p1 = splineFunc(u + dt);
                dist = glm::length(p1 - p0);
            }

        if (p1.x == 0 && p1.y == 0)
            continue;

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
    if (key == ' ')
        editMode = !editMode;
    glutPostRedisplay();
}

void mousefunc(int button, int state, int x, int y)
{
    if (!editMode)
        return;
    float xx = 1.0f * x / winW;
    float yy = 1.0f - (1.0f * y / winH);
    if (button == GLUT_LEFT_BUTTON)
    {
        dragging = (state == GLUT_DOWN);
        if (!dragging) dragIdx = -1;
        mouseX = x; mouseY = y;
        if (dragging)
        {
            dragIdx = -1;
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

    // When they release the right button, add a new knot
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        float t = -1;
        float mindist = HUGE_VAL;
        glm::vec2 click(xx, yy);
        // XXX: Wowzers this is a slow way to do this, better to cache some shit
        // Find the 't' value for the new knot
        for (float u = 0.0; u < 1.0 - 0.01f; u += 0.01f)
        {
            glm::vec2 pt = splineFunc(u);
            float curdist = glm::length(pt - click);
            if (curdist < mindist)
            {
                mindist = curdist;
                t = u;
            }
        }

        // Add knot
        insertKnot(t);
    }

    glutPostRedisplay();
}

void motionfunc(int x, int y)
{
    if (!editMode)
        return;
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

    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2);
    
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
    if (u > 1) u = 1;
    assert(u >= 0 && u <= 1);
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
        return (u >= knotVector[i] && u < knotVector[i+1]) ? 1 : 0;

    float adenom = (knotVector[i+k-1] - knotVector[i]);
    float bdenom = (knotVector[i+k] - knotVector[i+1]);

    float a = adenom == 0 ? 0 : ((u - knotVector[i]) * coxDeBoor(i, k-1, u)) / adenom;
    float b = bdenom == 0 ? 0 : ((knotVector[i+k] - u) * coxDeBoor(i+1, k-1, u)) / bdenom;

    return a + b;
}

void insertKnot(float u)
{
    assert(u > 0 && u < 1);
    std::vector<glm::vec2> newcps(cps.size() + 1);
    // p'0 = p0
    newcps[0] = cps[0];
    // p'n+1 = p'n
    newcps[cps.size()] = cps[cps.size() - 1];

    // Make new knot vector
    int j = -1;
    for (std::vector<float>::iterator it = knotVector.begin();
            it != knotVector.end(); it++, j++)
    {
        if (u < *it)
        {
            knotVector.insert(it, u);
            break;
        }
    }

    // Fill in new cps
    for (int i = 1; i < cps.size(); i++)
    {
        float a;
        if (i <= j - k)
        {
            a = 1;
        }
        else if (i <= j && i >= j - k + 1)
        {
            float adenom = knotVector[i+k] - knotVector[i];
            a = adenom == 0 ? 0 : (u - knotVector[i]) / adenom;
        }
        else
            a = 0;

        newcps[i] = ((1 - a) * cps[i-1]) + (a * cps[i]);
    }

    cps = newcps;
}
