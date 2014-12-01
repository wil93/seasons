#include "model.hpp"
#include "view.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <cmath>
#include <cassert>

static int timebase = 0;
static int virt_frames = 0;
static int real_frames = 0;
static Model* model;
static View* view;
static std::mutex mutex;
static bool terminated = false;

static void idle() {
  while (1) {
    mutex.lock();
    if (terminated) {
      // Se il thread principale mi vuole morto...
      return;
    }
    int time = glutGet(GLUT_ELAPSED_TIME);
    // Esegui un "tick" nel tempo
    model->tick();
    virt_frames++;
    time = 16 - glutGet(GLUT_ELAPSED_TIME) + time;
    mutex.unlock();
    if (time > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
  }
}

void changeSize(int w, int h) {
  // Evita una divisione per zero
  if (h == 0) {
    h = 1;
  }
  float ratio = w * 1.0f / h;

  // Use the Projection Matrix
  glMatrixMode(GL_PROJECTION);

  // Reset Matrix
  glLoadIdentity();

  // Set the viewport to be the entire window
  glViewport(0, 0, w, h);

  // Set the correct perspective.
  gluPerspective(45.0f, ratio, 0.1f, 100.0f);

  // Get Back to the Modelview
  glMatrixMode(GL_MODELVIEW);
}

void processSpecialKeys(int key, int x, int y) {
  assert(x != y || x == y); // no warning

  int step = 1;
  
  switch (key) {
  case GLUT_KEY_LEFT:
    view->angle -= step ;
    view->angle %= 360;
    break;
  case GLUT_KEY_RIGHT:
    view->angle += step;
    view->angle %= 360;
    break;
  case GLUT_KEY_UP:
    view->radius -= 0.1;
    break;
  case GLUT_KEY_DOWN:
    view->radius += 0.1;
    break;
  }
}

void processKeys(unsigned char key, int x, int y) {
  assert(x != y || x == y); // no warning

  float step = 0.1f;

  switch (key) {
  case 'z':
    view->y += step;
    break;
  case 'x':
    view->y -= step;
    view->y = std::max(0.0f, view->y);
    break;
  }
}

int main(int argc, char** argv) {
  srand(time(NULL));
  glutInit(&argc, argv);

  int screenWidth  = 0.7 * glutGet(GLUT_SCREEN_WIDTH);
  int screenHeight = 0.7 * glutGet(GLUT_SCREEN_HEIGHT);

  Tree* mainTree = Tree::newTree(Vertex3d(0, 0, 0), 0, 0, 0.8, 0.05, 5);

  model = new Model(
    new Sun(Vertex3d(4, 4, 0), 1.0f, 0.4f, 0.0f, 5.0f),
    new Floor(),
    mainTree,
    Tree::newTree(
      Vertex3d(mainTree->position.x, mainTree->position.y + mainTree->logHeight, mainTree->position.z),
      M_PI / 3, M_PI_4,
      0.5, 0.03,
      4
    ),
    Tree::newTree(
      Vertex3d(mainTree->position.x, mainTree->position.y + mainTree->logHeight, mainTree->position.z),
      -M_PI / 3, -M_PI_4,
      0.5, 0.03,
      4
    ),
    Tree::newTree(
      Vertex3d(mainTree->position.x, mainTree->position.y + mainTree->logHeight, mainTree->position.z),
      M_PI / 3, M_PI_2,
      0.5, 0.03,
      4
    ),
    Tree::newTree(
      Vertex3d(mainTree->position.x, mainTree->position.y + mainTree->logHeight, mainTree->position.z),
      M_PI / 3, -M_PI_2,
      0.5, 0.03,
      4
    ),
    1000,
    8
  );
  view = new View(screenWidth, screenHeight);

  glutDisplayFunc([](){
    int time = glutGet(GLUT_ELAPSED_TIME);
    if (time - timebase > 1000) {
      std::cerr << "FPS (virt): " << virt_frames * 1000.0 / (time - timebase) << std::endl;
      std::cerr << "FPS (real): " << real_frames * 1000.0 / (time - timebase) << std::endl;
      std::cerr << std::endl;
      timebase = time;
      virt_frames = real_frames = 0;
    }

    // Copia il model
    mutex.lock();
    Model* clone = model->clone();
    mutex.unlock();

    // Renderizza con calma la copia
    view->render(*clone);
    real_frames++;

    // Elimina la copia
    delete clone;

    time = 16 - glutGet(GLUT_ELAPSED_TIME) + time;
    if (time > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
  });

  glutIdleFunc([]() {
    glutPostRedisplay();
  });

  glutReshapeFunc(changeSize);
  glutSpecialFunc(processSpecialKeys);
  glutKeyboardFunc(processKeys);

  std::thread idleThread(idle);
  view->start();

  // L'utente e' uscito, termino il thread secondario
  mutex.lock();
  terminated = true;
  mutex.unlock();
  idleThread.join();

  delete model;
  delete view;
}
