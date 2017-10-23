//
//  IOKitFuzz.c
//  OSXFuzz
//

#ifndef iokitfuzz_calls_h
#define iokitfuzz_calls_h

#include "object_database.h"
#include "logger.h"
#include "helpers.h"
#include "gen_globals.h"

extern kern_return_t io_connect_get_notification_semaphore
(
	mach_port_t connection,
	natural_t notification_type,
	semaphore_t *semaphore
 );

// maybe use iokit/iokitmig.h directly?

// Read this data from frida log output.
typedef struct userclient_method {
    int selector;
    char *method_name;
    int scalar_input_cnt;
    int structure_input_cnt;        // This is used as the size.
    char *struct_type_hints[32];    // Used to make sure we generate structs of the right type.
    int scalar_output_cnt;
    int struct_output_cnt;
} userclient_method;



// Obtain the masterport used to communicate with IOKit
void BH_IOMasterPort()
{
    BH_Object h_BH_IOMasterPort = {0};
    mach_port_t masterPort = 0;
    int ret = -1;
    
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    logger("mach_port_t h_BH_IOMasterPort_in%s = 0;",vid);
    logger("mach_port_t h_BH_IOMasterPort_out%s = 0;",vid);
    
    // Lookup a mach_port_t to use
    h_BH_IOMasterPort = get_random_object_by_name("mach_port_t");
    logger("h_BH_IOMasterPort_in%s = get_specific_object(%d);",vid,h_BH_IOMasterPort.index);
    
    logger("//[Library Call]: BH_IOMasterPort");
    if (rand() % 2 == 0)
    {
        logger("IOMasterPort(MACH_PORT_NULL,&h_BH_IOMasterPort_out%s);",vid);
        ret = IOMasterPort(0, &masterPort);
    }
    else
    {
        logger("IOMasterPort(h_BH_IOMasterPort_in%s,&h_BH_IOMasterPort_out%s);",vid,vid);
        ret = IOMasterPort((mach_port_t)h_BH_IOMasterPort.value, &masterPort);
    }
    
    return_logger("IOMasterPort",ret);
    
    if (ret == KERN_SUCCESS)
    {
        // Put the master port return object.
        logger("put_random_object(h_BH_IOMasterPort_out%s,\"mach_port_t\");",vid);
        put_random_object((OBJECT)masterPort, "mach_port_t");
    }
}

#endif
