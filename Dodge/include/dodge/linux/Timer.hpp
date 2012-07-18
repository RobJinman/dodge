/*
 * Author: Rob Jinman <jinmane@gmail.com>
 * Date: 2011
 */

#ifndef __TIMER_HPP__
#define __TIMER_HPP__


#include <sys/time.h>


namespace Dodge {


class Timer {
   public:
      Timer() { reset(); }

      void reset();
      double getTime() const;

   private:
      double m_seconds;
};


}


#endif /*!__TIMER_HPP__*/
