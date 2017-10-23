//
// bughunt_syscalls.h
// OSXFuzz
//
// Created by alex on 07/09/2016.
//

#ifndef bughunt_syscalls_h
#define bughunt_syscalls_h

/// #include <mach/mach_traps.h>
// #include <sys/syscall.h>

#include <poll.h>
#include <sys/ev.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/sem.h>
#include <sys/semaphore.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <ucontext.h>

// Global array for syscall blacklist
int bsdcall_blacklist[32];

void bh_aue_unmount()
{
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    
    user_addr_t path = get_fuzzed_char16();
    logger("user_addr_t path%s = %d;", vid, path);
    
    int flags = get_fuzzed_int32();
    logger("int flags%s = %d;", vid, flags);
    
    logger("syscall(SYS_unmount, path%s, flags%s);", vid, vid);
    int ret = syscall(SYS_unmount, path, flags);
    
    return_logger("SYS_unmount", ret);
}

void bh_aue_getfh()
{
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    
    char * fname = get_fuzzed_char16();
    logger("char * fname%s = %d;", vid, fname);
    
    struct fhandle fhandlestruct;
    logger("struct fhandle fhandlestruct%s;", vid);
    
    fhandlestruct.fh_len = get_fuzzed_int32();
    logger("fhandlestruct%s.fh_len = %d;", vid, fhandlestruct.fh_len);
    
    fhandlestruct.fh_data[0] = get_fuzzed_char16();
    logger("fhandlestruct.fh_data[0] = %d;", vid, fhandlestruct.fh_data[0]);
    
    logger("syscall(SYS_getfh, fname%s, &fhandlestruct%s);", vid, vid);
    int ret = syscall(SYS_getfh, fname, &fhandlestruct);
    
    return_logger("SYS_getfh", ret);
    
}

void bh_aue_copyfile()
{
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    
    char * from = get_fuzzed_char16();
    logger("char * from%s = %d;", vid, from);
    
    char * to = get_fuzzed_char16();
    logger("char * to%s = %d;", vid, to);
    
    int mode = get_fuzzed_int32();
    logger("int mode%s = %d;", vid, mode);
    
    int flags = get_fuzzed_int32();
    logger("int flags%s = %d;", vid, flags);
    
    logger("syscall(SYS_copyfile, &from%s, &to%s, mode%s, flags%s);", vid, vid, vid, vid);
    int ret = syscall(SYS_copyfile, &from, &to, mode, flags);
    
    return_logger("SYS_copyfile", ret);
}

//end syscalls

void (*SYSCALLS[]) () = {
    bh_aue_getfh,
    bh_aue_copyfile,
    bh_aue_unmount
    // INSERT MORE SYSCALLS HERE.
};


void (*random_SYSCALL()) (void)
{
    unsigned int n = sizeof (SYSCALLS) / sizeof (SYSCALLS[0]);
    return SYSCALLS[rand() % n];
}

void mach_kernelrpc_mach_vm_allocate_trap()
{
    kern_return_t ret = -1;
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    
    mach_port_name_t target = mach_task_self();
    mach_vm_address_t addr = get_fuzzed_int8();
    mach_vm_size_t size = get_fuzzed_int8();
    int flags = 0;
    
    logger("mach_port_name_t target%s = mach_task_self();",vid);
    logger("mach_vm_address_t addr%s = %d;",vid,addr);
    logger("mach_vm_size_t size%s = %d;",vid,size);
    logger("int flags%s = 0;",vid);
    
    logger("_kernelrpc_mach_vm_allocate_trap(target%s,&addr%s,size%s,flags%s);",vid,vid,vid,vid);
    ret = _kernelrpc_mach_vm_allocate_trap(target,&addr,size,flags);
    return_logger("_kernelrpc_mach_vm_allocate_trap", ret);
}

void mach_kernelrpc_mach_vm_purgable_control_trap() {
    
    kern_return_t ret = -1;
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    
    mach_port_name_t target = mach_task_self();
    mach_vm_offset_t address = get_fuzzed_int8();
    vm_purgable_t control = rand() % 3; 
    int state = get_fuzzed_int8();
    
    logger("mach_port_name_t target%s = mach_task_self();",vid);
    logger("mach_vm_offset_t address%s = %d;",vid,address);
    logger("vm_purgable_t control%s = %d;",vid,control);
    logger("int state%s = %d;",vid,state);
    
    logger("_kernelrpc_mach_vm_purgable_control_trap(target%s,address%s,control%s,&state%s);",vid,vid,vid,vid);
    ret = _kernelrpc_mach_vm_purgable_control_trap(target,address,control,&state);
    return_logger("_kernelrpc_mach_vm_purgable_control_trap", ret);
}

void (*MACH_TRAPS[]) () = {
    mach_kernelrpc_mach_vm_allocate_trap,
    mach_kernelrpc_mach_vm_purgable_control_trap
    // INSERT MORE MACH TRAPS HERE!
};

void (*random_MACH_TRAP()) (void)
{
    unsigned int n = sizeof (MACH_TRAPS) / sizeof (MACH_TRAPS[0]);
    return MACH_TRAPS[rand() % n];
}


#endif /* bughunt_syscalls_h */
