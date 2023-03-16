/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: Jinsen Wu
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <glm/gtx/string_cast.hpp>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
char textureShaderBasePath[1024] = "../textureShader";
#endif

using namespace std;

// forward declaration
// fill positions and color array
void get_vertices();
void fill_lines(GLuint &ebo);

void generate_point(struct Pos &coords, vector<float> &position, vector<float> &color, vector<struct Frenet> &f);
void generate_cross_section(vector<struct Frenet> &f, vector<float> &position);
void generate_cross_section_color(vector<float> &position, vector<float> &color);
void do_vertex(struct Pos &coords);
void push_glm_to_vector(glm::vec3 &g, vector<float> &vec);
glm::vec3 find_triangle_normal(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);
void fill_cross_section(GLuint &ebo);
glm::vec3 find_point(vector<float> &position, int index);
glm::vec3 pseudo_normal(glm::vec3 n1, glm::vec3 n2);
void push_glm_to_color(glm::vec3 &n, vector<float> &color);
void fill_ground();

// set up vbo and vao
void set_one_vbo_one_vao(vector<float> &position, vector<float> &color, GLuint &vao);
void set_ebo(vector<int> &indexes, GLuint &ebo);

// background image
void set_matrix();

// hw2
struct Pos catmull_rom(float u, glm::mat3x4 &m);
void subdivide(float u0, float u1, float max_line_length, glm::mat3x4 &m);
void calculate_fernet(struct Frenet &f);

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0;   // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0;  // 1 if pressed, 0 if not

// Transformation mode
typedef enum
{
  ROTATE,
  TRANSLATE,
  SCALE
} CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// Color mode
typedef enum
{
  GRAYSCALE,
  RGB
} COLOR_MODE;
COLOR_MODE color_mode = GRAYSCALE;

// Drawing mode
typedef enum
{
  POINT,
  LINE,
  SMOOTH,
  SOLID_WIREFRAME,
} MODE_STATE;
MODE_STATE mode = POINT;

// animation mode
int animation = 0;
int counter = 0;

// state of the world
float landRotate[3] = {0.0f, 0.0f, 0.0f};
float landTranslate[3] = {0.0f, 0.0f, 0.0f};
float landScale[3] = {1.0f, 1.0f, 1.0f};

// attributes for window and image
int windowWidth = 1280;
int windowHeight = 720;
int image_width = 0, image_height = 0;
int image_center_x = 0, image_center_y = 0;
float height = 0;
float scale = 0.1;
float red = 0, green = 0, blue = 0, alpha = 1;

char windowTitle[512] = "CSCI 420 homework II";

// Input images

// vertices
vector<float> vertices, vertex_colors;
GLuint vao_vertices;

// lines
GLuint ebo_line;

// rail cross section
vector<float> cross_section_vertices, cross_section_vertex_colors;
GLuint vao_cross_section_vertices, ebo_cross_section_vertices;

// groud
GLuint vao_ground, ebo_ground;

OpenGLMatrix matrix;
BasicPipelineProgram *pipelineProgram;
BasicPipelineProgram *texturePipelineProgram;

// HW2
// represents one control point along the spline
struct Point
{
  double x;
  double y;
  double z;
};
// spline struct
// contains how many control points the spline has, and an array of control points
struct Spline
{
  int numControlPoints;
  Point *points;
};

// Coordinates and tangents for each position
struct Pos
{
  glm::vec3 position;
  glm::vec3 position_tan;
};

// Frenet Frame
struct Frenet
{
  glm::vec3 point;
  glm::vec3 tangent;
  glm::vec3 binormal;
  glm::vec3 normal;
};

vector<Frenet> frenets;

// the spline array
Spline *splines;
// total number of splines
int numSplines;

float s = 0.5;
glm::mat4 basis = glm::mat4(
    -s, 2 * s, -s, 0,
    2 - s, s - 3, 0, 1,
    s - 2, 3 - 2 * s, s, 0,
    s, -s, 0, 0);

int loadSplines(char *argv)
{
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file
  fileList = fopen(argv, "r");
  if (fileList == NULL)
  {
    printf("fileList can't open file\n");
    exit(1);
  }

  // stores the number of splines in a global variable
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline *)malloc(numSplines * sizeof(Spline));

  // reads through the spline files
  for (j = 0; j < numSplines; j++)
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL)
    {
      printf("fileSpline can't open file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf",
                  &splines[j].points[i].x,
                  &splines[j].points[i].y,
                  &splines[j].points[i].z) != EOF)
    {
      i++;
    }
  }

  free(cName);

  return 0;
}

int initTexture(const char *imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK)
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4)
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char *pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++)
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0;   // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0;   // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0;   // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0)
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }

  // de-allocate the pixel array -- it is no longer needed
  delete[] pixelsRGBA;

  return 0;
}

// write a screenshot to the specified filename
void saveScreenshot(const char *filename)
{
  unsigned char *screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    std::cout << "File " << filename << " saved successfully." << endl;
  else
    std::cout << "Failed to save file " << filename << '.' << endl;

  delete[] screenshotData;
}

void displayFunc()
{
  // render some stuff...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();

  // eye_z is based on the input image dimension
  matrix.LookAt(5.0, 10.0, 15.0,
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);

  int index = counter % frenets.size();
  Frenet frenet = frenets[index];
  glm::vec3 np = frenet.point + frenet.tangent;

  float offset = 0;

  // matrix.LookAt(frenet.point.x, frenet.point.y, frenet.point.z + offset,
  //               np.x, np.y, np.z + offset,
  //               frenet.normal.x, frenet.normal.y, frenet.normal.z);

  // Transformation
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.Rotate(landRotate[0], 1, 0, 0);
  matrix.Rotate(landRotate[1], 0, 1, 0);
  matrix.Rotate(landRotate[2], 0, 0, 1);
  matrix.Scale(landScale[0], landScale[1], landScale[2]);

  // bind shader
  pipelineProgram->Bind();

  set_matrix();

  // get loc for mode in vertex shader
  GLuint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");

  // drawing mode
  switch (mode)
  {
  default:
    glUniform1i(loc, 0);

    glBindVertexArray(vao_vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_line);
    glDrawElements(GL_LINES, frenets.size() * 2, GL_UNSIGNED_INT, 0);

    glBindVertexArray(vao_cross_section_vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_cross_section_vertices);
    glDrawElements(GL_TRIANGLES, cross_section_vertices.size() * 3, GL_UNSIGNED_INT, 0);

    glBindVertexArray(vao_ground);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_ground);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    break;
  }

  glutSwapBuffers();
}

void idleFunc()
{

  if (animation == 1)
  {
    ++counter;
  }

  // make the screen update
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(60.0f, (float)w / (float)h, 0.01f, 1000.0f);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = {x - mousePos[0], y - mousePos[1]};

  switch (controlState)
  {
  // translate the landscape
  case TRANSLATE:
    if (leftMouseButton)
    {
      // control x,y translation via the left mouse button
      landTranslate[0] += mousePosDelta[0] * 0.05f;
      landTranslate[1] -= mousePosDelta[1] * 0.05f;
    }
    if (middleMouseButton)
    {
      // control z translation via the middle mouse button
      landTranslate[2] += mousePosDelta[1] * 0.05f;
    }
    break;

  // rotate the landscape
  case ROTATE:
    if (leftMouseButton)
    {
      // control x,y rotation via the left mouse button
      landRotate[0] += mousePosDelta[1];
      landRotate[1] += mousePosDelta[0];
    }
    if (middleMouseButton)
    {
      // control z rotation via the middle mouse button
      landRotate[2] += mousePosDelta[1];
    }
    break;

  // scale the landscape
  case SCALE:
    if (leftMouseButton)
    {
      // control x,y scaling via the left mouse button
      landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
      landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
    }
    if (middleMouseButton)
    {
      // control z scaling via the middle mouse button
      landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
    }
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
  case GLUT_LEFT_BUTTON:
    leftMouseButton = (state == GLUT_DOWN);
    break;

  case GLUT_MIDDLE_BUTTON:
    middleMouseButton = (state == GLUT_DOWN);
    break;

  case GLUT_RIGHT_BUTTON:
    rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
  case GLUT_ACTIVE_CTRL:
    controlState = TRANSLATE;
    break;

  case GLUT_ACTIVE_SHIFT:
    controlState = SCALE;
    break;

  // if CTRL and SHIFT are not pressed, we are in rotate mode
  default:
    controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{

  switch (key)
  {
  case 27:   // ESC key
    exit(0); // exit the program
    break;

  case ' ':
    animation = 1 - animation;
    std::cout << "You pressed the spacebar." << endl;
    break;

  case 'x':
    // take a screenshot
    saveScreenshot("screenshot.jpg");
    break;

  // GLUT_ACTIVE_CTRL and GLUT_ACTIVE_ALT doesn't work on Mac
  // So we use 't' for translate
  case 't':
    controlState = TRANSLATE;
    break;
  }
}

void initScene(int argc, char *argv[])
{

  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // initialize program pipeline
  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  pipelineProgram->Bind();
  if (ret != 0)
    abort();

  texturePipelineProgram = new BasicPipelineProgram;
  ret = texturePipelineProgram->Init(textureShaderBasePath);
  texturePipelineProgram->Bind();
  if (ret != 0)
    abort();

  get_vertices();
  fill_lines(ebo_line);
  fill_cross_section(ebo_cross_section_vertices);
  fill_ground();

  glEnable(GL_DEPTH_TEST);

  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  // load the splines from the provided filename
  loadSplines(argv[1]);

  std::cout << "Initializing GLUT..." << endl;
  glutInit(&argc, argv);

  std::cout << "Initializing OpenGL..." << endl;

  printf("Loaded %d spline(s).\n", numSplines);
  for (int i = 0; i < numSplines; i++)
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

#ifdef __APPLE__
  glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

#ifdef __APPLE__
  // This is needed on recent Mac OS X versions to correctly display the window.
  glutReshapeWindow(windowWidth - 1, windowHeight - 1);
#endif

  // tells glut to use a particular display function to redraw
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

// init glew
#ifdef __APPLE__
  // nothing is needed on Apple
#else
  // Windows, Linux
  GLint result = glewInit();
  if (result != GLEW_OK)
  {
    cout << "error: " << glewGetErrorString(result) << endl;
    exit(EXIT_FAILURE);
  }
#endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}

void get_vertices()
{
  vertices.clear();
  vertex_colors.clear();

  // 3 columns, 4 rows
  glm::mat3x4 control, m;
  glm::vec3 position, init_pos;
  Point p1, p2, p3, p4;

  float max_line_length = 0.1;

  for (int s = 0; s < numSplines; ++s)
  {

    Spline spline = splines[s];

    for (int i = 0; i < spline.numControlPoints - 3; ++i)
    {
      p1 = spline.points[i];
      p2 = spline.points[i + 1];
      p3 = spline.points[i + 2];
      p4 = spline.points[i + 3];

      control = glm::mat3x4(
          p1.x, p2.x, p3.x, p4.x,
          p1.y, p2.y, p3.y, p4.y,
          p1.z, p2.z, p3.z, p4.z);

      m = basis * control;

      subdivide(0, 1, max_line_length, m);
    }
  }

  set_one_vbo_one_vao(vertices, vertex_colors, vao_vertices);

  generate_cross_section_color(cross_section_vertices, cross_section_vertex_colors);
  cout << "cross_section_vertices size: " << cross_section_vertices.size() / 3 << " cross_section_vertex_colors: " << cross_section_vertex_colors.size() / 3 << '\n';
  set_one_vbo_one_vao(cross_section_vertices, cross_section_vertex_colors, vao_cross_section_vertices);
}

/* Generate points for line mode */
void fill_lines(GLuint &ebo)
{
  vector<int> lines;

  int index = 1;
  for (; index < vertices.size() / 3; ++index)
  {
    lines.push_back(index - 1);
    lines.push_back(index);
  }

  lines.push_back(index - 1);
  lines.push_back(0);

  cout << "spline size: " << frenets.size() << '\n';
  cout << "lines size: " << lines.size() << '\n';

  set_ebo(lines, ebo);
}

void fill_cross_section(GLuint &ebo)
{
  vector<int> cross_section;

  int i = 0, num_vertices = vertices.size() / 3;
  for (; i < num_vertices; ++i)
  {

    int v0 = i * 4;
    int v1 = v0 + 1;
    int v2 = v0 + 2;
    int v3 = v0 + 3;

    int v4 = ((i + 1) % num_vertices) * 4;
    int v5 = v4 + 1;
    int v6 = v4 + 2;
    int v7 = v4 + 3;

    cross_section.push_back(v0);
    cross_section.push_back(v4);
    cross_section.push_back(v5);

    cross_section.push_back(v0);
    cross_section.push_back(v5);
    cross_section.push_back(v1);

    cross_section.push_back(v1);
    cross_section.push_back(v5);
    cross_section.push_back(v6);

    cross_section.push_back(v1);
    cross_section.push_back(v6);
    cross_section.push_back(v2);

    cross_section.push_back(v2);
    cross_section.push_back(v6);
    cross_section.push_back(v3);

    cross_section.push_back(v3);
    cross_section.push_back(v6);
    cross_section.push_back(v7);

    cross_section.push_back(v3);
    cross_section.push_back(v7);
    cross_section.push_back(v0);

    cross_section.push_back(v0);
    cross_section.push_back(v7);
    cross_section.push_back(v4);
  }

  cout << "cross_section size: " << cross_section.size() << '\n';
  set_ebo(cross_section, ebo);
}

void fill_ground() {

  vector<float> grounds, colors;
  vector<int> indexes;
  float c = 0.0f;

  // 0 | 1, 1
  grounds.push_back(1000.0f);
  grounds.push_back(-10.0f);
  grounds.push_back(1000.0f);

  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(alpha);

  // 1 | -1, 1
  grounds.push_back(-1000.0f);
  grounds.push_back(-10.0f);
  grounds.push_back(1000.0f);

  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(alpha);

  // 2 | 1, 1
  grounds.push_back(1000.0f);
  grounds.push_back(-10.0f);
  grounds.push_back(-1000.0f);

  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(alpha);

  // 3 | -1, -1
  grounds.push_back(-1000.0f);
  grounds.push_back(-10.0f);
  grounds.push_back(-1000.0f);

  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(c);
  colors.push_back(alpha);

  set_one_vbo_one_vao(grounds, colors, vao_ground);

  indexes.push_back(0);
  indexes.push_back(1);
  indexes.push_back(2);

  indexes.push_back(1);
  indexes.push_back(3);
  indexes.push_back(2);

  set_ebo(indexes, ebo_ground);

}

struct Pos catmull_rom(float u, glm::mat3x4 &m)
{

  glm::vec4 us = glm::vec4(powf(u, 3.0f), powf(u, 2.0f), u, 1);
  glm::vec4 us_tan = glm::vec4(3 * powf(u, 2.0f), 2 * u, 1, 0);
  glm::vec3 position = us * m;
  glm::vec3 position_tan = glm::normalize(us_tan * m);

  Pos pos = {position, position_tan};

  return pos;
}

void set_ebo(vector<int> &indexes, GLuint &ebo)
{

  int size = sizeof(int) * indexes.size();

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indexes.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

/* set up a generic vbo and vao */
void set_one_vbo_one_vao(vector<float> &position, vector<float> &color, GLuint &vao)
{
  // Set up vertices and color in buffer
  int size = sizeof(float) * position.size();
  int color_size = sizeof(float) * color.size();
  uintptr_t offset = 0;

  // bind vertices and colors with vbo
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size + color_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, position.data());
  glBufferSubData(GL_ARRAY_BUFFER, size, color_size, color.data());

  // set up vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // get the location of the shader variable "position"
  // enable it then set attributes
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // get the location of the shader variable "color"
  offset += size;
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void do_vertex(struct Pos &coords)
{
  generate_point(coords, vertices, vertex_colors, frenets);
  generate_cross_section(frenets, cross_section_vertices);
}

/* Universal vertices position and color generator */
void generate_point(struct Pos &coords, vector<float> &position, vector<float> &color, vector<struct Frenet> &f)
{

  float c = 0.0f;
  glm::vec3 p = coords.position;
  glm::vec3 t = coords.position_tan;

  // assign positions and colors
  position.push_back(p.x);
  position.push_back(p.y);
  position.push_back(p.z);
  color.push_back(c);
  color.push_back(c);
  color.push_back(c);
  color.push_back(alpha);

  // assign frenet
  Frenet frenet;

  frenet.point = p;
  frenet.tangent = t;

  if (f.size() == 0)
  {
    frenet.normal = glm::normalize(glm::cross(frenet.tangent, glm::vec3(1, 1, 1)));

    if (glm::isnan(frenet.normal.x)) {
      frenet.normal = glm::normalize(glm::cross(frenet.tangent, glm::vec3(0, 1, 0)));
    }

    frenet.binormal = glm::normalize(glm::cross(frenet.tangent, frenet.normal));
  }
  else
  {
    frenet.binormal = f.back().binormal;
    frenet.normal = f.back().normal;
    calculate_fernet(frenet);
  }

  // cout << "binormal: " << glm::to_string(frenet.binormal) << " normal: " << glm::to_string(frenet.normal) << '\n';

  f.push_back(frenet);
}

void generate_cross_section(vector<struct Frenet> &f, vector<float> &position)
{

  float a = 0.02f;

  Frenet &frenet = f.back();
  glm::vec3 &p = frenet.point;
  glm::vec3 &n = frenet.normal;
  glm::vec3 &b = frenet.binormal;

  glm::vec3 v0 = p + a * (n * -1.0f + b);
  glm::vec3 v1 = p + a * (n + b);
  glm::vec3 v2 = p + a * (n + b * -1.0f);
  glm::vec3 v3 = p + a * (n * -1.0f + b * -1.0f);

  push_glm_to_vector(v0, position);
  push_glm_to_vector(v1, position);
  push_glm_to_vector(v2, position);
  push_glm_to_vector(v3, position);

}

void push_glm_to_vector(glm::vec3 &g, vector<float> &vec)
{
  vec.push_back(g.x);
  vec.push_back(g.y);
  vec.push_back(g.z);
}

void generate_cross_section_color(vector<float> &position, vector<float> &color)
{

  int i = 0, num_vertices = vertices.size() / 3;
  float c = 0.0f;

  for (; i < num_vertices; ++i)
  {
    int index = i * 4;
    int i0 = index * 3;
    int i1 = i0 + 3;
    int i2 = i1 + 3;
    int i3 = i2 + 3;

    index = ((i + 1) % num_vertices) * 4;
    int i4 = index * 3;
    int i5 = i4 + 3;
    int i6 = i5 + 3;
    int i7 = i6 + 3;

    glm::vec3 p0 = find_point(position, i0);
    glm::vec3 p1 = find_point(position, i1);
    glm::vec3 p2 = find_point(position, i2);
    glm::vec3 p3 = find_point(position, i3);
    glm::vec3 p4 = find_point(position, i4);
    glm::vec3 p5 = find_point(position, i5);
    glm::vec3 p6 = find_point(position, i6);
    glm::vec3 p7 = find_point(position, i7);

    glm::vec3 t0 = find_triangle_normal(p0, p4, p5);
    glm::vec3 t1 = find_triangle_normal(p0, p7, p4);
    glm::vec3 t2 = find_triangle_normal(p1, p0, p5);
    glm::vec3 t3 = find_triangle_normal(p1, p5, p6);
    glm::vec3 t4 = find_triangle_normal(p2, p1, p6);
    glm::vec3 t5 = find_triangle_normal(p2, p6, p3);
    glm::vec3 t6 = find_triangle_normal(p3, p6, p7);
    glm::vec3 t7 = find_triangle_normal(p3, p7, p0);

    glm::vec3 n0 = pseudo_normal(t0, t1);
    glm::vec3 n1 = pseudo_normal(t2, t3);
    glm::vec3 n2 = pseudo_normal(t4, t5);
    glm::vec3 n3 = pseudo_normal(t6, t7);
 
    push_glm_to_color(t0, color);
    push_glm_to_color(t2, color);
    push_glm_to_color(t4, color);
    push_glm_to_color(t6, color);

  }
}

void push_glm_to_color(glm::vec3 &n, vector<float> &color)
{
  color.push_back(n.x);
  color.push_back(n.y);
  color.push_back(n.z);
  color.push_back(alpha);
}

glm::vec3 pseudo_normal(glm::vec3 n1, glm::vec3 n2) {
  return glm::normalize(glm::cross(n1, n2));
}

glm::vec3 find_point(vector<float> &position, int index) {
  return glm::vec3(position[index], position[index + 1], position[index + 2]);
}

glm::vec3 find_triangle_normal(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3)
{

  glm::vec3 v1 = p2 - p1;
  glm::vec3 v2 = p3 - p2;

  glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

  return normal;
}

void set_matrix()
{
  float m[16], p[16];

  // set up ModelView
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(m);

  // set up Projection
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(p);

  // set variable
  pipelineProgram->SetModelViewMatrix(m);
  pipelineProgram->SetProjectionMatrix(p);
}

void subdivide(float u0, float u1, float max_line_length, glm::mat3x4 &m)
{

  float umid = (u0 + u1) / 2;
  Pos x0 = catmull_rom(u0, m);
  Pos x1 = catmull_rom(u1, m);

  if (glm::length(x1.position - x0.position) > max_line_length)
  {
    subdivide(u0, umid, max_line_length, m);
    subdivide(umid, u1, max_line_length, m);
  }
  else
  {
    do_vertex(x0);
  }
}

void calculate_fernet(Frenet &f)
{
  glm::vec3 n = glm::normalize(glm::cross(f.binormal, f.tangent));
  f.normal = n;
  glm::vec3 b = glm::normalize(glm::cross(f.tangent, f.normal));
  f.binormal = b;
}

