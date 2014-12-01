#pragma once
#include <vector>
#ifdef __linux__
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <cmath>
#else
#define NOMINMAX
#include <Windows.h>
#include <gl/glut.h>
#include <gl/GLU.h>
#include <gl/GL.h>
#define _USE_MATH_DEFINES
#include <math.h>
#endif
#include <cstring>
#include <iostream>
#include <functional>

extern int branchDepth;

struct Model;

struct Year {
  static const int NUM_SEASONS = 4;
  static const int NUM_PERIODS = 6;
  int season;
  int period;
  void cicle();

  Year() : season(1), period(1) {}
};

struct RGBColor {
  GLfloat r, g, b, a;

  RGBColor() : r(0), g(0), b(0), a(0) {}
  RGBColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) : r(r), g(g), b(b), a(a) {}
};

struct Vertex2d {
  GLfloat x, y;

  Vertex2d() : x(0), y(0) {}
  Vertex2d(GLfloat x, GLfloat y) : x(x), y(y) {}
};

struct Snowflake {
  Vertex2d position;
  GLfloat weight;
  GLfloat startX;
  GLfloat lastZ;
  bool life;

  Snowflake() : life(true) {}
};

struct Snow {
  std::vector<Snowflake> flakes;
  static const int SNOW_FLAKES = 600;
  bool allDead;
  int Dflake;

  Snow() : allDead(false), Dflake(0) {}
};

struct Scene {
  int width;
  int height;
  float* pixels;

  Scene(int, int);
};

struct Flower {
  Vertex2d position;
  GLfloat size;
  RGBColor centerColor;
  RGBColor petalColor;
  GLfloat scaleFactor;
  GLfloat phase;

  Flower() : scaleFactor(0) {}
  Flower(Vertex2d, int, RGBColor, RGBColor);
  void swingFlower(Model*);
  void growFlower(Model*);
  void hideFlower(Model*);
};

struct Fruit {
  Vertex2d position;
  Vertex2d initPosition;
  Vertex2d fallPoint;
  GLfloat size;
  RGBColor color;
  GLfloat scaleFactor;
  GLfloat phase;
  bool stopped;
  int age;

  Fruit() : scaleFactor(0), stopped(false) {}
  Fruit(Vertex2d, Vertex2d, GLfloat, RGBColor, int);
  void swingFruit(Model*);
  void growFruit(Model*);
  void hideFruit(Model*);
  void fallFruit(Model*);
};

struct Floor {
  RGBColor COLOR[6];
  RGBColor background;
  int width;
  int height;
  float* pixels;
  float* pixels_copy;
  float* snow_pixels;
  std::vector<Flower> flowers;
  
  Floor(int, int);
  void tickFloor(int, int, int, int, int);
};

struct Leaf {
  Vertex2d position;
  Vertex2d initPosition;
  Vertex2d fallPoint;
  GLfloat size;
  GLfloat scaleFactor;
  GLfloat angle;
  GLfloat phase;
  RGBColor color;
  RGBColor initColor;
  int age;
  bool stopped;
  bool behindLog;

  Leaf() : scaleFactor(0), stopped(false) {}
  Leaf(Vertex2d, Vertex2d, GLfloat, GLfloat, RGBColor, int, bool);
  void growLeaf(Model*);
  void swingLeaf(Model*);
  void yellowLeaf(Model*);
  void fallLeaf(Model*);
  void hideLeaf(Model*);
};  

struct Tree {
  static const int CHILDNUM = 3;
  Vertex2d position;
  GLfloat logHeight;
  GLfloat logWidth;
  GLfloat snowTransparence;
  struct Branch {
    GLfloat angle;
    std::vector<Leaf> leaves;
    Branch *child[CHILDNUM];
    std::vector<Flower> flowers;
    std::vector<Fruit> fruits;

    Branch() {
      for (int i=0; i<CHILDNUM; i++) {
        child[i] = NULL;
      }
    }
  } *root;

  Tree() : snowTransparence(0.0f) {}
  Tree(Vertex2d, GLfloat, GLfloat, GLfloat, int, Floor*, bool);
  Tree::Branch* branchFractal(GLfloat, GLfloat, Vertex2d, GLfloat, unsigned char, Floor*, bool);
};

struct Sun {
  struct Circle {
    Vertex2d center;
    GLfloat radius;
    GLfloat phase;
    GLfloat angle;
    RGBColor color;

    Circle(Vertex2d center, GLfloat radius, GLfloat phase, GLfloat angle, RGBColor color) :
      center(center), radius(radius), phase(phase), angle(angle), color(color) {}
  };
  std::vector<Circle> circles;
};

struct Cloud {
  Vertex2d position;
  RGBColor color;
  GLfloat width;
  GLfloat height;
  GLfloat scale;
  GLfloat speedX;

  Cloud(Vertex2d position, RGBColor color, GLfloat width, GLfloat height, GLfloat scale, GLfloat speedX) :
    position(position), color(color), width(width), height(height), scale(scale), speedX(speedX) {}
  void step(Model*);
};

struct Model {
  int screenWidth;
  int screenHeight;
  struct Counter {
    static const int resolution = 1 << 8;
    int value;

    Counter() : value(0) {}
  } counter;
  Sun *sun;
  Tree *tree, *shrub1, *shrub2;
  int wind;
  Snow snow;
  Floor* floor;
  Scene* scene;
  Year current;
  std::vector<Cloud> clouds;

  Model();
  ~Model();
  void markSnowflake(GLfloat, GLfloat, int);
  void tick();
  void tickLeaves(Tree::Branch*, std::vector<void (Leaf::*)(Model*)>&);
  void tickFlowers(Tree::Branch*, std::vector<void (Flower::*)(Model*)>&);
  void tickFruits(Tree::Branch*, std::vector<void (Fruit::*)(Model*)>&);
  static const int SNOW_FLAKES = 600;
};
