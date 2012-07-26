/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#ifndef __ENTITY_HPP__
#define __ENTITY_HPP__


#ifdef DEBUG
#include <ostream>
#endif
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <cml/cml.h>
#include "EEvent.hpp"
#include "EventManager.hpp"
#include "CompoundPoly.hpp"
#include "rapidxml/rapidxml.hpp"
#include "Vec3f.hpp"


namespace Dodge {


class Entity : public boost::enable_shared_from_this<Entity> {
   public:
      Entity(long type);
      Entity(long name, long type);
      Entity(const Entity& copy);
      Entity(const Entity& copy, long name);

//      virtual void addToWorld() {}
//      virtual void removeFromWorld() {}

      virtual void onEvent(const EEvent* event) {}

      inline Vec3f getPosition() const;
      inline const CompoundPoly& getBoundingPoly() const;
      inline const cml::matrix44f_c& getMatrix() const;
      inline long getName() const;
      inline long getTypeName() const;
      inline const Vec3f& getScale() const;

      inline boost::shared_ptr<Entity> getSharedPtr();

      inline void setPosition(float32_t x, float32_t y, float32_t z);
      inline void setBoundingPoly(const CompoundPoly& poly);
      inline void setMatrix(const cml::matrix44f_c& mat);
      inline void setScale(const Vec3f& scale);

      virtual void draw(const Vec2f& at) const = 0;
      virtual void update() {}

      virtual void assignData(const rapidxml::xml_node<>* data);

#ifdef DEBUG
      virtual void dbg_print(std::ostream& out, int tab = 0) const;
#endif

      virtual ~Entity() {}

   protected:
      static EventManager m_eventManager;

   private:
      void recomputePoly();
      void deepCopy(const Entity& copy);

      long m_name;
      long m_type;
      Vec3f m_scale;
      cml::matrix44f_c m_matrix;
      CompoundPoly m_srcPoly;    // Bounding polygon
      CompoundPoly m_transPoly;  // m_srcPoly after transformation by m_matrix

      static int m_count;
      static long generateName();
};

typedef boost::shared_ptr<Entity> pEntity_t;


//===========================================
// Entity::getSharedPtr
//===========================================
inline boost::shared_ptr<Entity> Entity::getSharedPtr() {
   return shared_from_this();
}

//===========================================
// Entity::getPosition
//===========================================
inline Vec3f Entity::getPosition() const {
   return Vec3f(m_matrix(0, 3), m_matrix(1, 3), m_matrix(2, 3));
}

//===========================================
// Entity::setPosition
//===========================================
inline void Entity::setPosition(float32_t x, float32_t y, float32_t z) {
   m_matrix(0, 3) = x;
   m_matrix(1, 3) = y;
   m_matrix(2, 3) = z;
}

//===========================================
// Entity::getBoundingPoly
//===========================================
inline const CompoundPoly& Entity::getBoundingPoly() const {
   return m_transPoly;
}

//===========================================
// Entity::getMatrix
//===========================================
inline const cml::matrix44f_c& Entity::getMatrix() const {
   return m_matrix;
}

//===========================================
// Entity::getScale
//===========================================
inline const Vec3f& Entity::getScale() const {
   return m_scale;
}

//===========================================
// Entity::getName
//===========================================
inline long Entity::getName() const {
   return m_name;
}

//===========================================
// Entity::getTypeName
//===========================================
inline long Entity::getTypeName() const {
   return m_type;
}

//===========================================
// Entity::setBoundingPoly
//===========================================
inline void Entity::setBoundingPoly(const CompoundPoly& poly) {
   m_srcPoly = poly;
   recomputePoly();
}

//===========================================
// Entity::setMatrix
//===========================================
inline void Entity::setMatrix(const cml::matrix44f_c& mat) {
   m_matrix = mat;
}

//===========================================
// Entity::setScale
//===========================================
inline void Entity::setScale(const Vec3f& scale) {
   m_scale = scale;
}


}


#endif /*!__ENTITY_HPP__*/