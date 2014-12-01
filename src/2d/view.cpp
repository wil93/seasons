#include "view.hpp"
#include <iostream>

View::View(Model* model) {
  this->model = model;

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(this->model->screenWidth, this->model->screenHeight);
  glutInitWindowPosition(100, 150);
  glutCreateWindow("Progetto 2D");
  gluOrtho2D(0.0, this->model->screenWidth, 0.0, this->model->screenHeight);
}

void View::start() {
  glutMainLoop();
}

void View::render() {
  // Disegna sfondo
  RGBColor bg = this->model->floor->background;
  glClearColor(bg.r, bg.g, bg.b, bg.a);
  glClear(GL_COLOR_BUFFER_BIT);

  // Abilitazione antialiasing
  glEnable(GL_LINE_SMOOTH);

  // Disegno base
  this->drawFloor();

  // Disegno sole
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  for (auto circle: this->model->sun->circles) {
    // Sfasa il cerchio
    circle.center.x += circle.phase * cosf(circle.angle);
    circle.center.y += circle.phase * sinf(circle.angle);

    // Disegnalo
    glColor4f(circle.color.r, circle.color.g, circle.color.b, circle.color.a);
    glBegin(GL_TRIANGLE_FAN);
      glVertex2f(circle.center.x, circle.center.y);
      for (int i=0; i<=100; ++i) {
        float t = 2 * M_PI * i / 100;
        glVertex2f(circle.center.x + sinf(t) * circle.radius,
                   circle.center.y + cosf(t) * circle.radius);
      }
    glEnd();
  }
  glDisable(GL_BLEND);

  // Disegno le nuvole
  for (auto cloud: this->model->clouds) {
    cloud.height *= cloud.scale;
    GLfloat height = cloud.height;
    glPushMatrix();
    glColor4f(cloud.color.r, cloud.color.g, cloud.color.b, cloud.color.a);
    glTranslatef(cloud.position.x + height, cloud.position.y, 0);
    for (int i=0; i<cloud.width; i++) {
      glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        for (int j=0; j<=50; ++j) {
          float t = M_PI * j / 50;
          glVertex2f(cosf(t) * height, sinf(t) * height);
        }
      glEnd();
      glTranslatef(height * 1.3, 0, 0);
      height *= 0.7;
    }
    glPopMatrix();
  }

  // Disegno albero
  drawTree(*this->model->shrub1);
  drawTree(*this->model->shrub2);
  drawTree(*this->model->tree);
  
  // Disegno neve
  glPointSize(5.0);
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_POINTS);
  for (Snowflake flake: this->model->snow.flakes) {
    if (flake.life) {
      glVertex2f(flake.position.x, flake.position.y);
    }
  }
  glEnd();

  glFlush();
}

void View::drawFloor() {
  memcpy(this->model->floor->pixels_copy, this->model->floor->pixels, this->model->floor->height * this->model->floor->width * 3 * sizeof(float));
  for (int i=0; i < this->model->floor->width * (this->model->floor->height / 2) * 3; i++) {
    this->model->floor->pixels_copy[i] += this->model->floor->snow_pixels[i];
  }
  glDrawPixels(this->model->floor->width, this->model->floor->height, GL_RGB, GL_FLOAT, this->model->floor->pixels_copy);
}

void View::drawTree(const Tree& tree) {
  // Disegna i rami
  glPushMatrix();
  glTranslatef(tree.position.x, tree.position.y, 0);
  drawBranches(tree.root, Vertex2d(0.0, 0.0), tree.logHeight, tree.logWidth, 0, 0);
  glPopMatrix();

  // Disegna le foglie
  drawLeaves(tree.root);
  
  // Disegna i fiori
  drawFlowers(tree.root);
  
  // Disegna i frutti
  drawFruits(tree.root);
}

void View::drawBranches(Tree::Branch* branch, Vertex2d start, GLfloat height, GLfloat width, int depth, int prevAngle) {
  if (branch == NULL) {
    return;
  }

  GLfloat pos[] = {
    0.0, 1.2, 1.4, 3.7, 2.4, 1.2, -1.0, -2.2, 0.0
  };
  GLfloat pct[] = {
    0.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 11.0
  };

  // Disegna il ramo
  glPushMatrix();
  glTranslatef(start.x, start.y, 0);
  glRotatef(branch->angle * 180 / M_PI, 0.0, 0.0, 1.0);
  glColor3f(0.4, 0.1, 0.0);
  glBegin(GL_QUAD_STRIP);
    for (int i=0; i<9; i++) {
      glVertex2f(pos[i] - width, height * pct[i] / 10);
      glVertex2f(pos[i] + width, height * pct[i] / 10);
    }
  glEnd();

  // Contorno ?
  glColor3f(0.3, 0.0, 0.0);
  glLineWidth(2.5);
  glBegin(GL_LINE_STRIP);
    for (int i=0; i<9; i++) {
      glVertex2f(pos[i] - width, height * pct[i] / 10);
    }
    for (int i=8; i>=0; i--) {
      glVertex2f(pos[i] + width, height * pct[i] / 10);
    }
  glEnd();

  // Linee casuali
  glColor3f(0.5, 0.2, 0.0);
  glLineWidth(1);
  for (int j=-6; j<6; j++) {
    glBegin(GL_LINE_STRIP);
      for (int i=0; i<9; i++) {
        glVertex2f(pos[((i+j > 8 || i+j < 0) ? i : i+j)] + cosf(2 * M_PI * i / 9) * width, height * pct[((i+j > 8 || i+j < 0) ? i : i+j)] / 10);
      }
    glEnd();
  }
  glColor3f(0.3, 0.0, 0.0);
  glLineWidth(1);
  for (int j=-6; j<6; j++) {
    glBegin(GL_LINE_STRIP);
      for (int i=0; i<9; i++) {
        glVertex2f(pos[((i+j > 8 || i+j < 0) ? i : i+j)] + sinf(2 * M_PI * i / 9) * width, height * pct[((i+j > 8 || i+j < 0) ? i : i+j)] / 10);
      }
    glEnd();
  }
  
  // Neve sull'albero (linee bianche)
  if (depth > 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(5);
    glColor4f(1, 1, 1, this->model->tree->snowTransparence);
    glBegin(GL_LINE_STRIP);
      for (int i=1; i<9; i++) {
        if ((int)((branch->angle + prevAngle) * 180 / M_PI + 90) % 360  < 90 || (int)((branch->angle + prevAngle) * 180 / M_PI + 90) % 360 > 270) {
          glVertex2f(pos[i] - width, height * pct[i] / 10);
        } else {
          glVertex2f(pos[i] + width, height * pct[i] / 10);
        }
      }
    glEnd();
    glDisable(GL_BLEND);
  }

  for (int i=0; i<this->model->tree->CHILDNUM; i++) {
    drawBranches(branch->child[i], Vertex2d(0.0, height), height * 0.7, width * 0.6, depth + 1, branch->angle + prevAngle);
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
    glTranslatef(leaf.position.x, leaf.position.y, 0.0);
    glRotatef(leaf.angle + leaf.phase, 0, 0, 1);
    glTranslatef(0, leaf.size, 0);
    glScalef(leaf.scaleFactor, leaf.scaleFactor, 1.0);
    
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

    // Contorno
    glColor4f(leaf.color.r - 0.1, leaf.color.g - 0.1, leaf.color.b, leaf.color.a);
    glLineWidth(1.5);
    glBegin(GL_LINE_STRIP);
      glVertex2f(leaf.size * 0.5, -leaf.size * 0.5);
      glVertex2f(leaf.size *2, leaf.size);
      glVertex2f(leaf.size, leaf.size);
      glVertex2f(0, leaf.size * 2);
      glVertex2f(-leaf.size, leaf.size);
      glVertex2f(-leaf.size * 2, leaf.size);
      glVertex2f(-leaf.size * 0.5, -leaf.size * 0.5);
      glVertex2f(leaf.size * 0.5, -leaf.size * 0.5);
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
    glTranslatef(flower.position.x, flower.position.y, 0.0);
    glRotatef(flower.phase, 0, 0, 1);
    glTranslatef(0, flower.size, 0);
    glScalef(flower.scaleFactor, flower.scaleFactor, 1.0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int nPetals = 5;
    std::vector<Vertex2d> vertexes;
    for (int i=0; i<nPetals; i++) {
      vertexes.push_back(Vertex2d(cosf(2 * M_PI * i/nPetals) * flower.size, sinf(2 * M_PI * i/nPetals) * flower.size));
    }

    // Petali
    glColor4f(flower.petalColor.r, flower.petalColor.g, flower.petalColor.b, flower.petalColor.a);
    for (int i=0; i<nPetals; i++) {
      glBegin(GL_POLYGON);
        for (int j=0; j<nPetals; j++) {
          glVertex2f(cosf(2 * M_PI * i/nPetals) * flower.size + vertexes[j].x, sinf(2 * M_PI * i/nPetals) * flower.size + vertexes[j].y);
        }
      glEnd();
    }

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
    glTranslatef(fruit.position.x, fruit.position.y, 0.0);
    glRotatef(fruit.phase, 0, 0, 1);
    glTranslatef(0, fruit.size, 0);
    glScalef(fruit.scaleFactor, fruit.scaleFactor, 1.0);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    int nVertexes = 8;
    // Cerchio
    glColor4f(fruit.color.r, fruit.color.g, fruit.color.b, fruit.color.a);
    glBegin(GL_POLYGON);
      for (int i=0; i<nVertexes; i++) {
        glVertex2f(cosf(2 * M_PI * i/nVertexes) * fruit.size, sinf(2 * M_PI * i/nVertexes) * fruit.size);
      }
    glEnd();
    
    // Contorno cerchio
    glColor4f(fruit.color.r - 0.1, fruit.color.g - 0.1, fruit.color.b - 0.1, fruit.color.a);
    glLineWidth(1.5);
    glBegin(GL_LINE_STRIP);
      for (int i=0; i<nVertexes; i++) {
        glVertex2f(cosf(2 * M_PI * i/nVertexes) * fruit.size, sinf(2 * M_PI * i/nVertexes) * fruit.size);
      }
    glEnd();

    glDisable(GL_BLEND);
  
    glPopMatrix();
  }

  for (auto newBranch: branch->child) {
    drawFruits(newBranch);
  }
}
