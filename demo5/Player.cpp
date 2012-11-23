#include <set>
#include <dodge/ETransFinished.hpp>
#include <dodge/StringId.hpp>
#include "Player.hpp"


using namespace std;
using namespace Dodge;


//===========================================
// Player::Player
//===========================================
Player::Player(const XmlNode data)
   : Entity(data.firstChild().firstChild()),
     Item(data.firstChild()),
     PhysicalSprite<Box2dPhysics>(data.nthChild(1)) {

   init();

   try {
      XML_NODE_CHECK(data, Player);

      XmlNode node = data.nthChild(2);
      XML_NODE_CHECK(node, gridSize);
      m_gridSize = Vec2f(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, footSensor);
      m_footSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, headSensor);
      m_headSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, leftSensor);
      m_leftSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, rightSensor);
      m_rightSensor = Quad(node.firstChild());

      node = node.nextSibling();
      XML_NODE_CHECK(node, midSensor);
      m_midSensor = Quad(node.firstChild());
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class Player; ");
      throw;
   }
}

//===========================================
// Player::Player
//===========================================
Player::Player(const Player& copy, long name)
   : Entity(copy, name),
     Item(copy, name),
     PhysicalSprite<Box2dPhysics>(copy, name),
     m_mode(DIG_MODE) {

   m_gridSize = copy.m_gridSize;
   m_footSensor = copy.m_footSensor;
   m_headSensor = copy.m_headSensor;
   m_leftSensor = copy.m_leftSensor;
   m_rightSensor = copy.m_rightSensor;
   m_midSensor = copy.m_midSensor;
}

//===========================================
// Player::init
//===========================================
void Player::init() {
   m_mode = DIG_MODE;
}

//===========================================
// Player::clone
//===========================================
Player* Player::clone() const {
   return new Player(*this);
}

//===========================================
// Player::snapToGridH
//===========================================
void Player::snapToGridH(float32_t offset) {
   Vec2f pos = getTranslation_abs();
   pos.x = floor((pos.x / m_gridSize.x) + 0.5) * m_gridSize.x + offset;

   translate(pos - getTranslation_abs());
}

//===========================================
// Player::snapToGridV
//===========================================
void Player::snapToGridV(float32_t offset) {
   Vec2f pos = getTranslation_abs();
   pos.y = floor((pos.y / m_gridSize.y) + 0.5) * m_gridSize.y + offset;

   translate(pos - getTranslation_abs());
}

//===========================================
// Player::addToWorld
//===========================================
void Player::addToWorld() {
   PhysicalSprite<Box2dPhysics>::addToWorld();
}

//===========================================
// Player::removeFromWorld
//===========================================
void Player::removeFromWorld() {
   PhysicalSprite<Box2dPhysics>::removeFromWorld();
}

//===========================================
// Player::update
//===========================================
void Player::update() {
   static long gravityRegionStr = internString("gravityRegion");

   PhysicalSprite<Box2dPhysics>::update();

   bool b = false;
   vector<pEntity_t> vec;
   m_worldSpace.getEntities(Range(getBoundary()), vec);
   for (uint_t i = 0; i < vec.size(); ++i) {
      if (vec[i]->getTypeName() == gravityRegionStr) {
         if (Math::contains(vec[i]->getShape(), vec[i]->getTranslation_abs(), m_footSensor, getTranslation_abs())
            || Math::contains(vec[i]->getShape(), vec[i]->getTranslation_abs(), m_midSensor, getTranslation_abs())) {

            b = true;
            break;
         }
      }
   }

   if (b) {
      if (m_mode == DIG_MODE) {
         makeDynamic();
         stopTransformations();
      }

      m_mode = PLATFORM_MODE;
   }
   else {
      if (m_mode == PLATFORM_MODE) {
         makeStatic();
      }

      m_mode = DIG_MODE;
   }
}

//===========================================
// Player::draw
//===========================================
void Player::draw() const {
   PhysicalSprite<Box2dPhysics>::draw();

   Graphics2d graphics2d;
   graphics2d.setLineWidth(0);
   graphics2d.setFillColour(Colour(1.f, 0.f, 0.f, 0.4f));

   Vec2f pos = getTranslation_abs();
   graphics2d.drawPrimitive(m_footSensor, pos.x, pos.y, 9);
   graphics2d.drawPrimitive(m_headSensor, pos.x, pos.y, 9);
   graphics2d.drawPrimitive(m_leftSensor, pos.x, pos.y, 9);
   graphics2d.drawPrimitive(m_rightSensor, pos.x, pos.y, 9);
   graphics2d.drawPrimitive(m_midSensor, pos.x, pos.y, 9);
}

//===========================================
// Player::onEvent
//===========================================
void Player::onEvent(const EEvent* event) {
   static long transFinishedStr = internString("transFinished");

   if (event->getType() == transFinishedStr) {
      if (numActiveTransformations() == 0 && m_mode == DIG_MODE) {
         snapToGridV();
         snapToGridH();
      }
   }
}

//===========================================
// Player::grounded
//===========================================
bool Player::grounded() const {
   static long gravityRegionStr = internString("gravityRegion");

   vector<pEntity_t> vec;
   m_worldSpace.getEntities(getBoundary(), vec);

   for (uint_t i = 0; i < vec.size(); ++i) {
      if (vec[i].get() == this) continue;
      if (vec[i]->getTypeName() == gravityRegionStr) continue;

      if (Math::overlap(m_footSensor, getTranslation_abs(), vec[i]->getShape(), vec[i]->getTranslation_abs()))
         return true;
   }

   return false;
}

//===========================================
// Player::jump
//===========================================
void Player::jump() {
   static long jumpStr = internString("jump");

   if (grounded() && m_jumpTimer.getTime() > 0.2) {
      stopAnimation();
      playAnimation(jumpStr);
      applyLinearImpulse(Vec2f(0.0, 1.0), Vec2f(0.0, 0.0));
      m_jumpTimer.reset();
   }
}

//===========================================
// Player::move
//
// 0 = left, 1 = up, 2 = right, 3 = down
//===========================================
bool Player::move(int dir) {
   static long gravityRegionStr = internString("gravityRegion");
   static long moveLeftStr = internString("moveLeft");
   static long hitFromRightStr = internString("hitFromRight");
   static long moveRightStr = internString("moveRight");
   static long hitFromLeftStr = internString("hitFromLeft");
   static long moveUpStr = internString("moveUp");
   static long hitFromBottomStr = internString("hitFromBottom");
   static long moveDownStr = internString("moveDown");
   static long hitFromTopStr = internString("hitFromTop");

   if (m_mode == DIG_MODE) {
      Primitive* sensor = NULL;
      dir_t facing;
      long plyrAnim = 0, eventType = 0;
      switch (dir) {
         case 0:           // Left
            sensor = &m_leftSensor;
            facing = LEFT;
            plyrAnim = moveLeftStr;
            eventType = hitFromRightStr;
            break;
         case 1:           // Up
            sensor = &m_headSensor;
            facing = UP;
            plyrAnim = moveUpStr;
            eventType = hitFromBottomStr;
            break;
         case 2:           // Right
            sensor = &m_rightSensor;
            facing = RIGHT;
            plyrAnim = moveRightStr;
            eventType = hitFromLeftStr;
            break;
         case 3:           // Down
            sensor = &m_footSensor;
            facing = DOWN;
            plyrAnim = moveDownStr;
            eventType = hitFromTopStr;
            break;
      }

      playAnimation(plyrAnim);

      if (numActiveTransformations() == 0) {
         playTransformation(plyrAnim);

         vector<pEntity_t> vec;
         m_worldSpace.getEntities(getBoundary(), vec);
         for (uint_t i = 0; i < vec.size(); ++i) {
            if (vec[i].get() == this) continue;

            if (Math::overlap(*sensor, getTranslation_abs(), vec[i]->getShape(), vec[i]->getTranslation_abs())) {
               EEvent* event = new EEvent(eventType);
               vec[i]->onEvent(event);
            }
         }

         m_facing = facing;

         return true;
      }
   }
   else if (m_mode == PLATFORM_MODE) {
      switch (dir) {
         case 0: {
            bool b = false;
            vector<pEntity_t> vec;
            m_worldSpace.getEntities(Range(getBoundary()), vec);
            for (uint_t i = 0; i < vec.size(); ++i) {
               if (vec[i]->getTypeName() == gravityRegionStr) {
                  if (!Math::contains(vec[i]->getShape(), vec[i]->getTranslation_abs(), m_leftSensor, getTranslation_abs())) {
                     b = true;
                     break;
                  }
               }
            }

            if (b) {
               m_mode = DIG_MODE;
               snapToGridV();
               makeStatic();
               snapToGridH();
               move(0);
            }
            else {
               playAnimation(moveLeftStr);
               applyForce(Vec2f(-2.8, 0.0), Vec2f(0.0, 0.0));
            }
         }
         break;
         case 1:
            jump();
            break;
         case 2: {
            bool b = false;
            vector<pEntity_t> vec;
            m_worldSpace.getEntities(Range(getBoundary()), vec);
            for (uint_t i = 0; i < vec.size(); ++i) {
               if (vec[i]->getTypeName() == gravityRegionStr) {
                  if (!Math::contains(vec[i]->getShape(), vec[i]->getTranslation_abs(), m_rightSensor, getTranslation_abs())) {
                     b = true;
                     break;
                  }
               }
            }

            if (b) {
               m_mode = DIG_MODE;
               snapToGridV();
               makeStatic();
               snapToGridH();
               move(2);
            }
            else {
               playAnimation(moveRightStr);
               applyForce(Vec2f(2.8, 0.0), Vec2f(0.0, 0.0));
            }
         }
         break;
         case 3: {
            bool b = false;
            vector<pEntity_t> vec;
            m_worldSpace.getEntities(Range(getBoundary()), vec);
            for (uint_t i = 0; i < vec.size(); ++i) {
               if (vec[i]->getTypeName() == gravityRegionStr) {
                  if (!Math::contains(vec[i]->getShape(), vec[i]->getTranslation_abs(), m_footSensor, getTranslation_abs())) {
                     b = true;
                     break;
                  }
               }
            }

            if (b) {
               m_mode = DIG_MODE;
               snapToGridH();
               makeStatic();
               snapToGridV();
               move(3);
            }
         }
         break;
      }
      return true;
   }

   return false;
}

//===========================================
// Player::assignData
//===========================================
void Player::assignData(const XmlNode data) {
   try {
      XML_NODE_CHECK(data, Player)

      XmlNode node = data.firstChild();
      if (!node.isNull() && node.name() == "Item") {
         Item::assignData(node);
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "PhysicalSprite") {
         PhysicalSprite<Box2dPhysics>::assignData(node);
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "gridSize") {
         m_gridSize = Vec2f(node.firstChild());
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "footSensor") {
         m_footSensor = Quad(node.firstChild());
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "headSensor") {
         m_headSensor = Quad(node.firstChild());
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "leftSensor") {
         m_leftSensor = Quad(node.firstChild());
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "rightSensor") {
         m_rightSensor = Quad(node.firstChild());
         node = node.nextSibling();
      }

      if (!node.isNull() && node.name() == "midSensor") {
         m_midSensor = Quad(node.firstChild());
      }
   }
   catch (XmlException& e) {
      e.prepend("Error parsing XML for instance of class Player; ");
      throw;
   }
}
