/*
 * HttpInfo.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef HttpInfo_H_
#define HttpInfo_H_
#include <stdint.h>
#include <stddef.h>
#include "HttpCacheAlt.h"

class HttpInfo
{
public:
	typedef HttpCacheAlt::FragOffset FragOffset; ///< Import type.

	HttpCacheAlt *m_alt;

	HttpInfo()
		: m_alt(NULL)
	{

	}

	~HttpInfo()
	{
		clear();
	}

	void clear() {
		m_alt = NULL;
	}

	bool valid() const {
		return m_alt != NULL;
	}

	void create();
	void destroy();

	void copy(HttpInfo *to_copy);
	void copy_shallow(HttpInfo *info)
	{
		m_alt = info->m_alt;
	}

	void copy_frag_offsets_from(HttpInfo* src);
	HttpInfo & operator =(const HttpInfo & m);

	int marshal_length();
	int marshal(char *buf, int len);
	static int unmarshal(char *buf, int len, RefCountObj *block_ref);
	void set_buffer_reference(RefCountObj *block_ref);
	int get_handle(char *buf, int len);

	int32_t id_get() const
	{
		return m_alt->m_id;
	}

	int32_t rid_get()
	{
		return m_alt->m_rid;
	}

	void id_set(int32_t id)
	{
		m_alt->m_id = id;
	}

	void rid_set(int32_t id)
	{
		m_alt->m_rid = id;
	}

	INK_MD5 object_key_get();
	void object_key_get(INK_MD5 *);
	bool compare_object_key(const INK_MD5 *);
	int64_t object_size_get();

	void request_get(HttpHeader *hdr)
	{
		hdr->copy_shallow(&m_alt->m_request_hdr);
	}

	void response_get(HttpHeader *hdr)
	{
		hdr->copy_shallow(&m_alt->m_response_hdr);
	}

	HttpHeader *request_get()
	{
		return &m_alt->m_request_hdr;
	}

	HttpHeader *response_get()
	{
		return &m_alt->m_response_hdr;
	}

	URL *request_url_get(URL *url = NULL)
	{
		return m_alt->m_request_hdr.url_get(url);
	}

	time_t request_sent_time_get()
	{
		return m_alt->m_request_sent_time;
	}

	time_t response_received_time_get()
	{
		return m_alt->m_response_received_time;
	}

	void object_key_set(INK_MD5 & md5);
	void object_size_set(int64_t size);

	void request_set(const HttpHeader *req)
	{
		m_alt->m_request_hdr.copy(req);
	}

	void response_set(const HttpHeader *resp)
	{
		m_alt->m_response_hdr.copy(resp);
	}

	void request_sent_time_set(time_t t)
	{
		m_alt->m_request_sent_time = t;
	}

	void response_received_time_set(time_t t)
	{
		m_alt->m_response_received_time = t;
	}

	/// Get the fragment table.
	FragOffset* get_frag_table();
	/// Get the # of fragment offsets
	/// @note This is the size of the fragment offset table, and one less
	/// than the actual # of fragments.
	int get_frag_offset_count();
	/// Add an @a offset to the end of the fragment offset table.
	void push_frag_offset(FragOffset offset);

	// Sanity check functions
	static bool check_marshalled(char *buf, int len);

private:
	HttpInfo(const HttpInfo & h);
};



#endif /* HttpInfo_H_ */
