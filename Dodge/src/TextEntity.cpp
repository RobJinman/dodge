#include <TextEntity.hpp>
#include <renderer/Renderer.hpp>
#include <globals.hpp>


namespace Dodge {


//===========================================
// TextEntity::TextEntity
//===========================================
TextEntity::TextEntity(const XmlNode data)
   : Entity(data.firstChild()),
     m_renderer(Renderer::getInstance()),
     m_model(Renderer::TRIANGLES) {

   XmlNode node = data.firstChild();

   XML_NODE_CHECK(data, TextEntity);

   node = node.nextSibling();
   // TODO
}

//===========================================
// TextEntity::TextEntity
//===========================================
TextEntity::TextEntity(long type, const pFont_t font, const std::string& text, const Vec2f& size)
   : Entity(type),
     m_renderer(Renderer::getInstance()),
     m_font(font),
     m_size(size),
     m_model(Renderer::TRIANGLES) {

   setText(text);
}

//===========================================
// TextEntity::TextEntity
//===========================================
TextEntity::TextEntity(long name, long type, const pFont_t font, const std::string& text, const Vec2f& size)
   : Entity(name, type),
     m_renderer(Renderer::getInstance()),
     m_font(font),
     m_size(size),
     m_model(Renderer::TRIANGLES) {

   setText(text);
}

//===========================================
// TextEntity::TextEntity
//===========================================
TextEntity::TextEntity(const TextEntity& copy)
   : Entity(copy),
     m_renderer(Renderer::getInstance()),
     m_model(Renderer::TRIANGLES) {

   // TODO
}

//===========================================
// TextEntity::TextEntity
//===========================================
TextEntity::TextEntity(const TextEntity& copy, long name)
   : Entity(copy, name),
     m_renderer(Renderer::getInstance()),
     m_model(Renderer::TRIANGLES) {

   // TODO
}

//===========================================
// TextEntity::clone
//===========================================
TextEntity* TextEntity::clone() const {
   return new TextEntity(*this);
}

//===========================================
// TextEntity::assignData
//===========================================
void TextEntity::assignData(const XmlNode data) {
   // TODO
}

//===========================================
// TextEntity::render
//===========================================
void TextEntity::render() const {
   m_renderer.stageModel(&m_model);
}

//===========================================
// TextEntity::unrender
//===========================================
void TextEntity::unrender() const {
   m_renderer.unstageModel(&m_model);
}

//===========================================
// TextEntity::update
//===========================================
void TextEntity::update() {
   // TODO
}

#ifdef DEBUG
//===========================================
// TextEntity::dbg_print
//===========================================
void TextEntity::dbg_print(std::ostream& out, int tab) const {
   // TODO
}
#endif

//===========================================
// TextEntity::onEvent
//===========================================
void TextEntity::onEvent(const EEvent* event) {
   static long entityRotationStr = internString("entityRotation");
   static long entityTranslationStr = internString("entityTranslation");

   if (event->getType() == entityRotationStr
      || event->getType() == entityTranslationStr) {

      updateModel();
   }
}

//===========================================
// TextEntity::updateModel
//===========================================
void TextEntity::updateModel() const {
   float32_t x = getTranslation_abs().x;
   float32_t y = getTranslation_abs().y;
   float32_t z = getZ();

   float32_t texSectionX1 = m_font->getTextureSection().getPosition().x;
   float32_t texSectionY1 = m_font->getTextureSection().getPosition().y;
   float32_t texSectionX2 = m_font->getTextureSection().getSize().x;

   float32_t texW = m_font->getTexture()->getWidth();
   float32_t texH = m_font->getTexture()->getHeight();

   float32_t pxChW = m_font->getCharWidth();    // Char dimensions in pixels
   float32_t pxChH = m_font->getCharHeight();

   int rowLen = (static_cast<float32_t>(texSectionX2 - texSectionX1) / static_cast<float32_t>(pxChW)) + 0.5;

   StackAllocator::marker_t marker = gMemStack.getMarker();
   vvvtt_t* verts = reinterpret_cast<vvvtt_t*>(gMemStack.alloc(m_text.size() * 6 * sizeof(vvvtt_t)));

   int v = 0;
   for (uint_t i = 0; i < m_text.length(); ++i) {
      // Character world coords (pre-transformation)
      float32_t chX = x + static_cast<float32_t>(i) * m_size.x;
      float32_t chY = y;

      float32_t srcX = texSectionX1 + pxChW * (static_cast<float32_t>((m_text[i] - ' ') % rowLen));
      float32_t srcY = texSectionY1 + pxChH * static_cast<float32_t>((m_text[i] - ' ') / rowLen);

      verts[v] = { chX + m_size.x, chY,            z,    (srcX + pxChW) / texW, (srcY + pxChH) / texH };    ++v;
      verts[v] = { chX + m_size.x, chY + m_size.y, z,    (srcX + pxChW) / texW, srcY / texH           };    ++v;
      verts[v] = { chX,            chY,            z,    srcX / texW,           (srcY + pxChH) / texH };    ++v;
      verts[v] = { chX + m_size.x, chY + m_size.y, z,    (srcX + pxChW) / texW, srcY / texH           };    ++v;
      verts[v] = { chX,            chY + m_size.y, z,    srcX / texW,           srcY / texH           };    ++v;
      verts[v] = { chX,            chY,            z,    srcX / texW,           (srcY + pxChH) / texH };    ++v;
   }

   m_model.eraseVertices();
   m_model.setVertices(0, verts, 6 * m_text.size());
   m_model.setColour(getFillColour());
   m_model.setLineWidth(0);
   m_model.setTextureHandle(m_font->getTexture()->getHandle());

   m_renderer.freeBufferedModel(&m_model);
   m_renderer.bufferModel(&m_model);

   gMemStack.freeToMarker(marker);
}

//===========================================
// TextEntity::setText
//===========================================
void TextEntity::setText(const std::string& text) {
   m_text = text;
   updateModel();
}

//===========================================
// TextEntity::setFillColour
//===========================================
void TextEntity::setFillColour(const Colour& colour) {
   Entity::setFillColour(colour);
   m_model.setColour(getFillColour());
}

//===========================================
// TextEntity::~TextEntity
//===========================================
TextEntity::~TextEntity() {
   m_renderer.freeBufferedModel(&m_model);
   m_renderer.unstageModel(&m_model);
}


}