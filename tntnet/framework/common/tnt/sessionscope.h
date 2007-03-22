/* tnt/sessionscope.h
 * Copyright (C) 2005 Tommi Maekitalo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef TNT_SESSIONSCOPE_H
#define TNT_SESSIONSCOPE_H

#include <tnt/scope.h>
#include <time.h>

namespace tnt
{
  class Sessionscope : public Scope
  {
      time_t atime;
      unsigned timeout;
      static unsigned defaultTimeout;

    public:
      Sessionscope()
        : timeout(defaultTimeout)
        { touch(); }

      void touch()                               { time(&atime); }
      time_t getAtime() const                    { return atime; }
      unsigned getTimeout() const                { return timeout; }
      void setTimeout(unsigned t)                { timeout = t; }

      static void setDefaultTimeout(unsigned t)  { defaultTimeout = t; }
      static unsigned getDefaultTimeout()        { return defaultTimeout; }
  };
}

#endif // TNT_SESSIONSCOPE_H

