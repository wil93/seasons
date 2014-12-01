#include "view.hpp"
#include <iostream>
#include <thread>
#include <cmath>

View::View(int screenWidth, int screenHeight) : angle(20), radius(3.0f), x(0), y(1), z(0) {
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(screenWidth, screenHeight);
  glutInitWindowPosition(100, 150);
  glutCreateWindow("Progetto 3D");
  gluOrtho2D(0.0, screenWidth, 0.0, screenHeight);

  // Creo la luce
  glEnable(GL_LIGHTING);

  glEnable(GL_LIGHT0);
  glEnable(GL_NORMALIZE);
  float position[] = {0, 3, 0, 1};
  float ambient[]  = {0, 0, 0, 1};
  float diffuse[]  = {1, 1, 1, 1};
  float specular[] = {1, 1, 1, 1};
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  // OpenGL init
  glEnable(GL_DEPTH_TEST);

  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
}

void View::start() {
#ifdef __linux__
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
  glutMainLoop();
}

void View::render(const Model& model) {
  // Disegna sfondo
  RGBColor bg = model.floor->background;
  glClearColor(bg.r, bg.g, bg.b, bg.a);

  GLfloat fogColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};

  if (model.current.season == 4) {  // inverno
    // Nebbia
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, fogColor);
    float density = 0.0f;
    if (model.current.period == 1) {
      // Aumenta gradualmente
      density = 0.15f * model.counter.value / model.counter.resolution;
    } else if (model.current.period == model.current.NUM_PERIODS) {
      // Diminuisci gradualmente
      density = 0.15f * (1 - 1.0 * model.counter.value / model.counter.resolution);
    } else {
      // Massima nebbia
      density = 0.15f;
    }
    glFogf(GL_FOG_DENSITY, density);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 1.0f);
    glFogf(GL_FOG_END, 5.0f);
    glEnable(GL_FOG);
  } else {
    glDisable(GL_FOG);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  // Compute camera coordinates
  this->x = this->radius * sinf(this->angle * M_PI / 180);
  this->z = this->radius * cosf(this->angle * M_PI / 180);

  // Set the camera
  gluLookAt(this->x, this->y, this->z,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f);

  // Abilitazione antialiasing
  glEnable(GL_LINE_SMOOTH);

  // Disegno base
  this->drawFloor(model);

  // Disegno sole
  glPushMatrix();
  glTranslatef(model.sun->position.x, model.sun->position.y, model.sun->position.z);
  {
    float emission[] = {1, 1, 0.2, 1};
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
  }
  {
    float position[] = {0, 0, 0, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, position);
  }
  glutSolidSphere(model.sun->radius, 20, 20);
  {
    float emission[] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT, GL_EMISSION, emission);
  }
  glPopMatrix();

  // Disegno le nuvole
  for (auto cloud: model.clouds) {
    GLfloat radius = 0.4;
    for (int i=0; i<cloud.pieces; i++) {
      glPushMatrix();
      glColor4f(cloud.color.r, cloud.color.g, cloud.color.b, cloud.color.a);
      glTranslatef(
        cloud.distance * cosf(cloud.angle - cloud.scale * 5 * i * M_PI / 180),
        cloud.height,
        cloud.distance * sinf(cloud.angle - cloud.scale * 5 * i * M_PI / 180)
      );
      glScalef(cloud.scale, cloud.scale, cloud.scale);
      glutSolidSphere(radius * cloud.scale, 8, 8);
      radius *= 0.85;
      glPopMatrix();
    }
  }

  // Disegno albero
  drawTree(model, *(model.shrub1));
  drawTree(model, *(model.shrub2));
  drawTree(model, *(model.shrub3));
  drawTree(model, *(model.shrub4));
  drawTree(model, *(model.tree));

  // Disegno neve
  for (Snowflake flake: model.snow.flakes) {
    if (flake.life) {
      glPushMatrix();
      glTranslatef(flake.position.x, flake.position.y, flake.position.z);
      glColor3f(1.0,1.0,1.0);
      glutSolidSphere(0.01, 4, 4);
      glPopMatrix();
    }
  }
  glEnd();

  // Disegno la neve per terra
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLUquadric* quad;
  for (auto sp: model.floor->snowPoints){ 
    quad = gluNewQuadric();
    glColor4f(1.0, 1.0, 1.0, std::max(0.0f, sp.a - 0.60f));
    glPushMatrix();
    glTranslatef(sp.x, 0.0001, sp.z);
    glRotatef(-90, 1, 0, 0);
    if(sp.a-0.60f>0){
      gluDisk(quad, 0, 0.15, 5, 1);
    }
    glPopMatrix();
    gluDeleteQuadric(quad);

    quad = gluNewQuadric();
    glColor4f(1.0, 1.0, 1.0, std::max(0.0f, sp.a - 0.40f));
    glPushMatrix();
    glTranslatef(sp.x, 0.0002, sp.z);
    glRotatef(-90, 1, 0, 0);
    if(sp.a-0.40f>0){
      gluDisk(quad, 0, 0.09, 5, 1);
    }
    glPopMatrix();
    gluDeleteQuadric(quad);

    quad = gluNewQuadric();
    glColor4f(1.0, 1.0, 1.0, std::max(0.0f, sp.a));
    glPushMatrix();
    glTranslatef(sp.x, 0.0003, sp.z);
    glRotatef(-90, 1, 0, 0);
    if(sp.a>0){
      gluDisk(quad, 0, 0.05, 5, 1);
    }
    glPopMatrix();
    gluDeleteQuadric(quad);
  }
  glDisable(GL_BLEND);

  glutSwapBuffers();
}

void View::drawFloor(const Model& model) {
  glPushMatrix();
  glRotatef(-90,1,0,0);

  glColor3f(model.floor->COLOR[5].r, model.floor->COLOR[5].g, model.floor->COLOR[5].b);
  GLUquadric* island = gluNewQuadric();
  gluDisk(island,0,6,20,1);
  glColor3f(0,0.7f,1.0);
  GLUquadric* sea = gluNewQuadric();
  gluDisk(sea,6,20,20,1);
  glPopMatrix();
}

void View::drawTree(const Model& model, const Tree& tree) {
  // Disegna la neve sui rami
  drawBranches(model, tree.root, Vertex3d(tree.position.x, tree.position.y + 0.0015f, tree.position.z), tree.logHeight, tree.logWidth, 0, true);
  
  // Disegna i rami
  drawBranches(model, tree.root, Vertex3d(tree.position.x, tree.position.y, tree.position.z), tree.logHeight, tree.logWidth + 0.001, 0, false);

  // Disegna le foglie
  drawLeaves(tree.root);

  // Disegna i fiori
  drawFlowers(tree.root);

  // Disegna i frutti
  drawFruits(tree.root);
}

void View::drawBranches(const Model& model, Tree::Branch* branch, Vertex3d start, GLfloat height, GLfloat width, int depth, bool neve) {
  if (branch == NULL) {
    return;
  }

  // Disegna il ramo
  glPushMatrix();

  glTranslatef(start.x, start.y, start.z);
  glRotatef(branch->angleY * 180 / M_PI, 0.0, 1.0, 0.0);
  glRotatef(branch->angleZ * 180 / M_PI, 0.0, 0.0, 1.0);

  if (neve) {
    glColor3f(0.4 + (model.tree->snowTransparence * 0.6), 0.1 + (model.tree->snowTransparence * 0.9), model.tree->snowTransparence);
  } else {
    glColor3f(0.4, 0.1, 0.0);
  }
  
  glPushMatrix();
  glRotatef(-90, 1, 0, 0);
  GLUquadric* quad = gluNewQuadric();
  gluCylinder(quad, width * 1.1, width * 0.9, height, 10, 4);
  glPopMatrix();

  for (int i=0; i<model.tree->root->CHILDNUM; i++) {
    drawBranches(model, branch->child[i], Vertex3d(0.0, height, 0.0), height * 0.65, width * 0.6, depth + 1, neve);
  }
  glPopMatrix();
}


void View::drawLeaves(Tree::Branch* branch) {
  if (branch == NULL) {
    return;
  }

  // Disegna le foglie
  for (auto leaf: branch->leaves) {
    glPushMatrix();
    glTranslatef(leaf.position.x, leaf.position.y, leaf.position.z);
    glRotatef(leaf.angle + leaf.phase, 0, 0, 1);
    glRotatef(leaf.angle + leaf.phase, 0, 1, 0);
    glTranslatef(0, leaf.size, 0);
    glScalef(leaf.scaleFactor, leaf.scaleFactor, leaf.scaleFactor);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Foglia
    glColor4f(leaf.color.r, leaf.color.g, leaf.color.b, leaf.color.a);
    glBegin(GL_POLYGON);
      glVertex2f(leaf.size * 0.5, -leaf.size * 0.5);
      glVertex2f(leaf.size *2, leaf.size);
      glVertex2f(leaf.size, leaf.size);
      glVertex2f(0, leaf.size * 2);
      glVertex2f(-leaf.size, leaf.size);
      glVertex2f(-leaf.size * 2, leaf.size);
      glVertex2f(-leaf.size * 0.5, -leaf.size * 0.5);
    glEnd();

    glDisable(GL_BLEND);

    glPopMatrix();
  }

  for (auto newBranch: branch->child) {
    drawLeaves(newBranch);
  }
}


void View::drawFlowers(Tree::Branch* branch) {
  if (branch == NULL) {
    return;
  }

  // Disegna i fiori
  for (auto flower: branch->flowers) {
    glPushMatrix();
    glTranslatef(flower.position.x, flower.position.y, flower.position.z);
    glRotatef(flower.angle, 0, 1, 0);
    glRotatef(flower.angle, 1, 0, 0);
    glRotatef(flower.phase, 0, 0, 1);
    glTranslatef(0, flower.size, 0);
    glScalef(flower.scaleFactor, flower.scaleFactor, flower.scaleFactor);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat nPetals = 5.0f;
    std::vector<Vertex3d> vertexes;
    for (int i=0; i<nPetals; i++) {
      vertexes.push_back(Vertex3d(cosf(2.0f * M_PI * i/nPetals) * flower.size, sinf(2.0f * M_PI * i/nPetals) * flower.size, 0 ));
    }

    // Petali
    glColor4f(flower.petalColor.r, flower.petalColor.g, flower.petalColor.b, flower.petalColor.a);
    for (int i=0; i<nPetals; i++) {
      glBegin(GL_POLYGON);
        for (int j=0; j<nPetals; j++) {
          glVertex3f(cosf(2 * M_PI * i/nPetals) * flower.size + vertexes[j].x, sinf(2 * M_PI * i/nPetals) * flower.size + vertexes[j].y, 0);
        }
      glEnd();
    }

    // Cerchio centrale
    glColor4f(flower.centerColor.r, flower.centerColor.g, flower.centerColor.b, flower.centerColor.a);
    glPushMatrix();
    glTranslatef(0, 0, 0.001f);
    glBegin(GL_POLYGON);
      for (int i=0; i<nPetals; i++) {
        glVertex3f(vertexes[i].x, vertexes[i].y, 0);
      }
    glEnd();
    glPopMatrix();
    
    glDisable(GL_BLEND);
    
    glPopMatrix();
  }

  for (auto newBranch: branch->child) {
    drawFlowers(newBranch);
  }
}

void View::drawFruits(Tree::Branch* branch) {
  if (branch == NULL) {
    return;
  }

  // Disegna i frutti
  for (auto fruit: branch->fruits) {
    glPushMatrix();
    glTranslatef(fruit.position.x, fruit.position.y, fruit.position.z);
    glRotatef(fruit.phase, 0, 0, 1);
    glRotatef(fruit.phase, 0, 1, 0);
    glTranslatef(0, fruit.size, 0);
    glScalef(fruit.scaleFactor, fruit.scaleFactor, fruit.scaleFactor);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    // Cerchio
    glColor4f(fruit.color.r, fruit.color.g, fruit.color.b, fruit.color.a);
    GLUquadric* quad = gluNewQuadric();
    gluSphere(quad, fruit.size, 8, 8);

    glDisable(GL_BLEND);
  
    glPopMatrix();
  }

  for (auto newBranch: branch->child) {
    drawFruits(newBranch);
  }
}
