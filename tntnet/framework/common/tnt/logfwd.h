/* tnt/logfwd.h
   Copyright (C) 2003 Tommi Mäkitalo

This file is part of tntnet.

Tntnet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Tntnet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tntnet; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA  02111-1307  USA
*/

#ifndef TNT_LOGFWD_H
#define TNT_LOGFWD_H

namespace log4cplus
{
  class Logger;
  Logger getLogger();
};

/// Makros zum definieren von Loggingkategorien pro Namespace

#define log_declare_namespace(ns)   \
  namespace ns { log4cplus::Logger getLogger(); }

#define log_declare_class()   \
  static log4cplus::Logger getLogger()

log_declare_namespace(tnt);
log_declare_namespace(tntcomp);
log_declare_namespace(ecpp_component);
log_declare_namespace(compcall);

#endif // TNT_LOGFWD_H
