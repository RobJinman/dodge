/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#include <cstring>
#include <cml/cml.h>
#include <Exception.hpp>
#include <math/shapes/LineSegment.hpp>
#include <StringId.hpp>
#include <renderer/Renderer.hpp>


using namespace cml;
using namespace std;


namespace Dodge {


//===========================================
// LineSegment::LineSegment
//===========================================
LineSegment::LineSegment(float32_t p1x, float32_t p1y, float32_t p2x, float32_t p2y)
   : m_model(Renderer::LINES),
     m_renderer(Renderer::getInstance()) {

   vvv_t verts[] = {
      {p1x, p1y, 0.f},
      {p2x, p2y, 0.f}
   };

   m_model.setVertices(0, verts, 2);
}

//===========================================
// LineSegment::LineSegment
//===========================================
LineSegment::LineSegment(const Vec2f& p1, const Vec2f& p2)
   : m_model(Renderer::LINES),
     m_renderer(Renderer::getInstance()) {

   vvv_t verts[] = {
      {p1.x, p1.y, 0.f},
      {p2.x, p2.y, 0.f}
   };

   m_model.setVertices(0, verts, 2);
}

//===========================================
// LineSegment::LineSegment
//===========================================
LineSegment::LineSegment(const XmlNode data)
   : m_model(Renderer::LINES),
     m_renderer(Renderer::getInstance()) {

   vvv_t verts[] = {
      {0.f, 0.f, 0.f},
      {0.f, 0.f, 0.f}
   };

   m_model.setVertices(0, verts, 2);

   try {
      XML_NODE_CHECK(data, LineSegment);

      XmlNode node = data.firstChild();
      XML_NODE_CHECK(node, Vec2f);
      Vec2f p1(node);

      node = node.nextSibling();
      XML_NODE_CHECK(node, Vec2f);
      Vec2f p2(node);

      m_model.setVertex(0, {p1.x, p1.y, 0.f});
      m_model.setVertex(1, {p2.x, p2.y, 0.f});
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class LineSegment; ");
      throw;
   }
}

//===========================================
// LineSegment::LineSegment
//===========================================
LineSegment::LineSegment(const LineSegment& copy)
   : m_model(copy.m_model),
     m_renderer(Renderer::getInstance()) {}

//===========================================
// LineSegment::operator=
//===========================================
LineSegment& LineSegment::operator=(const LineSegment& rhs) {
   m_model = rhs.m_model;
   return *this;
}

//===========================================
// LineSegment::typeId
//===========================================
long LineSegment::typeId() const {
   static long typeId = internString("LineSegment");

   return typeId;
}

//===========================================
// LineSegment::clone
//===========================================
LineSegment* LineSegment::clone() const {
   return new LineSegment(*this);
}

#ifdef DEBUG
//===========================================
// LineSegment::dbg_print
//===========================================
void LineSegment::dbg_print(std::ostream& out, int tab) const {
   for (int i = 0; i < tab; ++i) out << "\t";
   out << "LineSegment\n";

   for (int i = 0; i < tab + 1; ++i) out << "\t";
   out << "p1: (" << getPoint1().x << ", " << getPoint1().y << ")\n";

   for (int i = 0; i < tab + 1; ++i) out << "\t";
   out << "p2: (" << getPoint2().x << ", " << getPoint2().y << ")\n";
}
#endif

//===========================================
// LineSegment::setLineColour
//===========================================
void LineSegment::setLineColour(const Colour& colour) {
   m_model.setColour(colour);
}

//===========================================
// LineSegment::setLineWidth
//===========================================
void LineSegment::setLineWidth(int lineWidth) {
   m_model.setLineWidth(lineWidth);
}

//===========================================
// LineSegment::setRenderTransform
//===========================================
void LineSegment::setRenderTransform(float32_t x, float32_t y, int z) const {
   m_model.setMatrixElement(12, x);
   m_model.setMatrixElement(13, y);
   m_model.setMatrixElement(14, static_cast<float32_t>(z));
}

//===========================================
// LineSegment::render
//===========================================
void LineSegment::render() const {
   Renderer::getInstance().stageModel(&m_model);
}

//===========================================
// LineSegment::unrender
//===========================================
void LineSegment::unrender() const {
   Renderer::getInstance().unstageModel(&m_model);
}

//===========================================
// LineSegment::getMinimum
//===========================================
Vec2f LineSegment::getMinimum() const {
   Vec2f p1 = getPoint1();
   Vec2f p2 = getPoint2();

   return Vec2f(p1.x < p2.x ? p1.x : p2.x, p1.y < p2.y ? p1.y : p2.y);
}

//===========================================
// LineSegment::getMaximum
//===========================================
Vec2f LineSegment::getMaximum() const {
   Vec2f p1 = getPoint1();
   Vec2f p2 = getPoint2();

   return Vec2f(p1.x > p2.x ? p1.x : p2.x, p1.y > p2.y ? p1.y : p2.y);
}

//===========================================
// LineSegment::rotate
//===========================================
void LineSegment::rotate(float32_t rads, const Vec2f& pivot) {
   Vec2f p1 = getPoint1();
   Vec2f p2 = getPoint2();

   p1.rotate(pivot, rads);
   p2.rotate(pivot, rads);

   setPoint1(p1);
   setPoint2(p2);
}

//===========================================
// LineSegment::scale
//===========================================
void LineSegment::scale(const Vec2f& sv) {
   Vec2f p1 = getPoint1();
   Vec2f p2 = getPoint2();

   p1.x *= sv.x;
   p1.y *= sv.y;
   p2.x *= sv.x;
   p2.y *= sv.y;

   setPoint1(p1);
   setPoint2(p2);
}

//===========================================
// LineSegment::~LineSegment
//===========================================
LineSegment::~LineSegment() {
   m_renderer.unstageModel(&m_model);
}


}
