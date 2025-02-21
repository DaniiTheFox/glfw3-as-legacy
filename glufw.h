#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include <functional>
#include <unordered_map>

#define GLUT_DOUBLE 0x0001
#define GLUT_RGBA 0x0002
#define GLUT_DEPTH 0x0004

#define GLUT_CURSOR_NONE GLFW_CURSOR_HIDDEN
#define GLUT_CURSOR_LEFT_ARROW GLFW_CURSOR_NORMAL
#define GLUT_RIGHT_BUTTON GLFW_MOUSE_BUTTON_RIGHT
#define GLUT_LEFT_BUTTON GLFW_MOUSE_BUTTON_LEFT

static GLFWwindow* window = NULL;
static void (*displayFunc)(void) = NULL;
static void (*reshapeFunc)(int, int) = NULL;
static void (*keyboardFunc)(unsigned char, int, int) = NULL;
static void (*keyboardUpFunc)(unsigned char, int, int) = NULL;
static void (*mouseFunc)(int, int, int, int) = NULL;
static void (*motionFunc)(int, int) = NULL;
static void (*passiveMotionFunc)(int, int) = NULL;
static void (*idleFunc)(void) = NULL;

static int windowWidth = 800;
static int windowHeight = 600;

struct TimerCallback {
    std::function<void(int)> func;
    double triggerTime;
    int value;
};

static std::vector<TimerCallback> timerCallbacks;

void glutInit(int* argc, char** argv) {
    (void)argc; (void)argv;
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }
}

void glutInitDisplayMode(unsigned int mode) {
    (void)mode;
}

void glutInitWindowSize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void glutCreateWindow(const char* title) {
    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
}

void glutDisplayFunc(void (*func)(void)) { displayFunc = func; }
void glutReshapeFunc(void (*func)(int, int)) { reshapeFunc = func; }
void glutKeyboardFunc(void (*func)(unsigned char, int, int)) { keyboardFunc = func; }
void glutKeyboardUpFunc(void (*func)(unsigned char, int, int)) { keyboardUpFunc = func; }
void glutMouseFunc(void (*func)(int, int, int, int)) { mouseFunc = func; }
void glutMotionFunc(void (*func)(int, int)) { motionFunc = func; }
void glutPassiveMotionFunc(void (*func)(int, int)) { passiveMotionFunc = func; }
void glutIdleFunc(void (*func)(void)) { idleFunc = func; }

void glutSwapBuffers() {
    glfwSwapBuffers(window);
}

void glutPostRedisplay() {
    if (displayFunc) displayFunc();
}

void glutSetCursor(int cursor) {
    glfwSetInputMode(window, GLFW_CURSOR, cursor);
}

void glutTimerFunc(unsigned int msec, void (*func)(int), int value) {
    double currentTime = glfwGetTime();
    timerCallbacks.push_back({func, currentTime + msec / 1000.0, value});
}

void gluPerspective(GLdouble fov, GLdouble aspect, GLdouble zNear, GLdouble zFar) {
    GLdouble fH = tan((fov / 2.0) * (M_PI / 180.0)) * zNear;
    GLdouble fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void glfw_framebuffer_size_callback(GLFWwindow* win, int width, int height) {
    (void)win;
    glViewport(0, 0, width, height);
    if (reshapeFunc) reshapeFunc(width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void glfw_key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    (void)win; (void)scancode; (void)mods;
    if (keyboardFunc && action == GLFW_PRESS) {
        keyboardFunc((unsigned char)key, 0, 0);
    } else if (keyboardUpFunc && action == GLFW_RELEASE) {
        keyboardUpFunc((unsigned char)key, 0, 0);
    }
}

void glfw_mouse_button_callback(GLFWwindow* win, int button, int action, int mods) {
    (void)win; (void)mods;
    if (mouseFunc) {
        mouseFunc(button, action, 0, 0);
    }
}

void glfw_cursor_pos_callback(GLFWwindow* win, double xpos, double ypos) {
    (void)win;
    if (motionFunc) motionFunc((int)xpos, (int)ypos);
    if (passiveMotionFunc) passiveMotionFunc((int)xpos, (int)ypos);
}

void glutMainLoop() {
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        double currentTime = glfwGetTime();
        for (auto it = timerCallbacks.begin(); it != timerCallbacks.end();) {
            if (currentTime >= it->triggerTime) {
                it->func(it->value);
                it = timerCallbacks.erase(it);
            } else {
                ++it;
            }
        }
        
        if (idleFunc) idleFunc();
        if (displayFunc) displayFunc();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
