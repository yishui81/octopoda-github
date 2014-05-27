/*
 * HttpCacheAlt.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef HTTPCACHEALT_H_
#define HTTPCACHEALT_H_

enum
{
  CACHE_ALT_MAGIC_ALIVE = 0xabcddeed,
  CACHE_ALT_MAGIC_MARSHALED = 0xdcbadeed,
  CACHE_ALT_MAGIC_DEAD = 0xdeadeed
};

class HttpCacheAlt {
public:
  HttpCacheAlt();
  void copy(HttpCacheAlt *to_copy);
  void copy_frag_offsets_from(HttpCacheAlt* src);
  void destroy();

  uint32_t m_magic;

  // Writeable is set to true is we reside
  //  in a buffer owned by this structure.
  // INVARIANT: if own the buffer this HttpCacheAlt
  //   we also own the buffers for the request &
  //   response headers
  int32_t m_writeable;
  int32_t m_unmarshal_len;

  int32_t m_id;
  int32_t m_rid;

  int32_t m_object_key[4];
  int32_t m_object_size[2];

  HttpHeader m_request_hdr;
  HttpHeader m_response_hdr;

  time_t m_request_sent_time;
  time_t m_response_received_time;

  /// # of fragment offsets in this alternate.
  /// @note This is one less than the number of fragments.
  int m_frag_offset_count;
  /// Type of offset for a fragment.
  typedef uint64_t FragOffset;
  /// Table of fragment offsets.
  /// @note The offsets are forward looking so that frag[0] is the
  /// first byte past the end of fragment 0 which is also the first
  /// byte of fragment 1. For this reason there is no fragment offset
  /// for the last fragment.
  FragOffset *m_frag_offsets;
  /// # of fragment offsets built in to object.
  static int const N_INTEGRAL_FRAG_OFFSETS = 4;
  /// Integral fragment offset table.
  FragOffset m_integral_frag_offsets[N_INTEGRAL_FRAG_OFFSETS];

  // With clustering, our alt may be in cluster
  //  incoming channel buffer, when we are
  //  destroyed we decrement the refcount
  //  on that buffer so that it gets destroyed
  // We don't want to use a ref count ptr (Ptr<>)
  //  since our ownership model requires explicit
  //  destroys and ref count pointers defeat this
  RefCountObj *m_ext_buffer;
};

#endif /* HTTPCACHEALT_H_ */
