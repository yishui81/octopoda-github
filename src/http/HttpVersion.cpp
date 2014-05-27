/*
 * HttpVersion.cpp
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#include "HttpVersion.h"


/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPVersion::HTTPVersion()
:m_version(HTTP_VERSION(0, 9))
{
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPVersion::HTTPVersion(int32_t version)
:m_version(version)
{
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPVersion::HTTPVersion(int ver_major, int ver_minor)
  :
m_version(HTTP_VERSION(ver_major, ver_minor))
{
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HTTPVersion::set(HTTPVersion ver)
{
  m_version = ver.m_version;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HTTPVersion::set(int ver_major, int ver_minor)
{
  m_version = HTTP_VERSION(ver_major, ver_minor);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPVersion &
HTTPVersion::operator =(const HTTPVersion & hv)
{
  m_version = hv.m_version;

  return *this;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator ==(const HTTPVersion & hv)
{
  return (m_version == hv.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator !=(const HTTPVersion & hv)
{
  return (m_version != hv.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator >(const HTTPVersion & hv)
{
  return (m_version > hv.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator <(const HTTPVersion & hv)
{
  return (m_version < hv.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator >=(const HTTPVersion & hv)
{
  return (m_version >= hv.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HTTPVersion::operator <=(const HTTPVersion & hv)
{
  return (m_version <= hv.m_version);
}

