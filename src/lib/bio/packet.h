#pragma once
/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/**
 * $Id$
 * @file lib/bio/packet.h
 * @brief Binary IO abstractions for #fr_packet_t
 *
 * @copyright 2024 Network RADIUS SAS (legal@networkradius.com)
 */
RCSIDH(lib_bio_packet_h, "$Id$")

#include <freeradius-devel/util/packet.h>
#include <freeradius-devel/bio/base.h>

// @todo - _CONST

typedef struct fr_bio_packet_s fr_bio_packet_t;

/** Read a packet and pairs from the network
 *
 * @param bio		the packet-based bio
 * @param request_ctx_p	the request context associated with the response
 * @param packet_p	the response packet.  Contains raw protocol data (IDs, counts, etc.)
 * @param out_ctx	talloc context for the list
 * @param out		the decoded pairs from the packet
 * @return
 *	- <0 on error
 *	- 0 for success (*packet_p may still be NULL tho)
 */
typedef int (*fr_bio_packet_read_t)(fr_bio_packet_t *bio, void **request_ctx_p, fr_packet_t **packet_p, TALLOC_CTX *out_ctx, fr_pair_list_t *out);

/** Write a packet and pairs from the network
 *
 * @param bio		the packet-based bio
 * @param request_ctx	the request context
 * @param packet	the request packet.  Contains raw protocol data (IDs, counts, etc.)
 * @param list		the pairs to encode in the packet
 * @return
 *	- <0 on error (EOF, fail, etc,)
 *	- 0 for success
 */
typedef int (*fr_bio_packet_write_t)(fr_bio_packet_t *bio, void *request_ctx, fr_packet_t *packet, fr_pair_list_t *list);

/** Release an outgoing packet.
 *
 * @param bio		the packet-based bio
 * @param packet	the output packet descriptor.  Contains raw protocol data (IDs, counts, etc.)
 * @return
 *	- <0 on error
 *	- 0 for success
 */
typedef int (*fr_bio_packet_release_t)(fr_bio_packet_t *bio, fr_packet_t *packet);

struct fr_bio_packet_s {
	void			*uctx;		//!< user ctx, caller can manually set it.

	fr_bio_packet_read_t	read;		//!< read from the underlying bio
	fr_bio_packet_write_t	write;		//!< write to the underlying bio

	fr_bio_t		*bio;		//!< underlying bio for IO
};


/** Read a packet from a packet BIO
 *
 *  Note that the bio MAY return fr_bio_error(IO_WOULD_BLOCK), which is not a fatal error.  The caller has to
 *  check for that case, and handle blocking errors.  Typically by pushing the packet to a queue, and trying
 *  it again later.
 *
 * @param bio		the packet-based bio
 * @param[out] request_ctx_p	the larger context for the original request packet
 * @param[out] packet_p	Where the allocated #fr_packet_t will be stored
 * @param[out] out_ctx	for the output pairs
 * @param[out] out		decoded output pairs
 * @return
 *	- <0 on error.  This is fr_bio_error(FOO)
 *	- 0 for success
 */
static inline CC_HINT(nonnull) int fr_bio_packet_read(fr_bio_packet_t *bio, void **request_ctx_p, fr_packet_t **packet_p, TALLOC_CTX *out_ctx, fr_pair_list_t *out)
{
	return bio->read(bio, request_ctx_p, packet_p, out_ctx, out);
}

/** Write a packet to a packet BIO
 *
 *  Note that the bio MAY return fr_bio_error(IO_WOULD_BLOCK), which is not a fatal error.  The caller has to
 *  check for that case, and handle blocking errors.  Typically by pushing the packet to a queue, and trying
 *  it again later.
 *
 * @param bio		the packet-based bio
 * @param request_ctx	the larger context for the packet
 * @param packet	the output packet descriptor.  Contains raw protocol data (IDs, counts, etc.)
 * @param list		of pairs to write
 * @return
 *	- <0 on error.  This is fr_bio_error(FOO)
 *	- 0 for success
 */
static inline CC_HINT(nonnull) int fr_bio_packet_write(fr_bio_packet_t *bio, void *request_ctx, fr_packet_t *packet, fr_pair_list_t *list)
{
	return bio->write(bio, request_ctx, packet, list);
}
