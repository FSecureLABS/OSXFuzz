//
//  bughunt.h
//  OSXFuzz
//
//  Created by alex on 18/08/2016.
//
//

#ifndef bughunt_h
#define bughunt_h

#include "logger.h"

#include <sys/socket.h>
#include <netinet/in.h>

#if _M_IX86
typedef unsigned __int32 POINTER;
#elif _M_X64
typedef unsigned __int64 POINTER;
#endif

typedef enum
{
    _BOOL = 1,
    _BOOL_PTR = 2,
    _CHAR8 = 3,
    _CHAR8_PTR = 4,
    _CHAR16 = 5,
    _CHAR16_PTR = 6,
    _INT8 = 7,
    _INT8_PTR = 8,
    _INT16 = 9,
    _INT16_PTR = 10,
    _INT32 = 11,
    _INT32_PTR = 12,
    _INT64 = 13,
    _INT64_PTR = 14,
    _UINT8 = 15,
    _UINT8_PTR = 16,
    _UINT16 = 17,
    _UINT16_PTR = 18,
    _UINT32 = 19,
    _UINT32_PTR = 20,
    _UINT64 = 21,
    _UINT64_PTR = 22,
    _REAL32 = 23,
    _REAL32_PTR = 24,
    _REAL64 = 25,
    _REAL64_PTR = 26,
    _FD = 28,
    _SOCKET = 29, 
    _OBJECTPTR = 30,  // special type for structs
    _VOID_PTR = 27,
    
    NIL = 0
    
} DATATYPE;


/* SYSCALL DEFINITIONS... */

#define SYSCALL_ARGUMENT_N             ((size_t)33)

typedef struct
{
    unsigned int uid;
    DATATYPE argument_datatypes[SYSCALL_ARGUMENT_N];
    DATATYPE return_datatype;
} SYSCALL;

/* TYPEDEFS USED BY SYSCALLS... */

typedef uint8_t bool_t;

typedef char char8_t;
typedef wchar_t char16_t;


typedef float real32_t;
typedef double real64_t;

typedef unsigned long DWORD; 

/* FUNCTION PROTOTYPES... */

bool_t get_fuzzed_bool (void);
char8_t get_fuzzed_char8 (void);
char16_t get_fuzzed_char16 (void);
int8_t get_fuzzed_int8 (void);
int16_t get_fuzzed_int16 (void);
int32_t get_fuzzed_int32 (void);
int64_t get_fuzzed_int64 (void);
uint8_t get_fuzzed_uint8 (void);
uint16_t get_fuzzed_uint16 (void);
uint32_t get_fuzzed_uint32 (void);
uint64_t get_fuzzed_uint64 (void);
real32_t get_fuzzed_real32 (void);
real64_t get_fuzzed_real64 (void);


double random_double_0_to_1 (void);


/* FUNCTION IMPLEMENTATIONS... */

bool_t get_fuzzed_bool (void)
{
    bool_t bool_BH[] = {0, 1};
    bool_t n;
    
    n = bool_BH[rand() % sizeof(bool_BH) / sizeof(bool_BH[0])];
    //logger("//[Get Fuzzed Value] get_fuzzed_bool : n = %hd", n);
    return n;
}

char8_t get_fuzzed_char8 (void)
{
    char8_t char8_BH[] = { ' ', '\t', '\n', '\r', '\\', '/', '@', ':', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '+', '=', ':' };
    char8_t n;
    
    n = char8_BH[rand() % sizeof(char8_BH) / sizeof(char8_BH[0])];
    //logger("//[Get Fuzzed Value] get_fuzzed_char8 : n = %c", n);
    return n;
}

// What the heck is even this data type?
char16_t get_fuzzed_char16 (void)
{
    char16_t char16_BH[] = { ' ', '\t', '\n', '\r', '\\', '/', '@', ':', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '+', '=', ':' };
    char16_t n;
    
    n = char16_BH[rand() % sizeof(char16_BH) / sizeof(char16_BH[0])];
    //logger("//[Get Fuzzed Value] get_fuzzed_char8 : n = %c", n);
    return n;
}

int8_t get_fuzzed_int8 (void)
{
    // Change so it returns 1, 0, -1, more than anything else.
    int8_t n;
    n = 0;
    switch (rand() % 8) {
        case 0:
            n = 1;                   // one
            break;
        case 1:
            n = 0;
            break;
        case 2:
            n = -1;
            break;
        case 3:
            n = 8;
        case 4:
            n = 16;
        case 5:
            n = 32;
        case 6:
            n = 64;
        case 7:
            switch (rand() % 3) {
                case 0:
                    n = 0xff;                // max
                    break;
                case 1:
                    n = 1UL << (rand() & 7);  // 2^n (1 -> 128)
                    break;
                case 2:
                    n = rand() & 0xff;         // 0 -> 0xff
                    break;
                    
            }
            
    }
    
    //logger("//[Get Fuzzed Value] get_fuzzed_int8 : n = %hhd", n);
    return n;
}

int16_t get_fuzzed_int16 (void)
{
    // Change so it returns 1, 0, -1, more than enything else.
    int16_t n;
    n = 0; 
    switch (rand() % 9) {
        case 0:
            switch (rand() % 4) {
                case 0:
                    n = 0x8000 >> (rand() & 7); // 2^n (0x100 -> 0x8000)
                    break;
                case 1:
                    n = rand() & 0xffff;        // 0 -> 0xffff
                    break;
                case 2:
                    n = 0xff00 | (rand() & 0xff); // 0xff00 -> 0xffff
                    break;
                case 3:
                    n = 0xffff;                 // max
                    break;
            }
            break;
        case 1:
            n = 1;
            break;
        case 2:
            n = 0;
            break;
        case 3:
            n = -1;
            break;
        case 4:
            n = 8;
            break;
        case 5:
            n = 16;
            break;
        case 6:
            n = 32;
            break;
        case 7:
            n = 64;
            break;
        case 8:
            n = 128;
            break;
    }
    //logger("//[Get Fuzzed Value] get_fuzzed_int16 : n = %hd", n);
    return n;
}

int32_t get_fuzzed_int32 (void)
{
    // Change so it returns 1, 0, -1, more than enything else.
    int32_t n = 0;
    
    switch (rand() % 10) {
        case 0:
            switch (rand() % 11)
        {
            case 0:
                n = 0x80000000 >> (rand() & 0x1f);    // 2^n (1 -> 0x10000)
                break;
            case 1:
                n = rand();                           // 0 -> RAND_MAX (likely 0x7fffffff)
                break;
            case 2:
                n = (unsigned int)0xff << (4 * (rand() % 7));
                break;
            case 3:
                n = 0xffff0000;
                break;
            case 4:
                n = 0xffffe000;
                break;
            case 5:
                n = 0xffffff00 | (rand() & 0xff);
                break;
            case 6:
                n = 0xffffffff - 0x1000;
                break;
            case 7:
                n = 0x1000;
                break;
            case 8:
                n = 0x1000 * ((rand() % (0xffffffff / 0x1000)) + 1);
                break;
            case 9:
                n = 0xffffffff;                     // max
                break;
            case 10:
                n = 0x7fffffff;
                break;
        }
            break;
        case 1:
            n = 1;
            break;
        case 2:
            n = 0;
            break;
        case 3:
            n = -1;
            break;
        case 4:
            n = 8;
            break;
        case 5:
            n = 16;
            break;
        case 6:
            n = 32;
            break;
        case 7:
            n = 64;
            break;
        case 8:
            n = 128;
            break;
        case 9:
            n = 256;
    }
    //logger("//[Get Fuzzed Value] get_fuzzed_int32 : n = %ld", n);
    return n;
}

int64_t get_fuzzed_int64 (void)
{
    
    // Change so it returns 1, 0, -1, more than enything else.
    
    //int64_t n = (fuzzed_int64_BYTEARRAY.size / sizeof (int64_t));
    //n = (((int64_t*)fuzzed_int64_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_int64 : n = %lld", n);
    //return n;
    
    // Just a test...
    int64_t n = 0;
    
    switch(rand() % 10) {
        case 0:
            n = 0;
            break;
        case 1:
            n = 1;
            break;
        case 2:
            switch (rand() % 1) {
                case 0:
                    n = 0xffffffffffffffff;
                    break;
                case 1:
                    n = rand() & 0xffffffffffffffff;
                    break;
            }
            break;
        case 3:
            n = -1;
            break;
        case 4:
            n = 8;
            break;
        case 5:
            n = 16;
            break;
        case 6:
            n = 32;
            break;
        case 7:
            n = 64;
            break;
        case 8:
            n = 128;
            break;
        case 9:
            n = 256;
    }
    //logger("//[Get Fuzzed Value] get_fuzzed_int64 : n = %lld", n);
    return n;
}

uint8_t get_fuzzed_uint8 (void)
{
    // Change so it returns 1, or 0, more than enything else.
    
    //uint8_t n = (fuzzed_uint8_BYTEARRAY.size / sizeof (uint8_t));
    //n = (((uint8_t*)fuzzed_uint8_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_uint8 : n = %hhu", n);
    //return n;
    
    // Just a test...
    uint8_t n = (uint8_t)get_fuzzed_int8();
    //logger("//[Get Fuzzed Value] get_fuzzed_uint8 : n = %hhu", n);
    return n;
}

uint16_t get_fuzzed_uint16 (void)
{
    // Change so it returns 1, or 0, more than enything else.
    
    //uint16_t n = (fuzzed_uint16_BYTEARRAY.size / sizeof (uint16_t));
    //n = (((uint16_t*)fuzzed_uint16_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_uint16 : n = %hu", n);
    //return n;
    
    // Just a test...
    uint16_t n = get_fuzzed_int16();
    //logger("//[Get Fuzzed Value] get_fuzzed_uint16 : n = %hu", n);
    return n;
}

uint32_t get_fuzzed_uint32 (void)
{
    // Change so it returns 1, or 0, more than enything else.
    
    //uint32_t n = (fuzzed_uint32_BYTEARRAY.size / sizeof (uint32_t));
    //n = (((uint32_t*)fuzzed_uint32_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_uint32 : n = %lu", n);
    //return n;
    
    // Just a test...
    uint32_t n = get_fuzzed_int32();
    //logger("//[Get Fuzzed Value] get_fuzzed_uint32 : n = %lu", n);
    return n;
}

uint64_t get_fuzzed_uint64 (void)
{
    // Change so it returns 1, or 0, more than enything else.
    
    //uint64_t n = (fuzzed_uint64_BYTEARRAY.size / sizeof (uint64_t));
    //n = (((uint64_t*)fuzzed_uint64_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_uint64 : n = %llu", n);
    //return n;
    
    // Just a test...
    uint64_t n = get_fuzzed_int64();
    //logger("//[Get Fuzzed Value] get_fuzzed_uint64 : n = %llu", n);
    return n;
}

real32_t get_fuzzed_real32 (void)
{
    // Fix this and the rest of the real number ones to use the correct format string specifier.
    //real32_t n = (fuzzed_real32_BYTEARRAY.size / sizeof (real32_t));
    //n = (((real32_t*)fuzzed_real32_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_real32 : n = %lf", n);
    //return n;
    
    // Just a test...
    real32_t n = random_double_0_to_1();
    //logger("//[Get Fuzzed Value] get_fuzzed_real32 : n = %lf", n);
    return n;
}

real64_t get_fuzzed_real64 (void)
{
    //real64_t n = (fuzzed_real64_BYTEARRAY.size / sizeof (real64_t));
    //n = (((real64_t*)fuzzed_real64_BYTEARRAY.bytes)[random_DWORD_0_to_N (n)]);
    //logger("//[Get Fuzzed Value] get_fuzzed_real64 : n = %llf", n);
    //return n;
    
    // Just a test...
    real64_t n = random_double_0_to_1();
    //logger("//[Get Fuzzed Value] get_fuzzed_real64 : n = %llf", n);
    return n;
}

// Revisit this and change so it makes sense, currently seems to be used incorrectly when generating arguments for system calls.
//BYTE* get_fuzzed_block_ptr (const DATATYPE* const datatypes, DWORD n) { }

double random_double_0_to_1 (void)
{
    /* RETURNS A PSEUDO-RANDOM double IN THE RANGE [0.0 ... 1.0]... */
    /* CONSIDER REPLACING WITH SOMETHING LIKE A MERSENNE TWISTER..? */
    /*
     return (((((double)rand ()) / ((double)((DWORD)(RAND_MAX + 1))))
     * (((double)rand ()) / ((double)((DWORD)(RAND_MAX + 1))))));
     */
    //return (double) rand() / (double)(RAND_MAX + 1);
    // http://stackoverflow.com/questions/1340729/how-do-you-generate-a-random-double-uniformly-distributed-between-0-and-1-from-c
    return ((double)rand() / (double)RAND_MAX);
}

DWORD random_DWORD_0_to_N (DWORD n)
{
    /* RETURNS A PSEUDO-RANDOM DWORD IN THE RANGE [0 ... n]... */
    return ((DWORD)(random_double_0_to_1 () * n));
}




// Borrowed from netusse by Clement LECIGNE

const int socket_types[] = {
    SOCK_DGRAM,
    SOCK_STREAM,
#ifdef SOCK_PACKET
    SOCK_PACKET,
#endif
    SOCK_SEQPACKET,
    SOCK_RDM,
};

const int socket_protos[] = {
    IPPROTO_IP,
    IPPROTO_ICMP,
    IPPROTO_IGMP,
    IPPROTO_IPIP,
    IPPROTO_TCP,
    IPPROTO_EGP,
    IPPROTO_PUP,
    IPPROTO_UDP,
    IPPROTO_IDP,
#ifdef IPPROTO_DCCP
    IPPROTO_DCCP,
#endif
    IPPROTO_RSVP,
    IPPROTO_GRE,
    IPPROTO_IPV6,
    IPPROTO_ESP,
    IPPROTO_AH,
#ifdef IPPROTO_BEETPH
    IPPROTO_BEETPH,
#endif
    IPPROTO_PIM,
#ifdef IPPROTO_COMP
    IPPROTO_COMP,
#endif
#ifdef IPPROTO_SCTP
    IPPROTO_SCTP,
#endif
#ifdef IPPROTO_UDPLITE
    IPPROTO_UDPLITE,
#endif
    IPPROTO_DIVERT
    
};

const int socket_domains[] = {
#ifdef         AF_UNSPEC
    AF_UNSPEC,
#endif
#ifdef         AF_UNIX
    AF_UNIX,
#endif
#ifdef         AF_LOCAL
    AF_LOCAL,
#endif
#ifdef         AF_INET
    AF_INET,
#endif
#ifdef         AF_AX25
    AF_AX25,
#endif
#ifdef         AF_IPX
    AF_IPX,
#endif
#ifdef AF_APPLETALK
    AF_APPLETALK,
#endif
#ifdef         AF_NETROM
    AF_NETROM,
#endif
#ifdef         AF_BRIDGE
    AF_BRIDGE,
#endif
#ifdef         AF_ATMPVC
    AF_ATMPVC,
#endif
#ifdef        AF_X25
    AF_X25,
#endif
#ifdef        AF_INET6
    AF_INET6,
#endif
#ifdef        AF_ROSE
    AF_ROSE,
#endif
#ifdef        AF_DECnet
    AF_DECnet,
#endif
#ifdef        AF_NETBEUI
    AF_NETBEUI,
#endif
#ifdef        AF_SECURITY
    AF_SECURITY,
#endif
#ifdef        AF_KEY
    AF_KEY,
#endif
#ifdef        AF_NETLINK
    AF_NETLINK,
#endif
#ifdef        AF_ROUTE
    AF_ROUTE,
#endif
#ifdef        AF_PACKET
    AF_PACKET,
#endif
#ifdef        AF_ASH
    AF_ASH,
#endif
#ifdef        AF_ECONET
    AF_ECONET,
#endif
#ifdef        AF_ATMSVC
    AF_ATMSVC,
#endif
#ifdef        AF_RDS
    AF_RDS,
#endif
#ifdef        AF_SNA
    AF_SNA,
#endif
#ifdef        AF_IRDA
    AF_IRDA,
#endif
#ifdef        AF_PPPOX
    AF_PPPOX,
#endif
#ifdef        AF_WANPIPE
    AF_WANPIPE,
#endif
#ifdef        AF_LLC
    AF_LLC,
#endif
#ifdef        AF_CAN
    AF_CAN,
#endif
#ifdef        AF_TIPC
    AF_TIPC,
#endif
#ifdef        AF_BLUETOOTH
    AF_BLUETOOTH,
#endif
#ifdef        AF_IUCV
    AF_IUCV,
#endif
#ifdef        AF_RXRPC
    AF_RXRPC,
#endif
#ifdef        AF_ISDN
    AF_ISDN,
#endif
#ifdef        AF_PHONET
    AF_PHONET
#endif
    36,
    37,
    38,
    39
};

#define _SOCKRAND(name) ((rand() % 4) ? rand() % 255 : name[rand()%(sizeof(name)/sizeof(name[0]))]);

/* return a random valid socket.
 */
int random_socket(void)
{
    int ret = -1;
    int domain = 0;
    int type = 0;
    int protocol = 0;
    do
    {
        domain = _SOCKRAND(socket_domains);
        type   = _SOCKRAND(socket_types);
        protocol  = _SOCKRAND(socket_protos);
    }
    while ((ret = socket(domain, type, protocol)) < 0);
    return ret;
}


#endif /* bughunt_h */
