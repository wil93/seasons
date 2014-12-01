#include "model.hpp"
#include <cassert>
#include <cmath>
#include <algorithm>

Model::Model() {
  this->screenWidth  = 0.7 * glutGet(GLUT_SCREEN_WIDTH);
  this->screenHeight = 0.7 * glutGet(GLUT_SCREEN_HEIGHT);

  // Crea il sole
  this->sun = new Sun();
  this->sun->circles.push_back(Sun::Circle(
    Vertex2d(this->screenWidth * 0.15f, this->screenHeight * 0.8f),
    55, 2, 1, RGBColor(1.0, 0.9, 0.0, 0.3)
  ));
  this->sun->circles.push_back(Sun::Circle(
    Vertex2d(this->screenWidth * 0.15f, this->screenHeight * 0.8f),
    45, 2, 0, RGBColor(1.0, 0.95, 0.0, 0.9)
  ));
  this->sun->circles.push_back(Sun::Circle(
    Vertex2d(this->screenWidth * 0.15f, this->screenHeight * 0.8f),
    40, 0, 0, RGBColor(1.0, 1.0, 0.0, 1.0)
  ));

  // Crea le nuvole
  for (int i=0; i<8; i++) {
    int circles = rand() % 3 + 3;
    this->clouds.push_back(Cloud(
      Vertex2d(0, this->screenHeight * (rand() % 30 + 60) / 100.0f),
      RGBColor(0.9, 0.9, 0.9, 1.0),
      1.0f * circles, 0.5f * circles * circles * circles, 0,
      (rand() % 10) * 3 / 10.0f + 3
    ));
  }

  // Crea il pavimento
  this->floor = new Floor(this->screenWidth, 2 * this->screenHeight / 5);

  this->tree = new Tree(
    Vertex2d(this->screenWidth / 2.0f, this->screenHeight / 15.0f),
    0,
    this->screenHeight * 3 / 10.0, this->screenWidth / 80.0,
    5, this->floor, false
  );
  this->shrub1 = new Tree(
    Vertex2d(this->screenWidth / 2.0f, this->tree->position.y + this->tree->logHeight),
    M_PI / 8,
    this->tree->logHeight / 2.0, this->tree->logWidth / 2,
    3, this->floor, true
  );
  this->shrub2 = new Tree(
    Vertex2d(this->screenWidth / 2.0f, this->tree->position.y + this->tree->logHeight),
    -M_PI / 8,
    this->tree->logHeight / 2.0f, this->tree->logWidth / 2.0,
    3, this->floor, true
  );
  
  // Aggiunge la neve
  for (int i=0; i<this->snow.SNOW_FLAKES; i++) {
    this->snow.flakes.push_back(Snowflake());
    this->snow.flakes.back().position.y = rand() % screenHeight + screenHeight + 2;
    this->snow.flakes.back().weight = (rand() % 100) / 100.0f + 2.0f;
    this->snow.flakes.back().startX = rand() % screenWidth;
    this->snow.flakes.back().lastZ = rand() % (int)(this->floor->height / 2 - this->snow.flakes.back().weight) / 1.0f + this->snow.flakes.back().weight;
  }  
}

Model::~Model() {
  delete this->tree;
  delete this->shrub1;
  delete this->shrub2;
  delete this->floor;
}

void Year::cicle() {
  if(this->period == this->NUM_PERIODS){
    if(this->season == this->NUM_SEASONS) 
      this->season=0;
    this->season++;
    std::cerr << "STAGIONE = " << this->season << "\n";
    this->period=0;
  } 
  this->period++;
}

void Model::markSnowflake(GLfloat posX, GLfloat posY, int dimension){
  for (int i=-dimension;i<=dimension;i++){
    for (int j=-dimension;j<=dimension;j++){
      if (posX+i < this->floor->width && posX+i >= 0 && posY+j < this->floor->height / 2 && posY+j >= 0) {
        int distance = abs(i) + abs(j);
        int pos = (int)posX + i + ((int)posY + j) * this->screenWidth;
        GLfloat instantR = 0.8f * (dimension * 2 + 1 - distance) / (dimension * 2 + 1);
        GLfloat instantG = 0.3f * (dimension * 2 + 1 - distance) / (dimension * 2 + 1);
        GLfloat instantB = 0.9f * (dimension * 2 + 1 - distance) / (dimension * 2 + 1);
        this->floor->snow_pixels[3 * pos + 0] += instantR;
        this->floor->snow_pixels[3 * pos + 1] += instantG;
        this->floor->snow_pixels[3 * pos + 2] += instantB;
      }
    }
  }
}

void Floor::tickFloor(int from, int to, int step, int steps, int screenWidth) {
  if (step % 16) {
    return;  // fai un tick sì e 15 no, altrimenti è troppo lento
  }
  for (int i=0; i < this->width * this->height * 3 ; i+=3) {
    int x = (i/3) % screenWidth;
    int y = (i/3) / screenWidth;
    this->background = RGBColor(
      this->COLOR[from].r + step * (this->COLOR[to].r - this->COLOR[from].r) / steps,
      this->COLOR[from].g + step * (this->COLOR[to].g - this->COLOR[from].g) / steps,
      this->COLOR[from].b + step * (this->COLOR[to].b - this->COLOR[from].b) / steps,
      1.0f
    );
    if (y < this->height / 2) {
      float padding = 10;
      float fix = (this->height / 2 - y < padding) * (padding - this->height / 2 + y) / padding;
      this->pixels[i + 0] = this->COLOR[5].r + fix * (this->COLOR[0].r - this->COLOR[5].r);
      this->pixels[i + 1] = this->COLOR[5].g + fix * (this->COLOR[0].g - this->COLOR[5].g);
      this->pixels[i + 2] = this->COLOR[5].b + fix * (this->COLOR[0].b - this->COLOR[5].b);
    } else {
      float border = sinf(x * 2 * M_PI / screenWidth) * 30 + (this->height - 30);
      float padding = 15;
      if (y > border) {
        this->pixels[i + 0] = this->background.r;
        this->pixels[i + 1] = this->background.g;
        this->pixels[i + 2] = this->background.b;
      } else {
        float fix = (border - y < padding) * (padding - border + y) / padding;
        this->pixels[i + 0] = COLOR[0].r + fix * (this->background.r - COLOR[0].r);
        this->pixels[i + 1] = COLOR[0].g + fix * (this->background.g - COLOR[0].g);
        this->pixels[i + 2] = COLOR[0].b + fix * (this->background.b - COLOR[0].b);
      }
    }
  }
}

void Model::tick() {
  std::vector<void (Leaf::*)(Model*)> leavesFuncs;
  std::vector<void (Flower::*)(Model*)> flowersFuncs;
  std::vector<void (Fruit::*)(Model*)> fruitsFuncs;
  switch (this->current.season) {
  case 1: //primavera
    if (this->current.period == 1) {
      this->floor->tickFloor(4, 1, this->counter.value, this->counter.resolution, this->screenWidth);
      if (this->tree->snowTransparence > 0) {
        this->tree->snowTransparence -= 0.008f;
      }
    }

    if (this->snow.allDead == 1) {
      this->snow.Dflake=0;
      for (Snowflake& flake: this->snow.flakes) {
        flake.position.y=rand() % screenHeight + screenHeight + 2;
        flake.life = 1;
        this->snow.Dflake++;
        if (this->snow.Dflake==this->snow.SNOW_FLAKES-1){
          this->snow.allDead=false;
          this->snow.Dflake=0;
        }
      }
    }

    for (int i=0; i < (this->floor->height / 2) * this->floor->width * 3; i+=3) {
      if (0 > (this->floor->snow_pixels[i + 0] -= 0.016))
        this->floor->snow_pixels[i + 0] = 0;
      if (0 > (this->floor->snow_pixels[i + 1] -= 0.006))
        this->floor->snow_pixels[i + 1] = 0;
      if (0 > (this->floor->snow_pixels[i + 2] -= 0.018))
        this->floor->snow_pixels[i + 2] = 0;
    }

    // Roba delle foglie
    leavesFuncs.push_back(&Leaf::growLeaf);
    leavesFuncs.push_back(&Leaf::swingLeaf);
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);
    
    flowersFuncs.push_back(&Flower::growFlower);
    flowersFuncs.push_back(&Flower::swingFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);
    
    break;
  case 2: // estate
    if (this->current.period == 1) {
      this->floor->tickFloor(1, 2, this->counter.value, this->counter.resolution, this->screenWidth);
    }

    leavesFuncs.push_back(&Leaf::swingLeaf);
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);

    flowersFuncs.push_back(&Flower::swingFlower);
    flowersFuncs.push_back(&Flower::hideFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);
    
    fruitsFuncs.push_back(&Fruit::swingFruit);
    fruitsFuncs.push_back(&Fruit::growFruit);
    this->tickFruits(this->tree->root, fruitsFuncs);
    this->tickFruits(this->shrub1->root, fruitsFuncs);
    this->tickFruits(this->shrub2->root, fruitsFuncs);

    break;
  case 3: // autunno
    if (this->current.period == 1) {
      this->floor->tickFloor(2, 3, this->counter.value, this->counter.resolution, this->screenWidth);
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

    flowersFuncs.push_back(&Flower::swingFlower);
    this->tickFlowers(this->tree->root, flowersFuncs);
    this->tickFlowers(this->shrub1->root, flowersFuncs);
    this->tickFlowers(this->shrub2->root, flowersFuncs);

    fruitsFuncs.push_back(&Fruit::fallFruit);
    if (this->current.period > 1) 
      fruitsFuncs.push_back(&Fruit::hideFruit);
    this->tickFruits(this->tree->root, fruitsFuncs);
    this->tickFruits(this->shrub1->root, fruitsFuncs);
    this->tickFruits(this->shrub2->root, fruitsFuncs);

    break;
  case 4: // inverno
    if (this->current.period == 1) {
      this->floor->tickFloor(3, 4, this->counter.value, this->counter.resolution, this->screenWidth);
    }

    if (this->current.period < 3) {
      leavesFuncs.push_back(&Leaf::hideLeaf);
    }
    if (this->current.period < 3 && this->tree->snowTransparence < 0.9) {
      this->tree->snowTransparence += 0.002f;
    }
    this->tickLeaves(this->tree->root, leavesFuncs);
    this->tickLeaves(this->shrub1->root, leavesFuncs);
    this->tickLeaves(this->shrub2->root, leavesFuncs);

    // Muovi neve
    for (Snowflake& flake: this->snow.flakes) {
      if(flake.life){
        flake.position.y -= flake.weight;
        flake.position.x = flake.startX + sinf(flake.startX + 2 * M_PI * counter.value / counter.resolution) * 40;
        if (flake.position.y < flake.lastZ) {
          if (flake.position.x >= 0 && flake.position.x < this->floor->width) {
            this->markSnowflake(flake.position.x, flake.position.y, 5);
          }
          flake.position.y = rand() % 50 + screenHeight + 2;
          flake.startX = rand() % this->screenWidth;
          flake.lastZ  = rand() % (int)(this->floor->height / 2 - flake.weight) / 1.0f + flake.weight;
          if (this->current.period > this->current.NUM_PERIODS - 2) {
            flake.life = 0;
            this->snow.Dflake++;
            if(this->snow.Dflake == this->snow.SNOW_FLAKES - 1) {
              this->snow.allDead = 1;
            }
          }
        }
      }
    }
    break;
  }

  // Muove il sole
  for (auto& circle: this->sun->circles) {
    circle.angle += 0.05f;
  }

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
  if (model->current.season != 2) {
    if (this->position.x < -(this->width * this->height * this->scale)) {
      this->position.x = model->screenWidth + rand() % (model->screenWidth / 4);
      this->position.y = model->screenHeight * (rand() % 30 + 60) / 100.0;
      if (model->current.season == 1) {
        this->color = RGBColor(0.9, 0.9, 0.9, 1.0);
        this->scale = 1;
      } else if (model->current.season == 3) {
        this->color = RGBColor(0.5, 0.5, 0.5, 1.0);
        this->scale = 1.5;
      } else {
        this->color = RGBColor(0.3, 0.3, 0.3, 1.0);
        this->scale = 2;
      }
    }
  }
  this->position.x -= this->speedX;
}

Floor::Floor(int screenWidth, int screenHeight) {
  COLOR[0] = RGBColor(0.32f, 0.48f, 0.25f, 1.0f);  // collina
  COLOR[1] = RGBColor(0.57f, 0.79f, 0.83f, 1.0f);  // primavera
  COLOR[2] = RGBColor(0.38f, 0.82f, 0.89f, 1.0f);  // estate
  COLOR[3] = RGBColor(0.41f, 0.59f, 0.61f, 1.0f);  // autunno
  COLOR[4] = RGBColor(0.66f, 0.66f, 0.66f, 1.0f);  // inverno
  COLOR[5] = RGBColor(0.20f, 0.70f, 0.10f, 1.0f);  // erba

  this->width = screenWidth;
  this->height = screenHeight;

  int vectorSize = this->width * this->height;
  this->pixels = new float[vectorSize * 3];
  this->pixels_copy = new float[vectorSize * 3];
  int snowSize = this->width * (this->height / 2);
  this->snow_pixels = new float[snowSize * 3]();
}

Leaf::Leaf(Vertex2d position, Vertex2d fallPoint, GLfloat size, GLfloat angle, RGBColor color, int age, bool behindLog) {
  this->position = position;
  this->initPosition = position;
  this->size = size;
  this->angle = angle;
  this->color = color;
  this->initColor = color;
  this->age = age;
  this->fallPoint = fallPoint;
  this->behindLog = behindLog;
}

Tree::Tree(Vertex2d position, GLfloat angle, GLfloat logHeight, GLfloat logWidth, int depth, Floor* floor, bool behindLog) {
  this->position = position;
  this->logHeight = logHeight;
  this->logWidth = logWidth;

  // Genera i rami
  this->root = this->branchFractal(angle, 0, position, logHeight, depth, floor, behindLog);
}

Flower::Flower(Vertex2d position, int size, RGBColor centerColor, RGBColor petalColor) {
  this->position = position;
  this->size = size;
  this->centerColor = centerColor;
  this->petalColor = petalColor;
}

Fruit::Fruit(Vertex2d position, Vertex2d fallPoint, GLfloat size, RGBColor color, int age) {
  this->position = position;
  this->initPosition = position;
  this->fallPoint = fallPoint;
  this->size = size;
  this->color = color;
  this->age = age;
}

Tree::Branch* Tree::branchFractal(GLfloat cur_angle, GLfloat prev_angle, Vertex2d initPoint, GLfloat height, unsigned char depth, Floor* floor, bool behindLog) {
  Vertex2d finalPoint = Vertex2d(
    initPoint.x + height * cosf(prev_angle + cur_angle + M_PI_2),
    initPoint.y + height * sinf(prev_angle + cur_angle + M_PI_2)
  );

  // Crea un ramo vuoto, direzionato
  Tree::Branch* fractal = new Branch();
  fractal->angle = cur_angle;

  // Scegli un numero di foglie e creale
  int foglie = (depth > 4) ? 0 : rand() % (depth + 20) + 1;
  for (int i=0; i<foglie; i++) {
    GLfloat perc = std::max(rand() % 100, rand() % 100) / 100.0;
    Vertex2d position(
      initPoint.x * (1 - perc) + finalPoint.x * perc,
      initPoint.y * (1 - perc) + finalPoint.y * perc
    );
    Vertex2d fallPoint(
      position.x,
      (float) (((behindLog) ? rand() % (int)(floor->height / 4) + floor->height / 4: rand() % (int)floor->height / 2))
    );
    position.x += rand() % 20 - 10.0f;
    GLfloat size = 5.0f;
    GLfloat leaf_angle = (cur_angle + prev_angle) * 180 / M_PI + rand() % 90 - 45.0f;
    RGBColor color((rand() % 4) / 10.0f, 1.0f - (rand() % 3) / 10.0f, (rand() % 3) / 10.0f, 1);
    int age = rand() % 256;
    fractal->leaves.push_back(Leaf(position, fallPoint, size, leaf_angle, color, age, behindLog));
  }
  
  // Scegli un numero di fiori e frutti e creali
  int flowers = (depth > 4) ? 0 : rand() % 4;
  for (int i=0; i<flowers; i++) {
    GLfloat perc = std::max(rand() % 100, rand() % 100) / 100.0;
    Vertex2d position(
      initPoint.x * (1 - perc) + finalPoint.x * perc,
      initPoint.y * (1 - perc) + finalPoint.y * perc
    );
    Vertex2d fallPointFruit(
      position.x,
      (float) (rand() % (int)(floor->height / 2))
    );
    GLfloat size = rand() % 3 + 2;
    GLfloat sizeFruit = 8;
    RGBColor centerColor(0.96, 0.76, 0.1, 1);
    RGBColor petalColor(0.9, 0.1, 0.6, 1);
    RGBColor fruitColor(1 + ((rand()%10 - 5.0f) / 100.0f), 0.6f + ((rand()%10 - 5.0f) / 100.0f), 0, 1);

    int age = rand() % 256;
    fractal->flowers.push_back(Flower(position, size, centerColor, petalColor));
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
    GLfloat new_angle = (angles[i] + rand() % 20 - 10) / 180.0 * M_PI;
    fractal->child[i] = branchFractal(new_angle, cur_angle + prev_angle, finalPoint, height * 0.7, depth - 1, floor, behindLog);
  }
  return fractal;
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
  assert(model != NULL); // no warning
  if (color.r < 0.7)
    this->color.r += 0.002;
  if (color.g > 0.5) 
    this->color.g -= 0.0004;
}

// Caduta foglie
void Leaf::fallLeaf(Model* model) {
  if (!this->stopped && this->age < model->counter.value + (model->counter.resolution * (model->current.period - 4))) {
    this->position.y -= 1.5;
    this->position.x += sinf(this->angle + 2 * M_PI * model->counter.value / model->counter.resolution) * 1;
    if (this->position.y < this->fallPoint.y) {
      this->stopped = true;
      if (!this->behindLog && (this->position.x + this->size * 3 > model->tree->position.x - model->tree->logWidth 
          && this->position.x - this->size * 3 < model->tree->position.x + model->tree->logWidth) 
          && this->position.y > model->tree->position.y) {
        this->stopped = false;
      }
    }
  }
}

// Scomparsa foglie
void Leaf::hideLeaf(Model* model) {
  assert(model != NULL); // no warning
  if (color.a > 0) {
    this->color.a -= 0.003;
  }
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
  
  for(auto& leaf: root->leaves) {
    for(auto func: funcs) {
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
  if (this->scaleFactor < 1) {
    this->scaleFactor += 1.0f / (model->counter.resolution * 2);
  }
  this->centerColor.a = 1;
  this->petalColor.a = 1;
}

//Scomparsa fiori
void Flower::hideFlower(Model* model) {
  assert(model != NULL); // no warning
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
  
  for(auto& flower: root->flowers) {
    for(auto func: funcs) {
    
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
  assert(model != NULL); // no warning
  if (this->color.a > 0)
    this->color.a -= 0.004;
  if (this->color.a < 0) {
    this->scaleFactor = 0;
  }
}

// Oscillamento frutti
void Fruit::swingFruit(Model* model) {
  this->phase = sinf(2 * M_PI * model->counter.value / model->counter.resolution + this->position.x) * 10;
}

// Caduta frutti
void Fruit::fallFruit(Model* model) {
  if (!this->stopped && this->age < model->counter.value + (model->counter.resolution * (model->current.period - 1))) {
    this->position.y -= 8;
    if (this->position.y < this->fallPoint.y) {
      this->stopped = true;
      if (this->position.x + this->size > model->tree->position.x - model->tree->logWidth 
          && this->position.x - this->size < model->tree->position.x + model->tree->logWidth
          && this->position.y > model->tree->position.y) {
        this->stopped = false;
      }
    }
  }
}

void Model::tickFruits(Tree::Branch* root, std::vector<void (Fruit::*)(Model*)> &funcs) {
  if (root == NULL) {
    return;
  }

  for(auto& fruit: root->fruits) {
    for(auto func: funcs) {
    
      std::bind(func, &fruit, this)();
    }
  }

  // Ricorri in tutti i rami figli
  for (auto& branch: root->child) {
      tickFruits(branch, funcs);
  }
}
