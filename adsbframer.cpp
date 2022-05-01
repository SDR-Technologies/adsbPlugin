
// This specific module was adapted from RTL1090
//
//==========================================================================================

#include "adsbframer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FRAMER_DEBUG (0)

ADSBFramer::ADSBFramer()
{
    buffer = NULL ;
    queue = new rawADSBQueue(10);
    squares_precompute();
    verbose_output = 0;
    short_output = 0;
    quality = 10;
    allowed_errors = 5;
}

bool ADSBFramer::hasFrames() {
    return( !queue->isEmpty() );
}

ADSBRawMSG *ADSBFramer::pop() {
    ADSBRawMSG *result ;
    queue->consume(result);
    return( result );
}


void ADSBFramer::newDatas(char *buf, uint32_t blen ) {
    int len;
    buffer = (uint8_t *)buf ;
    len = magnitute(buffer, blen);
    manchester((uint16_t*)buffer, len);
    messages((uint16_t*)buffer, len);
}

void ADSBFramer::manchester(uint16_t *buf, int len)
/* overwrites magnitude buffer with valid bits (BADSAMPLE on errors) */
{
    /* a and b hold old values to verify local manchester */
    uint16_t a=0, b=0;
    uint16_t bit;
    int i, i2, start, errors;
    int maximum_i = len - 1;        // len-1 since we look at i and i+1
    // todo, allow wrap across buffers
    i = 0;
    while (i < maximum_i) {
        /* find preamble */
        for ( ; i < (len - preamble_len); i++) {
            if (!preamble(buf, i)) {
                continue;}
            a = buf[i];
            b = buf[i+1];
            for (i2=0; i2<preamble_len; i2++) {
                buf[i+i2] = MESSAGEGO;}
            i += preamble_len;
            break;
        }
        i2 = start = i;
        errors = 0;
        /* mark bits until encoding breaks */
        for ( ; i < maximum_i; i+=2, i2++) {
            bit = single_manchester(a, b, buf[i], buf[i+1]);
            a = buf[i];
            b = buf[i+1];
            if (bit == BADSAMPLE) {
                errors += 1;
                if (errors > allowed_errors) {
                    buf[i2] = BADSAMPLE;
                    break;
                } else {
                    bit = a > b;
                    /* these don't have to match the bit */
                    a = 0;
                    b = 65535;
                }
            }
            buf[i] = buf[i+1] = OVERWRITE;
            buf[i2] = bit;
        }
    }
}


uint16_t ADSBFramer::single_manchester(uint16_t a, uint16_t b, uint16_t c, uint16_t d)
/* takes 4 consecutive real samples, return 0 or 1, BADSAMPLE on error */
{
    int bit, bit_p;
    bit_p = a > b;
    bit   = c > d;

    if (quality == 0) {
        return bit;}

    if (quality == 5) {
        if ( bit &&  bit_p && b > c) {
            return BADSAMPLE;}
        if (!bit && !bit_p && b < c) {
            return BADSAMPLE;}
        return bit;
    }

    if (quality == 10) {
        if ( bit &&  bit_p && c > b) {
            return 1;}
        if ( bit && !bit_p && d < b) {
            return 1;}
        if (!bit &&  bit_p && d > b) {
            return 0;}
        if (!bit && !bit_p && c < b) {
            return 0;}
        return BADSAMPLE;
    }

    if ( bit &&  bit_p && c > b && d < a) {
        return 1;}
    if ( bit && !bit_p && c > a && d < b) {
        return 1;}
    if (!bit &&  bit_p && c < a && d > b) {
        return 0;}
    if (!bit && !bit_p && c < b && d > a) {
        return 0;}
    return BADSAMPLE;
}

int ADSBFramer::preamble(uint16_t *buf, int i)
/* returns 0/1 for preamble at index i */
{
    int i2;
    uint16_t low  = 0;
    uint16_t high = 65535;
    for (i2=0; i2<preamble_len; i2++) {
        switch (i2) {
            case 0:
            case 2:
            case 7:
            case 9:
                //high = min16(high, buf[i+i2]);
                high = buf[i+i2];
                break;
            default:
                //low  = max16(low,  buf[i+i2]);
                low = buf[i+i2];
                break;
        }
        if (high <= low) {
            return 0;}
    }
    return 1;
}

void ADSBFramer::messages(uint16_t *buf, int len)
{
    int i ;
    int data_i, index, shift, frame_len;
    // todo, allow wrap across buffers
    for (i=0; i<len; i++) {
        if (buf[i] > 1) {
            continue;}
        frame_len = long_frame;
        data_i = 0;
        for (index=0; index<14; index++) {
            adsb_frame[index] = 0;}
        for(; i<len && buf[i]<=1 && data_i<frame_len; i++, data_i++) {
            if (buf[i]) {
                index = data_i / 8;
                shift = 7 - (data_i % 8);
                adsb_frame[index] |= (uint8_t)(1<<shift);
            }
            if (data_i == 7) {
                if (adsb_frame[0] == 0) {
                    break;}
                if (adsb_frame[0] & 0x80) {
                    frame_len = long_frame;}
                else {
                    frame_len = short_frame;}
            }
        }
        if (data_i < (frame_len-1)) {
            continue;}
        makeFrame(adsb_frame, frame_len);

    }
}


void ADSBFramer::makeFrame(int *frame, int len)
{
    int i, df;
    unsigned char *buffer ;

    if ( len < short_frame) { //<=
        return;
    }

    df = (frame[0] >> 3) & 0x1f;
    if (quality == 0 && !(df==11 || df==17 || df==18 || df==19)) {
        return;
    }


    buffer = (unsigned char *)malloc( (len+2) * sizeof( unsigned char));
    memset( buffer, (char)0, len+2);
    for (i=0; i<((len+7)/8); i++) {
        buffer[i] = (char)frame[i];
    }
    ADSBRawMSG* msg = (ADSBRawMSG *)malloc( sizeof(ADSBRawMSG));
    msg->len = len+1 ;
    msg->msg = buffer ;
    queue->add( msg );
}

int ADSBFramer::magnitute(uint8_t *buf, int len)
/* takes i/q, changes buf in place (16 bit), returns new len (16 bit) */
{
    int i;
    uint16_t *m;
    //float snr = 0 ;

    for (i=0; i<len; i+=2) {
        m = (uint16_t*)(&buf[i]);
        *m = squares[buf[i]] + squares[buf[i+1]];
        //snr += (float)*m ;
    }
    //snr /= len ;

    return len/2;
}

void ADSBFramer::squares_precompute(void)
/* equiv to abs(x-128) ^ 2 */
{
    int i, j;
    // todo, check if this LUT is actually any faster
    for (i=0; i<256; i++) {
        j = abs8(i);
        squares[i] = (uint16_t)(j*j);
    }
}

int ADSBFramer::abs8(int x)
/* do not subtract 128 from the raw iq, this handles it */
{
    if (x >= 128) {
        return x - 128;}
    return 128 - x;
}
