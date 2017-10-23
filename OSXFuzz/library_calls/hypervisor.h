
#ifndef HYPERVISOR_H
#define HYPERVISOR_H

#include <Hypervisor/hv.h>

// This is OSX (x86) only.
// http://www.pagetable.com/?p=764

// Create a vm instance for current task.

void BH_hv_vm_create()
{
    int ret = -1;
    
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());

    hv_vm_options_t flags = HV_VM_DEFAULT; 

    logger("//[Library Call]: BH_hv_vm_create");
    logger("hv_vm_create(%d);",flags); 
    ret = hv_vm_create(flags);
    return_logger("hv_vm_create", ret);
}

// Destroy vm instance for current task.
void BH_hv_vm_destroy()
{
    int ret = -1;
	logger("//[Library Call]: BH_hv_vm_destroy");
	logger("hv_vm_destroy();");
	ret = hv_vm_destroy();
    return_logger("hv_vm_destroy", ret);
}

#endif

