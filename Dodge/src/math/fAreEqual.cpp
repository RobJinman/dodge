/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#include <memory>
#include <map>
#include <math/common.hpp>
#include <math/fAreEqual.hpp>
#include <math/shapes/LineSegment.hpp>
#include <math/shapes/Ellipse.hpp>
#include <math/shapes/Polygon.hpp>
#include <math/Vec2f.hpp>
#include <Exception.hpp>
#include <StringId.hpp>


using namespace Dodge;
using namespace std;


namespace {


typedef bool (*funcPtr_t)(const Shape&, const Shape&);
typedef std::map<std::pair<long, long>, funcPtr_t> dispatchTable_t;


//===========================================
// lsegLsegAreEqual
//===========================================
bool lsegLsegAreEqual(const Shape& lseg1, const Shape& lseg2) {
   return static_cast<const LineSegment&>(lseg1).LineSegment::operator==(static_cast<const LineSegment&>(lseg2));
}

//===========================================
// lsegEllipseAreEqual
//===========================================
bool lsegEllipseAreEqual(const Shape& lseg, const Shape& elps) {
   throw Exception("Error in operator==(); Attempt to compare LineSegment with Ellipse", __FILE__, __LINE__);
}

//===========================================
// lsegPolyAreEqual
//===========================================
bool lsegPolyAreEqual(const Shape& lseg, const Shape& poly) {
   throw Exception("Error in operator==(); Attempt to compare LineSegment with Polygon", __FILE__, __LINE__);
}

//===========================================
// ellipseEllipseAreEqual
//===========================================
bool ellipseEllipseAreEqual(const Shape& elps1, const Shape& elps2) {
   return static_cast<const Dodge::Ellipse&>(elps1).Ellipse::operator==(static_cast<const Dodge::Ellipse&>(elps2));
}

//===========================================
// ellipsePolyAreEqual
//===========================================
bool ellipsePolyAreEqual(const Shape& elps, const Shape& poly) {
   throw Exception("Error in operator==(); Attempt to compare Ellipse with Polygon", __FILE__, __LINE__);
}

//===========================================
// ellipseBoxAreEqual
//===========================================
bool ellipseBoxAreEqual(const Shape& elps, const Shape& box) {
   throw Exception("Error in operator==(); Attempt to compare Ellipse with Box", __FILE__, __LINE__);
}

//===========================================
// polyPolyAreEqual
//===========================================
bool polyPolyAreEqual(const Shape& poly1, const Shape& poly2) {
   return static_cast<const Dodge::Polygon&>(poly1).Polygon::operator==(static_cast<const Dodge::Polygon&>(poly2));
}

//===========================================
// makePair
//===========================================
pair<long, long> makePair(long a, long b) {
   return a < b ? pair<long, long>(a, b) : pair<long, long>(b, a);
}

//===========================================
// initDispatchTable
//===========================================
dispatchTable_t* initDispatchTable() {
   dispatchTable_t* tbl = new dispatchTable_t;

   (*tbl)[makePair(internString("LineSegment"), internString("LineSegment"))] = &lsegLsegAreEqual;
   (*tbl)[makePair(internString("LineSegment"), internString("Ellipse"))] = &lsegEllipseAreEqual;
   (*tbl)[makePair(internString("LineSegment"), internString("Polygon"))] = &lsegPolyAreEqual;

   (*tbl)[makePair(internString("Ellipse"), internString("Ellipse"))] = &ellipseEllipseAreEqual;
   (*tbl)[makePair(internString("Ellipse"), internString("Polygon"))] = &ellipsePolyAreEqual;

   (*tbl)[makePair(internString("Polygon"), internString("Polygon"))] = &polyPolyAreEqual;

   return tbl;
}


}


//===========================================
// operator==
//===========================================
bool operator==(const Shape& obj1, const Shape& obj2) {
   static unique_ptr<dispatchTable_t> tbl(initDispatchTable());

   dispatchTable_t::iterator it = tbl->find(makePair(obj1.typeId(), obj2.typeId()));
   if (it == tbl->end())
      throw Exception("Error in operator==(); Unrecognised shape types", __FILE__, __LINE__);

   return it->second(obj1, obj2);
}
