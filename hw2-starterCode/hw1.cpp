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
#endif

using namespace std;

// forward declaration
// fill positions and color array
void fill_points();
void fill_lines();
void fill_triangles();
void fill_triangle_strip();
void get_surrounding_points_height(int x, int y);

void generate_point_new(glm::vec3 &coords, vector<float> &position, vector<float> &color);
void generate_point(int x, int y, vector<float> &position, vector<float> &color);
void do_point(int x, int y);
void do_solid_line(int x, int y);
void do_line(glm::vec3 &coords);
void do_triangle(int x, int y);
void do_triangle_strip(int x, int y);

// set up vbo and vao
void set_one_vbo_one_vao(vector<float> &position, vector<float> &color, GLuint &vao);
void set_ebo(vector<float> &position, vector<float> &color, GLuint &ebo, GLuint &vao);
void set_points_buffer();
void set_lines_buffer();
void set_triangles_buffer();
void set_smooth_buffer();
void set_solid_line_buffer();
void set_triangle_strip_buffer();

// background image
void get_background_image(ImageIO *image, vector<float> &background_position, vector<float> &background_color, GLuint &background_vao, string name);
void fill_triangles_background(ImageIO *image, vector<float> &background_position, vector<float> &background_color, GLuint &background_vao);
void do_triangle_background(int x, int y, ImageIO *image, vector<float> &background_position, vector<float> &background_color);
void generate_point_background(int x, int y, ImageIO *image, vector<float> &position, vector<float> &color);
void draw_background(int background_n);
void set_matrix();


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
  TRIANGLE,
  SMOOTH,
  SOLID_WIREFRAME,
  TRIANGLE_STRIP,
  TRIANGLE_ELEMENT
} MODE_STATE;
MODE_STATE mode = POINT;

// animation mode
int animation = 0;
int counter = 0;
int start = 0;
int shake = 0;

// state of the world
float landRotate[3] = {0.0f, 0.0f, 0.0f};
float landTranslate[3] = {0.0f, 0.0f, 0.0f};
float landScale[3] = {1.0f, 1.0f, 1.0f};

// state of the background1
float backgroundRotate1[3] = {0.0f, 0.0f, 0.0f};
float backgroundTranslate1[3] = {0.0f, 100.0f, 100.0f};
float backgroundScale1[3] = {1.5f, 4.0f, 1.5f};

// state of the background2
float backgroundRotate2[3] = {45.0f, 0.0f, 0.0f};
float backgroundTranslate2[3] = {0.0f, 100.0f, 100.0f};
float backgroundScale2[3] = {1.5f, 4.0f, 1.0f};

// state of the background3
float backgroundRotate3[3] = {45.0f, 0.0f, 0.0f};
float backgroundTranslate3[3] = {0.0f, 100.0f, 100.0f};
float backgroundScale3[3] = {2.5f, 2.5f, 2.5f};

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
ImageIO *heightmapImage;
ImageIO *backgroundImage1;
ImageIO *backgroundImage2;
ImageIO *backgroundImage3;

// points
vector<float> points, point_colors;
GLuint vao_point;

// lines
vector<float> lines, line_colors;
GLuint vao_line;

// triangles
vector<float> triangles, triangle_colors;
GLuint vao_triangle;

// smooth
vector<float> p_left, p_right, p_up, p_down;
GLuint vao_smooth;

// solid-wireframe
vector<float> solid_line_colors, solid_lines;
GLuint vao_solid_line;

// triangle_strip
vector<float> triangle_strip, triangle_strip_colors;
GLuint vao_triangle_strip;

// triangle_element
GLuint ebo_triangle_element, vao_triangle_element;

// background
vector<float> background_position_1, background_color_1;
GLuint background_vao_1;
vector<float> background_position_2, background_color_2;
GLuint background_vao_2;
vector<float> background_position_3, background_color_3;
GLuint background_vao_3;

OpenGLMatrix matrix;
BasicPipelineProgram *pipelineProgram;

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
  Point * points;
};

// the spline array 
Spline * splines;
// total number of splines 
int numSplines;

float s = 0.5;

glm::mat4 basis = glm::mat4(
  -s, 2*s, -s, 0,
  2-s, s-3, 0, 1,
  s-2, 3-2*s, s, 0,
  s, -s, 0, 0
);


int loadSplines(char * argv) 
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file 
  fileList = fopen(argv, "r");
  if (fileList == NULL) 
  {
    printf ("fileList can't open file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable 
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files 
  for (j = 0; j < numSplines; j++) 
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) 
    {
      printf ("fileSpline can't open file\n");
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

int initTexture(const char * imageFilename, GLuint textureHandle)
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
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
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
  delete [] pixelsRGBA;

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
  matrix.LookAt(0.0, 0.0, 5.0, 
                0.0, 0.0, 0.0, 
                0.0, 1.0, 0.0);


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
    glBindVertexArray(vao_line);
    glDrawArrays(GL_LINES, 0, lines.size() / 3);
    glBindVertexArray(0);
    break;
  }

  glutSwapBuffers();
}

void idleFunc()
{

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
    fill_points();
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

  fill_lines();

  glEnable(GL_DEPTH_TEST);

  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char ** argv)
{
  if (argc<2)
  {  
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }

  // load the splines from the provided filename
  loadSplines(argv[1]);

  std::cout << "Initializing GLUT..." << endl;
  glutInit(&argc, argv);

  std::cout << "Initializing OpenGL..." << endl;

  printf("Loaded %d spline(s).\n", numSplines);
  for(int i=0; i<numSplines; i++)
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

/* Generate points for point mode */
void fill_points()
{
  points.clear(), point_colors.clear();
  for (int x = 0; x < image_width; ++x)
    for (int y = 0; y < image_height; ++y)
      do_point(x, y);

  // set up vbo and vao
  set_points_buffer();
}

/* Generate points for line mode */
void fill_lines()
{
  lines.clear(), line_colors.clear();

  // 3 columns, 4 rows
  glm::mat3x4 control, m; 
  glm::vec3 position, init_pos;
  Point p1, p2, p3, p4;

  float step = 0.1f;
  float u = 0.0f;
  int count = 0;

  for (int i = 0; i < splines->numControlPoints - 3; ++i) {
    p1 = splines->points[i];
    p2 = splines->points[i+1];
    p3 = splines->points[i+2];
    p4 = splines->points[i+3];

    control = glm::mat3x4(
        p1.x, p2.x, p3.x, p4.x, 
        p1.y, p2.y, p3.y, p4.y, 
        p1.z, p2.z, p3.z, p4.z);

    m = basis * control;

    while (u <= 1.0) {
      glm::vec4 us = glm::vec4(powf(u, 3.0f), powf(u, 2.0f), u, 1);

      if (lines.size() > 3) {
        do_line(position);
      }

      position = us * m;
      ++count;

      if (lines.size() == 0)
        init_pos = position;

      do_line(position);
      u += step;
    }

    u = 0.0f;
  }

  do_line(position);
  do_line(init_pos);

  // set up vbo and vao
  set_lines_buffer();
}

/* Generate points for triangle mode */
void fill_triangles()
{
  triangles.clear(), triangle_colors.clear();
  p_left.clear(), p_right.clear(), p_up.clear(), p_down.clear();
  for (int y = 0; y < image_height; ++y)
  {
    for (int x = 0; x < image_width; ++x)
    {

      // we want a triangle like
      //  __
      // | /
      // |/
      // the down point is the current point
      // and
      //  /|
      // /_|
      // the down left point is the current point

      // the top-right point is always present
      // so we don't pick the topmost and right most points as starting point
      if (x < image_width - 1 && y < image_height - 1)
      {

        // first triangle
        // bottom-left
        do_triangle(x, y);

        // this is for smooth mode
        get_surrounding_points_height(x, y);

        // top-left
        do_triangle(x, y + 1);
        get_surrounding_points_height(x, y + 1);

        // top-right
        do_triangle(x + 1, y + 1);
        get_surrounding_points_height(x + 1, y + 1);

        // second triangle
        // bottom-left
        do_triangle(x, y);
        get_surrounding_points_height(x, y);

        // bottom-right
        do_triangle(x + 1, y);
        get_surrounding_points_height(x + 1, y);

        // top-right
        do_triangle(x + 1, y + 1);
        get_surrounding_points_height(x + 1, y + 1);
      }
    }
  }

  // set up vbo and vao
  set_triangles_buffer();
  set_smooth_buffer();
  set_ebo(triangles, triangle_colors, ebo_triangle_element, vao_triangle_element);
}

/* generate points for triangle strip */
void fill_triangle_strip()
{
  triangle_strip.clear(), triangle_strip_colors.clear();
  for (int y = 0; y < image_height - 1; ++y)
  {
    for (int x = 0; x < image_width; ++x)
    {

      // add vertices in zigzag order
      int row = y & 1 ? (image_width - x - 1) : x;

      do_triangle_strip(row, y);
      do_triangle_strip(row, y + 1);
    }
  }

  // set up vbo and vao
  set_triangle_strip_buffer();
}

/* set up vbo and vao for point mode */
void set_points_buffer()
{
  set_one_vbo_one_vao(points, point_colors, vao_point);
}

/* set up vbo and vao for line mode */
void set_lines_buffer()
{
  set_one_vbo_one_vao(lines, line_colors, vao_line);
}

/* set up vbo and vao for triangle mode */
void set_triangles_buffer()
{
  set_one_vbo_one_vao(triangles, triangle_colors, vao_triangle);
}

/* set up vbo and vao for smoothened height field */
void set_smooth_buffer()
{
  // p_left, right, up, down have the same size
  int p_size = sizeof(float) * p_left.size();
  int triangles_size = sizeof(float) * triangles.size();
  int triangle_colors_size = sizeof(float) * triangle_colors.size();
  uintptr_t offset = 0;

  // initialize vbos
  GLuint vbo_p_left, vbo_p_right, vbo_p_up, vbo_p_down, vbo_smooth_triangle;
  glGenBuffers(1, &vbo_p_left);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_left);
  glBufferData(GL_ARRAY_BUFFER, p_size, p_left.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &vbo_p_right);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_right);
  glBufferData(GL_ARRAY_BUFFER, p_size, p_right.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &vbo_p_up);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_up);
  glBufferData(GL_ARRAY_BUFFER, p_size, p_up.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &vbo_p_down);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_down);
  glBufferData(GL_ARRAY_BUFFER, p_size, p_down.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &vbo_smooth_triangle);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_smooth_triangle);
  glBufferData(GL_ARRAY_BUFFER, triangles_size + triangle_colors_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, triangles_size, triangles.data());
  glBufferSubData(GL_ARRAY_BUFFER, triangles_size, triangle_colors_size, triangle_colors.data());

  // initialize vao
  glGenVertexArrays(1, &vao_smooth);
  glBindVertexArray(vao_smooth);

  // send to shader
  glBindBuffer(GL_ARRAY_BUFFER, vbo_smooth_triangle);
  GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)offset);

  offset += triangles_size;
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)(offset));

  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_left);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "p_left");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_right);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "p_right");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_up);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "p_up");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo_p_down);
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "p_down");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  // unbind vbo and vao
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

/* set up vbo and vao for solid_wireframe */
void set_solid_line_buffer()
{
  set_one_vbo_one_vao(solid_lines, solid_line_colors, vao_solid_line);
}

/* set up vbo and vao for triangle_strip */
void set_triangle_strip_buffer()
{
  set_one_vbo_one_vao(triangle_strip, triangle_strip_colors, vao_triangle_strip);
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

/* set up generic ebo */
void set_ebo(vector<float> &position, vector<float> &color, GLuint &ebo, GLuint &vao)
{

  vector<int> indices;
  vector<float> vertices, vertices_color;
  map<vector<float>, int> mp;

  int position_i = 0, color_i, index = 0;
  float x = 0, y = 0, z = 0;
  float r = 0, g = 0, b = 0, a = 0;
  intptr_t offset = 0;

  // iterate through each index in position and color
  for (int i = 0; i < position.size() / 3; ++i)
  {
    x = position[position_i++], y = position[position_i++], z = position[position_i++];
    r = color[color_i++], g = color[color_i++], b = color[color_i++], a = color[color_i++];

    // map each pixel to a index
    vector<float> combo;
    combo.push_back(x), combo.push_back(y), combo.push_back(z);

    // if the combo is not in map already, add to map with the cur index, and add index to indices
    // else add the combo index to indices
    // for vertices and vertices_color, we want to have nonrepetitive pixel inside, the index of vertices
    // should match that of indices
    if (mp.find(combo) == mp.end())
    {
      mp[combo] = index;
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
      vertices_color.push_back(r);
      vertices_color.push_back(g);
      vertices_color.push_back(b);
      vertices_color.push_back(a);
      indices.push_back(index++);
    }
    else
    {
      indices.push_back(mp[combo]);
    }
  }

  int indices_size = sizeof(int) * indices.size();
  int size = sizeof(float) * vertices.size();
  int color_size = sizeof(float) * vertices_color.size();

  // set up vbo and ebo
  // note that the vbo here is different from triangle mode
  // each pixel only appears once
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size + color_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertices.data());
  glBufferSubData(GL_ARRAY_BUFFER, size, color_size, vertices_color.data());

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices.data(), GL_STATIC_DRAW);

  // set up vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)offset);

  offset += size;
  loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (void *)offset);

  // unbind vao, vbo and ebo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

/* get the height of neighbor points */
void get_surrounding_points_height(int x, int y)
{

  // if the current point is on an edge without neighbor(s) on some side
  // we take the height of the point itself
  height = x == 0 ? heightmapImage->getPixel(x, y, 0) : heightmapImage->getPixel(x - 1, y, 0);
  p_left.push_back(height * scale);

  height = x == image_width - 1 ? heightmapImage->getPixel(x, y, 0) : heightmapImage->getPixel(x + 1, y, 0);
  p_right.push_back(height * scale);

  height = y == image_height - 1 ? heightmapImage->getPixel(x, y, 0) : heightmapImage->getPixel(x, y + 1, 0);
  p_up.push_back(height * scale);

  height = y == 0 ? heightmapImage->getPixel(x, y, 0) : heightmapImage->getPixel(x, y - 1, 0);
  p_down.push_back(height * scale);
}

/* Create a point for point mode */
void do_point(int x, int y)
{
  generate_point(x, y, points, point_colors);
}

/* Create a point for line mode */
void do_line(glm::vec3 &coords)
{
  generate_point_new(coords, lines, line_colors);
}

void generate_point_new(glm::vec3 &coords, vector<float> &position, vector<float> &color)
{

  float c = 1.0f;

  // assign positions and colors
  position.push_back(coords.x);
  position.push_back(coords.y);
  position.push_back(coords.z);
  color.push_back(c);
  color.push_back(c);
  color.push_back(c);
  color.push_back(alpha);
}



/* Create a point for solid wireframe */
void do_solid_line(int x, int y)
{

  // match the image center to the world center
  int world_x = x - image_center_x;
  int world_y = y - image_center_y;

  // get the greyscale from the image
  height = heightmapImage->getPixel(x, y, 0);
  height *= scale;

  // assign positions and colors
  solid_lines.push_back(-world_x);
  solid_lines.push_back(height);
  solid_lines.push_back(-world_y);
  solid_line_colors.push_back(0.0);
  solid_line_colors.push_back(0.0);
  solid_line_colors.push_back(0.0);
  solid_line_colors.push_back(alpha);
}

/* Create a point for triangle mode */
void do_triangle(int x, int y)
{
  generate_point(x, y, triangles, triangle_colors);
}

/* Create a point for triangle strip */
void do_triangle_strip(int x, int y)
{
  generate_point(x, y, triangle_strip, triangle_strip_colors);
}

/* Universal vertices position and color generator */
void generate_point(int x, int y, vector<float> &position, vector<float> &color)
{

  // match the image center to the world center
  int world_x = x - image_center_x;
  int world_y = y - image_center_y;

  // get the greyscale from the image
  height = heightmapImage->getPixel(x, y, 0);
  red = green = blue = height / 255.0;

  // get color for each pixel in RGB mode
  if (color_mode == RGB)
  {
    red = heightmapImage->getPixel(x, y, 0) / 255.0;
    green = heightmapImage->getPixel(x, y, 1) / 255.0;
    blue = heightmapImage->getPixel(x, y, 2) / 255.0;
    height = (red + green + blue) * 255.0 / 3;
  }

  height *= scale;

  // assign positions and colors
  position.push_back(-world_x);
  position.push_back(height);
  position.push_back(-world_y);
  color.push_back(red);
  color.push_back(green);
  color.push_back(blue);
  color.push_back(alpha);
}

/* Load an image for background layer */
void get_background_image(ImageIO *image, vector<float> &background_position, vector<float> &background_color, GLuint &background_vao, string name)
{

  if (animation == 1)
  {
    image = new ImageIO();
    string filename = "heightmap/" + name + ".jpg";
    if (image->loadJPEG(filename.c_str()) != ImageIO::OK)
    {
      cout << "Error reading image "
           << "heightmap/horror.jpg"
           << "." << endl;
      exit(EXIT_FAILURE);
    }

    // background layer only has triangle mode
    fill_triangles_background(image, background_position, background_color, background_vao);
  }
}

/* background triangle */
void fill_triangles_background(ImageIO *image, vector<float> &background_position, vector<float> &background_color, GLuint &background_vao)
{
  int background_width = image->getWidth();
  int background_height = image->getHeight();
  int background_center_x = background_width / 2;
  int background_center_y = background_height / 2;

  background_position.clear(), background_color.clear();
  for (int y = 0; y < background_height; ++y)
  {
    for (int x = 0; x < background_width; ++x)
    {

      // we want a triangle like
      //  __
      // | /
      // |/
      // the down point is the current point
      // and
      //  /|
      // /_|
      // the down left point is the current point

      // the top-right point is always present
      // so we don't pick the topmost and right most points as starting point
      if (x < background_width - 1 && y < background_height - 1)
      {

        // first triangle
        // bottom-left
        do_triangle_background(x, y, image, background_position, background_color);

        // top-left
        do_triangle_background(x, y + 1, image, background_position, background_color);

        // top-right
        do_triangle_background(x + 1, y + 1, image, background_position, background_color);

        // second triangle
        // bottom-left
        do_triangle_background(x, y, image, background_position, background_color);

        // bottom-right
        do_triangle_background(x + 1, y, image, background_position, background_color);

        // top-right
        do_triangle_background(x + 1, y + 1, image, background_position, background_color);
      }
    }
  }

  set_one_vbo_one_vao(background_position, background_color, background_vao);
}

/* Create a point for background triangle */
void do_triangle_background(int x, int y, ImageIO *image, vector<float> &background_position, vector<float> &background_color)
{
  generate_point_background(x, y, image, background_position, background_color);
}

/* Universal vertices position and color generator for background */
void generate_point_background(int x, int y, ImageIO *image, vector<float> &position, vector<float> &color)
{

  int background_width = image->getWidth();
  int background_height = image->getHeight();
  int background_center_x = background_width / 2;
  int background_center_y = background_height / 2;

  // match the image center to the world center
  int world_x = x - background_center_x;
  int world_y = y - background_center_y;

  // get the greyscale from the image
  height = image->getPixel(x, y, 0);
  red = green = blue = height / 255.0;

  // get color for each pixel in RGB mode
  if (image->getBytesPerPixel() == 3)
  {
    red = image->getPixel(x, y, 0) / 255.0;
    green = image->getPixel(x, y, 1) / 255.0;
    blue = image->getPixel(x, y, 2) / 255.0;
    height = (red + green + blue) * 255.0 / 3;
  }

  height *= 0.05;

  // assign positions and colors
  position.push_back(-world_x);
  position.push_back(height);
  position.push_back(-world_y);
  color.push_back(red);
  color.push_back(green);
  color.push_back(blue);
  color.push_back(alpha);
}

void draw_background(int background_n) {

  // reset modelview matrix
  matrix.PopMatrix();
  matrix.PushMatrix();
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();
  float eye_z = 150 + pow((image_height - 128) / 128, 1.5) * 50;
  matrix.LookAt(0, image_center_y + 50, eye_z, 0, 0, 0, 0, 1, 0);

  float *backgroundRotate;
  float *backgroundTranslate;
  float *backgroundScale;
  GLuint *background_vao;
  int num_vertices = 0;

  // chose background layer to draw
  switch(background_n) {
    case 1:
      backgroundRotate = backgroundRotate1;
      backgroundTranslate = backgroundTranslate1;
      backgroundScale = backgroundScale1;
      background_vao = &background_vao_1;
      num_vertices = background_position_1.size();
      break;
    case 2:
      backgroundRotate = backgroundRotate2;
      backgroundTranslate = backgroundTranslate2;
      backgroundScale = backgroundScale2;
      background_vao = &background_vao_2;
      num_vertices = background_position_2.size();
      break;
    case 3:
      backgroundRotate = backgroundRotate3;
      backgroundTranslate = backgroundTranslate3;
      backgroundScale = backgroundScale3;
      background_vao = &background_vao_3;
      num_vertices = background_position_3.size();
      break;
    default:
      backgroundRotate = backgroundRotate1;
      backgroundTranslate = backgroundTranslate1;
      backgroundScale = backgroundScale1;
      background_vao = &background_vao_1;
      num_vertices = background_position_1.size();
      break;
  }

  // Transformation
  matrix.Translate(backgroundTranslate[0], backgroundTranslate[1], backgroundTranslate[2]);
  matrix.Rotate(backgroundRotate[0], 1, 0, 0);
  matrix.Rotate(backgroundRotate[1], 0, 1, 0);
  matrix.Rotate(backgroundRotate[2], 0, 0, 1);
  matrix.Scale(backgroundScale[0], backgroundScale[1], backgroundScale[2]);
  set_matrix();

  glBindVertexArray(*background_vao);
  glDrawArrays(GL_TRIANGLES, 0, num_vertices / 3);

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
