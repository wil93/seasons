#include "model.hpp"
#include <cassert>
#include <cmath>
#include <string.h>
#include <algorithm>

Model::Model(Sun* sun, Floor* floor, Tree* tree, Tree* shrub1, Tree* shrub2, Tree* shrub3, Tree* shrub4, int snowflakes, int clouds) {
  // Crea il sole
  this->sun = sun;

  // Crea le nuvole
  for (int i=0; i<clouds; i++) {
    this->clouds.push_back(Cloud(
      RGBColor(0.9, 0.9, 0.9, 1.0),
      rand() / float(RAND_MAX) + 2,
      rand() % 2 + 2,
      1.0f,
      (rand() % 10) * 3 / 10.0f + 3,
      2 * float(M_PI) * rand() / float(RAND_MAX),
      1.5f * rand() / float(RAND_MAX) + 4
    ));
  }

  // Crea il pavimento
  this->floor = floor;

  // Crea gli alberi
  this->tree = tree;
  this->shrub1 = shrub1;
  this->shrub2 = shrub2;
  this->shrub3 = shrub3;
  this->shrub4 = shrub4;

  // Aggiunge la neve
  for (int i=0; i<snowflakes; i++) { // creazione fiocchi
    this->snow.flakes.push_back(Snowflake());
    this->snow.flakes.back().position.y =  (rand() % 100) / 33.0f + 4;
    this->snow.flakes.back().weight = 0.015f + (rand() % 100) / 15000.0f;
    GLfloat randAngle = (rand() / float(RAND_MAX)) * 2 * M_PI;
    this->snow.flakes.back().startX = (rand() % 500) * sin(randAngle) / 100.0f;
    this->snow.flakes.back().startZ = (rand() % 500) * cos(randAngle) / 100.0f;
  }
}

Model::~Model() {
  delete this->tree;
  delete this->shrub1;
  delete this->shrub2;
  delete this->shrub3;
  delete this->shrub4;
  delete this->floor;
  delete this->sun;
}

Model* Model::clone() {
  Model* clone = new Model(NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0);
  clone->sun = new Sun(this->sun->position, this->sun->w, this->sun->radius, this->sun->angle, this->sun->distance);
  clone->floor = this->floor->clone();
  clone->tree = this->tree->clone();
  clone->shrub1 = this->shrub1->clone();
  clone->shrub2 = this->shrub2->clone();
  clone->shrub3 = this->shrub3->clone();
  clone->shrub4 = this->shrub4->clone();
  clone->counter = this->counter;
  clone->current = this->current;
  clone->clouds = this->clouds;
  clone->snow = this->snow;
  return clone;
}

void Floor::tickFloor(int from, int to, int step, int steps) {
  if (step % 16) {
    return;  // fai un tick sì e 15 no, altrimenti è troppo lento
  }
  this->background = RGBColor(
    this->COLOR[from].r + step * (this->COLOR[to].r - this->COLOR[from].r) / steps,
    this->COLOR[from].g + step * (this->COLOR[to].g - this->COLOR[from].g) / steps,
    this->COLOR[from].b + step * (this->COLOR[to].b - this->COLOR[from].b) / steps,
    1.0f
  );
}

void Year::cicle() {
  if (this->period == this->NUM_PERIODS) {
    if (this->season == this->NUM_SEASONS) {
      this->season = 0;
    }
    this->season++;
    std::cerr << "STAGIONE = " << this->season << "\n";
    this->period=0;
  }
  this->period++;
}

void Model::tick() {
  std::vector<void (Leaf::*)(Model*)> leavesFuncs;
  std::vector<void (Flower::*)(Model*)> flowersFuncs;
  std::vector<void (Fruit::*)(Model*)> fruitsFuncs;
  switch (this->current.season) {
  case 1: //primavera
    if (this->current.period == 1) {
      this->floor->tickFloor(4, 1, this->counter.value, this->counter.resolution);
      if (this->tree->snowTransparence > 0) {
        this->tree->snowTransparence -= 0.01f;
      }
    }

    if (!this->floor->snowPoints.empty()) {
      for (auto& sp: this->floor->snowPoints) {
        sp.a -= 0.01f;
      }
    }

    // Roba delle foglie
    leavesFuncs.push_back(&Leaf::growLeaf);
    leavesFuncs.push_back(&Leaf::swingLeaf);
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);
    this->tickLeaves(this->shrub3->root, leavesFuncs);
    this->tickLeaves(this->shrub4->root, leavesFuncs);
    
    flowersFuncs.push_back(&Flower::growFlower);
    flowersFuncs.push_back(&Flower::swingFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);
    this->tickFlowers(this->shrub3->root, flowersFuncs);
    this->tickFlowers(this->shrub4->root, flowersFuncs);
    
    break;
  case 2: // estate
    if (!this->floor->snowPoints.empty()) {
      this->floor->snowPoints.clear();
    }

    if (this->current.period == 1) {
      this->floor->tickFloor(1, 2, this->counter.value, this->counter.resolution);
    }

    leavesFuncs.push_back(&Leaf::swingLeaf);
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);
    this->tickLeaves(this->shrub3->root, leavesFuncs);
    this->tickLeaves(this->shrub4->root, leavesFuncs);

    flowersFuncs.push_back(&Flower::swingFlower);
    flowersFuncs.push_back(&Flower::hideFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);
    this->tickFlowers(this->shrub3->root, flowersFuncs);
    this->tickFlowers(this->shrub4->root, flowersFuncs);
    
    fruitsFuncs.push_back(&Fruit::swingFruit);
    fruitsFuncs.push_back(&Fruit::growFruit);
    this->tickFruits(this->tree->root, fruitsFuncs);
    this->tickFruits(this->shrub1->root, fruitsFuncs);
    this->tickFruits(this->shrub2->root, fruitsFuncs);
    this->tickFruits(this->shrub3->root, fruitsFuncs);
    this->tickFruits(this->shrub4->root, fruitsFuncs);

    break;
  case 3: // autunno
    if (this->current.period == 1) {
      this->floor->tickFloor(2, 3, this->counter.value, this->counter.resolution);
    }

    leavesFuncs.push_back(&Leaf::swingLeaf);
    if (this->current.period < 5) {
      leavesFuncs.push_back(&Leaf::yellowLeaf);
    }
    if (this->current.period > 3) {
      leavesFuncs.push_back(&Leaf::fallLeaf);
    }
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);    
    this->tickLeaves(this->shrub3->root, leavesFuncs);
    this->tickLeaves(this->shrub4->root, leavesFuncs);    

    flowersFuncs.push_back(&Flower::swingFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);
    this->tickFlowers(this->shrub3->root, flowersFuncs);
    this->tickFlowers(this->shrub4->root, flowersFuncs);

    fruitsFuncs.push_back(&Fruit::fallFruit);
    if (this->current.period > 1) 
      fruitsFuncs.push_back(&Fruit::hideFruit);
    this->tickFruits(this->tree->root, fruitsFuncs);
    this->tickFruits(this->shrub1->root, fruitsFuncs);
    this->tickFruits(this->shrub2->root, fruitsFuncs);
    this->tickFruits(this->shrub3->root, fruitsFuncs);
    this->tickFruits(this->shrub4->root, fruitsFuncs);

    break;
  case 4: // inverno
    if (this->current.period == 1) {
      this->floor->tickFloor(3, 4, this->counter.value, this->counter.resolution);
    }

    if (this->current.period < 3) {
      leavesFuncs.push_back(&Leaf::hideLeaf);
    }
    if (this->current.period < 3 && this->tree->snowTransparence < 1) {
      this->tree->snowTransparence += 0.002f;
    }
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);
    this->tickLeaves(this->shrub3->root, leavesFuncs);
    this->tickLeaves(this->shrub4->root, leavesFuncs);

    // Muovi neve
    for (Snowflake& flake: this->snow.flakes) {
      if (this->current.period < this->current.NUM_PERIODS - 2) {
        if (this->snow.allDead) {
          flake.life = true;
          this->snow.dead++;
          if (this->snow.dead == this->snow.flakes.size()) {
            this->snow.allDead = 0;
            this->snow.dead = 0;
          }
        }
      }
      if (flake.life) {
        flake.position.y -= flake.weight;
        flake.position.x =  flake.startX +sinf(flake.startX + 2 * M_PI * counter.value / counter.resolution)/3 ;
        flake.position.z = flake.startZ + cosf(flake.startZ + 2 * M_PI * counter.value / counter.resolution)/3 ;
        if (flake.position.y < 0 ) {
          this->floor->snowPoints.push_back(SnowPoint(flake.position.x, flake.position.z, 1.0f));
          flake.position.y = (rand() % 100) / 33.0f + 4.0f;
          GLfloat randAngle = (rand() / float(RAND_MAX)) * 2 * M_PI;
          flake.startX = (rand() % 500) * sin(randAngle) / 100.0f;
          flake.startZ = (rand() % 500) * cos(randAngle) / 100.0f;
          if (this->current.period > this->current.NUM_PERIODS - 2) {
            flake.life = 0;
            this->snow.dead++;
            if (this->snow.dead == this->snow.flakes.size()) {
              this->snow.allDead = true;
              this->snow.dead = 0;
            }
          }
        }
      }
    }
    // fine muovi neve
    break;
  }

  // Muovi il sole
  this->sun->angle += 0.01f;
  this->sun->position.x = sinf(this->sun->angle) * this->sun->distance;
  this->sun->position.z = cosf(this->sun->angle) * this->sun->distance;

  // Muovi le nuvole
  for (auto& cloud: this->clouds) {
    cloud.step(this);
  }

  // Aggiorna il contatore
  counter.value++;
  if (counter.value == counter.resolution) {
    counter.value = 0;
    this->current.cicle();
  }
}

void Cloud::step(Model* model) {
  this->angle += this->speed / 500;

  float scale[] = {
    1, 0, 1.1, 1.3
  };

  RGBColor color[] = {
    RGBColor(0.9, 0.9, 0.9, 1.0),
    RGBColor(0.9, 0.9, 0.9, 1.0),
    RGBColor(0.6, 0.6, 0.6, 1.0),
    RGBColor(0.3, 0.3, 0.3, 1.0),
  };

  int curr = model->current.season - 1;
  int prev = (model->current.season + 2) % 4;
  float perc = 1.0f * model->counter.value / model->counter.resolution;

  if (model->current.period == 1) {
    this->scale = scale[curr] * perc + scale[prev] * (1 - perc);
    this->color = color[curr] * perc + color[prev] * (1 - perc);
  } else {
    this->scale = scale[curr];
    this->color = color[curr];
  }
}

RGBColor Floor::COLOR[] = {
  RGBColor(0.32f, 0.48f, 0.25f, 1.0f),  // collina
  RGBColor(0.57f, 0.79f, 0.83f, 1.0f),  // primavera
  RGBColor(0.38f, 0.82f, 0.89f, 1.0f),  // estate
  RGBColor(0.41f, 0.59f, 0.61f, 1.0f),  // autunno
  RGBColor(0.66f, 0.66f, 0.66f, 1.0f),  // inverno
  RGBColor(0.20f, 0.70f, 0.10f, 1.0f)   // erba
};

Leaf::Leaf(Vertex3d position, Vertex3d fallPoint, GLfloat size, GLfloat angle, GLfloat perc, RGBColor color, int age, bool stopped) {
  this->position = position;
  this->initPosition = position;
  this->size = size;
  this->angle = angle;
  this->color = color;
  this->initColor = color;
  this->age = age;
  this->fallPoint = fallPoint;
  this->perc = perc;
  this->scaleFactor = 0;
  this->stopped = stopped;
}

Tree::Tree(Vertex3d position, GLfloat logHeight, GLfloat logWidth) {
  this->position = position;
  this->logHeight = logHeight;
  this->logWidth = logWidth;
  this->snowTransparence = 0.0f;

  // Genera i rami
  Matrix transformation;
  this->root = root;
}

Tree* Tree::clone() {
  Tree* clone = new Tree(this->position, this->logHeight, this->logWidth);
  clone->snowTransparence = this->snowTransparence;
  clone->root = this->root->clone();
  return clone;
}

Tree::Branch* Tree::Branch::clone() {
  Tree::Branch* clone = new Tree::Branch();
  clone->angleZ = this->angleZ;
  clone->angleY = this->angleY;
  for (auto leaf: this->leaves) {
    clone->leaves.push_back(leaf);
  }
  for (auto flower: this->flowers) {
    clone->flowers.push_back(flower);
  }
  for (auto fruit: this->fruits) {
    clone->fruits.push_back(fruit);
  }
  for (int i=0; i<this->CHILDNUM; i++) {
    if (this->child[i] != NULL) {
      clone->child[i] = this->child[i]->clone();
    }
  }
  return clone;
}

Flower::Flower(Vertex3d position, GLfloat size, GLfloat angle, RGBColor centerColor, RGBColor petalColor) {
  this->position = position;
  this->size = size;
  this->angle = angle;
  this->centerColor = centerColor;
  this->petalColor = petalColor;
  this->scaleFactor = 0;
}

Fruit::Fruit(Vertex3d position, Vertex3d fallPoint, GLfloat size, RGBColor color, int age) {
  this->position = position;
  this->initPosition = position;
  this->fallPoint = fallPoint;
  this->size = size;
  this->color = color;
  this->age = age;
  this->scaleFactor = 0;
  this->stopped = false;
}

// Crescita foglie
void Leaf::growLeaf(Model* model) {
  if (scaleFactor < 1)
    this->scaleFactor += 1.0f / (model->counter.resolution * 2);
  this->color.a = 1;
}

// Oscillamento foglie
void Leaf::swingLeaf(Model* model) {
  if (!this->stopped)
    this->phase = sinf(2 * M_PI * model->counter.value / model->counter.resolution + this->angle) * 15;
}

// Ingiallimento foglie
void Leaf::yellowLeaf(Model* model) {
  if (color.r < 0.7)
    this->color.r += 0.002;
  if (color.g > 0.5) 
    this->color.g -= 0.0004;
}

// Caduta foglie
void Leaf::fallLeaf(Model* model) {
  if (!this->stopped && this->age < model->counter.value + (model->counter.resolution * (model->current.period - 4))) {
    this->position.y -= 0.005;
    this->position.x += sinf(this->angle + 2 * M_PI * model->counter.value / model->counter.resolution) * 0.005;
    this->position.z += cosf(this->angle + 2 * M_PI * model->counter.value / model->counter.resolution) * 0.005;
    if (this->position.y < this->fallPoint.y + 0.01) {
      this->stopped = true;
    }
  }
}

// Scomparsa foglie
void Leaf::hideLeaf(Model* model) {
  if (color.a > 0)
    this->color.a -= 0.003;
  if (color.a < 0) {
    this->scaleFactor = 0;
    this->position = initPosition;
    this->color = initColor;
    this->stopped = false;
  }
}


void Model::tickLeaves(Tree::Branch* root, std::vector<void (Leaf::*)(Model*)> &funcs) {
  if (root == NULL) {
    return;
  }

  // Esegui le funzioni richieste su ciascuna foglia
  for (auto& leaf: root->leaves) {
    for (auto func: funcs) {
      std::bind(func, &leaf, this)();
    }
  }
  
  // Ricorri in tutti i rami figli
  for (auto& branch: root->child) {
      tickLeaves(branch, funcs);
  }
}

// Crescita fiori
void Flower::growFlower(Model* model) {
  if (this->scaleFactor < 1)
    this->scaleFactor += 1.0f / (model->counter.resolution * 2);
  this->centerColor.a = 1;
  this->petalColor.a = 1;
}

// Scomparsa fiori
void Flower::hideFlower(Model* model) {
  if (this->centerColor.a > 0)
    this->centerColor.a -= 0.004;
  if (this->petalColor.a > 0)
    this->petalColor.a -= 0.004;
  if (this->petalColor.a < 0) {
    this->scaleFactor = 0;
  }
}

// Oscillamento fiori
void Flower::swingFlower(Model* model) {
  this->phase = sinf(2 * M_PI * model->counter.value / model->counter.resolution + this->position.x) * 15;
}

void Model::tickFlowers(Tree::Branch* root, std::vector<void (Flower::*)(Model*)> &funcs) {
  if (root == NULL) {
    return;
  }
  
  for (auto& flower: root->flowers) {
    for (auto func: funcs) {
      std::bind(func, &flower, this)();
    }
  }
  
  // Ricorri in tutti i rami figli
  for (auto& branch: root->child) {
      tickFlowers(branch, funcs);
  }
}

// Crescita frutti
void Fruit::growFruit(Model* model) {
  if (this->scaleFactor < 1)
    this->scaleFactor += 0.5f / (model->counter.resolution * 2);
  this->color.a = 1;
  this->stopped = false;
  this->position = initPosition;
}

//Scomparsa frutti
void Fruit::hideFruit(Model* model) {
  if (this->color.a > 0)
    this->color.a -= 0.004;
  if (this->color.a < 0) {
    this->scaleFactor = 0;
  }
}

// Oscillamento frutti
void Fruit::swingFruit(Model* model) {
  this->phase = sinf(2 * M_PI * model->counter.value / model->counter.resolution + this->position.x) * 15;
}

// Caduta frutti
void Fruit::fallFruit(Model* model) {
  if (!this->stopped && this->age < model->counter.value + (model->counter.resolution * (model->current.period - 1))) {
    this->position.y -= 0.02;
    if (this->position.y < this->fallPoint.y) {
      this->stopped = true;
    }
  }
}

void Model::tickFruits(Tree::Branch* root, std::vector<void (Fruit::*)(Model*)> &funcs) {
  if (root == NULL) {
    return;
  }

  for (auto& fruit: root->fruits) {
    for (auto func: funcs) {
      std::bind(func, &fruit, this)();
    }
  }

  // Ricorri in tutti i rami figli
  for (auto& branch: root->child) {
    tickFruits(branch, funcs);
  }
}
