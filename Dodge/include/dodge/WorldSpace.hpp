/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#ifndef __WORLD_SPACE_HPP__
#define __WORLD_SPACE_HPP__


#include <memory>
#include <set>
#include "EventManager.hpp"
#include "SpatialContainer.hpp"
#include "Entity.hpp"
#include "StringId.hpp"
#ifdef DEBUG
#include "renderer/Colour.hpp"
#endif


namespace Dodge {


class WorldSpace {
   public:
      inline void init(std::unique_ptr<SpatialContainer<pEntity_t> > container);

      inline void trackEntity(pEntity_t entity);
      inline void untrackEntity(pEntity_t entity);
      inline void untrackAll();
      inline void insertEntity(pEntity_t entity);
      inline void insertAndTrackEntity(pEntity_t entity);
      inline void removeEntity(pEntity_t entity);
      inline void removeAndUntrackEntity(pEntity_t entity);
      inline void removeAll();
      inline void removeAndUntrackAll();
      inline void getEntities(const Range& region, std::vector<pEntity_t>& entities) const;

#ifdef DEBUG
      inline void dbg_draw(const Colour& colour, Renderer::int_t lineWidth, int z) const;
#endif

   private:
      static bool m_init;

      static EventManager m_eventManager;
      static std::unique_ptr<SpatialContainer<pEntity_t> > m_container;
      static std::set<pEntity_t> m_tracking;

      void entityMovedHandler(EEvent* e);
};

//===========================================
// WorldSpace::init
//===========================================
inline void WorldSpace::init(std::unique_ptr<SpatialContainer<pEntity_t> > container) {
   m_container = std::move(container);

   Functor<void, TYPELIST_1(EEvent*)> fEntMovedHandler(this, &WorldSpace::entityMovedHandler);
   m_eventManager.registerCallback(internString("entityBoundingBox"), fEntMovedHandler);

   m_init = true;
}

//===========================================
// WorldSpace::trackEntity
//===========================================
inline void WorldSpace::trackEntity(pEntity_t entity) {
   if (!m_init)
      throw Exception("Error tracking entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_tracking.insert(entity);
}

//===========================================
// WorldSpace::untrackEntity
//===========================================
inline void WorldSpace::untrackEntity(pEntity_t entity) {
   if (!m_init)
      throw Exception("Error untracking entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_tracking.erase(entity);
}

//===========================================
// WorldSpace::untrackAll
//===========================================
inline void WorldSpace::untrackAll() {
   if (!m_init)
      throw Exception("Error untracking entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_tracking.clear();
}

//===========================================
// WorldSpace::insertEntity
//===========================================
inline void WorldSpace::insertEntity(pEntity_t entity) {
   if (!m_init)
      throw Exception("Error inserting entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_container->insert(entity, entity->getBoundary());
}

//===========================================
// WorldSpace::insertAndTrackEntity
//===========================================
inline void WorldSpace::insertAndTrackEntity(pEntity_t entity) {
   insertEntity(entity);
   trackEntity(entity);
}

//===========================================
// WorldSpace::removeEntity
//===========================================
inline void WorldSpace::removeEntity(pEntity_t entity) {
   if (!m_init)
      throw Exception("Error removing entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_container->remove(entity, m_container->getBoundary());
}

//===========================================
// WorldSpace::removeAll
//===========================================
inline void WorldSpace::removeAll() {
   if (!m_init)
      throw Exception("Error removing entity; WorldSpace not initialised", __FILE__, __LINE__);

   m_container->removeAll();
}

//===========================================
// WorldSpace::removeAndUntrackEntity
//===========================================
inline void WorldSpace::removeAndUntrackEntity(pEntity_t entity) {
   removeEntity(entity);
   untrackEntity(entity);
}

//===========================================
// WorldSpace::removeAndUntrackAll
//===========================================
inline void WorldSpace::removeAndUntrackAll() {
   removeAll();
   untrackAll();
}

//===========================================
// WorldSpace::getEntities
//===========================================
inline void WorldSpace::getEntities(const Range& region, std::vector<pEntity_t>& entities) const {
   if (!m_init)
      throw Exception("Error retrieving entities; WorldSpace not initialised", __FILE__, __LINE__);

   m_container->getEntries(region, entities);
}

#ifdef DEBUG
//===========================================
// WorldSpace::dbg_draw
//===========================================
inline void WorldSpace::dbg_draw(const Colour& colour, Renderer::int_t lineWidth, int z) const {
   if (!m_init)
      throw Exception("Error drawing worldspace; WorldSpace not initialised", __FILE__, __LINE__);

   m_container->dbg_draw(colour, lineWidth, z);
}
#endif


}


#endif
