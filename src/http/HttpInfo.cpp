/*
 * HttpInfo.cpp
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#include "HttpInfo.h"



HttpInfo::HttpInfo() {
	// TODO Auto-generated constructor stub

}

HttpInfo::~HttpInfo() {
	// TODO Auto-generated destructor stub
}

inline void
HttpInfo::destroy()
{
	if (m_alt) {
		if (m_alt->m_writeable) {
			m_alt->destroy();
		} else if (m_alt->m_ext_buffer) {
			  if (m_alt->m_ext_buffer->refcount_dec() == 0) {
				  m_alt->m_ext_buffer->free();
			  }
		}
	}
	clear();
}

inline HttpInfo &
HttpInfo::operator =(const HttpInfo & m)
{
	m_alt = m.m_alt;
	return *this;
}

inline INK_MD5
HttpInfo::object_key_get()
{
	INK_MD5 val;
	int32_t* pi = reinterpret_cast<int32_t*>(&val);

	pi[0] = m_alt->m_object_key[0];
	pi[1] = m_alt->m_object_key[1];
	pi[2] = m_alt->m_object_key[2];
	pi[3] = m_alt->m_object_key[3];

	return val;
}

inline void
HttpInfo::object_key_get(INK_MD5 *md5)
{
	int32_t* pi = reinterpret_cast<int32_t*>(md5);
	pi[0] = m_alt->m_object_key[0];
	pi[1] = m_alt->m_object_key[1];
	pi[2] = m_alt->m_object_key[2];
	pi[3] = m_alt->m_object_key[3];
}

inline bool
HttpInfo::compare_object_key(const INK_MD5 *md5)
{
	int32_t const* pi = reinterpret_cast<int32_t const*>(md5);
	return ((m_alt->m_object_key[0] == pi[0]) &&
		  (m_alt->m_object_key[1] == pi[1]) &&
		  (m_alt->m_object_key[2] == pi[2]) &&
		  (m_alt->m_object_key[3] == pi[3])
		 );
}

inline int64_t
HttpInfo::object_size_get()
{
	int64_t val;
	int32_t* pi = reinterpret_cast<int32_t*>(&val);

	pi[0] = m_alt->m_object_size[0];
	pi[1] = m_alt->m_object_size[1];
	return val;
}

inline void
HttpInfo::object_key_set(INK_MD5 & md5)
{
	int32_t* pi = reinterpret_cast<int32_t*>(&md5);
	m_alt->m_object_key[0] = pi[0];
	m_alt->m_object_key[1] = pi[1];
	m_alt->m_object_key[2] = pi[2];
	m_alt->m_object_key[3] = pi[3];
}

inline void
HttpInfo::object_size_set(int64_t size)
{
	int32_t* pi = reinterpret_cast<int32_t*>(&size);
	m_alt->m_object_size[0] = pi[0];
	m_alt->m_object_size[1] = pi[1];
}

inline HttpInfo::FragOffset*
HttpInfo::get_frag_table()
{
	return m_alt ? m_alt->m_frag_offsets : 0;
}

inline int
HttpInfo::get_frag_offset_count() {
	return m_alt ? m_alt->m_frag_offset_count : 0;
}


void
HttpInfo::create()
{
	m_alt = HttpCacheAlt::alloc();
			HttpCacheAltAllocator.alloc();
}

void
HttpInfo::copy(HttpInfo *hi)
{
	if (m_alt && m_alt->m_writeable) {
		destroy();
	}

	create();
	m_alt->copy(hi->m_alt);
}

void
HttpInfo::copy_frag_offsets_from(HttpInfo* src)
{
	if (m_alt && src->m_alt){
		m_alt->copy_frag_offsets_from(src->m_alt);
	}
}


int
HttpInfo::marshal_length()
{
	int len = HTTP_ALT_MARSHAL_SIZE;

	if (m_alt->m_request_hdr.valid()) {
		len += m_alt->m_request_hdr.m_heap->marshal_length();
	}

	if (m_alt->m_response_hdr.valid()) {
		len += m_alt->m_response_hdr.m_heap->marshal_length();
	}

	if (m_alt->m_frag_offset_count > HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS) {
		len -= sizeof(m_alt->m_integral_frag_offsets);
		len += sizeof(FragOffset) * m_alt->m_frag_offset_count;
	}

	return len;
}

int
HttpInfo::marshal(char *buf, int len)
{
	int tmp;
	int used = 0;
	HttpCacheAlt *marshal_alt = (HttpCacheAlt *) buf;
	// non-zero only if the offsets are external. Otherwise they get
	// marshalled along with the alt struct.
	int frag_len = (0 == m_alt->m_frag_offset_count || m_alt->m_frag_offsets == m_alt->m_integral_frag_offsets) ? 0 : sizeof(HttpCacheAlt::FragOffset) * m_alt->m_frag_offset_count;

	ink_assert(m_alt->m_magic == CACHE_ALT_MAGIC_ALIVE);

	// Make sure the buffer is aligned
	//    ink_assert(((intptr_t)buf) & 0x3 == 0);

	// If we have external fragment offsets, copy the initial ones
	// into the integral data.

	if (frag_len) {

		memcpy(m_alt->m_integral_frag_offsets, m_alt->m_frag_offsets, sizeof(m_alt->m_integral_frag_offsets));
		frag_len -= sizeof(m_alt->m_integral_frag_offsets);
		// frag_len should never be non-zero at this point, as the offsets
		// should be external only if too big for the internal table.

	}

	// Memcpy the whole object so that we can use it
	//   live later.  This involves copying a few
	//   extra bytes now but will save copying any
	//   bytes on the way out of the cache
	memcpy(buf, m_alt, sizeof(HttpCacheAlt));
	marshal_alt->m_magic = CACHE_ALT_MAGIC_MARSHALED;
	marshal_alt->m_writeable = 0;
	marshal_alt->m_unmarshal_len = -1;
	marshal_alt->m_ext_buffer = NULL;

	buf += HTTP_ALT_MARSHAL_SIZE;
	used += HTTP_ALT_MARSHAL_SIZE;

	if (frag_len) {

		marshal_alt->m_frag_offsets = static_cast<FragOffset*>(reinterpret_cast<void*>(used));
		memcpy(buf, m_alt->m_frag_offsets + HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS, frag_len);

		buf += frag_len;
		used += frag_len;

	} else {

		marshal_alt->m_frag_offsets = 0;

	}

	// The m_{request,response}_hdr->m_heap pointers are converted
	//    to zero based offsets from the start of the buffer we're
	//    marshalling in to
	if (m_alt->m_request_hdr.valid()) {

		tmp = m_alt->m_request_hdr.m_heap->marshal(buf, len - used);
		marshal_alt->m_request_hdr.m_heap = (HdrHeap *)(intptr_t)used;

		ink_assert(((intptr_t) marshal_alt->m_request_hdr.m_heap) < len);

		buf += tmp;
		used += tmp;

	} else {

		marshal_alt->m_request_hdr.m_heap = NULL;

	}

	if (m_alt->m_response_hdr.valid()) {

		tmp = m_alt->m_response_hdr.m_heap->marshal(buf, len - used);
		marshal_alt->m_response_hdr.m_heap = (HdrHeap *)(intptr_t)used;

		ink_assert(((intptr_t) marshal_alt->m_response_hdr.m_heap) < len);

		used += tmp;

	} else {

		marshal_alt->m_response_hdr.m_heap = NULL;

	}

	// The prior system failed the marshal if there wasn't
	//   enough space by measuring the space for every
	//   component. Seems much faster to check once to
	//   see if we spammed memory
	ink_release_assert(used <= len);

	return used;
}

int
HttpInfo::unmarshal(char *buf, int len, RefCountObj *block_ref)
{
	HttpCacheAlt *alt = (HttpCacheAlt *) buf;
	int orig_len = len;

	if (alt->m_magic == CACHE_ALT_MAGIC_ALIVE) {

		// Already unmarshaled, must be a ram cache
		//  it
		ink_assert(alt->m_unmarshal_len > 0);
		ink_assert(alt->m_unmarshal_len <= len);
		return alt->m_unmarshal_len;

	} else if (alt->m_magic != CACHE_ALT_MAGIC_MARSHALED) {

		ink_assert(!"HttpInfo::unmarshal bad magic");
		return -1;

	}

	ink_assert(alt->m_unmarshal_len < 0);
	alt->m_magic = CACHE_ALT_MAGIC_ALIVE;

	ink_assert(alt->m_writeable == 0);

	len -= HTTP_ALT_MARSHAL_SIZE;

	if (alt->m_frag_offset_count > HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS) {

		// stuff that didn't fit in the integral slots.
		int extra = sizeof(FragOffset) * alt->m_frag_offset_count - sizeof(alt->m_integral_frag_offsets);
		char* extra_src = buf + reinterpret_cast<intptr_t>(alt->m_frag_offsets);

		// Actual buffer size, which must be a power of two.
		// Well, technically not, because we never modify an unmarshalled fragment
		// offset table, but it would be a nasty bug should that be done in the
		// future.
		int bcount = HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS * 2;

		while (bcount < alt->m_frag_offset_count){
			bcount *= 2;
		}

		alt->m_frag_offsets = static_cast<FragOffset*>(ats_malloc(bcount * sizeof(FragOffset))); // WRONG - must round up to next power of 2.

		memcpy(alt->m_frag_offsets, alt->m_integral_frag_offsets, sizeof(alt->m_integral_frag_offsets));
		memcpy(alt->m_frag_offsets + HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS, extra_src, extra);
		len -= extra;

	} else if (alt->m_frag_offset_count > 0) {

		alt->m_frag_offsets = alt->m_integral_frag_offsets;

	} else {

		alt->m_frag_offsets = 0; // should really already be zero.

	}

	HdrHeap *heap = (HdrHeap *) (alt->m_request_hdr.m_heap ? (buf + (intptr_t) alt->m_request_hdr.m_heap) : 0);
	HttpHeaderImpl *hh = NULL;
	int tmp = 0;

	if (heap != NULL) {

		tmp = heap->unmarshal(len, HDR_HEAP_OBJ_HTTP_HEADER, (HdrHeapObjImpl **) & hh, block_ref);
		if (hh == NULL || tmp < 0) {

			ink_assert(!"HttpInfo::request unmarshal failed");
			return -1;

		}
		len -= tmp;

		alt->m_request_hdr.m_heap = heap;
		alt->m_request_hdr.m_http = hh;
		alt->m_request_hdr.m_mime = hh->m_fields_impl;
		alt->m_request_hdr.m_url_cached.m_heap = heap;
	}

	heap = (HdrHeap *) (alt->m_response_hdr.m_heap ? (buf + (intptr_t) alt->m_response_hdr.m_heap) : 0);
	if (heap != NULL) {

		tmp = heap->unmarshal(len, HDR_HEAP_OBJ_HTTP_HEADER, (HdrHeapObjImpl **) & hh, block_ref);
		if (hh == NULL || tmp < 0) {
			ink_assert(!"HttpInfo::response unmarshal failed");
			return -1;
		}
		len -= tmp;

		alt->m_response_hdr.m_heap = heap;
		alt->m_response_hdr.m_http = hh;
		alt->m_response_hdr.m_mime = hh->m_fields_impl;

	}

	alt->m_unmarshal_len = orig_len - len;

	return alt->m_unmarshal_len;
}

// bool HttpInfo::check_marshalled(char* buf, int len)
//  Checks a marhshalled HttpInfo buffer to make
//    sure it's sane.  Returns true if sane, false otherwise
//
bool
HttpInfo::check_marshalled(char *buf, int len)
{
	HttpCacheAlt *alt = (HttpCacheAlt *) buf;

	if (alt->m_magic != CACHE_ALT_MAGIC_MARSHALED) {
		return false;
	}

	if (alt->m_writeable != false) {
		return false;
	}

	if (len < HTTP_ALT_MARSHAL_SIZE) {
		return false;
	}

	if (alt->m_request_hdr.m_heap == NULL) {
		return false;
	}

	if ((intptr_t) alt->m_request_hdr.m_heap > len) {
		return false;
	}

	HdrHeap *heap = (HdrHeap *) (buf + (intptr_t) alt->m_request_hdr.m_heap);
	if (heap->check_marshalled(len) == false) {
		return false;
	}

	if (alt->m_response_hdr.m_heap == NULL) {
		return false;
	}

	if ((intptr_t) alt->m_response_hdr.m_heap > len) {
		return false;
	}

	heap = (HdrHeap *) (buf + (intptr_t) alt->m_response_hdr.m_heap);
	if (heap->check_marshalled(len) == false) {
		return false;
	}

	return true;
}


// void HttpInfo::set_buffer_reference(RefCountObj* block_ref)
//
//    Setting a buffer reference for the alt is separate from
//     the unmarshalling operation because the clustering
//     utilizes the system differently than cache does
//    The cache maintains external refcounting of the buffer that
//     the alt is in & doesn't always destroy the alt when its
//     done with it because it figures it doesn't need to since
//     it is managing the buffer
//    The receiver of ClusterRPC system has the alt manage the
//     buffer itself and therefore needs to call this function
//     to set up the reference
//
void
HttpInfo::set_buffer_reference(RefCountObj *block_ref)
{
	//ink_assert(m_alt->m_magic == CACHE_ALT_MAGIC_ALIVE);

	// Free existing reference
	if (m_alt->m_ext_buffer != NULL) {

		if (m_alt->m_ext_buffer->refcount_dec() == 0) {
			m_alt->m_ext_buffer->free();
		}

	}
	// Set up the ref count for the external buffer
	//   if there is one
	if (block_ref) {
		block_ref->refcount_inc();
	}

	m_alt->m_ext_buffer = block_ref;
}

int
HttpInfo::get_handle(char *buf, int len)
{

	// All the offsets have already swizzled to pointers.  All we
	//  need to do is set m_alt and make sure things are sane
	HttpCacheAlt *a = (HttpCacheAlt *) buf;

	if (a->m_magic == CACHE_ALT_MAGIC_ALIVE) {

		m_alt = a;
		//ink_assert(m_alt->m_unmarshal_len > 0);
		//ink_assert(m_alt->m_unmarshal_len <= len);
		return m_alt->m_unmarshal_len;

	}

	clear();
	return -1;
}

void
HttpInfo::push_frag_offset(FragOffset offset)
{
	//ink_assert(m_alt);

	if (0 == m_alt->m_frag_offsets) {

		m_alt->m_frag_offsets = m_alt->m_integral_frag_offsets;

	} else if (m_alt->m_frag_offset_count >= HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS
			&& 0 == (m_alt->m_frag_offset_count & (m_alt->m_frag_offset_count-1))) {

		// need more space than in integral storage and we're at an upgrade
		// size (power of 2).
		FragOffset* nf = static_cast<FragOffset*>(ats_malloc(sizeof(FragOffset) * (m_alt->m_frag_offset_count * 2)));

		memcpy(nf, m_alt->m_frag_offsets, sizeof(FragOffset) * m_alt->m_frag_offset_count);

		if (m_alt->m_frag_offsets != m_alt->m_integral_frag_offsets){

			ats_free(m_alt->m_frag_offsets);

		}
		m_alt->m_frag_offsets = nf;
	}

	m_alt->m_frag_offsets[m_alt->m_frag_offset_count++] = offset;
}
