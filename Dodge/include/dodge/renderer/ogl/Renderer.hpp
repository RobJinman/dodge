/*
 * Author: Rob Jinman <admin@robjinman.com>
 * Date: 2012
 */

#ifndef __RENDERER_HPP__
#define __RENDERER_HPP__


#ifdef WIN32
   #include <windows.h>
   #include <windowsx.h>
   #include <GLEW/glew.h>
#elif defined LINUX
   #include <GLEW/glew.h>
#else
   #ifdef GL_FIXED_PIPELINE
      #include <GLES/gl.h>
   #else
      #include <GLES2/gl2.h>
   #endif
#endif
#include <cml/cml.h>
#include <map>
#include <queue>
#include <cstring>
#include <mutex>
#include <thread>
#include <atomic>
#include <boost/variant.hpp>
#include "Colour.hpp"
#include "../Camera.hpp"
#include "../../WinIO.hpp"
#include "../../StackAllocator.hpp"
#include "../../definitions.hpp"


namespace Dodge {


class IModel;
class RenderMode;
class SceneGraph;


#if defined WIN32 | defined LINUX
inline void gl_bindBuffer(GLenum target, GLuint buf) { glBindBufferARB(target, buf); }
inline void gl_genBuffers(GLsizei n, GLuint* bufs) { glGenBuffersARB(n, bufs); }
inline void gl_bufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { glBufferDataARB(target, size, data, usage); }
inline void gl_deleteBuffers(GLsizei n, const GLuint* bufs) { glDeleteBuffersARB(n, bufs); }
#else
inline void gl_bindBuffer(GLenum target, GLuint buf) { glBindBuffer(target, buf); }
inline void gl_genBuffers(GLsizei n, GLuint* bufs) { glGenBuffers(n, bufs); }
inline void gl_bufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage) { glBufferData(target, size, data, usage); }
inline void gl_deleteBuffers(GLsizei n, const GLuint* bufs) { glDeleteBuffers(n, bufs); }
#endif


class Renderer {
   public:
      static Renderer& getInstance() {
         if (!m_instance) m_instance = new Renderer;
         return *m_instance;
      }

      typedef GLint int_t;
      typedef GLfloat float_t;
      typedef GLfloat vertexElement_t;
      typedef GLfloat matrixElement_t;
      typedef GLfloat colourElement_t;
      typedef GLfloat texCoordElement_t;
      typedef byte_t textureData_t;
      typedef GLuint textureHandle_t;

      enum mode_t {
         UNDEFINED,
         TEXTURED_ALPHA,
         NONTEXTURED_ALPHA,
         FIXED_FUNCTION
         // ...
      };

      enum primitive_t {
         TRIANGLES,
         LINES,
         QUADS,
         TRIANGLE_STRIP
      };

      enum exceptionType_t {
         UNKNOWN_EXCEPTION,
         EXCEPTION,
         RENDERER_EXCEPTION
         // ...
      };

      struct exceptionWrapper_t {
         exceptionWrapper_t(exceptionType_t t, void* d)
            : type(t), data(d) {}

         exceptionType_t type;
         void* data;
      };

      //-----Main Thread-----
      inline void attachCamera(pCamera_t camera);
      inline Camera& getCamera() const;

      void onWindowResize(int_t w, int_t h);

      void bufferModel(IModel* model);
      void freeBufferedModel(IModel* model);

      void loadTexture(const textureData_t* texture, int_t width, int_t height, textureHandle_t* handle);
      void unloadTexture(textureHandle_t handle);

      void draw(const IModel* model);
#ifdef DEBUG
      inline long getFrameRate() const;
#endif
      void start();
      void stop();
      void tick(const Colour& bgColour = Colour(0.f, 0.f, 0.f, 1.f));
      //---------------------

   private:
      Renderer();

      typedef enum {
         MSG_TEX_HANDLE_REQ,
         MSG_TEX_UNLOAD_REQ,
         MSG_CONSTRUCT_VBO,
         MSG_DESTROY_VBO,
         MSG_VP_RESIZE_REQ
         // ...
      } msgType_t;

      struct msgTexHandleReq_t {
         const textureData_t* texData;
         int_t w;
         int_t h;

         textureHandle_t* retVal;
      };

      struct msgTexUnloadReq_t {
         textureHandle_t handle;
      };

      struct msgVpResizeReq_t {
         int_t w;
         int_t h;
      };

      struct msgConstructVbo_t {
         IModel* model;
      };

      struct msgDestroyVbo_t {
         GLuint handle;
      };

      typedef boost::variant<
         msgTexHandleReq_t,
         msgTexUnloadReq_t,
         msgVpResizeReq_t,
         msgConstructVbo_t,
         msgDestroyVbo_t
         // ...
      > msgData_t;

      struct Message {
         Message(msgType_t type_, msgData_t data_)
            : type(type_), data(data_) {}

         msgType_t type;
         msgData_t data;
      };

      struct renderState_t {
         enum status_t {
            IS_IDLE,
            IS_PENDING_RENDER,
            IS_BEING_RENDERED,
            IS_BEING_UPDATED
         };

         status_t status;
         std::unique_ptr<SceneGraph> sceneGraph;
         cml::matrix44f_c P;
         Colour bgColour;
      };

      static Renderer* m_instance;

      //-----Main Thread-----
      void checkForErrors();
      void queueMsg(Message msg);
      //---------------------

      //----Render Thread----
      void renderLoop();
      void processMessages();
      void init();
      void clear();
      void constructVbo(IModel* model);
      void destroyVbo(GLuint handle);
      void setMode(mode_t mode);
      void constructRenderModes();
      void processMessage(const Message& msg);
      textureHandle_t loadGLTexture(const textureData_t* texture, int_t w, int_t h);
#ifdef DEBUG
      void computeFrameRate();
#endif
      //---------------------

      GLint primitiveToGLType(primitive_t primitiveType) const;

      bool m_fixedPipeline;
      bool m_vboSupport;

      std::map<long, GLuint> m_vboMap;
      std::mutex m_vboMapMutex; // TODO: Make thread-safe map container

      std::map<mode_t, RenderMode*> m_renderModes;
      RenderMode* m_activeRenderMode;
      mode_t m_mode;

      std::atomic<bool> m_init;

      renderState_t m_state[3];
      int m_idxLatest;
      int m_idxRender;
      int m_idxUpdate;
      mutable std::mutex m_stateChangeMutex;
      std::atomic<long long> m_frameNumber;

      pCamera_t m_camera;

      std::atomic<bool> m_running;
      std::thread* m_thread;

      std::vector<Message> m_msgQueue;
      StackAllocator m_scratchSpace;
      mutable std::mutex m_msgQueueMutex;
      std::atomic<bool> m_msgQueueEmpty;

      exceptionWrapper_t m_exception;
      std::atomic<bool> m_errorPending;

#ifdef DEBUG
      std::atomic<long> m_frameRate;
#endif
};

#ifdef DEBUG
//===========================================
// Renderer::getFrameRate
//===========================================
inline long Renderer::getFrameRate() const {
   return m_frameRate;
}
#endif

//===========================================
// Renderer::attachCamera
//===========================================
inline void Renderer::attachCamera(pCamera_t camera) {
   m_camera = camera;
}

//===========================================
// Renderer::getCamera
//===========================================
inline Camera& Renderer::getCamera() const {
   return *m_camera;
}


}


#endif /*!__RENDERER_HPP__*/
