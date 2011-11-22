#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "parser.h"
#include <unistd.h>

// Vars
static animation anim;
static GLUquadric *gluquad;
static int curframe;

// Camera Vars
float camR;
float camAngle;
const float camRDel = 1;
const float camAngleDel = 5;

// Animation state
bool playing;
bool loop;

// Parser function
void parse_file(std::istream &is, animation *output);
void print_animation(animation *anim);

void render_ibar();
void set_pose(int frame);
keyframe interpolate(int frame, keyframe prev, keyframe next);
glm::vec4 getquat(const glm::vec4 &rot);

/** PROTOTYPES **/
void redraw();
void initGL();
void resize(GLint w, GLint h);
void keyfunc(GLubyte key, GLint x, GLint y);
void idlefunc();


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

    set_pose(curframe);
    render_ibar();

    glutSwapBuffers();
}

keyframe interpolate(int frame, keyframe prev, keyframe next)
{
    float u = static_cast<float>(frame - prev.framenum) / (next.framenum - prev.framenum);
    u = std::max(std::min(u, 1.f), 0.f);

    /*
    std::cout << "Interpolating between " << prev.framenum << " and " << next.framenum
        << " at frame " << frame << " using factor " << u << '\n';
        */

    glm::vec4 prevquat = getquat(prev.rotation);
    glm::vec4 nextquat = getquat(next.rotation);
    glm::vec4 interquat = u * nextquat + (1 - u) * prevquat;
    interquat /= glm::length(interquat);
    glm::vec4 interrot = glm::vec4(interquat[1], interquat[2], interquat[3],
            2*acos(interquat[0]) / M_PI * 180.f);

    keyframe ret;
    ret.framenum = frame;
    ret.translation = (1 - u) * prev.translation + u * next.translation;
    ret.scale = (1 - u) * prev.scale + u * next.scale;
    ret.rotation = interrot;

    return ret;
}

glm::vec4 getquat(const glm::vec4 &rot)
{
    float rad = rot[3] * M_PI / 180.f / 2.f;

    return glm::vec4(cosf(rad),
            rot[0] * sinf(rad),
            rot[1] * sinf(rad),
            rot[2] * sinf(rad));
}

void set_pose(int frame)
{
    // iterate through the frames and find the 'last' frame
    keyframe last;
    keyframe next;
    for (int i = 1; i < anim.keyframes.size(); i++)
    {
        last = anim.keyframes[i - 1];
        next = anim.keyframes[i];
        
        if (frame >= last.framenum && frame < next.framenum)
            break;
    }

    std::cout << "Camera [r, angle] = [" << camR << ' ' << camAngle << "]\n";

    keyframe kf = interpolate(frame, last, next);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camR * cosf(camAngle * M_PI / 180), camR * sinf(camAngle * M_PI / 180), 0, 0, 0, 0, 0, 0, 1);
    
    glTranslatef(kf.translation[0], kf.translation[1], kf.translation[2]);
    glRotatef(kf.rotation[3], kf.rotation[0], kf.rotation[1], kf.rotation[2]);
    glScalef(kf.scale[0], kf.scale[1], kf.scale[2]);
}

void render_ibar()
{
    const float r = 0.2;
    glPushMatrix();
    glTranslatef(0, 0, -1);
    glColor3f(0.f, 0.f, 1.f);
    gluCylinder(gluquad, r, r, 2.0, 32, 32);

    glPushMatrix();
    glTranslatef(0, 0, 2);
    glRotatef(90, 1, 0, 0);
    glColor3f(1.f, 1.f, 0.f);
    gluCylinder(gluquad, r, r, 1.0, 32, 32);

    glTranslatef(0, 0, -1);
    glColor3f(0.f, 1.f, 0.f);
    gluCylinder(gluquad, r, r, 1.0, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glColor3f(1.f, 0.f, 0.f);
    gluCylinder(gluquad, r, r, 1.0, 32, 32);

    glTranslatef(0, 0, -1);
    glColor3f(0.5f, 0.5f, 1.f);
    gluCylinder(gluquad, r, r, 1.0, 32, 32);
    glPopMatrix();
    glPopMatrix();
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
    if (key == 's' || key == 'S')
        playing = false;
    if (key == 'p' || key == 'p')
        playing = true;
    if (key == 'F' || key == 'f')
        curframe += 1;
    if (key == 'R' || key == 'r')
        curframe -= 1;
    if (key == 'L' || key == 'l')
        loop = !loop;
    if (key == '0')
        curframe = 0;

    if (loop)
        curframe %= anim.nframes;

    glutPostRedisplay();
}

void specialfunc(int key, GLint x, GLint y)
{
    if (key == GLUT_KEY_LEFT)
        camAngle -= camAngleDel;
    if (key == GLUT_KEY_RIGHT)
        camAngle += camAngleDel;
    if (key == GLUT_KEY_UP)
        camR -= camRDel;
    if (key == GLUT_KEY_DOWN)
        camR += camRDel;

    glutPostRedisplay();
}

void mousefunc(int button, int state, int x, int y)
{
}

void motionfunc(int x, int y)
{
}

void idlefunc()
{
    if (playing) curframe += 1;
    if (loop) curframe %= anim.nframes;

    glutPostRedisplay();

    usleep(1000 * 16);
}

/**
 * Set up OpenGL state.  This does everything so when we draw we only need to
 * actually draw the sphere, and OpenGL remembers all of our other settings.
 */
void initGL()
{
    // Enable back-face culling:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable depth-buffer test.
    glEnable(GL_DEPTH_TEST);

    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-0.5, 0.5, -0.5, 0.5, 1, 1000);

    // Initial cam vars
    camR = 80.f;
    camAngle = 45;
    playing = true;
    loop = true;

    gluquad = gluNewQuadric();
    assert(gluquad);
    gluQuadricOrientation(gluquad, GLU_OUTSIDE);
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
    glutSpecialFunc(specialfunc);
    glutMouseFunc(mousefunc);
    glutMotionFunc(motionfunc);
    glutIdleFunc(idlefunc);

    // From here on, GLUT has control,
    glutMainLoop();

    // so we should never get to this point.
    return 1;
}

