/****************************************************************
 *                                                              *
 * @copyright  Copyright (c) 2020 SDR-Technologies SAS          *
 * @author     Sylvain AZARIAN - s.azarian@sdr-technologies.fr  *
 * @project    SDR Virtual Machine                              *
 *                                                              *
 * Code propriete exclusive de la société SDR-Technologies SAS  *
 *                                                              *
 ****************************************************************/


#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

/** @brief Interface class that provides plugin's metadata and instantiate exported classes */
struct IPluginFactory {
    /** Get Plugin Name */
    virtual const char* Name() const = 0 ;
    /** Get Plugin Version */
    virtual const char* Version() const = 0;

    virtual size_t NumberOfClasses() const = 0;
    virtual const char* GetClassName(size_t index) const = 0;

    /** Instantiate a class from its name */
    virtual void* Factory(const char* className) const = 0;
};

#endif // PLUGIN_INTERFACE_H
