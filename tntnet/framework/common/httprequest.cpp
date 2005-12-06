/* httprequest.cpp
   Copyright (C) 2003-2005 Tommi Maekitalo

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

#include <tnt/httprequest.h>
#include <tnt/httpparser.h>
#include <sstream>
#include <cxxtools/log.h>
#include <cxxtools/thread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <tnt/sessionscope.h>
#include <errno.h>
#include "config.h"

namespace tnt
{
  log_define("tntnet.httprequest")

  ////////////////////////////////////////////////////////////////////////
  // HttpRequest
  //
  unsigned HttpRequest::serial_ = 0;

  HttpRequest::HttpRequest(const std::string& url)
    : ssl(false),
      lang_init(false),
      requestScope(0),
      applicationScope(0),
      sessionScope(0),
      applicationScopeLocked(false),
      sessionScopeLocked(false)
  {
    std::istringstream s("GET " + url + " HTTP/1.1\r\n\r\n");
    parse(s);
  }

  HttpRequest::HttpRequest(const HttpRequest& r)
    : pathinfo(r.pathinfo),
      args(r.args),
      qparam(r.qparam),
      peerAddr(r.peerAddr),
      serverAddr(r.serverAddr),
      ct(r.ct),
      mp(r.mp),
      ssl(r.ssl),
      serial(r.serial),
      lang_init(r.lang_init),
      lang(r.lang),
      requestScope(r.requestScope),
      applicationScope(r.applicationScope),
      sessionScope(r.sessionScope),
      applicationScopeLocked(false),
      sessionScopeLocked(false)
  {
    if (requestScope)
      requestScope->addRef();
    if (applicationScope)
      applicationScope->addRef();
    if (sessionScope)
      sessionScope->addRef();
  }

  HttpRequest::~HttpRequest()
  {
    releaseLocks();

    if (requestScope)
      requestScope->release();
    if (applicationScope)
      applicationScope->release();
    if (sessionScope)
      sessionScope->release();
  }

  HttpRequest& HttpRequest::operator= (const HttpRequest& r)
  {
    pathinfo = r.pathinfo;
    args = r.args;
    qparam = r.qparam;
    peerAddr = r.peerAddr;
    serverAddr = r.serverAddr;
    ct = r.ct;
    mp = r.mp;
    ssl = r.ssl;
    serial = r.serial;
    lang_init = r.lang_init;
    lang = r.lang;
    requestScope = r.requestScope;
    applicationScope = r.applicationScope;
    sessionScope = r.sessionScope;
    applicationScopeLocked = false;
    sessionScopeLocked = false;

    if (requestScope)
      requestScope->addRef();
    if (applicationScope)
      applicationScope->addRef();
    if (sessionScope)
      sessionScope->addRef();

    return *this;
  }

  void HttpRequest::clear()
  {
    HttpMessage::clear();
    pathinfo.clear();
    args.clear();
    qparam.clear();
    ct = Contenttype();
    mp = Multipart();
    lang_init = false;
    if (requestScope)
    {
      requestScope->release();
      requestScope = 0;
    }
    httpcookies.clear();
    encodingRead = false;

    releaseLocks();

    if (applicationScope)
    {
      applicationScope->release();
      applicationScope = 0;
    }

    if (sessionScope)
    {
      sessionScope->release();
      sessionScope = 0;
    }
  }

  void HttpRequest::parse(std::istream& in)
  {
    Parser p(*this);
    p.parse(in);
    if (!p.failed())
      doPostParse();
  }

  void HttpRequest::doPostParse()
  {
    qparam.parse_url(getQueryString());
    if (getMethod() == "POST")
    {
      std::istringstream in(getHeader(httpheader::contentType));
      in >> ct;

      if (in)
      {
        log_debug(httpheader::contentType << ' ' << in.str());
        log_debug("type=" << ct.getType() << " subtype=" << ct.getSubtype());
        if (ct.isMultipart())
        {
          log_debug("multipart-boundary=" << ct.getBoundary());
          mp.set(ct.getBoundary(), getBody());
          for (Multipart::const_iterator it = mp.begin();
               it != mp.end(); ++it)
          {
            // hochgeladene Dateien nicht in qparam übernehmen
            if (it->getFilename().empty())
            {
              std::string multipartBody(it->getBodyBegin(), it->getBodyEnd());
              log_debug("multipart-item name=" << it->getName()
                     << " body=" << multipartBody);
              qparam.add(it->getName(), multipartBody);
            }
          }
        }
        else
        {
          qparam.parse_url(getBody());
        }
      }
      else if (ct.getType() == "application"
            && ct.getSubtype() == "x-www-form-urlencoded")
        qparam.parse_url(getBody());
    }

    {
      static cxxtools::Mutex monitor;
      cxxtools::MutexLock lock(monitor);
      serial = ++serial_;
    }
  }

  std::string HttpRequest::getPeerIp() const
  {
#ifdef HAVE_INET_NTOP
    char buffer[32];
    const char* r = inet_ntop(AF_INET,
                              &(peerAddr.sin_addr),
                              buffer,
                              sizeof(buffer));
    if (r == 0)
    {
      std::ostringstream msg;
      msg << "error " << errno << " in inet_ntop: " << strerror(errno);
      throw std::runtime_error(msg.str());
    }
    return std::string(buffer);
#else
    static cxxtools::Mutex monitor;
    cxxtools::MutexLock lock(monitor);

    char* p = inet_ntoa(peerAddr.sin_addr);
    return std::string(p);
#endif
  }

  std::string HttpRequest::getServerIp() const
  {
    static cxxtools::Mutex monitor;
    cxxtools::MutexLock lock(monitor);

    char* p = inet_ntoa(serverAddr.sin_addr);
    return std::string(p);
  }

  std::string HttpRequest::getLang() const
  {
    if (!lang_init)
    {
      static const std::string LANG = "LANG";
      log_debug("HttpRequest::getLang() " << qparam.dump());

      lang = qparam[LANG];

      if (lang.empty())
      {
        const char* LANG = ::getenv("LANG");
        if (LANG)
          lang = LANG;
      }
      else
      {
        log_debug("LANG from query-parameter");
      }

      log_debug("LANG=" << lang);
      lang_init = true;
    }

    return lang;
  }

  const Cookies& HttpRequest::getCookies() const
  {
    log_debug("HttpRequest::getCookies()");

    if (!httpcookies.hasCookies())
    {
      log_debug("cookies found");

      header_type::const_iterator it = header.find(httpheader::cookie);
      if (it != header.end())
      {
        log_debug("parse cookie-header " << it->second);
        const_cast<HttpRequest*>(this)->httpcookies.set(it->second);
      }
    }

    return httpcookies;
  }

  const Encoding& HttpRequest::getEncoding() const
  {
    if (!encodingRead)
    {
      encoding.parse(getHeader(httpheader::acceptEncoding));
      encodingRead = true;
    }
    return encoding;
  }

  namespace
  {
    class CompareNoCase
    {
      public:
        bool operator() (char c1, char c2) const
          { return tolower(c1) == tolower(c2); }
    };
  }

  bool HttpRequest::keepAlive() const
  {
    header_type::const_iterator it = header.find(httpheader::connection);
    return it == header.end() ? getMajorVersion() == 1 && getMinorVersion() == 1
                              : it->second.size() >= httpheader::connectionKeepAlive.size()
                                && std::equal(it->second.begin(),
                                              it->second.end(),
                                              httpheader::connectionKeepAlive.begin(),
                                              CompareNoCase());
  }

  void HttpRequest::setApplicationScope(Scope* s)
  {
    if (applicationScope)
    {
      releaseApplicationScopeLock();
      applicationScope->release();
    }

    if (s)
      s->addRef();
    applicationScope = s;
  }

  void HttpRequest::setSessionScope(Sessionscope* s)
  {
    if (sessionScope)
    {
      releaseSessionScopeLock();
      sessionScope->release();
    }

    if (s)
      s->addRef();
    sessionScope = s;
  }

  void HttpRequest::clearSession()
  {
    if (sessionScope)
    {
      releaseSessionScopeLock();
      sessionScope->release();
      sessionScope = 0;
    }
  }

  void HttpRequest::ensureApplicationScopeLock()
  {
    log_trace("ensureApplicationScopeLock; thread " << pthread_self());

    ensureSessionScopeLock();
    if (applicationScope && !applicationScopeLocked)
    {
      log_debug("lock application scope; thread" << pthread_self());
      applicationScope->lock();
      applicationScopeLocked = true;
    }
    else
      log_debug("applicationscope locked already");
  }

  void HttpRequest::ensureSessionScopeLock()
  {
    log_trace("ensureSessionScopeLock; thread " << pthread_self());

    if (sessionScope && !sessionScopeLocked)
    {
      log_debug("lock sessionscope; thread " << pthread_self());
      sessionScope->lock();
      sessionScopeLocked = true;
    }
    else
      log_debug("sessionscope locked already");
  }

  void HttpRequest::releaseApplicationScopeLock()
  {
    log_trace("releaseApplicationScopeLock; thread " << pthread_self());

    if (applicationScope && applicationScopeLocked)
    {
      log_debug("unlock applicationscope");
      applicationScopeLocked = false;
      applicationScope->unlock();
    }
    else
      log_debug("applicationscope not locked");
  }

  void HttpRequest::releaseSessionScopeLock()
  {
    log_trace("releaseSessionScopeLock; thread " << pthread_self());

    releaseApplicationScopeLock();

    if (sessionScope && sessionScopeLocked)
    {
      log_debug("unlock sessionscope");
      sessionScopeLocked = false;
      sessionScope->unlock();
    }
    else
      log_debug("sessionscope not locked");
  }

  Scope& HttpRequest::getRequestScope()
  {
    if (requestScope == 0)
      requestScope = new Scope();
    return *requestScope;
  }

  Scope& HttpRequest::getApplicationScope()
  {
    ensureApplicationScopeLock();
    return *applicationScope;
  }

  Sessionscope& HttpRequest::getSessionScope()
  {
    if (!sessionScope)
      sessionScope = new Sessionscope();
    ensureSessionScopeLock();
    return *sessionScope;
  }

  bool HttpRequest::hasSessionScope() const
  {
    return sessionScope != 0 && !sessionScope->empty();
  }
}

