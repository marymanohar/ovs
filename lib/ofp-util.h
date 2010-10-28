/*
 * Copyright (c) 2008, 2009, 2010 Nicira Networks.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OFP_UTIL_H
#define OFP_UTIL_H 1

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "flow.h"

struct ofpbuf;
struct ofp_action_header;

/* OpenFlow protocol utility functions. */
void *make_openflow(size_t openflow_len, uint8_t type, struct ofpbuf **);
void *make_nxmsg(size_t openflow_len, uint32_t subtype, struct ofpbuf **);
void *make_openflow_xid(size_t openflow_len, uint8_t type,
                        uint32_t xid, struct ofpbuf **);
void *make_nxmsg_xid(size_t openflow_len, uint32_t subtype, uint32_t xid,
                     struct ofpbuf **);
void *put_openflow(size_t openflow_len, uint8_t type, struct ofpbuf *);
void *put_openflow_xid(size_t openflow_len, uint8_t type, uint32_t xid,
                       struct ofpbuf *);
void update_openflow_length(struct ofpbuf *);
struct ofpbuf *make_flow_mod(uint16_t command, const struct flow *,
                             size_t actions_len);
struct ofpbuf *make_add_flow(const struct flow *, uint32_t buffer_id,
                             uint16_t max_idle, size_t actions_len);
struct ofpbuf *make_del_flow(const struct flow *);
struct ofpbuf *make_add_simple_flow(const struct flow *,
                                    uint32_t buffer_id, uint16_t out_port,
                                    uint16_t max_idle);
struct ofpbuf *make_packet_in(uint32_t buffer_id, uint16_t in_port,
                              uint8_t reason,
                              const struct ofpbuf *payload, int max_send_len);
struct ofpbuf *make_packet_out(const struct ofpbuf *packet, uint32_t buffer_id,
                               uint16_t in_port,
                               const struct ofp_action_header *,
                               size_t n_actions);
struct ofpbuf *make_buffered_packet_out(uint32_t buffer_id,
                                        uint16_t in_port, uint16_t out_port);
struct ofpbuf *make_unbuffered_packet_out(const struct ofpbuf *packet,
                                          uint16_t in_port, uint16_t out_port);
struct ofpbuf *make_echo_request(void);
struct ofpbuf *make_echo_reply(const struct ofp_header *rq);
int check_ofp_message(const struct ofp_header *, uint8_t type, size_t size);
int check_ofp_message_array(const struct ofp_header *, uint8_t type,
                            size_t size, size_t array_elt_size,
                            size_t *n_array_elts);

struct flow_stats_iterator {
    const uint8_t *pos, *end;
};
const struct ofp_flow_stats *flow_stats_first(struct flow_stats_iterator *,
                                              const struct ofp_stats_reply *);
const struct ofp_flow_stats *flow_stats_next(struct flow_stats_iterator *);

struct actions_iterator {
    const union ofp_action *pos, *end;
};
const union ofp_action *actions_first(struct actions_iterator *,
                                      const union ofp_action *,
                                      size_t n_actions);
const union ofp_action *actions_next(struct actions_iterator *);
int validate_actions(const union ofp_action *, size_t n_actions,
                     int max_ports);
bool action_outputs_to_port(const union ofp_action *, uint16_t port);

void normalize_match(struct ofp_match *);
char *ofp_match_to_literal_string(const struct ofp_match *match);

int ofputil_pull_actions(struct ofpbuf *, unsigned int actions_len,
                         union ofp_action **, size_t *);

/* OpenFlow vendors.
 *
 * These functions map OpenFlow 32-bit vendor IDs (as used in struct
 * ofp_vendor_header) into 4-bit values to embed in an "int".  The 4-bit values
 * are only used internally in Open vSwitch and never appear on the wire, so
 * particular codes used are not important.
 */

/* Vendor error numbers currently used in Open vSwitch. */
#define OFPUTIL_VENDORS                                     \
    /*             vendor name              vendor value */ \
    OFPUTIL_VENDOR(OFPUTIL_VENDOR_OPENFLOW, 0x00000000)     \
    OFPUTIL_VENDOR(OFPUTIL_VENDOR_NICIRA,   NX_VENDOR_ID)

/* OFPUTIL_VENDOR_* definitions. */
enum ofputil_vendor_codes {
#define OFPUTIL_VENDOR(NAME, VENDOR_ID) NAME,
    OFPUTIL_VENDORS
    OFPUTIL_N_VENDORS
#undef OFPUTIL_VENDOR
};

/* Error codes.
 *
 * We embed system errno values and OpenFlow standard and vendor extension
 * error codes into a single 31-bit space using the following encoding.
 * (Bit 31 is unused and assumed 0 to avoid negative "int" values.)
 *
 *   30                                                   0
 *  +------------------------------------------------------+
 *  |                           0                          |  success
 *  +------------------------------------------------------+
 *
 *   30 29                                                0
 *  +--+---------------------------------------------------+
 *  | 0|                    errno value                    |  errno value
 *  +--+---------------------------------------------------+
 *
 *   30 29   26 25            16 15                       0
 *  +--+-------+----------------+--------------------------+
 *  | 1|   0   |      type      |           code           |  standard OpenFlow
 *  +--+-------+----------------+--------------------------+  error
 *
 *   30 29   26 25            16 15                       0
 *  +--+-------+----------------+--------------------------+  Nicira
 *  | 1| vendor|      type      |           code           |  NXET_VENDOR
 *  +--+-------+----------------+--------------------------+  error extension
 *
 * C and POSIX say that errno values are positive.  We assume that they are
 * less than 2**29.  They are actually less than 65536 on at least Linux,
 * FreeBSD, OpenBSD, and Windows.
 *
 * The 'vendor' field holds one of the OFPUTIL_VENDOR_* codes defined above.
 * It must be nonzero.
 *
 * Negative values are not defined.
 */

/* Currently 4 bits are allocated to the "vendor" field.  Make sure that all
 * the vendor codes can fit. */
BUILD_ASSERT_DECL(OFPUTIL_N_VENDORS <= 16);

/* These are macro versions of the functions defined below.  The macro versions
 * are intended for use in contexts where function calls are not allowed,
 * e.g. static initializers and case labels. */
#define OFP_MKERR(TYPE, CODE) ((1 << 30) | ((TYPE) << 16) | (CODE))
#define OFP_MKERR_VENDOR(VENDOR, TYPE, CODE) \
        ((1 << 30) | ((VENDOR) << 26) | ((TYPE) << 16) | (CODE))
#define OFP_MKERR_NICIRA(TYPE, CODE) \
        OFP_MKERR_VENDOR(OFPUTIL_VENDOR_NICIRA, TYPE, CODE)

/* Returns the standard OpenFlow error with the specified 'type' and 'code' as
 * an integer. */
static inline int
ofp_mkerr(uint16_t type, uint16_t code)
{
    return OFP_MKERR(type, code);
}

/* Returns the OpenFlow vendor error with the specified 'vendor', 'type', and
 * 'code' as an integer.  'vendor' must be an OFPUTIL_VENDOR_* constant. */
static inline int
ofp_mkerr_vendor(uint8_t vendor, uint16_t type, uint16_t code)
{
    assert(vendor < OFPUTIL_N_VENDORS);
    return OFP_MKERR_VENDOR(vendor, type, code);
}

/* Returns the OpenFlow vendor error with Nicira as vendor, with the specific
 * 'type' and 'code', as an integer. */
static inline int
ofp_mkerr_nicira(uint16_t type, uint16_t code)
{
    return OFP_MKERR_NICIRA(type, code);
}

/* Returns true if 'error' encodes an OpenFlow standard or vendor extension
 * error codes as documented above. */
static inline bool
is_ofp_error(int error)
{
    return (error & (1 << 30)) != 0;
}

/* Returns true if 'error' appears to be a system errno value. */
static inline bool
is_errno(int error)
{
    return !is_ofp_error(error);
}

/* Returns the "vendor" part of the OpenFlow error code 'error' (which must be
 * in the format explained above).  This is normally one of the
 * OFPUTIL_VENDOR_* constants.  Returns OFPUTIL_VENDOR_OPENFLOW (0) for a
 * standard OpenFlow error. */
static inline uint8_t
get_ofp_err_vendor(int error)
{
    return (error >> 26) & 0xf;
}

/* Returns the "type" part of the OpenFlow error code 'error' (which must be in
 * the format explained above). */
static inline uint16_t
get_ofp_err_type(int error)
{
    return (error >> 16) & 0x3ff;
}

/* Returns the "code" part of the OpenFlow error code 'error' (which must be in
 * the format explained above). */
static inline uint16_t
get_ofp_err_code(int error)
{
    return error & 0xffff;
}

struct ofpbuf *make_ofp_error_msg(int error, const struct ofp_header *);

#endif /* ofp-util.h */
