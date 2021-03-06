/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#include <renderer/Renderer.hpp>
#include <globals.hpp>
#include <ui/EntityUi.hpp>
#include <ui/EUiEvent.hpp>
#include <Entity.hpp>


using namespace std;


namespace Dodge {


WinIO EntityUi::m_winIO = WinIO();
EventManager EntityUi::m_eventManager = EventManager();


//===========================================
// EntityUi::EntityUi
//===========================================
EntityUi::EntityUi(Entity* entity)
   : m_entity(entity), m_hasFocus_(false), m_hasFocus(false), m_mouseOver(false), m_active(false) {

   m_winIO.registerCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::mouseMoveHandler));
}

//===========================================
// EntityUi::EntityUi
//===========================================
EntityUi::EntityUi(const EntityUi& copy, Entity* entity)
   : m_entity(entity), m_hasFocus_(false), m_hasFocus(false), m_mouseOver(false), m_active(false) {

   m_winIO.registerCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::mouseMoveHandler));
}

//===========================================
// EntityUi::addToWorld
//===========================================
void EntityUi::addToWorld() {
   m_winIO.registerCallback(WinIO::EVENT_BTN1PRESS, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn1PressHandler));
   m_winIO.registerCallback(WinIO::EVENT_BTN1RELEASE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn1ReleaseHandler));
   m_winIO.registerCallback(WinIO::EVENT_BTN3PRESS, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn3PressHandler));
   m_winIO.registerCallback(WinIO::EVENT_BTN3RELEASE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn3ReleaseHandler));
   m_winIO.registerCallback(WinIO::EVENT_KEYDOWN, Functor<void, TYPELIST_1(int)>(this, &EntityUi::keyDownHandler));
   m_winIO.registerCallback(WinIO::EVENT_KEYUP, Functor<void, TYPELIST_1(int)>(this, &EntityUi::keyUpHandler));

   m_active = true;

   if (m_mouseOver) {
      m_mouseOver = false;
      mouseMoveHandler(m_latestCursorPos.x, m_latestCursorPos.y);
   }
}

//===========================================
// EntityUi::removeFromWorld
//===========================================
void EntityUi::removeFromWorld() {
   m_winIO.unregisterCallback(WinIO::EVENT_BTN1PRESS, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn1PressHandler));
   m_winIO.unregisterCallback(WinIO::EVENT_BTN1RELEASE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn1ReleaseHandler));
   m_winIO.unregisterCallback(WinIO::EVENT_BTN3PRESS, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn3PressHandler));
   m_winIO.unregisterCallback(WinIO::EVENT_BTN3RELEASE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::btn3ReleaseHandler));
   m_winIO.unregisterCallback(WinIO::EVENT_KEYDOWN, Functor<void, TYPELIST_1(int)>(this, &EntityUi::keyDownHandler));
   m_winIO.unregisterCallback(WinIO::EVENT_KEYUP, Functor<void, TYPELIST_1(int)>(this, &EntityUi::keyUpHandler));

   m_active = false;
}

//===========================================
// EntityUi::EntityUi
//===========================================
EntityUi::~EntityUi() {
   EntityUi::removeFromWorld();
   m_winIO.unregisterCallback(WinIO::EVENT_MOUSEMOVE, Functor<void, TYPELIST_2(int, int)>(this, &EntityUi::mouseMoveHandler));
}

//===========================================
// EntityUi::inRange
//
// Takes screen coords (x, y) and outputs world coords (wx, wy).
// Returns true if (wx, wy) is inside entity's bounding box
//
// TODO: Use entity's shape instead of bounding box
//===========================================
bool EntityUi::inRange(int x, int y, float32_t& wx, float32_t& wy) const {
   y = m_winIO.getWindowHeight() - y;

   Vec2f viewPos = Renderer::getInstance().getCamera().getTranslation();

   wx = viewPos.x + static_cast<float32_t>(x) * gGetPixelSize().x;
   wy = viewPos.y + static_cast<float32_t>(y) * gGetPixelSize().y;

   return m_entity->getBoundary().contains(Vec2f(wx, wy));
}

//===========================================
// EntityUi::update
//===========================================
void EntityUi::update() {
   m_hasFocus = m_hasFocus_;
}

//===========================================
// EntityUi::setFocus
//
// Doesn't take effect until next call to update().
// This is to prevent a widget's focus from being changed
// while keyboard/mouse callbacks are being processed.
//===========================================
void EntityUi::setFocus(bool b) {
   m_hasFocus_ = b;

   if (b)
      onGainFocus();
   else
      onLoseFocus();
}

//===========================================
// EntityUi::getSize
//===========================================
size_t EntityUi::getSize() const {
   return sizeof(EntityUi)
      + m_callbacks.size() * sizeof(pair<uiEvent_t, callback_t>); // This is an under-estimate
}

//===========================================
// EntityUi::btn1PressHandler
//===========================================
void EntityUi::btn1PressHandler(int x, int y) {
   float32_t wx, wy;
   if (inRange(x, y, wx, wy)) {
      this->onBtn1Press(wx, wy);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_BTN1_PRESS);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_BTN1_PRESS, m_entity->getSharedPtr());
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::btn1ReleaseHandler
//===========================================
void EntityUi::btn1ReleaseHandler(int x, int y) {
   float32_t wx, wy;
   if (inRange(x, y, wx, wy)) {
      this->onBtn1Release(wx, wy);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_BTN1_RELEASE);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_BTN1_RELEASE, m_entity->getSharedPtr());
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::btn3PressHandler
//===========================================
void EntityUi::btn3PressHandler(int x, int y) {
   float32_t wx, wy;
   if (inRange(x, y, wx, wy)) {
      this->onBtn3Press(wx, wy);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_BTN3_PRESS);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_BTN3_PRESS, m_entity->getSharedPtr());
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::btn3ReleaseHandler
//===========================================
void EntityUi::btn3ReleaseHandler(int x, int y) {
   float32_t wx, wy;
   if (inRange(x, y, wx, wy)) {
      this->onBtn3Release(wx, wy);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_BTN3_RELEASE);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_BTN3_RELEASE, m_entity->getSharedPtr());
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::keyDownHandler
//===========================================
void EntityUi::keyDownHandler(int key) {
   if (hasFocus()) {
      this->onKeyDown(key);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_KEY_DOWN);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_2(pEntity_t, int)> >(i->second)(m_entity->getSharedPtr(), key);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_KEY_DOWN, m_entity->getSharedPtr()); // TODO
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::keyUpHandler
//===========================================
void EntityUi::keyUpHandler(int key) {
   if (hasFocus()) {
      this->onKeyUp(key);

      try {
         map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_KEY_UP);
         if (i != m_callbacks.end())
            boost::get<Functor<void, TYPELIST_2(pEntity_t, int)> >(i->second)(m_entity->getSharedPtr(), key);

         EUiEvent* uiEvent = new EUiEvent(UIEVENT_KEY_UP, m_entity->getSharedPtr()); // TODO
         m_eventManager.immediateDispatch(uiEvent);
      }
      catch (boost::bad_get& e) {
         Exception ex("Bad callback function; ", __FILE__, __LINE__);
         ex.append(e.what());
         throw ex;
      }
   }
}

//===========================================
// EntityUi::mouseMoveHandler
//===========================================
void EntityUi::mouseMoveHandler(int x, int y) {
   float32_t wx, wy;

   m_latestCursorPos = Vec2i(x, y);

   if (inRange(x, y, wx, wy)) {
      if (!m_mouseOver) {
         if (m_active) {
            onHoverOn(wx, wy);

            try {
               map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_HOVER_ON);
               if (i != m_callbacks.end())
                  boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

               EUiEvent* uiEvent = new EUiEvent(UIEVENT_HOVER_ON, m_entity->getSharedPtr()); // TODO
               m_eventManager.immediateDispatch(uiEvent);
            }
            catch (boost::bad_get& e) {
               m_mouseOver = true;
               Exception ex("Bad callback function; ", __FILE__, __LINE__);
               ex.append(e.what());
               throw ex;
            }
         }
      }
      m_mouseOver = true;
   }
   else {
      if (m_mouseOver) {
         if (m_active) {
            onHoverOff(wx, wy);

            try {
               map<uiEvent_t, callback_t>::iterator i = m_callbacks.find(UIEVENT_HOVER_OFF);
               if (i != m_callbacks.end())
                  boost::get<Functor<void, TYPELIST_3(pEntity_t, float32_t, float32_t)> >(i->second)(m_entity->getSharedPtr(), wx, wy);

               EUiEvent* uiEvent = new EUiEvent(UIEVENT_HOVER_OFF, m_entity->getSharedPtr()); // TODO
               m_eventManager.immediateDispatch(uiEvent);
            }
            catch (boost::bad_get& e) {
               m_mouseOver = false;
               Exception ex("Bad callback function; ", __FILE__, __LINE__);
               ex.append(e.what());
               throw ex;
            }
         }
      }
      m_mouseOver = false;
   }
}


}
