/*
 * HttpVersion.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef HTTPVERSION_H_
#define HTTPVERSION_H_

#define HTTP_VERSION(a,b)  ((((a) & 0xFFFF) << 16) | ((b) & 0xFFFF))
#define HTTP_MINOR(v)      ((v) & 0xFFFF)
#define HTTP_MAJOR(v)      (((v) >> 16) & 0xFFFF)

class HTTPVersion
{
public:
  HTTPVersion();
  HTTPVersion(int32_t version);
  HTTPVersion(int ver_major, int ver_minor);

  void set(HTTPVersion ver);
  void set(int ver_major, int ver_minor);

  HTTPVersion & operator =(const HTTPVersion & hv);
  int operator ==(const HTTPVersion & hv);
  int operator !=(const HTTPVersion & hv);
  int operator >(const HTTPVersion & hv);
  int operator <(const HTTPVersion & hv);
  int operator >=(const HTTPVersion & hv);
  int operator <=(const HTTPVersion & hv);

public:
    int32_t m_version;
};

class IOBufferReader;




#endif /* HTTPVERSION_H_ */
