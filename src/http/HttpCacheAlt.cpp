/*
 * HttpCacheAlt.cpp
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#include "HttpCacheAlt.h"
#include "HdrHeap.h"
#include <stddef.h>

ClassAllocator<HttpCacheAlt> HttpCacheAltAllocator("HttpCacheAltAllocator");
const int HTTP_ALT_MARSHAL_SIZE = ROUND(sizeof(HttpCacheAlt), HDR_PTR_SIZE);

/*-------------------------------------------------------------------------
  -------------------------------------------------------------------------*/
HttpCacheAlt::HttpCacheAlt():
	m_magic(CACHE_ALT_MAGIC_ALIVE),
	m_writeable(1),
	m_unmarshal_len(-1),
	m_id(-1),
	m_rid(-1),
	m_request_hdr(),
	m_response_hdr(),
	m_request_sent_time(0),
	m_response_received_time(0),
	m_frag_offset_count(0),
	m_frag_offsets(0),
	m_ext_buffer(NULL)
{

	m_object_key[0] = 0;
	m_object_key[1] = 0;
	m_object_key[2] = 0;
	m_object_key[3] = 0;
	m_object_size[0] = 0;
	m_object_size[1] = 0;
}

void
HttpCacheAlt::destroy()
{
//	ink_assert(m_magic == CACHE_ALT_MAGIC_ALIVE);
//	ink_assert(m_writeable);

	m_magic = CACHE_ALT_MAGIC_DEAD;
	m_writeable = 0;
	m_request_hdr.destroy();
	m_response_hdr.destroy();
	m_frag_offset_count = 0;

	if (m_frag_offsets && m_frag_offsets != m_integral_frag_offsets) {
		ats_free(m_frag_offsets);
		m_frag_offsets = 0;
	}

	HttpCacheAltAllocator.free(this);
}

void
HttpCacheAlt::copy(HttpCacheAlt *to_copy)
{

	m_magic = to_copy->m_magic;
	// m_writeable =      to_copy->m_writeable;
	m_unmarshal_len = to_copy->m_unmarshal_len;
	m_id = to_copy->m_id;
	m_rid = to_copy->m_rid;
	m_object_key[0] = to_copy->m_object_key[0];
	m_object_key[1] = to_copy->m_object_key[1];
	m_object_key[2] = to_copy->m_object_key[2];
	m_object_key[3] = to_copy->m_object_key[3];
	m_object_size[0] = to_copy->m_object_size[0];
	m_object_size[1] = to_copy->m_object_size[1];

	if (to_copy->m_request_hdr.valid()) {
		m_request_hdr.copy(&to_copy->m_request_hdr);
	}

	if (to_copy->m_response_hdr.valid()) {
		m_response_hdr.copy(&to_copy->m_response_hdr);
	}

	m_request_sent_time = to_copy->m_request_sent_time;
	m_response_received_time = to_copy->m_response_received_time;
	this->copy_frag_offsets_from(to_copy);
}

void
HttpCacheAlt::copy_frag_offsets_from(HttpCacheAlt *src)
{
	m_frag_offset_count = src->m_frag_offset_count;

	if (m_frag_offset_count > 0) {

		if (m_frag_offset_count > N_INTEGRAL_FRAG_OFFSETS) {

			/* Mixed feelings about this - technically we don't need it to be a
			 power of two when copied because currently that means it is frozen.
			 But that could change later and it would be a nasty bug to find.
			 So we'll do it for now. The relative overhead is tiny.
			*/
			int bcount = HttpCacheAlt::N_INTEGRAL_FRAG_OFFSETS * 2;
			while (bcount < m_frag_offset_count) {
				bcount *= 2;
			}
			m_frag_offsets = static_cast<FragOffset*>(ats_malloc(sizeof(FragOffset) * bcount));

		} else {
			m_frag_offsets = m_integral_frag_offsets;
		}

		memcpy(m_frag_offsets, src->m_frag_offsets, sizeof(FragOffset) * m_frag_offset_count);
	}
}



