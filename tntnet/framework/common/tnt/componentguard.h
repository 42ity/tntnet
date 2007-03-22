/* tnt/componentguard.h
 * Copyright (C) 2003-2005 Tommi Maekitalo
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


#ifndef TNT_COMPONENTGUARD_H
#define TNT_COMPONENTGUARD_H

namespace tnt
{
  class ComponentGuard
  {
      Component* comp;

      // non compyable
      ComponentGuard(const ComponentGuard&);
      ComponentGuard& operator=(const ComponentGuard&);

    public:
      ComponentGuard(Component* comp_)
        : comp(comp_)
        { }
      ~ComponentGuard()
        { comp->drop(); }

      const Component* getPtr() const      { return comp; }
      Component* getPtr()                  { return comp; }
      operator const Component* () const   { return comp; }
      const Component* operator-> () const { return comp; }
      Component* operator-> ()             { return comp; }
  };

}

#endif // TNT_COMPONENTGUARD_H
