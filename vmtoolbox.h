/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/

#ifndef VMTOOLBOX_H
#define VMTOOLBOX_H

#include "vmtypes.h"
#include "vmsystem.h"


typedef struct  {
    int items ;
    unsigned char *ptr ;
} vmByteArray ;


class StackTools {
public:
    virtual void throwException( void *stack, const char *reason = nullptr );

    // SDRVM JS Stack Accessors
    virtual int getStackSize( void *stack );

    virtual double getDouble( void *stack , int stack_pos ) ;
    virtual void pushDouble( void *stack , double x );

    virtual int getInt( void *stack , int stack_pos ) ;
    virtual void pushInt( void *stack , int x );

    virtual bool getBool( void *stack , int stack_pos ) ;
    virtual void pushBool( void *stack , bool x );

    virtual const char* getString( void *stack, int stack_pos );
    virtual void pushString( void *stack, const char *string );

    virtual CpxBlock* getIQData( void *stack, int stack_pos );
    virtual void pushIQData( void *stack, CpxBlock *iq );

    virtual vmByteArray *getByteArray( void *stack, int stack_pos );
    virtual void pushByteArray( void *stack, vmByteArray *data );

    virtual IJSClass* getObject(void *stack);

    virtual CpxSampleQueue* getQueue( char *queueName, bool create_if_nofound );
    virtual FFTWHelper *getFFTWHelper();

    virtual TMBox* getMBox( char *boxname );
} ;

extern StackTools *vmtools ;


#endif // VMTOOLBOX_H
