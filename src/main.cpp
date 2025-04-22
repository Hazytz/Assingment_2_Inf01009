
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
#include <chrono>
#include <iostream>
#include <unistd.h>

using namespace std;

#define WINDOW_TITLE "Assingment 2"

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
int frameCount = 0;
double fps = 0.0;
auto lastTime = std::chrono::high_resolution_clock::now();
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
int open = -1;
float hfov = 90.0f;
float vfov = 60.0f;
glm::vec3 lightPos(0.0f, 0.0f, 100.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 objectColor(1.0f, 1.0f, 1.0f);

void UResizeWindow(int, int);
void URenderGraphics(void);
void URenderGraphicsClose2GL(void);
void UCreateShader(void);
void UCreateBuffers(void);
glm::mat4 RotateCamera(float angle);
void mouseWheel(int button, int dir, int x, int y);
void mouseCallback(int xpos, int ypos);
void mouseButtonCallback(int button, int state, int x, int y);
void setModelColor(float r, float g, float b);
glm::vec3 calculateCenter();
void UpdateFPS();
GLuint compileShader(GLenum type, const GLchar *source);
GLuint createShaderProgram(const GLchar *vertexSource,
                           const GLchar *fragmentSource);
void setCommonUniforms(GLuint shaderProgram, int type);

GLenum err;

int numTris = 0;

const GLchar *vertexShaderSource_GouraudAD = GLSL(
    330,

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;

    out vec3 mobileColor;

    uniform mat4 model; uniform mat4 view; uniform mat4 projection; uniform mat4 mvpcgl;
    uniform vec3 lightPos; uniform vec3 lightColor; uniform vec3 objectColor;

    void main() {
      gl_Position = projection * view * model * vec4(position, 1.0);
      vec3 ambient = 0.1 * lightColor;
      vec3 norm = normalize(normal);
      vec3 lightDir = normalize(lightPos - vec3(model * vec4(position, 1.0)));
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * lightColor;
      mobileColor = (ambient + diffuse) * objectColor;
    });
const GLchar *vertexShaderSource_GouraudADS = GLSL(
    330,

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;

    out vec3 mobileColor;

    uniform mat4 model; uniform mat4 view; uniform mat4 projection;
    uniform vec3 lightPos; uniform vec3 viewPos; uniform vec3 lightColor;
    uniform vec3 objectColor;

    void main() {
      gl_Position = projection * view * model * vec4(position, 1.0);
      vec3 ambient = 0.1 * lightColor;
      vec3 norm = normalize(normal);
      vec3 fragPos = vec3(model * vec4(position, 1.0));
      vec3 lightDir = normalize(lightPos - fragPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * lightColor;
      vec3 viewDir = normalize(viewPos - fragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
      vec3 specular = 0.5 * spec * lightColor;
      mobileColor = (ambient + diffuse + specular) * objectColor;
    });
const GLchar *vertexShaderSource_Phong = GLSL(
    330,

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;

    out vec3 FragPos; out vec3 Normal;

    uniform mat4 model; uniform mat4 view; uniform mat4 projection;

    void main() {
      FragPos = vec3(model * vec4(position, 1.0));
      Normal = mat3(transpose(inverse(model))) * normal;
      gl_Position = projection * view * vec4(FragPos, 1.0);
    });
const GLchar *vertexShaderSource_NoShading = GLSL(
    330,

    layout(location = 0) in vec3 position;
    layout(location = 1) in vec3 normal;

    out vec3 mobileColor; uniform mat4 model; uniform mat4 view;
    uniform mat4 projection; uniform vec3 objectColor;

    void main() {
      gl_Position = projection * view * model * vec4(position, 1.0);
      mobileColor = objectColor;
    });
const GLchar *fragmentShaderSource_Phong = GLSL(
    330,

    in vec3 FragPos;
    in vec3 Normal; out vec4 gpuColor;

    uniform vec3 lightPos; uniform vec3 viewPos; uniform vec3 lightColor;
    uniform vec3 objectColor;

    void main() {
      vec3 ambient = 0.1 * lightColor;
      vec3 norm = normalize(Normal);
      vec3 lightDir = normalize(lightPos - FragPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * lightColor;
      vec3 viewDir = normalize(viewPos - FragPos);
      vec3 reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
      vec3 specular = 0.5 * spec * lightColor;
      vec3 result = (ambient + diffuse + specular) * objectColor;
      gpuColor = vec4(result, 1.0);
    });
const GLchar *fragmentShaderSource_Basic = GLSL(
    330,

    in vec3 mobileColor;
    out vec4 gpuColor;

    void main() { gpuColor = vec4(mobileColor, 1.0); });

GLuint shaderProgram_GouraudAD;
GLuint shaderProgram_GouraudADS;
GLuint shaderProgram_Phong;
GLuint shaderProgram_NoShading;

int main(int argc, char *argv[]) {

  while (open != 0 && open != 1) {
    std::cout << "Enter 0 for Close2GL or 1 for OpenGL: ";
    std::cin >> open;
  }

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

  shaderProgram_GouraudAD = createShaderProgram(vertexShaderSource_GouraudAD,
                                                fragmentShaderSource_Basic);
  shaderProgram_GouraudADS = createShaderProgram(vertexShaderSource_GouraudADS,
                                                 fragmentShaderSource_Basic);
  shaderProgram_Phong =
      createShaderProgram(vertexShaderSource_Phong, fragmentShaderSource_Phong);
  shaderProgram_NoShading = createShaderProgram(vertexShaderSource_NoShading,
                                                fragmentShaderSource_Basic);

  shaderProgram = shaderProgram_NoShading;

  MeshData triangles = readfile(filename);

  std::cout << "Loaded " << triangles.numTris << " triangles.\n";

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

  glUseProgram(shaderProgram);

  tris = triangles.tris;
  numTris = triangles.numTris;

  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL Error: " << err << std::endl;
  }

  modelCenter = calculateCenter();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  if (open == 1)
    glutDisplayFunc(URenderGraphics);
  else
    glutDisplayFunc(URenderGraphicsClose2GL);

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

void URenderGraphics(void) {

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGLUT_NewFrame();
  ImGui::NewFrame();

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  //
  // Create buffers for vertices and normals
  //
  glCreateBuffers(NumBuffers, Buffers);
  //
  // Bind the vextex buffer, allocate storage for it, and load the vertex
  // coordinates. Then, bind the normal buffer, allocate storage and load normal
  // coordinates
  //
  glBindBuffer(GL_ARRAY_BUFFER, Buffers[VtxBuffer]);
  glBufferStorage(
      GL_ARRAY_BUFFER, numTris * 9 * sizeof(GL_FLOAT), Vert,
      GL_DYNAMIC_STORAGE_BIT); // Vert is the array with coordinates of
  // the vertices created after reading the file
  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);
  //
  glBindBuffer(GL_ARRAY_BUFFER, Buffers[NormBuffer]);
  glBufferStorage(GL_ARRAY_BUFFER, numTris * 9 * sizeof(GL_FLOAT), Vert_Normal,
                  GL_DYNAMIC_STORAGE_BIT); // Vert_Normal is the
  // array with vertex normals created after reading the file
  glVertexAttribPointer(vNormalVertex, 3, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vNormalVertex);
  //
  // set the number of vertices, which will be used in the openglDisplay
  // function for calling glDrawArrays

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);

  NumVertices = numTris * 3;

  glBindVertexArray(0);

  glutMouseWheelFunc(mouseWheel);

  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(VAO);

  glm::mat4 model(1.0f);
  model = glm::translate(model, -modelCenter);

  glm::mat4 view(1.0f);

  // Locked camera
  if (freeCam == 0) {

    if (key_is_pressed(XK_a)) {
      yaw -= 1;
    }
    if (key_is_pressed(XK_d)) {
      yaw += 1;
    }
    if (key_is_pressed(XK_w)) {
      pitch += 1;
    }
    if (key_is_pressed(XK_s)) {
      pitch -= 1;
    }

    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 direction;
    direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    direction.y = sin(glm::radians(pitch));
    direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

    cameraPos = target - glm::normalize(direction) * distcam;

    cameraFront = glm::normalize(target - cameraPos);
    view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
  }

  // Free camera
  if (freeCam == 1) {
    glutMouseFunc(mouseButtonCallback);
    glutMotionFunc(mouseCallback);
    if (key_is_pressed(XK_a)) {
      cameraPos -=
          glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key_is_pressed(XK_d)) {
      cameraPos +=
          glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key_is_pressed(XK_w)) {
      cameraPos += cameraSpeed * cameraFront;
    }
    if (key_is_pressed(XK_s)) {
      cameraPos -= cameraSpeed * cameraFront;
    }
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }

  glm::mat4 projection;
  projection = glm::perspective(
      45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, nearplane, farplane);

  GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
  GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
  GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
  GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
  GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
  GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

  glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
  glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
  glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

  glm::vec3 objectColor(modelColor[0], modelColor[1], modelColor[2]);
  glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));

  glutPostRedisplay();
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, NumVertices);

  ImGui::Begin("Viewer Controls");
  ImGui::Text("FPS: %.2f", fps);
  ImGui::Text("Camera mode: %s", freeCam ? "FreeCam" : "LockedCam");
  ImGui::SliderFloat("Near Plane", &nearplane, 0.1f, farplane - 0.1f);
  ImGui::SliderFloat("Far Plane", &farplane, nearplane + 0.1f, 1000.0f);
  ImGui::ColorEdit3("Model Color", modelColor);
  ImGui::SliderFloat3("Light Position", &lightPos[0], -200.0f, 200.0f);
  ImGui::Checkbox("Free Camera", (bool *)&freeCam);
  ImGui::Text("Shading Mode:");
  if (ImGui::Button("Gouraud AD")) {
    shaderProgram = shaderProgram_GouraudAD;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("Gouraud ADS")) {
    shaderProgram = shaderProgram_GouraudADS;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("Phong")) {
    shaderProgram = shaderProgram_Phong;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("No Shading")) {
    shaderProgram = shaderProgram_NoShading;
    setCommonUniforms(shaderProgram, 1);
  }
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
  UpdateFPS();
}

void URenderGraphicsClose2GL(void) {
  glutMouseFunc(ImGui_ImplGLUT_MouseFunc);
  glutMotionFunc(ImGui_ImplGLUT_MotionFunc);
  glutPassiveMotionFunc(ImGui_ImplGLUT_MotionFunc);
  glutMouseWheelFunc(mouseWheel);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGLUT_NewFrame();
  ImGui::NewFrame();

  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4c2gl modelcgl(1.0f);
  modelcgl[0][0] = 1.0f;
  modelcgl[1][1] = 1.0f;
  modelcgl[2][2] = 1.0f;
  modelcgl[3][3] = 1.0f;
  modelcgl[0][3] = -modelCenter.x;
  modelcgl[1][3] = -modelCenter.y;
  modelcgl[2][3] = -modelCenter.z;

  mat4c2gl viewcgl;

  if (freeCam == 0) {
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
  } else {
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
  }

  mat4c2gl projectioncgl;
  projectioncgl.setProjectionMatrixHV(vfov, hfov, nearplane, farplane);

  GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
  GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
  GLint projLoc = glGetUniformLocation(shaderProgram, "projection");


  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glCreateBuffers(NumBuffers, Buffers);

  std::vector<GLfloat> culledVertices;
  std::vector<GLfloat> culledNormals;

  mat4c2gl mvpcgl = projectioncgl * viewcgl * modelcgl;
  mat4c2gl mvcgl = viewcgl * modelcgl;

  culledVertices.clear();
  culledNormals.clear();

  bool cullClockwise = false;

  for (int i = 0; i < numTris; ++i) {
    glm::vec3 v0 = glm::make_vec3(&Vert[i * 9 + 0]);
    glm::vec3 v1 = glm::make_vec3(&Vert[i * 9 + 3]);
    glm::vec3 v2 = glm::make_vec3(&Vert[i * 9 + 6]);

    glm::vec4 clip0 = mvpcgl * glm::vec4(v0, 1.0f);
    glm::vec4 clip1 = mvpcgl * glm::vec4(v1, 1.0f);
    glm::vec4 clip2 = mvpcgl * glm::vec4(v2, 1.0f);

    glm::vec3 ndc0 = glm::vec3(clip0) / clip0.w;
    glm::vec3 ndc1 = glm::vec3(clip1) / clip1.w;
    glm::vec3 ndc2 = glm::vec3(clip2) / clip2.w;

    auto inFrustum = [](const glm::vec3 &ndc) {
      return ndc.x >= -1 && ndc.x <= 1 && ndc.y >= -1 && ndc.y <= 1 &&
             ndc.z >= -1 && ndc.z <= 1;
    };

    if (!(inFrustum(ndc0) || inFrustum(ndc1) || inFrustum(ndc2)))
      continue;

    glm::vec3 mv_v0 = glm::vec3(mvcgl * glm::vec4(v0, 1.0f));
    glm::vec3 mv_v1 = glm::vec3(mvcgl * glm::vec4(v1, 1.0f));
    glm::vec3 mv_v2 = glm::vec3(mvcgl * glm::vec4(v2, 1.0f));

    glm::vec3 v0v1 = mv_v1 - mv_v0;
    glm::vec3 v0v2 = mv_v2 - mv_v0;
    glm::vec3 normal = glm::normalize(glm::cross(v0v1, v0v2));
    glm::vec3 viewDir = glm::normalize(mv_v0);

    float facing = glm::dot(normal, viewDir);
    if ((cullClockwise && facing >= 0) || (!cullClockwise && facing <= 0))
      continue;

    for (int j = 0; j < 9; ++j)
      culledVertices.push_back(Vert[i * 9 + j]);
    for (int j = 0; j < 9; ++j)
      culledNormals.push_back(Vert_Normal[i * 9 + j]);
  }

  glUniformMatrix4fv(mvpcglLoc, 1, GL_FALSE, mvpcgl.toGLMatrix().data());
  glUniformMatrix4fv(mvcglLoc, 1, GL_FALSE, mvcgl.toGLMatrix().data());

  GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
  GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
  GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
  GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");

  glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
  glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
  glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

  glm::vec3 objectColor(modelColor[0], modelColor[1], modelColor[2]);
  glUniform3fv(objectColorLoc, 1, glm::value_ptr(objectColor));

  NumVertices = culledVertices.size() / 3;

  glBindBuffer(GL_ARRAY_BUFFER, Buffers[VtxBuffer]);
  glBufferStorage(GL_ARRAY_BUFFER, culledVertices.size() * sizeof(GLfloat),
                  culledVertices.data(), GL_DYNAMIC_STORAGE_BIT);

  glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vPosition);

  glBindBuffer(GL_ARRAY_BUFFER, Buffers[NormBuffer]);
  glBufferStorage(GL_ARRAY_BUFFER, culledNormals.size() * sizeof(GLfloat),
                  culledNormals.data(), GL_DYNAMIC_STORAGE_BIT);

  glVertexAttribPointer(vNormalVertex, 3, GL_FLOAT, GL_FALSE, 0,
                        BUFFER_OFFSET(0));
  glEnableVertexAttribArray(vNormalVertex);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);

  glBindVertexArray(0);

  glutPostRedisplay();
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES, 0, NumVertices);

  ImGui::Begin("Viewer Controls");
  ImGui::Text("FPS: %.2f", fps);
  ImGui::Text("Camera mode: %s", freeCam ? "FreeCam" : "LockedCam");
  ImGui::SliderFloat("Near Plane", &nearplane, 0.1f, farplane - 0.1f);
  ImGui::SliderFloat("Far Plane", &farplane, nearplane + 0.1f, 1000.0f);
  ImGui::SliderFloat("Hfov", &hfov, 0.0f, 200.0f);
  ImGui::SliderFloat("Vfov", &vfov, 00.0f, 200.0f);
  ImGui::ColorEdit3("Model Color", modelColor);
  ImGui::SliderFloat3("Light Position", &lightPos[0], -200.0f, 200.0f);
  ImGui::Checkbox("Free Camera", (bool *)&freeCam);
  ImGui::Text("Shading Mode:");
  if (ImGui::Button("Gouraud AD")) {
    shaderProgram = shaderProgram_GouraudAD;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("Gouraud ADS")) {
    shaderProgram = shaderProgram_GouraudADS;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("Phong")) {
    shaderProgram = shaderProgram_Phong;
    setCommonUniforms(shaderProgram, 0);
  }
  if (ImGui::Button("No Shading")) {
    shaderProgram = shaderProgram_NoShading;
    setCommonUniforms(shaderProgram, 1);
  }
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
  UpdateFPS();
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

void UpdateFPS() {
  frameCount++;
  auto currentTime = std::chrono::high_resolution_clock::now();
  double elapsed =
      std::chrono::duration<double>(currentTime - lastTime).count();

  if (elapsed >= 1.0) { // a cada 1 segundo
    fps = frameCount / elapsed;
    frameCount = 0;
    lastTime = currentTime;
  }
}

GLuint compileShader(GLenum type, const GLchar *source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << "Shader Compilation Error:\n" << infoLog << std::endl;
  }

  return shader;
}

GLuint createShaderProgram(const GLchar *vertexSource,
                           const GLchar *fragmentSource) {
  GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
  GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "Shader Linking Error:\n" << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return program;
}

void setCommonUniforms(GLuint shaderProgram, int type) {
  glUseProgram(shaderProgram);

  GLint objColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
  glUniform3f(objColorLoc, 1.0f, 0.0f, 0.0f);

  if (type != 1) {
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    glUniform3f(lightPosLoc, 2.0f, 4.0f, 1.0f);

    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    glUniform3f(viewPosLoc, 0.0f, 0.0f, 5.0f);
  }
}