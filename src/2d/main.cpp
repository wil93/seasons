#include "model.hpp"
#include "view.hpp"
#include <iostream>
#include <thread>

static int timebase = 0;
static int virt_frames = 0;
static int real_frames = 0;
static Model* model;
static View* view;

static void idle() {
  while (1) {
    int time = glutGet(GLUT_ELAPSED_TIME);
    model->tick();
    virt_frames++;
    time = 16 - glutGet(GLUT_ELAPSED_TIME) + time;
    if (time > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
  }
}

void keyboard(unsigned char key, int x, int y) {
  printf("%d %d\n", x, y);
  printf("kb: %c\n", key);
}

int main(int argc, char** argv) {
  srand(time(NULL));
  glutInit(&argc, argv);

  model = new Model();
  view = new View(model);

  glutDisplayFunc([](){
    int time = glutGet(GLUT_ELAPSED_TIME);
    if (time - timebase > 1000) {
      std::cerr << "FPS (virt): " << virt_frames * 1000.0 / (time - timebase) << std::endl;
      std::cerr << "FPS (real): " << real_frames * 1000.0 / (time - timebase) << std::endl;
      std::cerr << std::endl;
      timebase = time;
      virt_frames = real_frames = 0;
    }
    view->render();
    real_frames++;
    time = 16 - glutGet(GLUT_ELAPSED_TIME) + time;
    if (time > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(time));
    }
  });

  glutIdleFunc([]() {
    glutPostRedisplay();
  });

  glutKeyboardFunc(keyboard);

  std::thread idleThread(idle);
  view->start();

  delete model;
  delete view;
}
