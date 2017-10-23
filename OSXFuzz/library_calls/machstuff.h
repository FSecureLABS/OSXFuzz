
#ifndef machstuff_calls_h
#define machstuff_calls_h

// MIG interface calls
// MACH traps are in bughunt_thread.h
// Basically all the .defs need to be fuzzed as they can create MIG messages.

#include <Availability.h>

// task.h first
// server side impl in osfmk/kern/task.c

// This is normally private so we cannot use it from a header.
extern kern_return_t mach_zone_force_gc(mach_port_t);

void BH_mach_ports_register()
{
    BH_Object h_BH_mach_ports_register_task = {0};
    BH_Object h_BH_mach_ports_register_port = {0};
    mach_msg_type_number_t init_port_setCnt = get_fuzzed_int32();
    kern_return_t ret = -1;
    
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    logger("task_t h_BH_mach_ports_register_task%s = 0;",vid);
    logger("mach_port_t h_BH_mach_ports_register_port%s = 0;",vid);
    logger("mach_msg_type_number_t init_port_setCnt%s = %d;",vid,init_port_setCnt);
    
    h_BH_mach_ports_register_task = get_random_object_by_name("task_t");
    logger("h_BH_mach_ports_register_task%s = get_specific_object(%d);",vid,h_BH_mach_ports_register_task.index);
    
    h_BH_mach_ports_register_port = get_random_object_by_name("mach_port_t");
    logger("h_BH_mach_ports_register_port%s = get_specific_object(%d);",vid,h_BH_mach_ports_register_port.index);
    
    logger("//[Library Call]: BH_mach_ports_register");
    logger("mach_ports_register(get_specific_object(%d),get_specific_object(%d),init_port_setCnt%s);",h_BH_mach_ports_register_task.index,h_BH_mach_ports_register_port.index,vid);
    ret = mach_ports_register((task_t)h_BH_mach_ports_register_task.value, h_BH_mach_ports_register_port.value, init_port_setCnt);
    return_logger("mach_ports_register", ret);
}

#endif
