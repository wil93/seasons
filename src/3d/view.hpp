#pragma once
#include "model.hpp"

struct View {
  // Angolo di rivoluzione della telecamera
  int angle;
  // Deistanza telecamera-centro
  float radius;
  // Posizione della telecamera
  float x, y, z;

  View(int, int);
  void start();
  void render(const Model&);
  void drawCloud(const Cloud&);
  void drawFloor(const Model&);
  void drawTree(const Model&, const Tree&);
  void drawBranches(const Model&, Tree::Branch*, Vertex3d, GLfloat, GLfloat, int, bool);
  void drawLeaves(Tree::Branch*);
  void drawFlowers(Tree::Branch*);
  void drawFruits(Tree::Branch*);
};
