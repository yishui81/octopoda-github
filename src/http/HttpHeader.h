/*
 * HttpHeader.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef HTTPHEADER_H_
#define HTTPHEADER_H_
#include "HdrHeap.h"


class HttpHeaderImpl:public HdrHeapObjImpl
{
  // HdrHeapObjImpl is 4 bytes
  HTTPType m_polarity;          // request or response or unknown
  int32_t m_version;              // cooked version number
  // 12 bytes means 4 bytes padding here on 64-bit architectures
  union
  {
    struct
    {
      URLImpl *m_url_impl;
      const char *m_ptr_method;
      uint16_t m_len_method;
      int16_t m_method_wks_idx;
    } req;

    struct
    {
      const char *m_ptr_reason;
      uint16_t m_len_reason;
      int16_t m_status;
    } resp;
  } u;

  MIMEHdrImpl *m_fields_impl;

  // Marshaling Functions
  int marshal(MarshalXlate *ptr_xlate, int num_ptr, MarshalXlate *str_xlate, int num_str);
  void unmarshal(intptr_t offset);
  void move_strings(HdrStrHeap *new_heap);

  // Sanity Check Functions
  void check_strings(HeapCheck *heaps, int num_heaps);
};



class HttpHeader: public MIMEHdr
{
public:
	HttpHeaderImpl *m_http;
  // This is all cached data and so is mutable.
  mutable URL m_url_cached;
  mutable MIMEField *m_host_mime;
  mutable int m_host_length; ///< Length of hostname.
  mutable int m_port; ///< Target port.
  mutable bool m_target_cached; ///< Whether host name and port are cached.
  mutable bool m_target_in_url; ///< Whether host name and port are in the URL.
  /// Set if the port was effectively specified in the header.
  /// @c true if the target (in the URL or the HOST field) also specified
  /// a port. That is, @c true if whatever source had the target host
  /// also had a port, @c false otherwise.
  mutable bool m_port_in_header;

  HttpHeader();
  ~HttpHeader();

  int valid() const;

  void create(HTTPType polarity, HdrHeap *heap = NULL);
  void clear();
  void reset();
  void copy(const HttpHeader *hdr);
  void copy_shallow(const HttpHeader *hdr);

  int unmarshal(char *buf, int len, RefCountObj *block_ref);

  int print(char *buf, int bufsize, int *bufindex, int *dumpoffset);

  int length_get();

  HTTPType type_get() const;

  HTTPVersion version_get();
  void version_set(HTTPVersion version);

  const char *method_get(int *length);
  int method_get_wksidx();
  void method_set(const char *value, int length);

  URL *url_create(URL *url);

  URL *url_get() const;
  URL *url_get(URL *url);
  /** Get a string with the effective URL in it.
      If @a length is not @c NULL then the length of the string
      is stored in the int pointed to by @a length.

      Note that this can be different from getting the @c URL
      and invoking @c URL::string_get if the host is in a header
      field and not explicitly in the URL.
   */
  char* url_string_get(
    Arena* arena = 0, ///< Arena to use, or @c malloc if NULL.
    int* length = 0 ///< Store string length here.
  );
  /** Get a string with the effective URL in it.
      This is automatically allocated if needed in the request heap.

      @see url_string_get
   */
  char* url_string_get_ref(
    int* length = 0 ///< Store string length here.
  );

  /** Print the URL.
      Output is not null terminated.
      @return 0 on failure, non-zero on success.
   */
  int url_print(
      char* buff, ///< Output buffer
      int length, ///< Length of @a buffer
      int* offset, ///< [in,out] ???
      int* skip ///< [in,out] ???
  );

  /** Get the URL path.
      This is a reference, not allocated.
      @return A pointer to the path or @c NULL if there is no valid URL.
  */
  char const* path_get(
		       int* length ///< Storage for path length.
		       );

  /** Get the target host name.
      The length is returned in @a length if non-NULL.
      @note The results are cached so this is fast after the first call.
      @return A pointer to the host name.
  */
  char const* host_get(int* length = 0);

  /** Get the target port.
      If the target port is not found then it is adjusted to the
      default port for the URL type.
      @note The results are cached so this is fast after the first call.
      @return The canonicalized target port.
  */
  int port_get();

  /** Get the URL scheme.
      This is a reference, not allocated.
      @return A pointer to the scheme or @c NULL if there is no valid URL.
  */
  char const* scheme_get(
		       int* length ///< Storage for path length.
		       );
  void url_set(URL *url);
  void url_set_as_server_url(URL *url);
  void url_set(const char *str, int length);

  /// Check location of target host.
  /// @return @c true if the host was in the URL, @c false otherwise.
  /// @note This returns @c false if the host is missing.
  bool is_target_in_url() const;

  /// Check if a port was specified in the target.
  /// @return @c true if the port was part of the target.
  bool is_port_in_header() const;

  /// If the target is in the fields and not the URL, copy it to the @a url.
  /// If @a url is @c NULL the cached URL in this header is used.
  /// @note In the default case the copy is avoided if the cached URL already
  /// has the target. If @a url is non @c NULL the copy is always performed.
  void set_url_target_from_host_field(URL* url = 0);

  /// Mark the target cache as invalid.
  /// @internal Ugly but too many places currently that touch the
  /// header internals, they must be able to do this.
  void mark_target_dirty() const;

  HTTPStatus status_get();
  void status_set(HTTPStatus status);

  const char *reason_get(int *length);
  void reason_set(const char *value, int length);

  MIMEParseResult parse_req(HTTPParser *parser, const char **start, const char *end, bool eof);
  MIMEParseResult parse_resp(HTTPParser *parser, const char **start, const char *end, bool eof);

  MIMEParseResult parse_req(HTTPParser *parser, IOBufferReader *r, int *bytes_used, bool eof);
  MIMEParseResult parse_resp(HTTPParser *parser, IOBufferReader *r, int *bytes_used, bool eof);

public:
  // Utility routines
  bool is_cache_control_set(const char *cc_directive_wks);
  bool is_pragma_no_cache_set();

protected:
  /** Load the target cache.
      @see m_host, m_port, m_target_in_url
  */
  void _fill_target_cache() const;
  /** Test the cache and fill it if necessary.
      @internal In contrast to @c _fill_target_cache, this method
      is inline and checks whether the cache is already filled.
      @ _fill_target_cache @b always does a cache fill.
  */
  void _test_and_fill_target_cache() const;

  static Arena* const USE_HDR_HEAP_MAGIC;

private:
  // No gratuitous copies!
  HttpHeader(const HttpHeader & m);
  HttpHeader & operator =(const HttpHeader & m);

  friend class UrlPrintHack; // don't ask.
};
#endif /* HTTPHEADER_H_ */
