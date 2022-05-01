#ifndef MODESDECODER_H
#define MODESDECODER_H

/* ModeSDecoder : ADSB Mode S decoder class
 * Sylvain AZARIAN - 2013
 * Most of the code comes from opensource
 * Mode1090, a Mode S messages decoder for RTLSDR devices.
 *
*/
// ten90, a Mode S/A/C message decoding library.
// adapted to QT/C++ by  F4GKR Sylvain AZARIAN
//
// Copyright (C) 2012 by Salvatore Sanfilippo <antirez@gmail.com>
// Copyright 2013 John Wiseman <jjwiseman@gmail.com>
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <semaphore.h>

#include "adsbframer.h"

#define MODES_PREAMBLE_US 8       /* microseconds */
#define MODES_LONG_MSG_BITS 112
#define MODES_SHORT_MSG_BITS 56
#define MODES_FULL_LEN (MODES_PREAMBLE_US+MODES_LONG_MSG_BITS)
#define MODES_LONG_MSG_BYTES (112/8)
#define MODES_SHORT_MSG_BYTES (56/8)

#define MODES_ICAO_CACHE_LEN 1024 /* Power of two required. */
#define MODES_ICAO_CACHE_TTL 60   /* Time to live of cached addresses. */
#define MODES_UNIT_FEET 0
#define MODES_UNIT_METERS 1

#define MODES_DEBUG_DEMOD (1<<0)
#define MODES_DEBUG_DEMODERR (1<<1)
#define MODES_DEBUG_BADCRC (1<<2)
#define MODES_DEBUG_GOODCRC (1<<3)
#define MODES_DEBUG_NOPREAMBLE (1<<4)
#define MODES_DEBUG_NET (1<<5)
#define MODES_DEBUG_JS (1<<6)

/* When debug is set to MODES_DEBUG_NOPREAMBLE, the first sample must be
 * at least greater than a given level for us to dump the signal. */
#define MODES_DEBUG_NOPREAMBLE_LEVEL 25

#define MODES_INTERACTIVE_REFRESH_TIME 250      /* Milliseconds */
#define MODES_INTERACTIVE_ROWS 15               /* Rows on screen */
#define MODES_INTERACTIVE_TTL 60                /* TTL before being removed */

/* The struct we use to store information about a decoded message. */
struct modesMessage {
    /* Generic fields */
    unsigned char msg[MODES_LONG_MSG_BYTES]; /* Binary message. */
    int msgbits;                /* Number of bits in message */
    int msgtype;                /* Downlink format # */
    int crcok;                  /* True if CRC was valid */
    uint32_t crc;               /* Message CRC */
    int errorbit;               /* Bit corrected. -1 if no bit corrected. */
    int aa1, aa2, aa3;          /* ICAO Address bytes 1 2 and 3 */
    int phase_corrected;        /* True if phase correction was applied. */

    /* DF 11 */
    int ca;                     /* Responder capabilities. */

    /* DF 17 */
    int metype;                 /* Extended squitter message type. */
    int mesub;                  /* Extended squitter message subtype. */
    int heading_is_valid;
    int heading;
    int aircraft_type;
    int fflag;                  /* 1 = Odd, 0 = Even CPR message. */
    int tflag;                  /* UTC synchronized? */
    int raw_latitude;           /* Non decoded latitude */
    int raw_longitude;          /* Non decoded longitude */
    char flight[9];             /* 8 chars flight number. */
    int ew_dir;                 /* 0 = East, 1 = West. */
    int ew_velocity;            /* E/W velocity. */
    int ns_dir;                 /* 0 = North, 1 = South. */
    int ns_velocity;            /* N/S velocity. */
    int vert_rate_source;       /* Vertical rate source. */
    int vert_rate_sign;         /* Vertical rate sign. */
    int vert_rate;              /* Vertical rate. */
    int velocity;               /* Computed from EW and NS velocity. */

    /* DF4, DF5, DF20, DF21 */
    int fs;                     /* Flight status for DF4,5,20,21 */
    int dr;                     /* Request extraction of downlink request. */
    int um;                     /* Request extraction of downlink request. */
    int identity;               /* 13 bits identity (Squawk). */

    /* Fields used by multiple message types. */
    int altitude, unit;
};


#ifdef _WIN32
/*** mutex / critical section **/
#define P_MUTEX HANDLE
#else
#define P_MUTEX sem_t
#endif


/* Structure used to describe an aircraft in iteractive mode. */
struct aircraft {
    uint32_t addr;      /* ICAO address */
    char hexaddr[7];    /* Printable ICAO address */
    char flight[9];     /* Flight number */
    int altitude;       /* Altitude */
    int speed;          /* Velocity computed from EW and NS components. */
    int vert_rate_sign;         /* Vertical rate sign. */
    int vert_rate;              /* Vertical rate. */
    int track;          /* Angle of flight. */
    uint64_t seen;        /* ms elapsed since epoch at which the last packet was received. */
    long messages;      /* Number of Mode S messages received. */
    /* Encoded latitude and longitude as extracted by odd and even
     * CPR encoded messages. */
    int odd_cprlat;
    int odd_cprlon;
    int even_cprlat;
    int even_cprlon;
    bool position_valid ;
    double lat, lon;    /* Coordinated obtained from CPR encoded data. */
    long long odd_cprtime, even_cprtime;
    struct aircraft *next; /* Next aircraft in our linked list. */
    P_MUTEX mutex ;
};

#define ADSBUPDATE_TYPE_NEWAIRCRAFT (0)
#define ADSBUPDATE_TYPE_AIRCRAFTMOVE (1)
#define ADSBUPDATE_TYPE_AIRCRAFTLOST (2)

typedef struct {
    uint32_t addr;      /* ICAO address */
    uint8_t  update_type ;
    struct aircraft *ac ;
} ADSBUpdate ;

typedef ConsumerProducerQueue<ADSBUpdate *> ADSBUpdateQueue ;

class ModeSDecoder
{
public:

    ModeSDecoder();
    void pushRawMSG( ADSBRawMSG *msg );
    bool hasMSG();
    ADSBUpdate* popMSG();

private:
    ADSBUpdateQueue *queue ;
    int metric;                     /* Use metric units. */
    int aggressive;                 /* Aggressive detection algorithm. */
    int fix_errors;                 /* Single bit error correction if true. */
    uint32_t *icao_cache;           /* Recently seen ICAO addresses cache. */
    int check_crc;                  /* Only display messages with good CRC. */

    /* Interactive mode */
    struct aircraft *aircrafts;
    long long interactive_last_update;  /* Last screen update in milliseconds */

    uint32_t modesChecksum(unsigned char *msg, int bits) ;
    int fixSingleBitErrors(unsigned char *msg, int bits) ;
    int fixTwoBitsErrors(unsigned char *msg, int bits);
    int bruteForceAP(unsigned char *msg, struct modesMessage *mm) ;
    int ICAOAddressWasRecentlySeen(uint32_t addr) ;
    void addRecentlySeenICAOAddr(uint32_t addr) ;
    uint32_t ICAOCacheHashAddress(uint32_t a) ;
    int modesMessageLenByType(int type);
    int decodeAC13Field(unsigned char *msg, int *unit) ;
    int decodeAC12Field(unsigned char *msg, int *unit) ;

    void decodeCPR(struct aircraft *a) ;
    double cprDlonFunction(double lat, int isodd) ;
    int cprNFunction(double lat, int isodd) ;
    int cprNLFunction(double lat) ;
    int cprModFunction(int a, int b) ;

    struct aircraft *processReceivedData(struct modesMessage *mm) ;
    void decodeModesMessage(struct modesMessage *mm, unsigned char *msg, int len_msg ) ;
    struct aircraft *findAircraft(uint32_t addr) ;
    struct aircraft *createNewAircraft(uint32_t addr) ;
    void removeStaleAircrafts(void) ;

    char *getMEDescription(int metype, int mesub) ;
    long long mstime(void) ;
};

#endif // MODESDECODER_H
