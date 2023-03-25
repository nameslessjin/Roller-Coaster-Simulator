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
#include <glm/gtc/type_ptr.hpp>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
char textureShaderBasePath[1024] = "../textureShader";
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
char textureShaderBasePath[1024] = "../textureShader";
#endif

using namespace std;

const char ground_image_file[1024] = "Waterpl.jpg";
const char sky_image_file[1024] = "Natur17l.jpg";
const char ambrosia_image_file[1024] = "Ambrosia.jpg";

// Forward declaration
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
  float tex_inc;
};
vector<Frenet> frenets;
vector<Frenet> frenets_v;

// Vertex Color structs
struct Vertex_Color
{
  vector<float> vertex;
  vector<float> color;
  vector<float> texCoords;
};

struct VAO_VBO
{
  GLuint vao, ebo, texture;
};

// Cross Section structs
struct Cross_Section_Vertex
{
  vector<float> cross_section_vertices;
  Vertex_Color left, right, up, down;
};

struct Cross_Section_Buffer
{
  VAO_VBO left, right, up, down;
};

struct Cross_Section
{

  float alpha = 0.04f;
  int cross_section_side_size = 0;

  Cross_Section_Vertex csv;
  Cross_Section_Buffer csb;

  // T shape
  Cross_Section_Vertex csv_t;
  Cross_Section_Buffer csb_t;
};

// environment plane
struct Plane
{
  GLuint vao;
  GLuint ebo;
  GLuint texture;
  glm::vec3 upper_left;
  glm::vec3 upper_right;
  glm::vec3 bottom_left;
  glm::vec3 bottom_right;
};

struct Environment
{
  Plane ground;
  Plane sky;
  Plane front;
  Plane back;
  Plane left;
  Plane right;
};

typedef enum
{
  X,
  Y,
  Z
} SECTION_DIRECTION;

// fill positions and color array
void get_vertices();
void fill_lines(GLuint &ebo);
void generate_point(Pos &coords, vector<float> &position, vector<float> &color, vector<Frenet> &f, int subdivide_time);
void generate_cross_section_vector(Frenet &f, Cross_Section &cs, float shift, float b_multiplier, bool isVertical);
void generate_cross_section(Cross_Section &cs);
int generate_cross_section_single(Cross_Section_Vertex &csv, Cross_Section_Buffer &buffer, SECTION_DIRECTION sd, int skip);
void do_vertex(Pos &coords, vector<float> &vs, vector<float> &vc, vector<Frenet> &f, int subdivide_time);
void push_glm_to_vector(glm::vec3 &g, vector<float> &vec);
glm::vec3 find_point(vector<float> &position, int index);
void push_glm_to_color(glm::vec3 &n, vector<float> &color);
void fill_plane(vector<float> &plane, GLuint &vao, GLuint &ebo, float repeat_x, float repeat_y);
void push_side_to_vector(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec3 &d, vector<float> &vec);
void push_side_to_color(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec3 &d, vector<float> &color);
void push_cross_section_index(vector<int> &indexes, int i);
void render_normal_binormal();
void render_cross_section(Cross_Section &cs);
void render_cross_section_single(Cross_Section_Buffer &buffer, int size);
void calculate_physical_velocity();
void push_glm_to_color_texture(glm::vec4 &n, vector<float> &color);
void generate_environment_texture(Environment &e);
void render_environment(BasicPipelineProgram *pipeline, Environment &e);
void generate_environment(Environment &e);
void push_side(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &dir, int i, Vertex_Color &cv, vector<int> &csi, float tex_incre);
void render_elements(VAO_VBO &v, int size);
void generate_cross_section_texture(Cross_Section &cs, string filename);
void fill_texCoords(vector<float> &texCoords, float repeat_x, float repeat_y, vector<float> &start, vector<float> &end);
void init_cross_section_texture(Cross_Section &cs, string filename);

// set up vbo and vao
void set_one_vao_basic(BasicPipelineProgram *pipeline, vector<float> &position, vector<float> &color, vector<float> &texCoord, GLuint &vao);
void set_vao_texture(BasicPipelineProgram *pipeline, vector<float> &position, vector<float> &color, vector<float> &texCoord, GLuint &vao);
void set_ebo(vector<int> &indexes, GLuint &ebo);

// background image
void set_matrix(BasicPipelineProgram *pipeline);
void set_light(BasicPipelineProgram *pipeline);

// calculate splines and frenet
struct Pos catmull_rom(float u, glm::mat3x4 &m);
void subdivide(float u0, float u1, float max_line_length, glm::mat3x4 &m, int &subdivide_time);
void calculate_frenet(struct Frenet &f);
void print_frenet(int index);

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0;   // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0;  // 1 if pressed, 0 if not

// speed and velocity control stats
int speed_coe = 1;
float max_line_length = 0.001;
float time_step = 0.0001f / speed_coe;
int default_speed_step = speed_coe * 10;
int speed_step = default_speed_step;
float gravity = 59.8f;

// Transformation mode
typedef enum
{
  ROTATE,
  TRANSLATE,
  SCALE
} CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

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
float alpha = 1.0f;

char windowTitle[512] = "CSCI 420 homework II";

// vertices
vector<float> vertices, vertex_colors, velocity;
GLuint vao_vertices, vao_normal, vao_binormal;
float max_height = INT_MIN * 1.0f; // max height of the track
float min_height = INT_MAX * 1.0f; // min height of the track

// lines
GLuint ebo_line;

// Cross section
Cross_Section cs_l, cs_r, cs_bar, cs_support;
float cross_section_separation = 3.0f;

// environment settings
float l = 5000.0f;    // side of the plane
float min_h = -10.0f; // min height of the environment
float sd = l;         // side distance from the center
float sh = l / 2;     // height of the sky
Environment env;

// pipeline and matrix
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
// the spline array
Spline *splines;

// total number of splines
int numSplines;

// basis
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
  // matrix.LookAt(5.0, 10.0, 15.0,
  //               0.0, 0.0, 0.0,
  //               0.0, 1.0, 0.0);

  // cout << "f: " << frenets.size() << '\n';
  // cout << "fv: " << frenets_v.size() << '\n';
  int index = counter % frenets_v.size();
  Frenet frenet = frenets_v[index];

  glm::vec3 eyes = frenet.point + frenet.normal * 0.5f;
  glm::vec3 focus = eyes + frenet.tangent;
  glm::vec3 up = frenet.normal;

  matrix.LookAt(eyes.x, eyes.y, eyes.z,
                focus.x, focus.y, focus.z,
                up.x, up.y, up.z);

  // bind shader
  pipelineProgram->Bind();
  set_light(pipelineProgram);

  // Transformation
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
  matrix.Rotate(landRotate[0], 1, 0, 0);
  matrix.Rotate(landRotate[1], 0, 1, 0);
  matrix.Rotate(landRotate[2], 0, 0, 1);
  matrix.Scale(landScale[0], landScale[1], landScale[2]);

  set_matrix(pipelineProgram);

  // glBindVertexArray(vao_vertices);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_line);
  // glDrawElements(GL_LINES, frenets.size() * 2, GL_UNSIGNED_INT, 0);

  // draw normal and binormal
  // glBindVertexArray(vao_normal);
  // glDrawArrays(GL_LINES, 0, frenets.size() * 2);
  // glBindVertexArray(vao_binormal);
  // glDrawArrays(GL_LINES, 0, frenets.size() * 2);

  render_cross_section(cs_r);
  render_cross_section(cs_l);
  render_cross_section_single(cs_bar.csb, cs_bar.cross_section_side_size);

  // cout << "cs_support: " << cs_support.csv.cross_section_vertices.size() << '\n';
  render_cross_section_single(cs_support.csb, cs_support.cross_section_side_size);

  // draw environments
  render_environment(texturePipelineProgram, env);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glutSwapBuffers();
}

void render_environment(BasicPipelineProgram *pipeline, Environment &e)
{

  pipeline->Bind();
  set_matrix(pipeline);

  glBindTexture(GL_TEXTURE_2D, env.ground.texture);
  glBindVertexArray(env.ground.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.ground.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindTexture(GL_TEXTURE_2D, env.sky.texture);
  glBindVertexArray(env.sky.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.sky.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindTexture(GL_TEXTURE_2D, env.front.texture);
  glBindVertexArray(env.front.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.front.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindTexture(GL_TEXTURE_2D, env.back.texture);
  glBindVertexArray(env.back.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.back.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindTexture(GL_TEXTURE_2D, env.left.texture);
  glBindVertexArray(env.left.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.left.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  glBindTexture(GL_TEXTURE_2D, env.right.texture);
  glBindVertexArray(env.right.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, env.right.ebo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void idleFunc()
{
  // animating
  if (animation == 1)
  {
    counter += speed_step;
  }

  // make the screen update
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(60.0f, (float)w / (float)h, 0.01f, 10000.0f);
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
    std::cout << (animation ? "Animation Start" : "Animation Stop") << endl;
    break;

  case 'x':
    // take a screenshot
    saveScreenshot("screenshot.jpg");
    break;

  case '.':

    // fast forward
    counter += default_speed_step * 10;
    break;
  case ',':

    // move backward
    counter -= default_speed_step * 10;
    if (counter < 0)
      counter = 0;
    break;

  case '1':

    // set to default speed
    speed_step = default_speed_step;
    break;

  case '2':

    // set to twice of the default speed
    speed_step = default_speed_step * 2;
    break;

  case '3':

    // set to half of the default speed
    speed_step = default_speed_step / 2;
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

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

  generate_cross_section_texture(cs_l, ambrosia_image_file);
  generate_cross_section_texture(cs_r, ambrosia_image_file);
  generate_cross_section_texture(cs_bar, ambrosia_image_file);
  generate_cross_section_texture(cs_support, ambrosia_image_file);

  get_vertices();
  calculate_physical_velocity();
  fill_lines(ebo_line);

  float l = 5000.0f;
  float h = 10.0f;
  float sd = l;
  float sh = l / 2;

  render_normal_binormal();

  generate_environment(env);
  generate_environment_texture(env);
  initTexture(sky_image_file, env.sky.texture);
  initTexture(sky_image_file, env.left.texture);
  initTexture(sky_image_file, env.right.texture);
  initTexture(sky_image_file, env.front.texture);
  initTexture(sky_image_file, env.back.texture);
  initTexture(ground_image_file, env.ground.texture);

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

void init_cross_section_texture(Cross_Section &cs, string filename)
{
  Cross_Section_Buffer &csb = cs.csb;
  Cross_Section_Buffer &csb_t = cs.csb_t;

  initTexture(filename.c_str(), csb.left.texture);
  initTexture(filename.c_str(), csb.right.texture);
  initTexture(filename.c_str(), csb.up.texture);
  initTexture(filename.c_str(), csb.down.texture);

  initTexture(filename.c_str(), csb_t.left.texture);
  initTexture(filename.c_str(), csb_t.right.texture);
  initTexture(filename.c_str(), csb_t.up.texture);
  initTexture(filename.c_str(), csb_t.down.texture);
}

void generate_environment(Environment &e)
{

  // 1, 1 | -1, 1 | 1, -1 | -1, -1
  vector<float> ground_coords{l, l, min_h, -l, l, min_h, l, -l, min_h, -l, -l, min_h};
  vector<float> sky_coords{l, l, sh, -l, l, sh, l, -l, sh, -l, -l, sh};
  vector<float> left_coords{l, sd, sh, -l, sd, sh, l, sd, -sh, -l, sd, -sh};
  vector<float> right_coords{l, -sd, sh, -l, -sd, sh, l, -sd, -sh, -l, -sd, -sh};
  vector<float> front_coords{sd, l, sh, sd, -l, sh, sd, l, -sh, sd, -l, -sh};
  vector<float> back_coords{-sd, l, sh, -sd, -l, sh, -sd, l, -sh, -sd, -l, -sh};

  fill_plane(ground_coords, e.ground.vao, e.ground.ebo, 1000.0f, 1000.0f);
  fill_plane(sky_coords, e.sky.vao, e.sky.ebo, 1.0f, 1.0f);
  fill_plane(left_coords, e.left.vao, e.left.ebo, 1.0f, 1.0f);
  fill_plane(right_coords, e.right.vao, e.right.ebo, 1.0f, 1.0f);
  fill_plane(front_coords, e.front.vao, e.front.ebo, 1.0f, 1.0f);
  fill_plane(back_coords, e.back.vao, e.back.ebo, 1.0f, 1.0f);
}

void generate_environment_texture(Environment &e)
{
  glGenTextures(1, &e.ground.texture);
  glGenTextures(1, &e.sky.texture);
  glGenTextures(1, &e.front.texture);
  glGenTextures(1, &e.back.texture);
  glGenTextures(1, &e.left.texture);
  glGenTextures(1, &e.right.texture);
}

void generate_cross_section_texture(Cross_Section &cs, string filename)
{

  Cross_Section_Buffer &csb = cs.csb;
  Cross_Section_Buffer &csb_t = cs.csb_t;

  glGenTextures(1, &csb.left.texture);
  glGenTextures(1, &csb.right.texture);
  glGenTextures(1, &csb.up.texture);
  glGenTextures(1, &csb.down.texture);
  glGenTextures(1, &csb_t.left.texture);
  glGenTextures(1, &csb_t.right.texture);
  glGenTextures(1, &csb_t.up.texture);
  glGenTextures(1, &csb_t.down.texture);

  init_cross_section_texture(cs, filename);
}

void render_cross_section(Cross_Section &cs)
{
  render_cross_section_single(cs.csb, cs.cross_section_side_size);
  render_cross_section_single(cs.csb_t, cs.cross_section_side_size);
}

void render_cross_section_single(Cross_Section_Buffer &buffer, int size)
{

  render_elements(buffer.right, size);
  render_elements(buffer.up, size);
  render_elements(buffer.left, size);
  render_elements(buffer.down, size);
}

void render_elements(VAO_VBO &v, int size)
{
  glBindTexture(GL_TEXTURE_2D, v.texture);
  glBindVertexArray(v.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v.ebo);
  glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
}

void get_vertices()
{
  vertices.clear();
  vertex_colors.clear();

  // 3 columns, 4 rows
  glm::mat3x4 control, m;
  glm::vec3 position, point, p1, p2, p3, p4;
  vector<glm::vec3> points;
  Point p;

  for (int s = 0; s < numSplines; ++s)
  {

    Spline spline = splines[s];
    float u = 0.0f;
    int numControlPoints = spline.numControlPoints;

    for (int i = 0; i < numControlPoints; ++i)
    {
      p = spline.points[i];
      point.x = p.x;
      point.y = p.y;
      point.z = p.z;
      points.push_back(point);
    }

    for (int i = 0; i < numControlPoints; ++i)
    {
      p1 = points[i];
      p2 = points[(i + 1) % numControlPoints];
      p3 = points[(i + 2) % numControlPoints];
      p4 = points[(i + 3) % numControlPoints];

      control = glm::mat3x4(
          p1.x, p2.x, p3.x, p4.x,
          p1.y, p2.y, p3.y, p4.y,
          p1.z, p2.z, p3.z, p4.z);

      m = basis * control;
      int subdivide_times = 0;
      subdivide(0, 1, max_line_length, m, subdivide_times);
    }
  }

  // set_one_vao_basic(pipelineProgram, vertices, vertex_colors, vao_vertices);
  for (int i = 0; i < frenets.size(); ++i)
  {

    Frenet &f = frenets[i], &f_next = frenets[(i + 50) % frenets.size()];

    generate_cross_section_vector(f, cs_r, cross_section_separation, 1.0f, false);
    generate_cross_section_vector(f, cs_l, -cross_section_separation, 1.0f, false);

    if (i % 200 == 0)
    {
      float b_multiplier = 2.0f;
      glm::vec3 p = f.point;
      float height = p.z - min_h;

      generate_cross_section_vector(f, cs_bar, 0.0f, b_multiplier, false);
      generate_cross_section_vector(f_next, cs_bar, 0.0f, b_multiplier, false);

      if (i % (frenets.size() / 5200 * 100) == 0 && height < min_height * 1.2)
      {
        float shift = -1.0f * cross_section_separation;
        generate_cross_section_vector(frenets[i], cs_support, shift, b_multiplier, true);
        generate_cross_section_vector(frenets[(i + 200) % frenets.size()], cs_support, shift, b_multiplier, true);
      }
    }
  }

  generate_cross_section(cs_r);
  generate_cross_section(cs_l);

  cs_bar.cross_section_side_size = generate_cross_section_single(cs_bar.csv, cs_bar.csb, X, 2);
  cs_support.cross_section_side_size = generate_cross_section_single(cs_support.csv, cs_support.csb, Z, 2);

  // cout << "cross_section_vertices size: " << cross_section_vertices.size() / 3 << '\n';
}

void calculate_physical_velocity()
{
  velocity.clear();

  // 3 columns, 4 rows
  glm::mat3x4 control, m;
  glm::vec3 position, point, p1, p2, p3, p4;
  vector<glm::vec3> points;
  vector<float> vc;
  Point p;
  int count = 100, j = 10;

  for (int s = 0; s < numSplines; ++s)
  {

    Spline spline = splines[s];
    float u = 0.0f, default_speed = 0.0005f;
    int numControlPoints = spline.numControlPoints;

    for (int i = 0; i < numControlPoints; ++i)
    {
      p = spline.points[i];
      point.x = p.x;
      point.y = p.y;
      point.z = p.z;
      points.push_back(point);
    }

    for (int i = 0; i < numControlPoints; ++i)
    {
      p1 = points[i];
      p2 = points[(i + 1) % numControlPoints];
      p3 = points[(i + 2) % numControlPoints];
      p4 = points[(i + 3) % numControlPoints];

      control = glm::mat3x4(
          p1.x, p2.x, p3.x, p4.x,
          p1.y, p2.y, p3.y, p4.y,
          p1.z, p2.z, p3.z, p4.z);

      m = basis * control;

      while (u <= 1.0)
      {

        Pos position = catmull_rom(u, m);

        glm::vec3 ground = glm::vec3(position.position.x, position.position.y, -10.0f);
        float height = glm::length(position.position - ground);

        u += time_step * sqrt(2 * gravity * (max_height - height)) / glm::length(position.position_tan) + default_speed;

        int subdivide_time = 0;
        do_vertex(position, velocity, vc, frenets_v, subdivide_time);
      }
      u = 0.0f;
    }
  }
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

  // cout << "spline size: " << frenets.size() << '\n';
  // cout << "lines size: " << lines.size() << '\n';

  set_ebo(lines, ebo);
}

void fill_texCoords(vector<float> &texCoords, float repeat_x, float repeat_y, vector<float> &start, vector<float> &end)
{

  texCoords.push_back(start[0]);
  texCoords.push_back(start[1]);

  texCoords.push_back(end[0] * repeat_x);
  texCoords.push_back(start[1]);
  texCoords.push_back(start[0]);
  texCoords.push_back(end[1] * repeat_y);

  texCoords.push_back(end[0] * repeat_x);
  texCoords.push_back(end[1] * repeat_y);

}

void fill_plane(vector<float> &plane, GLuint &vao, GLuint &ebo, float repeat_x, float repeat_y)
{

  vector<float> grounds, colors, texCoords, start{0.0f, 0.0f}, end{1.0f, 1.0f};
  vector<int> indexes;
  glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, alpha);

  // 0 | 1, 1

  int size = plane.size();

  for (int i = 0; i < size; ++i)
  {

    grounds.push_back(plane[i]);

    if (!(i + 1) % 3)
    {
      push_glm_to_color_texture(color, colors);
    }
  }

  fill_texCoords(texCoords, repeat_x, repeat_y, start, end);

  set_vao_texture(texturePipelineProgram, grounds, colors, texCoords, vao);

  indexes.push_back(0);
  indexes.push_back(1);
  indexes.push_back(2);

  indexes.push_back(1);
  indexes.push_back(3);
  indexes.push_back(2);

  set_ebo(indexes, ebo);
}

struct Pos catmull_rom(float u, glm::mat3x4 &m)
{

  glm::vec4 us = glm::vec4(powf(u, 3.0f), powf(u, 2.0f), u, 1);
  glm::vec4 us_tan = glm::vec4(3 * powf(u, 2.0f), 2 * u, 1, 0);
  glm::vec3 position = us * m;
  glm::vec3 position_tan = us_tan * m;

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
void set_one_vao_basic(BasicPipelineProgram *pipeline, vector<float> &position, vector<float> &color, vector<float> &texCoord, GLuint &vao)
{
  // Set up vertices and color in buffer
  int size = sizeof(float) * position.size();
  int color_size = sizeof(float) * color.size();
  int tex_size = sizeof(float) * texCoord.size();
  uintptr_t offset = 0;

  // bind vertices and colors with vbo
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size + color_size + tex_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, position.data());
  glBufferSubData(GL_ARRAY_BUFFER, size, color_size, color.data());
  glBufferSubData(GL_ARRAY_BUFFER, size + color_size, tex_size, texCoord.data());

  // set up vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // get the location of the shader variable "position"
  // enable it then set attributes
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  GLuint loc = glGetAttribLocation(pipeline->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // get the location of the shader variable "color"
  offset += size;
  loc = glGetAttribLocation(pipeline->GetProgramHandle(), "normal");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  offset += color_size;
  loc = glGetAttribLocation(pipeline->GetProgramHandle(), "texCoord");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void set_vao_texture(BasicPipelineProgram *pipeline, vector<float> &position, vector<float> &color, vector<float> &texCoord, GLuint &vao)
{
  // Set up vertices and color in buffer
  int size = sizeof(float) * position.size();
  int color_size = sizeof(float) * color.size();
  int tex_size = sizeof(float) * texCoord.size();

  uintptr_t offset = 0;

  // bind vertices and colors with vbo
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size + color_size + tex_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, position.data());
  glBufferSubData(GL_ARRAY_BUFFER, size, color_size, color.data());
  glBufferSubData(GL_ARRAY_BUFFER, size + color_size, tex_size, texCoord.data());

  // set up vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // get the location of the shader variable "position"
  // enable it then set attributes
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  GLuint loc = glGetAttribLocation(pipeline->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // get the location of the shader variable "color"
  offset += size;
  loc = glGetAttribLocation(pipeline->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  offset += color_size;
  loc = glGetAttribLocation(pipeline->GetProgramHandle(), "texCoord");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void do_vertex(Pos &coords, vector<float> &vs, vector<float> &vc, vector<Frenet> &f, int subdivide_time)
{
  generate_point(coords, vs, vc, f, subdivide_time);
}

/* Universal vertices position and color generator */
void generate_point(Pos &coords, vector<float> &position, vector<float> &color, vector<Frenet> &f, int subdivide_time)
{

  float c = 1.0f;
  glm::vec3 p = coords.position;
  glm::vec3 t = coords.position_tan;
  glm::vec3 ground = glm::vec3(p.x, p.y, -10.0f);

  // assign positions and colors
  position.push_back(p.x);
  position.push_back(p.y);
  position.push_back(p.z);
  color.push_back(c);
  color.push_back(c);
  color.push_back(c);

  // assign frenet
  Frenet frenet;

  frenet.tex_inc = 1.0f / subdivide_time;
  frenet.point = p;
  frenet.tangent = glm::normalize(t);
  max_height = max(max_height, glm::length(p - ground));
  min_height = min(min_height, glm::length(p - ground));

  if (f.size() == 0)
  {
    frenet.normal = glm::normalize(glm::cross(frenet.tangent, glm::vec3(0, 1, 0)));

    if (glm::isnan(frenet.normal.x))
    {
      frenet.normal = glm::normalize(glm::cross(frenet.tangent, glm::vec3(1, 1, 1)));
    }

    frenet.binormal = glm::normalize(glm::cross(frenet.tangent, frenet.normal));
  }
  else
  {
    Frenet &prev = f.back();
    frenet.binormal = prev.binormal;
    frenet.normal = prev.normal;
    calculate_frenet(frenet);
  }

  // cout << "binormal: " << glm::to_string(frenet.binormal) << " normal: " << glm::to_string(frenet.normal) << '\n';

  f.push_back(frenet);
}

void generate_cross_section_vector(Frenet &frenet, Cross_Section &cs, float shift, float b_multiplier, bool isVertical)
{

  float a = cs.alpha;

  vector<float> &position = cs.csv.cross_section_vertices;
  vector<float> &position_t = cs.csv_t.cross_section_vertices;

  glm::vec3 &p = frenet.point;
  glm::vec3 &n = frenet.normal;
  glm::vec3 &b = frenet.binormal;
  glm::vec3 b_shit = b * shift;

  glm::vec3 v0 = p + a * (n * -1.0f + b * b_multiplier + b_shit);
  glm::vec3 v1 = p + a * (n + b * b_multiplier + b_shit);
  glm::vec3 v2 = p + a * (n + b * -1.0f * b_multiplier + b_shit);
  glm::vec3 v3 = p + a * (n * -1.0f + b * -1.0f * b_multiplier + b_shit);

  glm::vec3 v0_t = p + a * (n * -3.0f + b * b_multiplier + b_shit);
  glm::vec3 v1_t = p + a * (n * 3.0f + b * b_multiplier + b_shit);
  glm::vec3 v2_t = p + a * (n * 3.0f + b * 0.5f * b_multiplier + b_shit);
  glm::vec3 v3_t = p + a * (n * -3.0f + b * 0.5f * b_multiplier + b_shit);

  if (isVertical)
  {
    glm::vec3 v0_shift = p + a * (n * -1.0f + b * b_multiplier - b_shit);
    glm::vec3 v3_shift = p + a * (n * -1.0f + b * -1.0f * b_multiplier - b_shit);
    v1 = v0.z < v0_shift.z ? v0 : v0_shift;
    v2 = v3.z < v3_shift.z ? v3 : v3_shift;
    v0 = glm::vec3(v1.x, v1.y, min_h);
    v3 = glm::vec3(v2.x, v2.y, min_h);

    v1_t = v0_t;
    v2_t = v3_t;
    v0_t = glm::vec3(v1_t.x, v1_t.y, min_h);
    v3_t = glm::vec3(v2_t.x, v2_t.y, min_h);
  }

  push_glm_to_vector(v0, position);
  push_glm_to_vector(v1, position);
  push_glm_to_vector(v2, position);
  push_glm_to_vector(v3, position);

  push_glm_to_vector(v0_t, position_t);
  push_glm_to_vector(v1_t, position_t);
  push_glm_to_vector(v2_t, position_t);
  push_glm_to_vector(v3_t, position_t);

  // cout << "v0: " << glm::to_string(v0) << " v1: " << glm::to_string(v1) << "v2: " << glm::to_string(v2) << " v3: " << glm::to_string(v3) << '\n';
}

void push_glm_to_vector(glm::vec3 &g, vector<float> &vec)
{
  vec.push_back(g.x);
  vec.push_back(g.y);
  vec.push_back(g.z);
}

void push_side_to_vector(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec3 &d, vector<float> &vec)
{
  push_glm_to_vector(a, vec);
  push_glm_to_vector(b, vec);
  push_glm_to_vector(c, vec);
  push_glm_to_vector(d, vec);
}

void generate_cross_section(Cross_Section &cs)
{
  cs.cross_section_side_size = generate_cross_section_single(cs.csv, cs.csb, Y, 1);
  generate_cross_section_single(cs.csv_t, cs.csb_t, Y, 1);
}

int generate_cross_section_single(Cross_Section_Vertex &csv, Cross_Section_Buffer &buffer, SECTION_DIRECTION sd, int skip)
{

  vector<int> cross_section_right_index, cross_section_up_index, cross_section_left_index, cross_section_down_index;
  vector<float> &vecs = csv.cross_section_vertices;

  int i = 0, num_vertices = vecs.size() / (3 * 4);

  for (; i < num_vertices; i += skip)
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

    glm::vec3 p0 = find_point(vecs, i0);
    glm::vec3 p1 = find_point(vecs, i1);
    glm::vec3 p2 = find_point(vecs, i2);
    glm::vec3 p3 = find_point(vecs, i3);
    glm::vec3 p4 = find_point(vecs, i4);
    glm::vec3 p5 = find_point(vecs, i5);
    glm::vec3 p6 = find_point(vecs, i6);
    glm::vec3 p7 = find_point(vecs, i7);

    Frenet f = frenets[i];

    glm::vec3 right = f.binormal;
    glm::vec3 left = -right;
    glm::vec3 up = f.normal;
    glm::vec3 down = -up;

    index = i * 4;
    switch (sd)
    {
    case Y:
      push_side(p0, p1, p4, p5, right, index, csv.right, cross_section_right_index, f.tex_inc);
      push_side(p1, p2, p5, p6, up, index, csv.up, cross_section_up_index, f.tex_inc);
      push_side(p2, p3, p6, p7, left, index, csv.left, cross_section_left_index, f.tex_inc);
      push_side(p3, p0, p7, p4, down, index, csv.down, cross_section_down_index, f.tex_inc);
      break;
    case X:
      right = f.tangent;
      left = -right;
      push_side(p4, p5, p7, p6, right, index, csv.right, cross_section_right_index, 1.0f);
      push_side(p1, p2, p5, p6, up, index, csv.up, cross_section_up_index, 1.0f);
      push_side(p3, p2, p0, p1, left, index, csv.left, cross_section_left_index, 1.0f);
      push_side(p3, p0, p7, p4, down, index, csv.down, cross_section_down_index, 1.0f);
      break;
    case Z:
      up = f.tangent;
      down = -up;
      push_side(p0, p1, p4, p5, right, index, csv.right, cross_section_right_index, 1.0f);
      push_side(p4, p5, p7, p6, up, index, csv.up, cross_section_up_index, 1.0f);
      push_side(p2, p3, p6, p7, left, index, csv.left, cross_section_left_index, 1.0f);
      push_side(p3, p2, p0, p1, down, index, csv.down, cross_section_down_index, 1.0f);
      break;
    default:
      break;
    }
  }

  set_one_vao_basic(pipelineProgram, csv.right.vertex, csv.right.color, csv.right.texCoords, buffer.right.vao);
  set_one_vao_basic(pipelineProgram, csv.up.vertex, csv.up.color, csv.up.texCoords, buffer.up.vao);
  set_one_vao_basic(pipelineProgram, csv.left.vertex, csv.left.color, csv.left.texCoords, buffer.left.vao);
  set_one_vao_basic(pipelineProgram, csv.down.vertex, csv.down.color, csv.down.texCoords, buffer.down.vao);

  set_ebo(cross_section_right_index, buffer.right.ebo);
  set_ebo(cross_section_up_index, buffer.up.ebo);
  set_ebo(cross_section_left_index, buffer.left.ebo);
  set_ebo(cross_section_down_index, buffer.down.ebo);

  return cross_section_right_index.size();
}

void push_side(glm::vec3 &p0, glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &dir, int i, Vertex_Color &cv, vector<int> &csi, float tex_incre)
{

  vector<float> start{0.0f, 0.0f}, end{1.0f, 0.0f};
  int texCoords_size = cv.texCoords.size();

  if (cv.texCoords.size() != 0)
  {
    start[1] = cv.texCoords[texCoords_size - 1];
  }

  start[1] = start[1] + tex_incre > 1.0f ? 0.0f : start[1];
  end[1] = start[1] + tex_incre;

  // cout << "start " << start[0] << " | " << start[1] << '\n';
  // cout << "end " << end[0] << " | " << end[1] << '\n';

  push_side_to_vector(p0, p1, p2, p3, cv.vertex);
  push_side_to_color(dir, dir, dir, dir, cv.color);
  fill_texCoords(cv.texCoords, 1.0f, 1.0f, start, end);
  push_cross_section_index(csi, i);
}

void push_cross_section_index(vector<int> &indexes, int i)
{
  indexes.push_back(i);
  indexes.push_back(i + 2);
  indexes.push_back(i + 3);
  indexes.push_back(i);
  indexes.push_back(i + 3);
  indexes.push_back(i + 1);
}

void push_glm_to_color(glm::vec3 &n, vector<float> &color)
{
  color.push_back(n.x);
  color.push_back(n.y);
  color.push_back(n.z);
}

void push_glm_to_color_texture(glm::vec4 &n, vector<float> &color)
{
  color.push_back(n.r);
  color.push_back(n.b);
  color.push_back(n.g);
  color.push_back(n.a);
}

void push_side_to_color(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, glm::vec3 &d, vector<float> &color)
{
  push_glm_to_color(a, color);
  push_glm_to_color(b, color);
  push_glm_to_color(c, color);
  push_glm_to_color(d, color);
}

glm::vec3 find_point(vector<float> &position, int index)
{
  return glm::vec3(position[index], position[index + 1], position[index + 2]);
}

void set_matrix(BasicPipelineProgram *pipeline)
{
  float m[16], p[16];

  // set up ModelView
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(m);

  // set up Projection
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(p);

  // set variable
  pipeline->SetModelViewMatrix(m);
  pipeline->SetProjectionMatrix(p);
}

void set_uniform(GLuint program, float *var, string name)
{
  GLint h = glGetUniformLocation(program, name.c_str());
  glUniform4fv(h, 1, var);
}

void set_light(BasicPipelineProgram *pipeline)
{

  float view[16], lightDirection[4] = {0.0f, 0.0f, 1.0f}, viewLightDirection[3], n[16];

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(view);

  glm::mat4 mat_view = glm::make_mat4(view);
  glm::vec4 vec_light_dir = glm::make_vec4(lightDirection);
  glm::vec4 vec_viewLightDirection = mat_view * vec_light_dir;

  viewLightDirection[0] = vec_viewLightDirection.x;
  viewLightDirection[1] = vec_viewLightDirection.y;
  viewLightDirection[2] = vec_viewLightDirection.z;

  // upload viewLightDirection to the GPU
  GLuint program = pipeline->GetProgramHandle();
  GLint h_viewLightDirection = glGetUniformLocation(program, "viewLightDirection");
  glUniform3fv(h_viewLightDirection, 1, viewLightDirection);

  // set up Normal matrix
  GLint h_normalMatrix = glGetUniformLocation(program, "normalMatrix");
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetNormalMatrix(n);
  glUniformMatrix4fv(h_normalMatrix, 1, GL_FALSE, n);

  // set properties
  float La[4] = {1.0, 1.0, 1.0}, Ld[4] = {1.0, 1.0, 1.0}, Ls[4] = {1.0, 1.0, 1.0};
  float ka[4] = {0.1, 0.1, 0.1}, kd[4] = {0.3, 0.3, 0.3}, ks[4] = {0.3, 0.3, 0.3}, alpha = 1.0;

  // La, Ka, Ld, kd, Ls, ks, alpha
  set_uniform(program, La, "La");
  set_uniform(program, Ld, "Ld");
  set_uniform(program, Ls, "Ls");
  set_uniform(program, ka, "ka");
  set_uniform(program, kd, "kd");
  set_uniform(program, ks, "ks");
  // set_uniform(program, &alpha, "alpha");
  GLint h_alpha = glGetUniformLocation(program, "alpha");
  glUniform1f(h_alpha, alpha);
}

void subdivide(float u0, float u1, float max_line_length, glm::mat3x4 &m, int &subdivide_time)
{

  float umid = (u0 + u1) / 2;
  Pos x0 = catmull_rom(u0, m);
  Pos x1 = catmull_rom(u1, m);
  ++subdivide_time;

  if (glm::length(x1.position - x0.position) > max_line_length)
  {
    subdivide(u0, umid, max_line_length, m, subdivide_time);
    subdivide(umid, u1, max_line_length, m, subdivide_time);
  }
  else
  {
    do_vertex(x0, vertices, vertex_colors, frenets, subdivide_time);
  }
}

void calculate_frenet(Frenet &f)
{
  glm::vec3 n = glm::normalize(glm::cross(f.binormal, f.tangent));
  f.normal = n;
  glm::vec3 b = glm::normalize(glm::cross(f.tangent, f.normal));
  f.binormal = b;
}

void render_normal_binormal()
{

  int num_vertices = vertices.size() / 3, i = 0;
  vector<float> normals, binormals, nc, bc;

  // render normal line segment as red and binormal as blue
  for (; i < num_vertices; ++i)
  {
    Frenet &f = frenets[i];
    glm::vec3 n = f.point + f.normal;
    glm::vec3 b = f.point + f.binormal;
    push_glm_to_vector(f.point, normals);
    push_glm_to_vector(n, normals);
    nc.push_back(1.0f);
    nc.push_back(0.0f);
    nc.push_back(0.0f);
    nc.push_back(1.0f);
    nc.push_back(0.0f);
    nc.push_back(0.0f);
    push_glm_to_vector(f.point, binormals);
    push_glm_to_vector(b, binormals);
    bc.push_back(0.0f);
    bc.push_back(0.0f);
    bc.push_back(1.0f);
    bc.push_back(0.0f);
    bc.push_back(0.0f);
    bc.push_back(1.0f);
    // cout << "index: " << i << " tangent: " << glm::length(f.tangent) << " normal: " << glm::length(f.normal)
    // << " binormal: " << glm::length(f.binormal) << '\n';
  }
  // set_one_vao_basic(pipelineProgram, normals, nc, vao_normal);
  // set_one_vao_basic(pipelineProgram, binormals, bc, vao_binormal);
}

void print_frenet(int index)
{

  Frenet frenet = frenets[index];

  cout << "index: " << index << " normal: " << glm::to_string(frenet.normal)
       << " binormal: " << glm::to_string(frenet.binormal) << '\n';
}
