/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/

#ifndef VMPLUGINS_H
#define VMPLUGINS_H

#include <vector>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "vmsystem.h"


class IJSClass
{
public:
    virtual const char* Name() = 0; // Return the name of the plugin
    virtual const char* JSTypeName() = 0 ; // The name to be used for the new JS type

    virtual ~IJSClass() = default;

    virtual IJSClass* allocNewInstance(ISDRVirtualMachineEnv *host) = 0 ;
    virtual void deleteInstance( IJSClass *instance ) = 0 ;

    virtual void declareMethods( ISDRVirtualMachineEnv *host ) = 0 ;

protected:

};

#endif // VMPLUGINS_H
