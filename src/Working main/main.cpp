
#include "X11/keysym.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/backends/imgui_impl_glut.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "mat4c2gl.h"
#include "readfile.h"
#include <GL/freeglut.h>
#include <GL/glew.h>
#include <X11/Xlib.h>
#include <iostream>
#include <unistd.h>

using namespace std;

#define WINDOW_TITLE "Assingment 1"

#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif

const char *filename = "cow_up.in";

#define CCW 0 // If one then render as counter-clockwise

#define BUFFER_OFFSET(offset) ((GLvoid *)(offset))
enum Buffer_IDs { VtxBuffer = 0, NormBuffer = 1, NumBuffers = 2 };
enum Attrib_IDs { vPosition = 0, vNormalVertex = 1 };
GLint shaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VAO;
GLuint Buffers[NumBuffers];
GLuint NumVertices;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.5f;
GLuint VBO_color;
bool leftMousePressed = false;
glm::vec3 modelCenter;
std::vector<Triangle> tris;
float Vert[90000];
float Vert_Normal[90000];

void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
glm::mat4 RotateCamera(float angle);
void mouseWheel(int button, int dir, int x, int y);
void mouseCallback(int xpos, int ypos);
void mouseButtonCallback(int button, int state, int x, int y);
void setModelColor(float r, float g, float b);
glm::vec3 calculateCenter();

GLenum err;

int numTris = 0;

// Vertex shader source
const GLchar *vertexShaderSource = GLSL(
    330,

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 color;

    out vec3 mobileColor;

    uniform mat4 model; uniform mat4 view; uniform mat4 projection;

    void main() {
      gl_Position = projection * view * model * vec4(position, 1.0f);
      mobileColor = color;
    });

// Fragment shader source
const GLchar *fragmentShaderSource = GLSL(
    330,

    in vec3 mobileColor;

    out vec4 gpuColor;

    void main() { gpuColor = vec4(mobileColor, 1.0); });

int main(int argc, char *argv[]) {

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(WindowWidth, WindowHeight);
  glutCreateWindow(WINDOW_TITLE);

  glutReshapeFunc(UResizeWindow);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsDark();

  ImGui_ImplGLUT_Init();
  ImGui_ImplGLUT_InstallFuncs();
  ImGui_ImplOpenGL3_Init();

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cout << "Failed to initialize GLEW" << std::endl;
    return -1;
  }

  GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);

  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  MeshData triangles = readfile(filename);

  std::cout << "Loaded " << triangles.numTris << " triangles.\n";

  /*if (!triangles.tris.empty()) {
    const Triangle &t = triangles.tris[0];
    std::cout << "First triangle vertices: (" << t.v0.x << ", " << t.v0.y
              << ", " << t.v0.z << ") | (" << t.v1.x << ", " << t.v1.y << ", "
              << t.v1.z << ") | (" << t.v2.x << ", " << t.v2.y << ", " << t.v2.z
              << ")\n";
  }*/

  for (int i = 0; i < triangles.numTris; i++) {
    // vertex coordinates
    Vert[9 * i] = triangles.tris[i].v0.x;
    Vert[9 * i + 1] = triangles.tris[i].v0.y;
    Vert[9 * i + 2] = triangles.tris[i].v0.z;
    Vert[9 * i + 3] = triangles.tris[i].v1.x;
    Vert[9 * i + 4] = triangles.tris[i].v1.y;
    Vert[9 * i + 5] = triangles.tris[i].v1.z;
    Vert[9 * i + 6] = triangles.tris[i].v2.x;
    Vert[9 * i + 7] = triangles.tris[i].v2.y;
    Vert[9 * i + 8] = triangles.tris[i].v2.z;
    // vertex normal coordinates
    Vert_Normal[9 * i] = triangles.tris[i].norm[0].x;
    Vert_Normal[9 * i + 1] = triangles.tris[i].norm[0].y;
    Vert_Normal[9 * i + 2] = triangles.tris[i].norm[0].z;
    Vert_Normal[9 * i + 3] = triangles.tris[i].norm[1].x;
    Vert_Normal[9 * i + 4] = triangles.tris[i].norm[1].y;
    Vert_Normal[9 * i + 5] = triangles.tris[i].norm[1].z;
    Vert_Normal[9 * i + 6] = triangles.tris[i].norm[2].x;
    Vert_Normal[9 * i + 7] = triangles.tris[i].norm[2].y;
    Vert_Normal[9 * i + 8] = triangles.tris[i].norm[2].z;
  }

  tris = triangles.tris;
  numTris = triangles.numTris;

  glUseProgram(shaderProgram);

  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL Error: " << err << std::endl;
  }

  modelCenter = calculateCenter();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  glutDisplayFunc(URenderGraphics);

  glutMainLoop();
  glDeleteVertexArrays(1, &VAO);

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGLUT_Shutdown();
  ImGui::DestroyContext();

  return 0;
}

bool key_is_pressed(KeySym ks) {
  Display *dpy = XOpenDisplay(":0");
  char keys_return[32];
  XQueryKeymap(dpy, keys_return);
  KeyCode kc2 = XKeysymToKeycode(dpy, ks);
  bool isPressed = !!(keys_return[kc2 >> 3] & (1 << (kc2 & 7)));
  XCloseDisplay(dpy);
  return isPressed;
}

void UResizeWindow(int w, int h) {
  WindowWidth = w;
  WindowHeight = h;
  glViewport(0, 0, WindowWidth, WindowHeight);
}

int angle = 0;

int freeCam = 0;

int instruct = 0;

glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

float distcam = 10.0f;

float yaw = -90.0f;

float pitch = 0.0f;

float farplane = 10000.0f;

float nearplane = 0.1f;

float lastX = 400, lastY = 300;

bool firstMouse = true;

float modelColor[3];

int open = 1;

void URenderGraphics(void) {

  glutMouseFunc(ImGui_ImplGLUT_MouseFunc);
  glutMotionFunc(ImGui_ImplGLUT_MotionFunc);
  glutPassiveMotionFunc(ImGui_ImplGLUT_MotionFunc);
  glutMouseWheelFunc(mouseWheel);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGLUT_NewFrame();
  ImGui::NewFrame();

  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(VAO);

  mat4c2gl modelcgl(1.0f);
  modelcgl[0][0] = 1.0f;
  modelcgl[1][1] = 1.0f;
  modelcgl[2][2] = 1.0f;
  modelcgl[3][3] = 1.0f;
  modelcgl[0][3] = -modelCenter.x;
  modelcgl[1][3] = -modelCenter.y;
  modelcgl[2][3] = -modelCenter.z;

  mat4c2gl viewcgl;

  if (key_is_pressed(XK_a))
    yaw -= 1;
  if (key_is_pressed(XK_d))
    yaw += 1;
  if (key_is_pressed(XK_w))
    pitch += 1;
  if (key_is_pressed(XK_s))
    pitch -= 1;

  pitch = glm::clamp(pitch, -89.0f, 89.0f);

  glm::vec3 direction;
  direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
  direction.y = sin(glm::radians(pitch));
  direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

  cameraPos = target - glm::normalize(direction) * distcam;
  cameraFront = glm::normalize(target - cameraPos);

  std::array<double, 3> eye = {cameraPos.x, cameraPos.y, cameraPos.z};
  std::array<double, 3> center = {0.0, 0.0, 0.0};
  std::array<double, 3> up = {cameraUp.x, cameraUp.y, cameraUp.z};

  viewcgl.setViewMatrix(eye, center, up);

  // FreeCam logic same as above...
  if (key_is_pressed(XK_a))
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (key_is_pressed(XK_d))
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (key_is_pressed(XK_w))
    cameraPos += cameraSpeed * cameraFront;
  if (key_is_pressed(XK_s))
    cameraPos -= cameraSpeed * cameraFront;

  ImGuiIO &io = ImGui::GetIO();
  if (io.MouseDown[0] && !io.WantCaptureMouse) {
    float sensitivity = 0.1f;
    yaw += io.MouseDelta.x * sensitivity;
    pitch -= io.MouseDelta.y * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
  }

  std::array<double, 3> eyecgl = {cameraPos.x, cameraPos.y, cameraPos.z};
  std::array<double, 3> centercgl = {cameraPos.x + cameraFront.x,
                                  cameraPos.y + cameraFront.y,
                                  cameraPos.z + cameraFront.z};
  std::array<double, 3> upcgl = {cameraUp.x, cameraUp.y, cameraUp.z};

  viewcgl.setViewMatrix(eyecgl, centercgl, upcgl);

  mat4c2gl projectioncgl;
  projectioncgl.setProjectionMatrix(
      45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, nearplane, farplane);

  GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
  GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
  GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glCreateBuffers(NumBuffers, Buffers);

  glBindBuffer(GL_ARRAY_BUFFER, Buffers[VtxBuffer]);
  glBufferStorage(GL_ARRAY_BUFFER, numTris * 9 * sizeof(GL_FLOAT), Vert,
                  GL_DYNAMIC_STORAGE_BIT);

  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);
  //
  glBindBuffer(GL_ARRAY_BUFFER, Buffers[NormBuffer]);
  glBufferStorage(GL_ARRAY_BUFFER, numTris * 9 * sizeof(GL_FLOAT), Vert_Normal,
                  GL_DYNAMIC_STORAGE_BIT);

  glVertexAttribPointer(vNormalVertex, 3, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vNormalVertex);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);

  NumVertices = numTris * 3;

  glBindVertexArray(0);

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelcgl.toGLMatrix().data());
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewcgl.toGLMatrix().data());
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, projectioncgl.toGLMatrix().data());

  glutPostRedisplay();
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, NumVertices);

  // ImGui GUI
  ImGui::Begin("Viewer Controls");
  ImGui::Text("Camera mode: %s", freeCam ? "FreeCam" : "LockedCam");
  ImGui::SliderFloat("Near Plane", &nearplane, 0.1f, farplane - 0.1f);
  ImGui::SliderFloat("Far Plane", &farplane, nearplane + 0.1f, 1000.0f);
  ImGui::ColorEdit3("Model Color", modelColor);
  ImGui::Checkbox("Free Camera", (bool *)&freeCam);
  if (ImGui::Button("Reset Camera")) {
    cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    distcam = 10.0f;
    yaw = -90.0f;
    pitch = 0.0f;
  }
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glutSwapBuffers();
  glBindVertexArray(0);
}

void mouseWheel(int button, int dir, int x, int y) {
  if (dir > 0) {
    distcam -= 5.5f;
  } else {
    distcam += 5.5f;
  }
  glutPostRedisplay();
}

void mouseCallback(int xpos, int ypos) {

  if (!leftMousePressed)
    return;

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}

void mouseButtonCallback(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      leftMousePressed = true;
      firstMouse = true;
    } else if (state == GLUT_UP) {
      leftMousePressed = false;
    }
  }
}

void setModelColor(float r, float g, float b) {
  float newColors[9 * numTris];

  for (int i = 0; i < numTris; i++) {
    for (int j = 0; j < 3; j++) {
      newColors[i * 9 + j * 3 + 0] = r;
      newColors[i * 9 + j * 3 + 1] = g;
      newColors[i * 9 + j * 3 + 2] = b;
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newColors), newColors);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

glm::vec3 calculateCenter() {
  glm::vec3 center(0.0f);
  for (const auto &tri : tris) {
    center += glm::vec3(tri.v0.x, tri.v0.y, tri.v0.z);
    center += glm::vec3(tri.v1.x, tri.v1.y, tri.v1.z);
    center += glm::vec3(tri.v2.x, tri.v2.y, tri.v2.z);
  }

  center /= static_cast<float>(tris.size() * 3);
  return center;
}