//
//  object_database.c
//  OSXFuzz
//
//

#ifndef OBJECT_DATABASE_H
#define OBJECT_DATABASE_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <IOKit/IOKitLib.h>
#include <IOSurface/IOSurfaceAPI.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Hypervisor/hv.h>
#include "logger.h"
#include "bughunt.h"

typedef void *OBJECT;

typedef struct {
    OBJECT value;
    int index;
    char *tag;
} BH_Object;

BH_Object get_random_object_by_name(char *creator);
OBJECT put_random_object(OBJECT object,char *ObjectCreator);

#define OBJECTS_N 128
#define OBJECT_CREATOR_SIZE 128

#define FD_N 128 // Updated value, old and original value was 32.
#define FD_CREATOR_SIZE 128

int HANDLES[FD_N];
// The handle creator is subject to removal in the future, don't need it anymore.
char* HANDLE_CREATOR[FD_CREATOR_SIZE]; //When debugging, we need to know what created the handle. This will hold the array in the same order of HANDLES of the handle creator.

// This tells us the first available, i.e. empoty slot in the array of handles.
// This will initially be set to wherever make_HANDLES stops populating. It will increment till the end of the array and then set back to zero to start overwriting from the begging of the array.
unsigned int HANDLES_ARRAY_AVAILABLE_SLOT_INDEX;
// Do we have empy slots in the array of handles.
int HANDLE_ARRAY_FULLY_POPULATED = 0;

unsigned int OBJECT_ARRAY_AVAILABLE_SLOT_INDEX;
int OBJECT_ARRAY_FULLY_POPULATED;

OBJECT OBJECTS[OBJECTS_N];

char* OBJECT_CREATOR[OBJECT_CREATOR_SIZE];

void make_fd(void)
{
    unsigned int fd_idx = 0;
    int temp_fd;
    
    for (fd_idx = 0; fd_idx < FD_N; fd_idx += 1) {
        HANDLES[fd_idx] = 0x0000000000000000;
    }
    //SOCKET vars here
    struct protoent *protoent;
    char protoname[] = "tcp";
    char protonameudp[] = "udp";
    //MEMFD vars
    char namestr[] = "memfd";
    
    for (fd_idx = 0; fd_idx < 64; fd_idx += 1) {
        while(HANDLES[fd_idx] == 0x0000000000000000) {
            switch (rand() % 6){
            case 0:
                temp_fd = open("/dev/random", O_RDWR);
                if (temp_fd == -1) {
                    logger("//cannot open /dev/random");
                }
                else {
                    logger("//Opened /dev/random with fd %d", temp_fd);
                    HANDLES[fd_idx] = temp_fd;
                    temp_fd = -1;
                }
            
                break;
                case 1:
                    protoent = getprotobyname(protoname);
                    if (protoent == NULL) {
                        logger("//getprotobyname failed");
                    }
                    temp_fd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
                    if (temp_fd == -1) {
                        logger ("//failed to open socket");
                    }
                    else{
                        logger("//opened socket with fd %d", temp_fd);
                        HANDLES[fd_idx] = temp_fd;
                        temp_fd = -1;
                    }
                break;
                    
                case 2:
                    protoent = getprotobyname(protonameudp);
                    if (protoent == NULL) {
                        logger("//getprotobynameudp failed");
                    }
                    temp_fd = socket(AF_INET, SOCK_STREAM, protoent->p_proto);
                    if (temp_fd == -1) {
                        logger ("//failed to open UDP socket");
                    }
                    else{
                        logger("//opened UDP socket with fd %d", temp_fd);
                        HANDLES[fd_idx] = temp_fd;
                        temp_fd = -1;
                    }
                    break;
                
                case 3:
                    temp_fd = open("/dev/null", O_RDWR);
                    if (temp_fd == -1) {
                        logger("//cannot open /dev/null");
                    }
                    else {
                        logger("//Opened /dev/null with fd %d", temp_fd);
                        HANDLES[fd_idx] = temp_fd;
                        temp_fd = -1;
                    }
                    break;
                
                case 4:
                    temp_fd = necp_open(0);
                    if (temp_fd == -1) {
                        logger("//cannot open necp_open fd");
                    }
                    else {
                        logger("//necp_open with fd %d", temp_fd);
                        HANDLES[fd_idx] = temp_fd;
                        temp_fd = -1;
                    }
                    break;
                case 5:
                {
                    temp_fd = random_socket();
                    if (temp_fd == -1) {
                        logger("//cannot open random socket");
                    }
                    else {
                        logger("//random socket open with fd %d", temp_fd);
                        HANDLES[fd_idx] = temp_fd;
                        temp_fd = -1;
                    }
                    break;
                }
            }
        }
        
    }
    
    HANDLES_ARRAY_AVAILABLE_SLOT_INDEX = 64;
}

BH_Object get_random_fd(void)
{
    BH_Object temp_fd;
    unsigned int n;
    
    if (HANDLE_ARRAY_FULLY_POPULATED) {
        n = sizeof (HANDLES) / sizeof (HANDLES[0]);
        n = rand() % n;
    }
    else {
        n = rand() % HANDLES_ARRAY_AVAILABLE_SLOT_INDEX;
    }
    
    temp_fd.index = n;
    temp_fd.value = HANDLES[n];
    
    logger("//[Handler_Function]: get_random_FD : n = %u, HANDLES[n] = 0x%08x, HANDLE_CREATOR[n] = %s", n, HANDLES[n], HANDLE_CREATOR[n]);
    
    return (temp_fd);
}

 //need fixing up when we debug...
int get_specific_fd(int n)
{
    // This is for pulling a FD by its index.
    // Providing a negative index in the array as a means of returning NULL.
    if (n < 0) {
        return NULL;
    }
    else {
        return (HANDLES[n]);
    }
}


void make_structs(){
    
}

/**
 Function which returns a random object of type passed as creator.
 **/

BH_Object get_random_object_by_name(char *creator)
{
    BH_Object temp_obj = {0};
    int indx[256] = {0};
    int count = 0;
    
    // Look to see if an object of this type exists already.
    // add to array if it does.
    for (int i = 0; i < OBJECTS_N; i++)
    {
        char *name = OBJECT_CREATOR[i];
        
        if (name == NULL)
            break;
            //return temp_obj;
        
        if (strcmp(name,creator) == 0)
        {
            //printf("++ Found object of type %s \n",creator);
            //printf("Adding to array at %d\n",count);
            indx[count] = i;
            count += 1;
            //temp_obj.index = i;
            //temp_obj.value = OBJECTS[i];
            //temp_obj.tag = OBJECT_CREATOR[i];
            //return temp_obj;
        }
    }
    
    // Choose a random one in the array.
    
    if (count > 0)
    {
        int idx = rand() % count;
        int elem = indx[idx];
        temp_obj.index = elem;
        temp_obj.value = OBJECTS[elem];
        temp_obj.tag = OBJECT_CREATOR[elem];
    }
    
    // If we have found an object, ensure it is the correct type.
    if (temp_obj.tag) {
        assert(strcmp(temp_obj.tag,creator) == 0);
    }
    
    return temp_obj;
}

/**
 Puts an object of type ObjectCreator to the object ObjectCreator
 **/
OBJECT put_random_object(OBJECT object,char *ObjectCreator)
{
    // Check if the handle is valid, discard if it isn't.
    if (object == NULL || (int)object == -1)
    {
        // invalid handle
        return 0;
    }
    else
    {
        // Start again 
        if (OBJECT_ARRAY_AVAILABLE_SLOT_INDEX + 1 == OBJECTS_N)
        {
            OBJECT_ARRAY_AVAILABLE_SLOT_INDEX = 0;
            OBJECT_ARRAY_FULLY_POPULATED = 1;
        }
    }
    
    int idx = OBJECT_ARRAY_AVAILABLE_SLOT_INDEX;
    OBJECTS[idx] = object;
    OBJECT_CREATOR[OBJECT_ARRAY_AVAILABLE_SLOT_INDEX] = ObjectCreator;
    
    return (OBJECTS[OBJECT_ARRAY_AVAILABLE_SLOT_INDEX++]);
}

BH_Object get_random_object(void)
{
    BH_Object temp_obj;
    unsigned int n;
    
    if (OBJECT_ARRAY_FULLY_POPULATED)
    {
        n = sizeof(OBJECTS) / sizeof(OBJECTS[0]);
        n = rand() % n;
    }
    else
    {
        n = rand() % OBJECT_ARRAY_AVAILABLE_SLOT_INDEX;
    }
    
    temp_obj.index = n;
    temp_obj.value = OBJECTS[n];
    
    return (temp_obj);
}

OBJECT get_specific_object(int n)
{
    if (n < 0)
        return NULL;
    else
        return (OBJECTS[n]);
    
}

void dump_OBJECTS(void)
{
    int n;
    for (n = 0; n < OBJECTS_N; n = n + 1)
    {
        printf("//[Handler_Function]: dump_HANDLES: n = %d, HANDLES[n] = 0x%08x\n",n,(unsigned int)OBJECTS[n]);
    }
}

// structs of {userClient, type, max_methods}

typedef struct {
    char *name;
    int type;
    int max_methods;
    void *fuzz_method;
} userclient;

// prop fuzz method is the list of properties supported by service.
typedef struct {
    char *name;
    void *prop_fuzz_method;
} ioservice;

// List of supported user clients go here which we can open with IOServiceOpen.
static const userclient user_clients[] = {
    
    // IOHIDFamily (IOHIDFamily.h)
    {"IOHIDSystem",1,1,0},
    {"IOHIDPointingDevice",1,1,0},
    {"AppleUSBMultitouch",1,1,0},
    
    // IOSurface
    {"IOSurface",0,1,0},
    {"IOSurface",1,25,0},
    
    // IOAccelerator
    {"IOAccelerator",0,1,0},  // IGAccelSurface
    {"IOAccelerator",1,1,0},  // IGAccelGLContext
    {"IOAccelerator",2,1,0},  // IGAccel2DContext
    {"IOAccelerator",3,1,0},  // ???
    {"IOAccelerator",5,1,0},  // IGAccelSharedUserClient
    
    {"IOFramebuffer",1,1,0},
    {"AppleIntelFramebuffer",1,1,0},
    {"IODisplayWrangler",1,1,0},
    
    {"Gen6Accelerator",1,1,0},
    
    {"IOPMrootDomain",0,0,0},
    
    // IOBluetooth
    {"IOBluetoothHCI",0,0,0},
    {"IOBluetoothHCI",1,0,0},
    // IOBluetoothSerialManager matches but no user client.
    // IONetworkStack same
    
    // Apple Keystore
    {"AppleKeyStore",1,0,0},
    {"AppleFDEKeyStore",1,0,0},
    {"IOReport",1,0,0},
    {"IOReportHub",0,0,0},
    
    // IOAudio
    {"IOAudioEngine",0,0,0},
    {"IOAudioEngine",1,0,0},
    {"AppleHDAEngineInput",1,0,0},
    {"AppleHDAEngineOutput",1,0,0},
    
    // IOUSB
    {"IOUSBInterface",1,0,0},
    {"IOUSBDevice",1,0,0},
    {"IOHDIXController",0,0,0},
    {"AppleUSBInterface",0,1,0},
    
    // Misc
    {"AGPM",1,0,0}, // AppleGraphicsPowerManagement
    {"AppleRTC",1,0,0},
    {"AppleSMC",1,0,0},
    {"AppleHV",1,0,0},
    {"AppleUpstream",1,0,0},
    {"AppleLMUController",1,0,0},
    {"AppleMobileFileIntegrity",1,0,0},
    {"CoreStorage",0,0,0},
    
    // Thunderbolt
    {"IOThunderboltController",0,0,0},
    
    {"IOPMrootDomain",0,0,0},
    
    {"AppleHSSPIHIDDriver",0,0,0},
    
    
    // iOS only
    // IOMobileFrameBuffer
    // AppleBaseband
    // AppleOscarCMA
    // AppleOscarAccelerometer
    // AppleOscarGyro
};

const userclient random_userclient()
{
    unsigned int n = sizeof (user_clients) / sizeof (user_clients[0]);
    return user_clients[rand() % n];
}


// These are used to init the objects database.

io_connect_t get_user_client(const char* name, int type) {
    kern_return_t err;
    CFDictionaryRef matching;
    io_service_t service;
    
    // try IOServiceMatching
    matching = IOServiceMatching(name);
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matching); // consume a ref on matching
    
    if (service == MACH_PORT_NULL) {
        // try IOServiceNameMatching
        matching = IOServiceNameMatching(name);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    }
    
    if (service == MACH_PORT_NULL) {
        // try everything and look for a partial name match
        matching = IOServiceMatching("IOService");
        io_iterator_t iterator;
        IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator);
        
        int found_it = 0;
        while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
            io_name_t object_name;
            IOObjectGetClass(service, object_name);
            if (strstr(object_name, name)) {
                found_it = 1;
                break;
            }
            IOObjectRelease(service);
        }
        IOObjectRelease(iterator);
        
        if (!found_it) {
            printf("njet matches %s\n",name);
            // couldn't find any matches for the name anywhere
            return MACH_PORT_NULL;
        }
    }
    
    io_connect_t conn = MACH_PORT_NULL;
    
    err = IOServiceOpen(service, mach_task_self(), type, &conn);
    if (err != KERN_SUCCESS){
        //printf("IOServiceOpen failed: %s\n", mach_error_string(err));
        IOObjectRelease(service);
        return MACH_PORT_NULL;
    }
    else {
        printf("Found %s\n",name);
    }
    IOObjectRelease(service);
    
    return conn;
}

// Returns a userclient connection back..
io_connect_t get_io_connect_t()
{
    io_connect_t connect;
    
    // First choose a random user client.
    userclient uc = random_userclient();
    printf("userclient = %s\n",uc.name);
    printf("userclient type = %d\n",uc.type);
    
    connect = get_user_client(uc.name, uc.type);
    
    return connect;
}


mach_port_t get_mach_port_t()
{
    mach_port_t port = kIOMasterPortDefault;
    int which = 0;
    switch (rand() % 7)
    {
        case 0:
            port = mach_task_self();
            break;
        case 1:
            port = mach_host_self();
            break;
        case 2:
            port = mach_thread_self();
            break;
        case 3:
        {
            which = 4;
            host_get_special_port(mach_host_self(), HOST_LOCAL_NODE, which, &port);
            break;
        }
        case 4:
        {
            host_get_io_master(mach_host_self(),&port);
            break;
        }
        case 5:
        {
            task_get_bootstrap_port(mach_task_self(), &port);
            break;
        }
        default:
            port = kIOMasterPortDefault;
    }
    
    return  port;
}

task_t get_task_t()
{
    task_t task;
    task_for_pid(mach_task_self(), getpid(), &task);
    return task;
}

IONotificationPortRef get_IONotificationPortRef2()
{
    return IONotificationPortCreate(kIOMasterPortDefault);
}

dispatch_queue_t get_dispatch_queue_t()
{
    dispatch_queue_t queue;
    queue = dispatch_queue_create("aaa",NULL);
    return queue;
}

mach_msg_header_t get_mach_msg_header_t()
{
    mach_msg_header_t msg;
    
    msg.msgh_bits = get_fuzzed_int32();
    msg.msgh_size = get_fuzzed_int32();
    msg.msgh_remote_port = (mach_port_t)get_random_object_by_name("mach_port_t").value;
    msg.msgh_local_port = (mach_port_t)get_random_object_by_name("mach_port_t").value;
    msg.msgh_voucher_port = (mach_port_t)get_random_object_by_name("mach_port_t").value;
    msg.msgh_id = get_fuzzed_int32();
    
    return msg;
}

io_object_t get_io_object_t()
{
    io_object_t obj;
    return obj;
}

io_iterator_t get_io_iterator_t()
{
    io_iterator_t t;
    CFDictionaryRef matching;
    
    matching = IOServiceMatching("IOService");
    
    IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &t);
    
    return t;
}

io_service_t get_io_service_t()
{
    io_service_t service;
    CFDictionaryRef matching;
    
    // try IOServiceMatching
    matching = IOServiceMatching("IOService");
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matching); // consume a ref on matching
    return service;
}

io_registry_entry_t get_io_registry_entry_t()
{
    io_registry_entry_t service  = 0;
    service = IORegistryGetRootEntry(kIOMasterPortDefault);
    return service;
}

// Helper methods (return structs).
// Star can take this and convert it from an XML
IOSurfaceRef get_IOSurfaceRef(char *vid)
{
    const long surface_width = 10;
    const long surface_height = 10;
    const long bytes_per_elem = 2;
    IOSurfaceRef surface;
    
    const CFTypeRef surf_attr_keys[] = {
        kIOSurfaceWidth,
        kIOSurfaceHeight,
        kIOSurfaceBytesPerElement
    };
    
    // Fuzz these params
    const CFTypeRef surf_attr_values[] = {
        CFNumberCreate(NULL,kCFNumberLongType, &surface_width),
        CFNumberCreate(NULL, kCFNumberLongType, &surface_height),
        CFNumberCreate(NULL, kCFNumberIntType, &bytes_per_elem)
    };
    
    CFDictionaryRef surf_attr;
    surf_attr = CFDictionaryCreate(NULL, (const void **)surf_attr_keys, (const void **) surf_attr_values, 3, NULL, (const CFDictionaryValueCallBacks *)&kCFTypeDictionaryKeyCallBacks);
    
    if (!surf_attr)
    {
        printf("Failed to create CFDictionary");
    }
    
    surface = IOSurfaceCreate(surf_attr);
    if (surface)
        logger("// Created IOSurface");
    
    logger("//[Helper Function] create_io_surface");
    return surface;
}

hv_vcpuid_t get_hv_vcpuid_t()
{
    hv_vm_create(HV_VM_DEFAULT);
    hv_vcpuid_t vcpu;
    hv_vcpu_create(&vcpu, HV_VCPU_DEFAULT);    
    return vcpu;
}

static kern_return_t _voucher_create_mach_voucher(const mach_voucher_attr_recipe_data_t *recipes, size_t recipes_size, mach_voucher_t *kvp)
{
    kern_return_t kr;
    mach_port_t mhp = mach_host_self();
    mach_voucher_t kv = MACH_VOUCHER_NULL;
    mach_voucher_attr_raw_recipe_array_t kvr;
    mach_voucher_attr_recipe_size_t kvr_size;
    kvr = (mach_voucher_attr_raw_recipe_array_t)recipes;
    kvr_size = (mach_voucher_attr_recipe_size_t)recipes_size;
    kr = host_create_mach_voucher(mhp, kvr, kvr_size, &kv);
    *kvp = kv;
    
    return kr;
}

// Generate a bunch of sample objects
void make_OBJECTS(void)
{
    unsigned int object_idx = 0;
    OBJECT tempobject;
    
    // Initialize the array of handles by setting every handle to -1.
    for (object_idx = 0; object_idx < OBJECTS_N; object_idx +=1)
    {
        OBJECTS[object_idx] = (OBJECT)-1;
    }
    
    // Populate each one of the handle slots sequentially (the first 64).
    for (object_idx = 0; object_idx < 64; object_idx += 1)
    {
        while ((int)OBJECTS[object_idx] == -1)
        {
            // Create some default types for looking up in object store
            // Unseeded PRNG should always return same values
            // Create some types for BSD calls such as FDs
            //
            
            switch(rand() % 12) {
                case 0:
                    tempobject = (OBJECT)get_user_client("IOHIDSystem",1);
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "IOHIDSystem");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "IOHIDSystem";
                    tempobject = (OBJECT)-1;
                    break;
                case 1:
                    tempobject = (OBJECT)get_io_connect_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "io_connect_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "io_connect_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 2:
                    tempobject = (OBJECT)get_mach_port_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "mach_port_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "mach_port_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 3:
                    tempobject = (OBJECT)get_IONotificationPortRef2();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "IONotificationPortRef");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "IONotificationPortRef";
                    tempobject = (OBJECT)-1;
                    break;
                case 4:
                    tempobject = (OBJECT)get_dispatch_queue_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "dispatch_queue_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "dispatch_queue_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 5:
                    tempobject = (OBJECT)get_io_object_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "io_object_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "io_object_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 6:
                    tempobject = (OBJECT)get_io_iterator_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "io_iterator_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "io_iterator_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 7:
                    tempobject = (OBJECT)get_IOSurfaceRef("test");
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "IOSurfaceRef");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "IOSurfaceRef";
                    tempobject = (OBJECT)-1;
                    break;
                case 8:
                    tempobject = (OBJECT)get_task_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "task_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "task_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 9:
                    tempobject = (OBJECT)get_hv_vcpuid_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "hv_vcpuid_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "hv_vcpuid_t";
                    tempobject = (OBJECT)-1;
                    break;
                case 10:
                    tempobject = (OBJECT)get_io_registry_entry_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "io_registry_entry_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "io_registry_entry_t";
                    tempobject = (OBJECT)-1;
                    break;
                default:
                    tempobject = (OBJECT)get_io_service_t();
                    logger("//[Handler_Function]: make_OBJECTS : n = %u, object = 0x%08X, OBJECT_CREATOR[n] = %s", object_idx, tempobject, "io_service_t");
                    OBJECTS[object_idx] = tempobject;
                    OBJECT_CREATOR[object_idx] = "io_service_t";
                    tempobject = (OBJECT)-1;
                    break;
            }
            
            OBJECT_ARRAY_AVAILABLE_SLOT_INDEX = 64;
        }
    }
}

#endif
