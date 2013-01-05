/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2013
 */

#ifndef __MAP_LOADER_HPP__
#define __MAP_LOADER_HPP__


#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include "../utils/Functor.hpp"
#include "xml/xml.hpp"
#include "AssetManager.hpp"
#include "Range.hpp"
#include "Exception.hpp"
#include "math/Vec2i.hpp"


namespace Dodge {


// The user of this class should make sure to not hold onto any assets that belong to a segment,
// as this would cause the asset to persist (due to boost::shared_ptr) and to later be reloaded,
// thereby duplicating it. Erasing assets from the asset manager is safe, however.
class MapLoader {
   public:
      void initialise(Functor<void, TYPELIST_1(const XmlNode)> setMapSettingsFunc,
         Functor<pAsset_t, TYPELIST_1(const XmlNode)> factoryFunc,
         Functor<void, TYPELIST_1(pAsset_t)> deleteAssetFunc,
         size_t targetMemoryUsage);

      void parseMapFile(const std::string& file);
      void update(const Vec2f& cameraPos);
      inline void freeAllAssets();

      inline const Range& getMapBoundary() const;

#ifdef DEBUG
      inline size_t dbg_getMemoryUsage() const;
#endif

   private:
      struct mapSegment_t {
         mapSegment_t() : loaded(false) {}

         std::string filePath;
         std::vector<long> assetIds;
         bool loaded;
      };

      class refCountTable_t {
         private:
            typedef unsigned int refCount_t;
            typedef long id_t;

         public:
            void incrRefCount(id_t id);
            void decrRefCount(id_t id);
            refCount_t getRefCount(id_t id) const;
            void clear();

         private:
            void insert(id_t id, refCount_t refCount);
            void erase(id_t id);

            // We want to look-up by id, but sort by reference count.
            // This requires two separate containers.
            std::set<std::pair<refCount_t, id_t> > m_byRefCount;
            std::map<id_t, refCount_t> m_byId;
      };

      static bool m_init;

      static refCountTable_t m_refCountTable;

      // Segments at the front of the list have been pending the longest
      static std::list<Vec2i> m_pendingUnload;

      static AssetManager m_assetManager;

      static Functor<void, TYPELIST_1(const XmlNode)> m_setMapSettingsFunc;
      static Functor<pAsset_t, TYPELIST_1(const XmlNode)> m_factoryFunc;
      static Functor<void, TYPELIST_1(pAsset_t)> m_deleteAssetFunc;

      static Range m_mapBoundary;
      static std::vector<std::vector<mapSegment_t> > m_segments;
      static Vec2i m_centreSegment;
      static Vec2f m_segmentSize;

      static size_t m_targetMemUsage;
      static size_t m_currentMemUsage;

      Vec2i getSegment(const Vec2f& pos) const;
      void setPendingUnload(const Vec2i& indices, bool b);
      void loadMapSettings(const XmlNode data);
      void loadSegment(const Vec2i& indices);
      void unloadSegments();
      void loadAssets(const XmlNode data, mapSegment_t* segment);
      void parseAssetsFile_r(const std::string& path, mapSegment_t* segment);

      size_t getMemoryUsage() const;
};

#ifdef DEBUG
//===========================================
// MapLoader::dbg_getMemoryUsage
//===========================================
inline size_t MapLoader::dbg_getMemoryUsage() const {
   if (!m_init)
      throw Exception("Error retrieving memory usage; MapLoader not initialised", __FILE__, __LINE__);

   return getMemoryUsage();
}
#endif

//===========================================
// MapLoader::freeAllAssets
//===========================================
inline void MapLoader::freeAllAssets() {
   if (!m_init)
      throw Exception("Error freeing assets; MapLoader not initialised", __FILE__, __LINE__);

   m_assetManager.freeAllAssets();
   m_currentMemUsage = 0;
   m_pendingUnload.clear();
   
}

//===========================================
// MapLoader::getMapBoundary
//===========================================
inline const Range& MapLoader::getMapBoundary() const {
   if (!m_init)
      throw Exception("Error retrieving map boundary; MapLoader not initialised", __FILE__, __LINE__);

   return m_mapBoundary;
}


}


#endif