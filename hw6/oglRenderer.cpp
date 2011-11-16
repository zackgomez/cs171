#include <iostream>
#include "GL/gl.h"
#include "GL/glut.h"
#include "parser.h"
#include "transforms.h" 
#include "glutils.h"

void parse_file(std::istream &input, Scene *output);

// Our scene
Scene scene;
std::vector<GLuint> textures;
bool wireframe;
bool translating, zooming, rotating;
int mouseX, mouseY;

Vector3 mouseTrans;
Matrix4 mouseRot;
float mouseZoom;

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
    // apply mouse transformations
    GLfloat oldMatrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, oldMatrix);
    glLoadIdentity();

    glTranslatef(mouseTrans(0), mouseTrans(1), mouseZoom);

    glTranslatef(0, 0, -3);
    Matrix4 columnMajor = mouseRot.transpose();
    glMultMatrixf(&columnMajor(0));
    glTranslatef(0, 0, 3);

    glMultMatrixf(oldMatrix);

   
    
    for (unsigned i = 0; i < scene.separators.size(); i++)
    {
        const Separator &sep = scene.separators[i];
        glPushMatrix();

        for (unsigned j = 0; j < sep.transforms.size(); j++)
        {
            const Transform &trans = sep.transforms[j];
            glTranslatef(trans.translation(0), trans.translation(1), trans.translation(2));
            glRotatef(trans.rotation(3) * 180 / M_PI, trans.rotation(0), trans.rotation(1), trans.rotation(2));
            glScalef(trans.scaling(0), trans.scaling(1), trans.scaling(2));
        }

        GLenum renderType = wireframe ? GL_LINE_LOOP : GL_POLYGON;

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glBegin(renderType);
        for (unsigned j = 0; j < sep.indices.size(); j++)
        {
            int idx = sep.indices[j];
            int texidx = sep.texindices[j];
            if (idx == -1)
            {
                glEnd();
                glBegin(renderType);
                continue;
            }

            Vector3 pt = sep.points[idx];
            Vector2 uv = sep.texcoords[texidx];
            glTexCoord2f(uv(0), uv(1));
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
    if (key == 'w' || key == 'W')
    {
        wireframe = !wireframe;
        glutPostRedisplay();
    }
    if (key == 'f' || key == 'F')
    {
        glShadeModel(GL_FLAT);
        glutPostRedisplay();
    }
    if (key == 'g' || key == 'G')
    {
        glShadeModel(GL_SMOOTH);
        glutPostRedisplay();
    }
}

void mousefunc(int button, int state, int x, int y)
{
    bool shift = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
    if (button == GLUT_MIDDLE_BUTTON && !shift)
    {
        translating = (state == GLUT_DOWN);
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
    if (button == GLUT_MIDDLE_BUTTON && shift)
    {
        zooming = (state == GLUT_DOWN);
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
    if (button == GLUT_LEFT_BUTTON)
    {
        rotating = (state == GLUT_DOWN);
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
}

void motionfunc(int x, int y)
{
    if (translating)
    {
        mouseTrans += makeVector3((x - mouseX) / 500.0f, (mouseY - y) / 500.0f, 0.0f);
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
    if (zooming)
    {
        mouseZoom += (mouseY - y) / 500.f;
        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
    if (rotating)
    {
        int delX = x - mouseX;
        int delY = y - mouseY;
        Vector3 dragLine = makeVector3(delY, delX, 0).normalize();
        float angle = sqrtf(delX * delX + delY * delY) / 100.0f;

        mouseRot = make_rotation(dragLine(0), dragLine(1), 0, angle) * mouseRot;

        mouseX = x; mouseY = y;
        glutPostRedisplay();
    }
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
    
    // Set up projection and modelview matrices ("camera" settings) 
    // Look up these functions to see what they're doing.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // take these params from scene.camera
    glFrustum(cam.left, cam.right, cam.bottom, cam.top, cam.nearDistance, cam.farDistance);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Camera transform
    glRotatef(-cam.orientation(3) * 180.0 / M_PI, cam.orientation(0), cam.orientation(1), cam.orientation(2));
    glTranslatef(-cam.position(0), -cam.position(1), -cam.position(2));

    wireframe = false;
    mouseTrans = makeVector3(0, 0, 0);
    mouseZoom = 0.0f;
    mouseRot = make_identity<float, 4>();
}

GLuint loadTexture(std::string filename)
{
    return 0;
}

void initScene()
{
    parse_file(std::cin, &scene);

    // Now we need to load each texture file
    for (int i = 0; i < scene.separators.size(); i++)
    {
        const char *filename = scene.separators[i].texture.c_str();
        GLuint tex = make_texture(filename);
        if (!tex)
        {
            std::cerr << "Unable to load texture in filename " << filename << '\n';
            std::cerr << "Texture: " << tex << '\n';
            exit(1);
        }
        textures.push_back(tex);
    }
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
        std::cout << "usage: " << argv[0] << " <xRes> <yRes> < <iv-file>\n";
        exit(1);
    }
    int xdim = atoi(argv[1]);
    int ydim = atoi(argv[2]);
    if (xdim == 0 || ydim == 0)
    {
        std::cout << "usage: " << argv[0] << " <xRes> <yRes> < <iv-file>\n";
        exit(1);
    }

    // OpenGL will take out any arguments intended for its use here.
    // Useful ones are -display and -gldebug.
    glutInit(&argc, argv);

    // Get a double-buffered, depth-buffer-enabled window, with an
    // alpha channel.
    // These options aren't really necessary but are here for examples.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    glutInitWindowSize(xdim, ydim);
    glutInitWindowPosition(300, 100);

    glutCreateWindow("CS171 HW4 - Zack Gomez");
    
    initGL();
    initScene();


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

