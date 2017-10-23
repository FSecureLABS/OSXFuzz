//
//  bughunt_thread.h
//  OSXFuzz
//
//  Created by alex on 18/08/2016.
//
//

#ifndef bughunt_thread_h
#define bughunt_thread_h

#include <sys/syscall.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <mach/clock_types.h>
#include <mach/mach_voucher.h>

#include "library_calls.h"
#include "bughunt.h"
#include "bughunt_syscalls.h"



int bughunt_thread(int syscall_count)
{
    int syscall_idx = 0;

    // The syscall_log_string will hold the string to be logged before the syscall invocation.
    char syscall_log_string[1024];
    memset(syscall_log_string, '\0', sizeof(syscall_log_string));
    
    for (syscall_idx = 0; syscall_idx < syscall_count; syscall_idx += 1)
    {
        
        while (TRUE)
        {
            (*random_LIBRARY_CALL())();
            
            if (rand() % 3 == 0)
            {
                (*random_MACH_TRAP())();
            }
            if (rand() % 4 == 0) {
                logger("// breaking out of library call loop ");
                break;
            }
        }
    (*random_SYSCALL())();
        
    } // end of for loop
    
    
    return 1;
}

#endif /* bughunt_thread_h */
