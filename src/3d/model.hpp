#pragma once
#ifdef __linux__
#include <GL/freeglut.h>
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
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

struct Model;

struct Year {
  static const int NUM_SEASONS = 4;
  static const int NUM_PERIODS = 6;
  int season;
  int period;

  Year() : season(1), period(1) {}
  void cicle();
};

struct RGBColor {
  GLfloat r, g, b, a;

  RGBColor operator * (float perc) {
    return RGBColor(
      r * perc,
      g * perc,
      b * perc,
      a
    );
  }

  RGBColor operator + (RGBColor other) {
    return RGBColor(
      r + other.r,
      g + other.g,
      b + other.b,
      a
    );
  }
  
  RGBColor() : r(0), g(0), b(0), a(0) {}
  RGBColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) : r(r), g(g), b(b), a(a) {}
};

struct Vertex3d {
  GLfloat x, y, z;
  
  Vertex3d(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {}
  Vertex3d() : x(0), y(0), z(0) {}
};

struct Matrix {
  GLfloat at[4][4];

  Matrix() {
    for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
        this->at[i][j] = (i == j) ? 1.0f : 0.0f;
      }
    }
  }

  Matrix multiply(const Matrix& other) {
    Matrix res;
    for (int i=0; i<4; i++) {
      for (int j=0; j<4; j++) {
        res.at[i][j] = 0;
        for (int k=0; k<4; k++) {
          res.at[i][j] += this->at[i][k] * other.at[k][j];
        }
      }
    }
    return res;
  }

  Vertex3d multiply(const Vertex3d& other) {
    return Vertex3d(
      this->at[0][0] * other.x + this->at[0][1] * other.y + this->at[0][2] * other.z,
      this->at[1][0] * other.x + this->at[1][1] * other.y + this->at[1][2] * other.z,
      this->at[2][0] * other.x + this->at[2][1] * other.y + this->at[2][2] * other.z
    );
  }

  Matrix rotate(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat sinAngle, cosAngle;
    GLfloat mag = sqrtf(x * x + y * y + z * z);

    sinAngle = sinf ( angle);
    cosAngle = cosf ( angle);
    if ( mag > 0.0f ) {
      GLfloat xx, yy, zz, xy, yz, zx, xs, ys, zs;
      GLfloat oneMinusCos;
      Matrix rotMat;

      x /= mag;
      y /= mag;
      z /= mag;

      xx = x * x;
      yy = y * y;
      zz = z * z;
      xy = x * y;
      yz = y * z;
      zx = z * x;
      xs = x * sinAngle;
      ys = y * sinAngle;
      zs = z * sinAngle;
      oneMinusCos = 1.0f - cosAngle;

      rotMat.at[0][0] = (oneMinusCos * xx) + cosAngle;
      rotMat.at[0][1] = (oneMinusCos * xy) - zs;
      rotMat.at[0][2] = (oneMinusCos * zx) + ys;
      rotMat.at[0][3] = 0.0f; 

      rotMat.at[1][0] = (oneMinusCos * xy) + zs;
      rotMat.at[1][1] = (oneMinusCos * yy) + cosAngle;
      rotMat.at[1][2] = (oneMinusCos * yz) - xs;
      rotMat.at[1][3] = 0.0f;

      rotMat.at[2][0] = (oneMinusCos * zx) - ys;
      rotMat.at[2][1] = (oneMinusCos * yz) + xs;
      rotMat.at[2][2] = (oneMinusCos * zz) + cosAngle;
      rotMat.at[2][3] = 0.0f; 

      rotMat.at[3][0] = 0.0f;
      rotMat.at[3][1] = 0.0f;
      rotMat.at[3][2] = 0.0f;
      rotMat.at[3][3] = 1.0f;

      return this->multiply(rotMat);
    }
    return Matrix();
  }
};

struct Snowflake {
  Vertex3d position;
  GLfloat weight;
  GLfloat startX;
  GLfloat startZ;
  bool life;

  Snowflake() : life(true) {}
}; 

struct SnowPoint {
  GLfloat x;
  GLfloat z;
  GLfloat a;

  SnowPoint(GLfloat x, GLfloat z, GLfloat a) : x(x), z(z), a(a) {}
};

struct Snow {
  std::vector<Snowflake> flakes;
  bool allDead;
  size_t dead;

  Snow() : allDead(true), dead(0) {}
};

struct Flower {
  Vertex3d position;
  GLfloat size;
  RGBColor centerColor;
  RGBColor petalColor;
  GLfloat scaleFactor;
  GLfloat angle;
  GLfloat phase;

  Flower(Vertex3d, GLfloat, GLfloat, RGBColor, RGBColor);
  void swingFlower(Model*);
  void growFlower(Model*);
  void hideFlower(Model*);
};

struct Fruit {
  Vertex3d position;
  Vertex3d initPosition;
  Vertex3d fallPoint;
  GLfloat size;
  RGBColor color;
  GLfloat scaleFactor;
  GLfloat phase;
  bool stopped;
  int age;
  
  Fruit(Vertex3d, Vertex3d, GLfloat, RGBColor, int);
  void swingFruit(Model*);
  void growFruit(Model*);
  void hideFruit(Model*);
  void fallFruit(Model*);
};

struct Floor {
  static RGBColor COLOR[6];
  RGBColor background;
  std::vector<SnowPoint> snowPoints;

  void tickFloor(int, int, int, int);
  Floor* clone() {
    Floor* clone = new Floor();
    clone->background = this->background;
    clone->snowPoints = this->snowPoints;
    return clone;
  }
};

struct Leaf {
  Vertex3d position;
  Vertex3d initPosition;
  Vertex3d fallPoint;
  GLfloat perc;
  GLfloat size;
  GLfloat scaleFactor;
  GLfloat angle;
  GLfloat phase;
  RGBColor color;
  RGBColor initColor;
  int age;
  bool stopped;

  Leaf(Vertex3d, Vertex3d, GLfloat, GLfloat, GLfloat, RGBColor, int, bool);
  void growLeaf(Model*);
  void swingLeaf(Model*);
  void yellowLeaf(Model*);
  void fallLeaf(Model*);
  void hideLeaf(Model*);
};

struct Tree {
  Vertex3d position;
  GLfloat logHeight;
  GLfloat logWidth;
  GLfloat snowTransparence;
  struct Branch {
    static const int CHILDNUM = 3;
    GLfloat angleZ;
    GLfloat angleY;
    std::vector<Leaf> leaves;
    Branch* child[CHILDNUM];
    std::vector<Flower> flowers;
    std::vector<Fruit> fruits;
    
    Branch() {
      for (int i=0; i<CHILDNUM; i++) {
        this->child[i] = NULL;
      }
    }
    ~Branch() {
      
      for (int i=0; i<CHILDNUM; i++) {
        if (this->child[i] != NULL) {
          delete this->child[i];
        }
      }
    }
    Branch* clone();
  } *root;

  static Tree* newTree(Vertex3d position, GLfloat angleZ, GLfloat angleY, GLfloat logHeight, GLfloat logWidth, unsigned char depth) {
    Tree* tree = new Tree(position, logHeight, logWidth);
    Matrix transformation;
    tree->root = branchFractal(transformation, angleZ, angleY, position, logHeight, logWidth, depth);
    return tree;
  }

  static Tree::Branch* branchFractal(Matrix transformation, GLfloat angleZ,
    GLfloat angleY, Vertex3d initPoint, GLfloat height, GLfloat width,
    unsigned char depth) {

    Vertex3d finalPoint(0, height, 0);

    transformation = transformation.rotate(angleY, 0, 1, 0); // M = Ry * M
    transformation = transformation.rotate(angleZ, 0, 0, 1); // M = Rz * M

    finalPoint = transformation.multiply(finalPoint);

    finalPoint.x += initPoint.x;
    finalPoint.y += initPoint.y;
    finalPoint.z += initPoint.z;

    // Crea un ramo vuoto, direzionato
    Tree::Branch* fractal = new Branch();
    fractal->angleZ = angleZ;
    fractal->angleY = angleY;

    // Scegli un numero di foglie e creale
    int foglie = (depth > 4) ? 0 : rand() % (depth + 20) + 1;
    for (int i=0; i<foglie; i++) {
      GLfloat perc = std::max(rand() % 100, rand() % 100) / 100.0;
      GLfloat leaf_angle = rand() % 360;
      Vertex3d position(
        width * sinf(leaf_angle * M_PI / 180),
        sqrtf((finalPoint.x - initPoint.x) * (finalPoint.x - initPoint.x) + (finalPoint.y - initPoint.y) * 
            (finalPoint.y - initPoint.y) + (finalPoint.z - initPoint.z) * (finalPoint.z - initPoint.z)) * perc,
        width * cosf(leaf_angle * M_PI / 180)
      );
      position = transformation.multiply(position);
      position.x += initPoint.x;
      position.y += initPoint.y;
      position.z += initPoint.z;

      Vertex3d fallPoint(
        position.x,
        0,
        position.z 
      );
      GLfloat size = 0.02f;
      RGBColor color((rand() % 4) / 10.0f, 1.0f - (rand() % 3) / 10.0f, (rand() % 3) / 10.0f, 1);
      int age = rand() % 256;

      fractal->leaves.push_back(Leaf(position, fallPoint, size, leaf_angle, perc, color, age, false));
    }
    
    // Scegli un numero di fiori e frutti e creali
    int flowers = (depth > 4) ? 0 : rand() % 4;
    for (int i=0; i<flowers; i++) {
      GLfloat percF = std::max(rand() % 100, rand() % 100) / 100.0;
      GLfloat fruit_angle = rand() % 360;
      GLfloat flower_angle = fruit_angle;
      Vertex3d position(
        width * sinf(fruit_angle * M_PI / 180),
        sqrtf((finalPoint.x - initPoint.x) * (finalPoint.x - initPoint.x) + (finalPoint.y - initPoint.y) * 
            (finalPoint.y - initPoint.y) + (finalPoint.z - initPoint.z) * (finalPoint.z - initPoint.z)) * percF,
        width * cosf(fruit_angle * M_PI / 180)
      );
      position = transformation.multiply(position);
      position.x += initPoint.x;
      position.y += initPoint.y;
      position.z += initPoint.z;
      Vertex3d fallPointFruit(
        position.x,
        0,
        position.z
      );
      GLfloat sizeF =  ((rand() % 3) + 2.0f) / 300.0f;
      GLfloat sizeFruit = 0.03f;
      RGBColor centerColor(0.96, 0.76, 0.1, 1);
      RGBColor petalColor(0.9, 0.1, 0.6, 1);
      RGBColor fruitColor(1 + ((rand()%10 - 5.0f) / 100.0f), 0.6f + ((rand()%10 - 5.0f) / 100.0f), 0, 1);

      int age = rand() % 256;
      fractal->flowers.push_back(Flower(position, sizeF, flower_angle, centerColor, petalColor));    
      fractal->fruits.push_back(Fruit(position, fallPointFruit, sizeFruit, fruitColor, age));    
    }

    // Se non ci sono rami "figli" ho finito
    if (depth == 0) {
      return fractal;
    }

    // Scegli le direzioni per i figli e creali
    int angles[] = {0, 45, 315};
    std::random_shuffle(angles, angles+3);
    int children = (depth > 2) ? 3 : rand() % 2 + 1;

    for (int i=0; i<children; i++) {
      GLfloat new_angleZ = (angles[i] + rand() % 30 - 15) / 180.0 * M_PI;
      GLfloat new_angleY = (angles[(i + 1) % children] + rand() % 30 - 15) / 180.0 * M_PI;
      fractal->child[i] = branchFractal(transformation, new_angleZ, new_angleY, finalPoint, height * 0.65, width * 0.6, depth - 1);
    }
    return fractal;
  }
  Tree(Vertex3d, GLfloat, GLfloat);
  Tree* clone();
  ~Tree() {
    delete root;
  }
};

struct Sun {
  Vertex3d position;
  GLfloat w;
  GLfloat radius;
  GLfloat angle;
  GLfloat distance;

  Sun(Vertex3d position, GLfloat w, GLfloat radius, GLfloat angle, GLfloat distance) : position(position), w(w), radius(radius), angle(angle), distance(distance) {}
  Sun* clone() {
    Sun* clone = new Sun(position, w, radius, angle, distance);
    return clone;
  }
};

struct Cloud {
  RGBColor color;
  GLfloat height;
  int pieces;
  GLfloat scale;
  GLfloat speed;
  GLfloat angle;
  GLfloat distance;

  Cloud(RGBColor color, GLfloat height, int pieces, GLfloat scale, GLfloat speed, GLfloat angle, GLfloat distance) :
    color(color), height(height), pieces(pieces), scale(scale), speed(speed), angle(angle), distance(distance) {}
  void step(Model*);
};

struct Model {
  std::vector<Vertex3d> points;
  int screenWidth;
  int screenHeight;
  struct Counter {
    static const int resolution = 1 << 8;
    int value;

    Counter() : value(0) {}
  } counter;
  Sun *sun;
  Tree *tree, *shrub1, *shrub2, *shrub3, *shrub4;
  Snow snow;
  Floor* floor;
  Year current;
  std::vector<Cloud> clouds;

  Model(Sun*, Floor*, Tree*, Tree*, Tree*, Tree*, Tree*, int, int);
  ~Model();
  Model* clone();
  void markSnowflake(GLfloat, GLfloat, int);
  void tick();
  void tickLeaves(Tree::Branch*, std::vector<void (Leaf::*)(Model*)>&);
  void tickFlowers(Tree::Branch*, std::vector<void (Flower::*)(Model*)>&);
  void tickFruits(Tree::Branch*, std::vector<void (Fruit::*)(Model*)>&);
};
