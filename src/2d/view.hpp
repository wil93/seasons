#pragma once
#include "model.hpp"

class View {
private:
  Model* model;

public:
  View(Model*);
  void start();
  void render();
  void drawCloud(const Cloud&);
  void drawFloor();
  void drawTree(const Tree&);
  void drawBranches(Tree::Branch*, Vertex2d, GLfloat, GLfloat, int, int);
  void drawLeaves(Tree::Branch*);
  void drawFlowers(Tree::Branch*);
  void drawFruits(Tree::Branch*);
};
