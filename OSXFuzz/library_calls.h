//
//  library_calls.h
//  OSXFuzz
//
//  Created by alex on 18/08/2016.
//
//

#ifndef library_calls_h
#define library_calls_h

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#include <IOKit/iokitmig.h>
#include <Availability.h>

#include "library_calls/iokit.h"
#include "library_calls/iohidfamily.h"
#include "library_calls/iosurface.h"
#include "library_calls/machstuff.h"
#include "library_calls/hypervisor.h"


// Disable these temporarily
#define IOHID_DISABLE 1
#define HYPERVISOR_DISABLE 1

void (*LIBRARY_CALLS[]) () = {
    
    // IOKit calls first
    #ifndef IOKIT_DISABLE
    
    BH_IOMasterPort,
    
    #endif
    

    
    #ifndef IOHID_DISABLE
    
    BH_IOHIDCreateSharedMemory,
    
    #endif
    
    #ifndef IOSURFACE_DISABLE
    // iosurface
    
    #endif
    
    #ifndef MACH_DISABLE
    
    BH_mach_ports_register,
    
    #if __MAC_OS_X_VERSION_MIN_REQUIRED >= 1012
    
    #endif
    
    #endif
    
    #ifndef HYPERVISOR_DISABLE
    // Hypervisor calls
    BH_hv_vm_create,
    BH_hv_vm_destroy,
    
    #endif
    
};

void (*random_LIBRARY_CALL()) (void)
{
    unsigned int n = sizeof (LIBRARY_CALLS) / sizeof (LIBRARY_CALLS[0]);
    return LIBRARY_CALLS[rand() % n];
}

#endif /* library_calls_h */
