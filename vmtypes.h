/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/

#ifndef VMTYPES_H
#define VMTYPES_H

#include <cstddef>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "ConsumerProducer.h"

typedef struct {
    float I;
    float Q;
} TYPECPX ;

typedef struct  {
    int16_t i;
    int16_t q;
} CS16 ;


typedef struct {
    uint32_t blkid ;
    CS16   *data ;
    uint8_t channels ;
    uint32_t length ;
    uint32_t buff_length ;
    uint64_t timestamp ;
    int64_t center_freq ;
    uint64_t tune_id ;
    uint64_t samplerate ;
    float int_to_float_factor ;
    float int_to_float_substract ;
} CS16Block ;


typedef struct {
    uint32_t blkid ;
    bool floatdata ;

    void *data ;
    int  channels ;

    uint32_t length ;
    uint32_t buff_length ;
    uint64_t timestamp ;
    int64_t center_freq ;
    uint64_t tune_id ;
    uint64_t samplerate ;
    float int_to_float_factor ;
    float int_to_float_substract ;

    bool gps_fix ;
    double latitude_N ;
    double longitude_E;
    double altitude ;

    char *json_attribute ;
} CpxBlock ;

typedef struct {
    uint32_t blkid ;
    float *data ;
    uint32_t length ;
    uint32_t buff_length ;
    uint64_t timestamp ;
    double samplerate ;
} FloatBlock ;

typedef  ConsumerProducerQueue<CpxBlock *> CpxSampleQueue ;


#define MALLOC_V4SF_ALIGNMENT 64 // with a 64-byte alignment, we are even aligned on L2 cache lines...
inline TYPECPX* cpxalloc( int nb_samples ) {
#ifdef _WIN32
    return( (TYPECPX*)_aligned_malloc(nb_samples*sizeof(TYPECPX), MALLOC_V4SF_ALIGNMENT ));
#else
    return( (TYPECPX*)aligned_alloc(MALLOC_V4SF_ALIGNMENT,nb_samples*sizeof(TYPECPX)));
#endif
}

inline void cpxfree(void *p) {
#ifdef _WIN32
    _aligned_free(p);
#else
    free(p) ;
#endif
}

#endif // VMTYPES_H
