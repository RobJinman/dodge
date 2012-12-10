#include <dodge/dodge.hpp>
#include <iostream>


using namespace std;
using namespace Dodge;


WinIO win;
Renderer& renderer = Renderer::getInstance();
double frameRate = 1000000;
Polygon shape;


void quit() {
   renderer.stop();
   win.destroyWindow();
   exit(0);
}

void keyDown(int key) {
   switch (key) {
      case WinIO::KEY_ESCAPE: quit(); break;
      case WinIO::KEY_F:
         cout << "Frame rate (main loop): " << frameRate << "fps\n";
         cout << "Frame rate (render loop): " << renderer.getFrameRate() << "fps\n" << flush;
      break;
      case WinIO::KEY_1: shape.render();     break;
      case WinIO::KEY_2: shape.unrender();   break;
   }
}

void computeFrameRate() {
   static Timer timer;
   static long i = 0;
   i++;

   if (i % 1000 == 0) {
      frameRate = 1000.0 / timer.getTime();
      timer.reset();
   }
}

void onWindowResize(int w, int h) {
   renderer.onWindowResize(w, h);
   renderer.getCamera().setProjection(static_cast<float32_t>(w) / static_cast<float32_t>(h), 1.f);
}

int main() {
   try {
      win.init("Rotating Square", 240, 240, false);
      win.registerCallback(WinIO::EVENT_WINCLOSE, Functor<void, TYPELIST_0()>(quit));
      win.registerCallback(WinIO::EVENT_KEYDOWN, Functor<void, TYPELIST_1(int)>(keyDown));
      win.registerCallback(WinIO::EVENT_WINRESIZE, Functor<void, TYPELIST_2(int, int)>(onWindowResize));

      pCamera_t camera(new Camera(240.0 / 240.0, 1.f));
      renderer.attachCamera(camera);

      renderer.setBgColour(Colour(0.2, 0.2, 0.2, 1.0));

      renderer.start();

      shape.addVertex(0.3, 0.3);
      shape.addVertex(0.7, 0.3);
      shape.addVertex(0.7, 0.7);
      shape.addVertex(0.3, 0.7);

      shape.setLineWidth(3);
      shape.setFillColour(Colour(0.0, 0.2, 0.9, 1.0));
      shape.setLineColour(Colour(0.7, 0.1, 0.1, 1.0));

      shape.render();

      long i = 1;
      while (1) {
         win.doEvents();
         computeFrameRate();

//         if (i % 1000 == 0)
//            shape.rotate((180000.0) / frameRate, Vec2f(0.5f, 0.5f));

         if (i % 1000 == 0) {
            static float f = -0.5;
            shape.setRenderTransform(f, 0.0, 9);

            f += 1000.0 / frameRate;
         }

         renderer.checkForErrors();

         ++i;
      }
   }
   catch (Exception& e) {
      e.prepend("An error occurred; ");
      cerr << e.what() << flush;

      quit();
   }

   return 0;
}