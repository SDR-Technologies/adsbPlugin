/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/


#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H
#include <thread>
#include "vmplugins.h"
#include "vmtypes.h"
#include "ConsumerProducer.h"
#include "librtlsdr/rtl-sdr.h"

typedef struct {
    unsigned char *buf ;
    uint32_t len ;

} RTLSDRBlock ;

typedef ConsumerProducerQueue<RTLSDRBlock *> TrtlQueue ;

typedef struct {
    bool stop ;
    TMBox *box ;
    TrtlQueue *queue ;
    rtlsdr_dev_t *rtlsdr_device ;
} ADSBThreadParams  ;

class ADSBPlugin : public IJSClass
{
public:

    ADSBPlugin() = default;

    const char* Name() ;
    const char* JSTypeName() ;
    ADSBPlugin* allocNewInstance(ISDRVirtualMachineEnv *host) ;
    void deleteInstance( IJSClass *instance ) ;
    void declareMethods( ISDRVirtualMachineEnv *host ) ;

    void init();

    bool isRunning();
    void stop();

    bool start( char *rtlsdr_serial_number );

private:
     TMBox *box ;
     std::thread *adsb ;

     ADSBThreadParams params ;
};

#endif // EXAMPLEPLUGIN_H
