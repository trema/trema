/*
 * IGMP header definitions
 *
 * Copyright (C) 2013 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef IGMP_H
#define IGMP_H


typedef struct igmp_header {
  uint8_t type;
  uint8_t code;
  uint16_t csum;
  uint32_t group;
} igmp_header_t;


#define IGMP_TYPE_MEMBERSHIP_QUERY     0x11
#define IGMP_TYPE_V1_MEMBERSHIP_REPORT 0x12
#define IGMP_TYPE_V2_MEMBERSHIP_REPORT 0x16
#define IGMP_TYPE_V2_LEAVE_GROUP       0x17
#define IGMP_TYPE_V3_MEMBERSHIP_REPORT 0x22


#endif // IGMP_H


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
