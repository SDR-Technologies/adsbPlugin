/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/

#include <math.h>
#include <string.h>
#include "vmtoolbox.h"
#include "adsbplugin.h"
#include "adsbframer.h"
#include "modesdecoder.h"
#include "json.hpp"


using json = nlohmann::json;

const char JSTypeNameStr[] = "ADSB" ;
const char BOXNAME[] = "ADSBMESSAGES" ;

const char* ADSBPlugin::Name() {
    return( (const char*) JSTypeNameStr );
}

const char* ADSBPlugin::JSTypeName() {
    return( (const char*) JSTypeNameStr );
}

ADSBPlugin* ADSBPlugin::allocNewInstance(ISDRVirtualMachineEnv *host) {
    (void)host ;
    ADSBPlugin* result = new ADSBPlugin();
    result->init();
    return( result );
}

// This is called when the allocated instance is no longer required
void ADSBPlugin::deleteInstance(IJSClass *instance) {
    delete instance ;
}

void ADSBPlugin::init() {
    adsb = nullptr ;
    box = vmtools->getMBox( (char *)BOXNAME ) ;
}

bool ADSBPlugin::isRunning() {
    return( adsb != nullptr );
}

void ADSBPlugin::stop() {
    if( adsb != nullptr ) {
        params.stop = true ;
        rtlsdr_cancel_async( params.rtlsdr_device ) ;
        if( adsb->joinable() )
            adsb->join();
        adsb = nullptr ;
    }
}

void adsb_thread( ADSBThreadParams *params ) ;
bool ADSBPlugin::start(char *rtlsdr_serial_number) {
    rtlsdr_dev_t *device ;
    int dev_index = 0 ;

    if( adsb != nullptr) {
        return(false);
    }

    if( rtlsdr_serial_number != nullptr ) {
        dev_index = rtlsdr_get_index_by_serial( (const char *)rtlsdr_serial_number );
        if( dev_index < 0 ) {
            fprintf( stderr, "Could not find RTLSDR device with serial [%s]\n", rtlsdr_serial_number );
            fflush(stderr);
            return(false);
        }
    }
    int rc = rtlsdr_open( &device, dev_index );
    if( rc < 0 ) {
        fprintf( stderr, "Could not open RTLSDR device\n");
        fflush(stderr);
        return(false);
    }
    params.rtlsdr_device = device ;
    params.stop = false ;
    params.box  = box ;
    params.queue = new TrtlQueue(10);
    adsb = new std::thread( adsb_thread, &params );
    return( true );
}


int isrunning_call( void *stack ) ;
int stop_call( void* stack ) ;
int start_call( void* stack ) ;

void ADSBPlugin::declareMethods( ISDRVirtualMachineEnv *host ) {
    host->addMethod( (const char *)"isRunning", isrunning_call, false);
    host->addMethod( (const char *)"start", start_call, true);
    host->addMethod( (const char *)"stop", stop_call, false);
}

int isrunning_call( void *stack ) {
    ADSBPlugin* p = (ADSBPlugin *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    vmtools->pushBool( stack, p->isRunning() );
    return(1);
}

int stop_call( void* stack ) {
    ADSBPlugin* p = (ADSBPlugin *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    p->stop();
    vmtools->pushBool( stack, true );
    return(1);
}


int start_call( void* stack ) {
    ADSBPlugin* p = (ADSBPlugin *)vmtools->getObject(stack);
    if( p == nullptr ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    if( p->isRunning() ) {
        vmtools->pushBool( stack, false );
        return(1);
    }
    const char *rtlsdr_serial_no = nullptr ;
    int n = vmtools->getStackSize( stack );
    if( n > 0 ) {
        rtlsdr_serial_no = vmtools->getString( stack, 0);
    }
    bool res = p->start( (char *)rtlsdr_serial_no );
    vmtools->pushBool( stack, res );
    return(1);
}

void rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx) {
    TrtlQueue *q = (TrtlQueue *)ctx ;
    if( q->isFull())
        return ;
    RTLSDRBlock* block = (RTLSDRBlock *)malloc( sizeof(RTLSDRBlock));
    if( block == nullptr )
        return ;
    block->len = len ;
    block->buf = (unsigned char *)malloc( len *sizeof(unsigned char));
    if( block->buf == nullptr )
        return ;
    memcpy( block->buf, buf, len *sizeof(unsigned char));
    q->add( block );
}

void rtlsdr_thread( ADSBThreadParams *params ) {
    rtlsdr_dev_t *rtlsdr_device = params->rtlsdr_device ;
    rtlsdr_reset_buffer(rtlsdr_device);
    rtlsdr_read_async(rtlsdr_device, rtlsdr_callback, (void *)params->queue, 0, 65536);
    // push a 0-len queue to unlock main thread
    RTLSDRBlock* block = (RTLSDRBlock *)malloc( sizeof(RTLSDRBlock));
    block->len = 0 ;
    block->buf = nullptr ;
    params->queue->add(block);
}

void postMessage( TMBox *box, ADSBUpdate *msg ) {
    uint8_t  update_type = msg->update_type ;
    uint32_t icao = msg->addr ;
    std::string jsonsource ;

    json mail ;
    mail["icao"] = icao ;
    mail["update_type"] = update_type ;
    if( msg->ac != nullptr ) {
        mail["flight"] = std::string( msg->ac->flight );
        mail["altitude"] = msg->ac->altitude ;
        mail["speed"] = msg->ac->speed ;
        mail["vert_rate_sign"] = msg->ac->vert_rate_sign ;
        mail["vert_rate"] = msg->ac->vert_rate ;
        mail["position_valid"] = msg->ac->position_valid ;
        mail["lat"] = msg->ac->lat ;
        mail["lon"] = msg->ac->lon ;
    }

    if( update_type == ADSBUPDATE_TYPE_AIRCRAFTLOST ) {
        mail["update_type_msg"] = "ADSBUPDATE_TYPE_AIRCRAFTLOST" ;
    }
    if( update_type == ADSBUPDATE_TYPE_AIRCRAFTMOVE ) {
        mail["update_type_msg"] = "ADSBUPDATE_TYPE_AIRCRAFTMOVE" ;
    }
    if( update_type == ADSBUPDATE_TYPE_NEWAIRCRAFT ) {
        mail["update_type_msg"] = "ADSBUPDATE_TYPE_NEWAIRCRAFT" ;
    }
    jsonsource = mail.dump();
    free(msg);

   // fprintf( stdout, "%s\n", jsonsource.c_str() ); fflush(stdout);

    TMBoxMessage *boxmessage = (TMBoxMessage *)malloc( sizeof(TMBoxMessage));
    boxmessage->fromTID = 0 ;
    boxmessage->memsize = jsonsource.length() + 1 ;
    boxmessage->payload = (char *)malloc( boxmessage->memsize);
    sprintf( boxmessage->payload, "%s", jsonsource.c_str());
    box->postMessage( boxmessage );
}

void adsb_thread( ADSBThreadParams *params ) {
    int rc ;
    RTLSDRBlock* block ;
    rtlsdr_dev_t *rtlsdr_device = params->rtlsdr_device ;
    TMBox *box = params->box ;
    TrtlQueue *queue = params->queue ;

    rc = rtlsdr_reset_buffer(rtlsdr_device);
    if (rc < 0) {
        fprintf(stderr, "Error: Failed to reset buffers.\n");
        fflush(stderr);
        return ;
    }
    rtlsdr_set_tuner_gain_mode( rtlsdr_device, 0);
    rc = rtlsdr_set_center_freq( rtlsdr_device, 1090e6 );
    if (rc != 0) {
        fprintf(stderr, "Error: Failed to set center frequency.\n");
        fflush(stderr);
        return ;
    }
    rc = rtlsdr_set_sample_rate( rtlsdr_device, 2000000 );
    if( rc != 0) {
        fprintf(stderr, "Error: Failed to set sampling rate.\n");
        fflush(stderr);
        return ;
    }

    new std::thread( rtlsdr_thread, params );
    ADSBFramer framer ;
    ModeSDecoder modeS ;
    while( !params->stop ) {
        queue->consume(block);
        if( block->len == 0 ) {
            free(block);
            continue ;
        }
        // push radio block
        framer.newDatas( (char *)block->buf, block->len );
        free( block->buf );
        free(block);
        //
        while( framer.hasFrames() ) {
            ADSBRawMSG *raw = framer.pop() ;
            if( raw == nullptr ) continue ;
            modeS.pushRawMSG( raw );
            while( modeS.hasMSG() ) {
                ADSBUpdate *modeSM = modeS.popMSG();
                postMessage( box, modeSM );
            }
        }
    }

}
