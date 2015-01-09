#include "TestCase4.hpp"


using namespace std;
using namespace Dodge;


// Tests functions Entity::setTranslation_abs() and Entity::setRotation_abs()


void TestCase4::setup() {
   m_entities.push_back(pEntity_t(new Entity(internString("BigShape0"), internString("Polygon"))));
   m_entities.back()->setShape(unique_ptr<Shape>(new Quad(Vec2f(0.4, 0.01))));
   m_entities.back()->setFillColour(Colour(0.0, 0.2, 0.7, 1.0));
   m_entities.back()->setLineColour(Colour(0.0, 1.0, 0.0, 1.0));
   m_entities.back()->setLineWidth(3);
   m_entities.back()->setTranslation(0.2, 0.1);
   m_entities.back()->setZ(1);
   m_entities.back()->setRotation(45.f);
   m_entities.back()->addToWorld();

   m_entities.push_back(pEntity_t(new Entity(internString("BigShape1"), internString("Polygon"))));
   m_entities.back()->setShape(unique_ptr<Shape>(new Quad(Vec2f(0.15, 0.075))));
   m_entities.back()->setFillColour(Colour(0.8, 0.4, 0.0, 1.0));
   m_entities.back()->setLineColour(Colour(0.0, 1.0, 0.0, 1.0));
   m_entities.back()->setLineWidth(3);
   m_entities.back()->setZ(3);
   m_entities.back()->rotate(30.f);
   m_entities.back()->addToWorld();

   m_entities[0]->addChild(m_entities[1]);
}

void TestCase4::update() {
   m_entities[0]->rotate(1.f);

   m_entities[0]->removeChild(m_entities[1]);

   m_entities[0]->rotate(-2.f);

   m_entities[0]->addChild(m_entities[1]);

   m_entities[0]->rotate(1.5f);

   m_entities[1]->setTranslation_abs(0.5f, 0.5f);
   m_entities[1]->setRotation_abs(30.f);

   for (unsigned int i = 0; i < m_entities.size(); ++i)
      m_entities[i]->update();

   for (unsigned int i = 0; i < m_entities.size(); ++i) {
      const Range& r = m_entities[i]->getBoundary();
      r.dbg_draw(Colour(), Colour(1.0, 0.0, 0.0, 0.5), 1, 5.f);

      m_entities[i]->draw();
   }
}

void TestCase4::terminate() {
   m_entities.clear();
}

TestCase4::~TestCase4() {
   terminate();
}
