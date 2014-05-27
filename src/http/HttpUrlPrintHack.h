/*
 * UrlPrintHack.h
 *
 *  Created on: 2014年5月27日
 *      Author: jacky
 */

#ifndef URLPRINTHACK_H_
#define URLPRINTHACK_H_

// Very ugly, but a proper implementation will require
// rewriting the URL class and all of its clients so that
// clients access the URL through the HTTP header instance
// unless they really need low level access. The header would
// need to either keep two versions of the URL (pristine
// and effective) or URl would have to provide access to
// the URL printer.

/// Hack the URL in the HTTP header to be 1.0 compliant, saving the
/// original values so they can be restored.
class UrlPrintHack {

friend class HttpHeader;

UrlPrintHack(HttpHeader* hdr)
{
	hdr->_test_and_fill_target_cache();

	if (hdr->m_url_cached.valid()) {

		URLImpl* ui = hdr->m_url_cached.m_url_impl;
		char port_buff[10];

		// Save values that can be modified.
		m_hdr = hdr; // mark as having saved values.
		m_len_host = ui->m_len_host;
		m_ptr_host = ui->m_ptr_host;
		m_len_port = ui->m_len_port;
		m_ptr_port = ui->m_ptr_port;

		/* Get dirty. We reach in to the URL implementation to
		set the host and port if
		1) They are not already set and
		2) The values were in a HTTP header field.
		*/
		if (!hdr->m_target_in_url && hdr->m_host_length && hdr->m_host_mime) {
			assert(0 == ui->m_ptr_host); // shouldn't be non-zero if not in URL.
			ui->m_ptr_host = hdr->m_host_mime->m_ptr_value;
			ui->m_len_host = hdr->m_host_length;
		}

		if (0 == hdr->m_url_cached.port_get_raw() && hdr->m_port_in_header) {

			assert(0 == ui->m_ptr_port); // shouldn't be set if not in URL.
			ui->m_ptr_port = port_buff;
			ui->m_len_port = sprintf(port_buff, "%.5d", hdr->m_port);
		}

	} else {
		m_hdr = 0;
	}
}

  /// Destructor.
  /// If we have a valid header, write the original URL values back.
~UrlPrintHack()
{
	if (m_hdr) {
		URLImpl* ui = m_hdr->m_url_cached.m_url_impl;
		ui->m_len_port = m_len_port;
		ui->m_len_host = m_len_host;
		ui->m_ptr_port = m_ptr_port;
		ui->m_ptr_host = m_ptr_host;
	}
}

	/// Check if the hack worked
	bool is_valid() const {
		return 0 != m_hdr;
	}

	/// Saved values.
	///@{
	char const* m_ptr_host;
	char const* m_ptr_port;
	int m_len_host;
	int m_len_port;
	HttpHeader* m_hdr;
  ///@}
};



#endif /* URLPRINTHACK_H_ */
