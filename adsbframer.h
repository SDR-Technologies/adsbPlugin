#ifndef ADSBFRAMER_H
#define ADSBFRAMER_H

/* ADSBFramer : decode ADS-B Binary frames from 2 MSPS raw I/Q from receiver
 * Sylvain AZARIAN - 2013
 * Most of the code comes from opensource
 * rtl_adsb from oscmocom
 *
*/


#include <stdint.h>
#include "ConsumerProducer.h"

#define DEFAULT_ASYNC_BUF_NUMBER	12
#define DEFAULT_BUF_LENGTH		(16 * 16384)
#define AUTO_GAIN			-100

#define MESSAGEGO    253
#define OVERWRITE    254
#define BADSAMPLE    255

#define preamble_len	16
#define long_frame		112
#define short_frame		56

typedef struct {
    unsigned char *msg ;
    int len ;
} ADSBRawMSG ;

typedef ConsumerProducerQueue<ADSBRawMSG *> rawADSBQueue ;

class ADSBFramer
{

public:
    ADSBFramer();
    void newDatas(char *buf, uint32_t blen ) ;
    bool hasFrames();
    ADSBRawMSG *pop();

private:
    uint8_t *buffer;
    uint16_t squares[256];
    int adsb_frame[14];
    int verbose_output ;
    int short_output  ;
    int quality  ;
    int allowed_errors ;
    rawADSBQueue *queue ;

    int magnitute(uint8_t *buf, int len) ;
    void manchester(uint16_t *buf, int len);
    uint16_t single_manchester(uint16_t a, uint16_t b, uint16_t c, uint16_t d);
    void squares_precompute(void) ;
    int abs8(int x) ;
    int preamble(uint16_t *buf, int i);
    void messages(uint16_t *buf, int len);
    void makeFrame(int *frame, int len);
};

#endif // ADSBFRAMER_H
