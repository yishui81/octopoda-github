/*
 * HttpHeader.cpp
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#include "HttpHeader.h"

inline static int
is_digit(char c)
{
	return ((c <= '9') && (c >= '0'));
}


inline
HttpHeader::HttpHeader()
  : MIMEHdr(), m_http(NULL), m_url_cached(), m_target_cached(false)
{
}


/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
inline
HttpHeader::~HttpHeader()
{                               /* nop */
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HttpHeader::valid() const
{
  return (m_http && m_mime && m_heap);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::create(HTTPType polarity, HdrHeap *heap)
{
  if (heap) {
    m_heap = heap;
  } else if (!m_heap) {
    m_heap = new_HdrHeap();
  }

  m_http = http_hdr_create(m_heap, polarity);
  m_mime = m_http->m_fields_impl;
}

inline void
HttpHeader::clear()
{

  if (m_http && m_http->m_polarity == HTTP_TYPE_REQUEST) {
    m_url_cached.clear();
  }
  this->HdrHeapSDKHandle::clear();
  m_http = NULL;
  m_mime = NULL;
}

inline void
HttpHeader::reset()
{
  m_heap = NULL;
  m_http = NULL;
  m_mime = NULL;
  m_url_cached.reset();
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::copy(const HttpHeader *hdr)
{
  ink_assert(hdr->valid());

  if (valid()) {
    http_hdr_copy_onto(hdr->m_http, hdr->m_heap, m_http, m_heap, (m_heap != hdr->m_heap) ? true : false);
  } else {
    m_heap = new_HdrHeap();
    m_http = http_hdr_clone(hdr->m_http, hdr->m_heap, m_heap);
    m_mime = m_http->m_fields_impl;
  }
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::copy_shallow(const HttpHeader *hdr)
{
  ink_assert(hdr->valid());

  m_heap = hdr->m_heap;
  m_http = hdr->m_http;
  m_mime = hdr->m_mime;

  if (hdr->type_get() == HTTP_TYPE_REQUEST && m_url_cached.valid())
    m_url_cached.copy_shallow(&hdr->m_url_cached);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HttpHeader::print(char *buf, int bufsize, int *bufindex, int *dumpoffset)
{
  ink_assert(valid());
  return http_hdr_print(m_heap, m_http, buf, bufsize, bufindex, dumpoffset);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HttpHeader::length_get()
{
  ink_assert(valid());
  return http_hdr_length_get(m_http);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::_test_and_fill_target_cache() const {
  if (!m_target_cached) this->_fill_target_cache();
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline char const*
HttpHeader::host_get(int* length)
{
  this->_test_and_fill_target_cache();
  if (m_target_in_url) {
    return url_get()->host_get(length);
  } else if (m_host_mime) {
    if (length) *length = m_host_length;
    return m_host_mime->m_ptr_value;
  }

  if (length) *length = 0;
  return NULL;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int
HttpHeader::port_get()
{
  this->_test_and_fill_target_cache();
  return m_port;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline bool
HttpHeader::is_target_in_url() const
{
  this->_test_and_fill_target_cache();
  return m_target_in_url;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline bool
HttpHeader::is_port_in_header() const
{
  this->_test_and_fill_target_cache();
  return m_port_in_header;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::mark_target_dirty() const
{
  m_target_cached = false;
}
/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPType
http_hdr_type_get(HttpHeaderImpl *hh)
{
  return (hh->m_polarity);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPType
HttpHeader::type_get() const
{
  ink_assert(valid());
  return http_hdr_type_get(m_http);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline int32_t
http_hdr_version_get(HttpHeaderImpl *hh)
{
  return (hh->m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPVersion
HttpHeader::version_get()
{
  ink_assert(valid());
  return HTTPVersion(http_hdr_version_get(m_http));
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::version_set(HTTPVersion version)
{
  ink_assert(valid());
  http_hdr_version_set(m_http, version.m_version);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline const char *
HttpHeader::method_get(int *length)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  return http_hdr_method_get(m_http, length);
}


inline int
HttpHeader::method_get_wksidx()
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  return (m_http->u.req.m_method_wks_idx);
}


/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::method_set(const char *value, int length)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  int method_wks_idx = hdrtoken_tokenize(value, length);
  http_hdr_method_set(m_heap, m_http, value, method_wks_idx, length, true);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline URL *
HttpHeader::url_create(URL *u)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  u->set(this);
  u->create(m_heap);
  return (u);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline URL *
HttpHeader::url_get() const
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  // It's entirely possible that someone changed URL in our impl
  // without updating the cached copy in the C++ layer.  Check
  // to see if this happened before handing back the url

  URLImpl *real_impl = m_http->u.req.m_url_impl;
  if (m_url_cached.m_url_impl != real_impl) {
    m_url_cached.set(this);
    m_url_cached.m_url_impl = real_impl;
    this->mark_target_dirty();
  }
  return (&m_url_cached);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline URL *
HttpHeader::url_get(URL *url)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  url->set(this);               // attach refcount
  url->m_url_impl = m_http->u.req.m_url_impl;
  return (url);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::url_set(URL *url)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  URLImpl *url_impl = m_http->u.req.m_url_impl;
  ::url_copy_onto(url->m_url_impl, url->m_heap, url_impl, m_heap, true);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::url_set_as_server_url(URL *url)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  URLImpl *url_impl = m_http->u.req.m_url_impl;
  ::url_copy_onto_as_server_url(url->m_url_impl, url->m_heap, url_impl, m_heap, true);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::url_set(const char *str, int length)
{
  URLImpl *url_impl;

  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  url_impl = m_http->u.req.m_url_impl;
  ::url_clear(url_impl);
  ::url_parse(m_heap, url_impl, &str, str + length, true);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPStatus
http_hdr_status_get(HttpHeaderImpl *hh)
{
  ink_assert(hh->m_polarity == HTTP_TYPE_RESPONSE);
  return (HTTPStatus) hh->u.resp.m_status;
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline HTTPStatus
HttpHeader::status_get()
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_RESPONSE);

  return (NULL == m_http) ? HTTP_STATUS_NONE : http_hdr_status_get(m_http);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::status_set(HTTPStatus status)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_RESPONSE);

  http_hdr_status_set(m_http, status);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline const char *
HttpHeader::reason_get(int *length)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_RESPONSE);

  return http_hdr_reason_get(m_http, length);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline void
HttpHeader::reason_set(const char *value, int length)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_RESPONSE);

  http_hdr_reason_set(m_heap, m_http, value, length, true);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline MIMEParseResult
HttpHeader::parse_req(HTTPParser *parser, const char **start, const char *end, bool eof)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_REQUEST);

  return http_parser_parse_req(parser, m_heap, m_http, start, end, true, eof);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline MIMEParseResult
HttpHeader::parse_resp(HTTPParser *parser, const char **start, const char *end, bool eof)
{
  ink_assert(valid());
  ink_assert(m_http->m_polarity == HTTP_TYPE_RESPONSE);

  return http_parser_parse_resp(parser, m_heap, m_http, start, end, true, eof);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline bool
HttpHeader::is_cache_control_set(const char *cc_directive_wks)
{
  ink_assert(valid());
  ink_assert(hdrtoken_is_wks(cc_directive_wks));

  HdrTokenHeapPrefix *prefix = hdrtoken_wks_to_prefix(cc_directive_wks);
  ink_assert(prefix->wks_token_type == HDRTOKEN_TYPE_CACHE_CONTROL);

  uint32_t cc_mask = prefix->wks_type_specific.u.cache_control.cc_mask;
  if (get_cooked_cc_mask() & cc_mask)
    return (true);
  else
    return (false);
}

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/

inline bool
HttpHeader::is_pragma_no_cache_set()
{
  ink_assert(valid());
  return (get_cooked_pragma_no_cache());
}

inline char*
HttpHeader::url_string_get_ref(int* length)
{
  return this->url_string_get(USE_HDR_HEAP_MAGIC, length);
}

inline char const*
HttpHeader::path_get(int* length)
{
  URL* url = this->url_get();
  return url ? url->path_get(length) : 0;
}

inline char const*
HttpHeader::scheme_get(int* length)
{
  URL* url = this->url_get();
  return url ? url->scheme_get(length) : 0;
}


void
HttpHeader::_fill_target_cache() const
{
	URL* url = this->url_get();
	char const* port_ptr;

	m_target_in_url = false;
	m_port_in_header = false;
	m_host_mime = NULL;

	// Check in the URL first, then the HOST field.
	if (0 != url->host_get(&m_host_length)) {

		m_target_in_url = true;
		m_port = url->port_get();
		m_port_in_header = 0 != url->port_get_raw();
		m_host_mime = NULL;

	} else if (0 != (m_host_mime = const_cast<HttpHeader*>(this)->get_host_port_values(0, &m_host_length, &port_ptr, 0))) {

		if (port_ptr) {

			m_port = 0;
			for ( ; is_digit(*port_ptr) ; ++port_ptr ){
				m_port = m_port * 10 + *port_ptr - '0';
			}

			m_port_in_header = (0 != m_port);
		}

		m_port = url_canonicalize_port(url->m_url_impl->m_url_type, m_port);
	}

	m_target_cached = true;
}

void
HttpHeader::set_url_target_from_host_field(URL* url) {

	this->_test_and_fill_target_cache();

	if (!url) {

		// Use local cached URL and don't copy if the target
		// is already there.
		if (!m_target_in_url && m_host_mime && m_host_length) {

			m_url_cached.host_set(m_host_mime->m_ptr_value, m_host_length);
			if (m_port_in_header) {
				m_url_cached.port_set(m_port);
			}
			m_target_in_url = true; // it's there now.
		}

	} else {

		int host_len = 0;
		char const *host = NULL;

		host = host_get(&host_len);
		url->host_set(host, host_len);

		if (m_port_in_header) {
			url->port_set(m_port);
		}
	}

}

char*
HttpHeader::url_string_get(Arena* arena, int* length) {

	char* zret = 0;
	UrlPrintHack hack(this);

	if (hack.is_valid()) {
		// The use of a magic value for Arena to indicate the internal heap is
		// even uglier but it's less so than duplicating this entire method to
		// change that one thing.

		zret = (arena == USE_HDR_HEAP_MAGIC)
		? m_url_cached.string_get_ref(length)
		: m_url_cached.string_get(arena, length)
		;
	}
	return zret;
}

int
HttpHeader::url_print(char* buff, int length, int* offset, int* skip) {

	int zret = 0;
	UrlPrintHack hack(this);

	if (hack.is_valid()) {
		zret = m_url_cached.print(buff, length, offset, skip);
	}

	return zret;

}

/***********************************************************************
 *                                                                     *
 *                        M A R S H A L I N G                          *
 *                                                                     *
 ***********************************************************************/

int
HttpHeader::unmarshal(char *buf, int len, RefCountObj *block_ref)
{
	m_heap = (HdrHeap *)buf;

	int res = m_heap->unmarshal(len, HDR_HEAP_OBJ_HTTP_HEADER, (HdrHeapObjImpl **) & m_http, block_ref);

	if (res > 0) {
		m_mime = m_http->m_fields_impl;
	} else {
		clear();
	}

	return res;
}


int
HttpHeaderImpl::marshal(MarshalXlate *ptr_xlate, int num_ptr, MarshalXlate *str_xlate, int num_str)
{

	if (m_polarity == HTTP_TYPE_REQUEST) {

		HDR_MARSHAL_STR(u.req.m_ptr_method, str_xlate, num_str);
		HDR_MARSHAL_PTR(u.req.m_url_impl, URLImpl, ptr_xlate, num_ptr);

	} else if (m_polarity == HTTP_TYPE_RESPONSE) {

		HDR_MARSHAL_STR(u.resp.m_ptr_reason, str_xlate, num_str);

	} else {

		ink_release_assert(!"unknown m_polarity");

	}

	HDR_MARSHAL_PTR(m_fields_impl, MIMEHdrImpl, ptr_xlate, num_ptr);

	return 0;
}


void
HttpHeaderImpl::unmarshal(intptr_t offset)
{

	if (m_polarity == HTTP_TYPE_REQUEST) {

		HDR_UNMARSHAL_STR(u.req.m_ptr_method, offset);
		HDR_UNMARSHAL_PTR(u.req.m_url_impl, URLImpl, offset);

	} else if (m_polarity == HTTP_TYPE_RESPONSE) {

		HDR_UNMARSHAL_STR(u.resp.m_ptr_reason, offset);

	} else {

		ink_release_assert(!"unknown m_polarity");

	}

	HDR_UNMARSHAL_PTR(m_fields_impl, MIMEHdrImpl, offset);
}


void
HttpHeaderImpl::move_strings(HdrStrHeap *new_heap)
{
	if (m_polarity == HTTP_TYPE_REQUEST) {

		HDR_MOVE_STR(u.req.m_ptr_method, u.req.m_len_method);

	} else if (m_polarity == HTTP_TYPE_RESPONSE) {

		HDR_MOVE_STR(u.resp.m_ptr_reason, u.resp.m_len_reason);

	} else {

		ink_release_assert(!"unknown m_polarity");

	}
}

void
HttpHeaderImpl::check_strings(HeapCheck *heaps, int num_heaps)
{

	if (m_polarity == HTTP_TYPE_REQUEST) {

		CHECK_STR(u.req.m_ptr_method, u.req.m_len_method, heaps, num_heaps);

	} else if (m_polarity == HTTP_TYPE_RESPONSE) {

		CHECK_STR(u.resp.m_ptr_reason, u.resp.m_len_reason, heaps, num_heaps);

	} else {

		ink_release_assert(!"unknown m_polarity");

	}
}

