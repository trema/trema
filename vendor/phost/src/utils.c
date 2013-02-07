/*
  Copyright (C) 2009-2013 NEC Corporation

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include "utils.h"

uint16_t calc_checksum(const void *data_, uint32_t length)
{
    const uint16_t *data = data_;
    int i;
    uint32_t sum;

    sum = 0;

    for(i=length; i > 1; i -= 2){
        sum += (uint16_t)*data;
        data++;
    }
    if(i > 0){
        sum += *(uint8_t*)data;
    }

    return ~((sum & 0xffff) + (sum >> 16));
}

char *hexdump(uint8_t *data, uint32_t length, char *out)
{
    if(length == 0){
        return NULL;
    }

    if(out == NULL){
        out = (char*)malloc(sizeof(char)*(length*2+1));
    }

    char *p;
    uint32_t i;

    memset(out, 0, sizeof(char)*(length*2+1));

    p = out;
    for(i=0; i<length; i++){
        sprintf(p, "%02x", *data);
        data++;
        p += 2;
    }

    return out;
}

int strtomac(char *str, uint8_t mac_addr[6])
{
    if(strlen(str) != 17){
        return -1;
    }

    int pos = 0;
    char hex[3];
    memset(hex, 0, sizeof(hex));

    while(pos < 6){
        if(*str == ':' || *str == '\0'){
            strncpy(hex, str-2, 2);
            mac_addr[pos] = (uint8_t)strtoul(hex, NULL, 16);
            pos++;
        }
        if(*str == '\0'){
            break;
        }
        str++;
    }

    if(pos != 6){
        return -1;
    }

    return 0;
}

int ipaddrtoul(char *str, uint32_t *ip_addr)
{
    if((strlen(str) < 7) || (strlen(str) > 15)){
        return -1;
    }

    char *p = str;
    char octet[4];
    uint8_t octets[4];
    int dots = 0;

    memset(octet, '\0', sizeof(octet));
    memset(octets, 0, sizeof(octets));

    while(dots < 4 || *p != '\0'){
        if(*p != '.' && *p != '\0'){
            if(strlen(octet) < 3){
                strncat(octet, p, 1);
            }
            else{
                return -1;
            }
        }
        else{
            if(atoi(octet) > 255 || strlen(octet) == 0){
                return -1;
            }
            octets[dots] = atoi(octet);
            memset(octet, '\0', sizeof(octet));
            dots++;
        }
        if(dots == 4){
            break;
        }
        p++;
    }

    if(dots != 4){
        return -1;
    }

    memset(ip_addr, 0, sizeof(uint32_t));
    *ip_addr = octets[0] << 24 | octets[1] << 16 | octets[2] << 8 | octets[3];

    return 0;
}

char *ultoipaddr(uint32_t ip_addr, char *addr)
{
    if(addr == NULL){
        return NULL;
    }

    memset(addr, '\0', 16);

    sprintf(addr, "%u.%u.%u.%u",
            (ip_addr & 0xff000000) >> 24,
            (ip_addr & 0x00ff0000) >> 16,
            (ip_addr & 0x0000ff00) >> 8,
            (ip_addr & 0x000000ff));

    return addr;
}

uint64_t htonll(uint64_t n)
{
    return htonl(1) == 1 ? n : ((uint64_t) htonl(n) << 32) | htonl(n >> 32);
}

uint64_t ntohll(uint64_t n)
{
    return htonl(1) == 1 ? n : ((uint64_t) ntohl(n) << 32) | ntohl(n >> 32);
}
