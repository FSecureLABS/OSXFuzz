//
//  helpers.h
//  OSXFuzz
//
//  Created by alex on 18/08/2016.
// This file is used to initialize a bunch of types we need for fuzzing..
//

#ifndef helpers_h
#define helpers_h

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>
#include <mach/host_special_ports.h>
#include <IOSurface/IOSurfaceAPI.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <CoreFoundation/CFNumber.h>
#include <Hypervisor/hv.h>



static void printKeys (const void* key, const void* value, void* context) {
    CFShow(key);
    CFShow(value);
}

const char *surface_args[] = {
    "IOSurfaceIsGlobal",
    "IOSurfaceBytesPerRow",
    "IOSurfaceBitsPerBlock",
    "IOSurfaceBytesPerElement",
    "IOSurfaceWidth",
    "IOSurfaceHeight",
    "IOSurfaceElementWidth",
    "IOSurfaceElementHeight",
    "IOSurfaceOffset",
    "IOSurfacePixelFormat",
    "IOSurfaceAllocSize",
    "IOSurfaceMemoryRegion",
    "IOSurfacePlaneInfo",
    "IOSurfacePlaneOffset",
    "IOSurfacePlaneWidth",
    "IOSurfacePlaneHeight",
    "IOSurfacePlaneBitsPerBlock",
    "IOSurfacePlaneBytesPerBlock",
    "IOSurfacePlaneBytesPerElement",
    "IOSurfacePlaneElementWidth",
    "IOSurfacePlaneBytesPerRow",
    "IOSurfaceAddress",
    "IOSurfaceType",
    "IOSurfaceYCbCrMatrix",
    "IOSurfaceCacheMode",
    "IOSurfacePurgeWhenNotInUse",
    "IOSurfaceName",
    "IOSurfaceClass"
};

const char * random_surface_args()
{
    unsigned int n = sizeof(surface_args) / sizeof(surface_args[0]);
    int idx = rand() % n;
    const char *str = surface_args[idx];
    return str;
}


char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;
    
    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }
    
    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);
    
    /* Add space for terminating null string so caller
     knows where the list of returned strings ends. */
    count++;
    
    result = malloc(sizeof(char*) * count);
    
    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);
        
        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }
    
    return result;
}


// Helper function to return a matching directory for IOKit.
CFMutableDictionaryRef get_IOServiceMatchingDict(char *vid)
{
    CFMutableDictionaryRef ref;
    userclient uc = random_userclient();
    char *service_name = uc.name;
    logger("matching%s = IOServiceMatching(\"%s\");",vid,service_name);
    ref = IOServiceMatching(service_name);
    return ref;
}

// This is a huge struct (lots of variables to set).
NXEventData get_NXEventData(char *vid)
{
    NXEventData eventData;
    
    return eventData;
}

NXTabletPointData get_NXTabletPointData(char *vid)
{
    NXTabletPointData data;
    
    return data;
}

NXTabletProximityData get_NXTabletProximityData(char *vid)
{
    NXTabletProximityData data;
    return data;
}

// Currently returns a file struct, should return and FD. Use the FD database in future.
FILE *get_random_FD(){
    FILE *fp = fopen("/tmp/test.txt", "ab+");
    return fp;
    
}

// Used for io_service_get_matching_services_bin etc
char * getSerializedBinary()
{
    char *buff;
    
    buff = malloc(1024);
    
    return buff;
}

char * getSerializedXML()
{
    char *buff;
    
    buff = malloc(1024);
    
    return buff;
}


////////////////// Core Foundation Types /////////////////////////////////
// TODO: Fix logging of CoreFoundation types
// https://developer.apple.com/reference/corefoundation/1666669-cfnumber
CFTypeRef get_CFNumber(char *vid)
{
    CFTypeRef ref;
    
    switch (rand() % 15)
    {
        // CFNumber
        case 0:
        {
            int8_t size = get_fuzzed_int8();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt8Type, &size);
            break;
        }
        case 1:
        {
            int16_t size = get_fuzzed_int16();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &size);
            break;
        }
        case 2:
        {
            int32_t size = get_fuzzed_int32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &size);
            break;
        }
        case 3:
        {
            int64_t size = get_fuzzed_int64();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &size);
            break;
        }
        case 4:
        {
            real32_t size = get_fuzzed_real32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &size);
            break;
        }
        case 5:
        {
            real64_t size = get_fuzzed_real64();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &size);
            break;
        }
        case 6:
        {
            char size = get_fuzzed_int8();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberCharType, &size);
            break;
        }
        case 7:
        {
            short size = get_fuzzed_int16();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberShortType, &size);
            break;
        }
        case 8:
        {
            int size = get_fuzzed_int32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &size);
            break;
        }
        case 9:
        {
            long size = get_fuzzed_int64();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongType, &size);
            break;
        }
        case 10:
        {
            long long size = get_fuzzed_int64();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongLongType, &size);
            break;
        }
        case 11:
        {
            float size = get_fuzzed_real32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &size);
            break;
        }
        case 12:
        {
            double size = get_fuzzed_real64();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &size);
            break;
        }
        case 13:
        {
            int size = get_fuzzed_int32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &size);
            break;
        }
        case 14:
        {
            int size = get_fuzzed_int32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberNSIntegerType, &size);
            break;
        }
        case 15:
        {
            float size = get_fuzzed_real32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberCGFloatType, &size);
            break;
        }

        default:
        {
            int32_t size = get_fuzzed_int32();
            ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &size);
            break;
        }

    }
    
    return ref;
}

CFTypeRef get_CFArray(char *vid)
{
    CFTypeRef ref;
    
    CFStringRef values[2];
    values[0] = CFSTR("aa");
    values[1] = CFSTR("bbb");

    ref = CFArrayCreate(kCFAllocatorDefault,(const void **)&values,2,NULL);

    
    return ref;
}

CFTypeRef get_CFAttributedString(char *vid)
{
    CFTypeRef ref;
    
    char *key_name = "IOSurfaceAddress";
    CFStringRef key = CFStringCreateWithFormat(NULL,NULL,CFSTR("%s"),key_name);
    ref = CFAttributedStringCreate(kCFAllocatorDefault,key,NULL);
    return ref;
}

CFTypeRef get_CFBag(char *vid)
{
    CFTypeRef ref;
    
    char *values[] = {"test"};
    
    ref = CFBagCreate(kCFAllocatorDefault, (const void **)&values, sizeof(values), NULL);

    return ref;
}


CFTypeRef get_CFBinaryHeap(char *vid)
{
    CFTypeRef ref;
    ref = CFBinaryHeapCreate(kCFAllocatorDefault,0,NULL,NULL);
    return ref;
}

CFTypeRef get_CFBitVector(char *vid)
{
    CFTypeRef ref;
    uint8_t bytes = get_fuzzed_uint8();
    CFIndex numBits = get_fuzzed_int32();
    ref = CFBitVectorCreate(kCFAllocatorDefault, &bytes, numBits);
    return ref;
}

CFTypeRef get_CFBundle(char *vid)
{
    CFTypeRef ref;
    CFStringRef bundlePath;
    CFURLRef bundleURL;
    
    bundlePath = CFStringCreateWithCString(NULL, "aaa", kCFStringEncodingMacRoman);
    bundleURL = CFURLCreateWithFileSystemPath(NULL, bundlePath, kCFURLPOSIXPathStyle, TRUE);
    CFRelease(bundlePath);
    
    ref = CFBundleCreate(kCFAllocatorDefault, bundleURL);
    return ref;
}

CFTypeRef get_CFCalendar(char *vid)
{
    CFTypeRef ref;
    
    ref = CFCalendarCreateWithIdentifier(kCFAllocatorDefault, kCFGregorianCalendar);
    return ref;
}

CFTypeRef get_CFCharacterSet(char *vid)
{
    CFTypeRef ref;
    ref = CFCharacterSetCreateMutable(kCFAllocatorDefault);
    return ref; 
}

CFTypeRef get_CFData(char *vid)
{
    CFTypeRef ref;
    unsigned char buffer[256];
    memset(buffer,0x41,sizeof(buffer));
    ref = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)&buffer, 256);
    return ref;
}

CFTypeRef get_CFDate(char *vid)
{
    CFTypeRef ref;
    CFAbsoluteTime at;
    ref = CFDateCreate(kCFAllocatorDefault, at);
    return ref;
}

CFTypeRef get_CFDateFormatter(char *vid)
{
    CFTypeRef ref;
    ref = CFDateFormatterCreate(NULL, NULL, kCFDateFormatterLongStyle, kCFDateFormatterLongStyle);
    return ref;
}

CFTypeRef get_CFError(char *vid)
{
    CFTypeRef ref;
    ref = CFErrorCreate(kCFAllocatorDefault, CFSTR("Taaa"), (CFIndex)0, NULL);
    return ref;
}

CFTypeRef get_CFFileDescriptorCreateRunLoopSource()
{
    CFTypeRef ref;
    int fd = 0;
    ref = CFFileDescriptorCreate(kCFAllocatorDefault, fd, 0, 0, 0);
    return ref;
}

CFTypeRef get_CFFileSecurity(char *vid)
{
    CFTypeRef ref;
    ref = CFFileSecurityCreate(NULL);
    return ref;
}

CFTypeRef get_CFMachPortCreate(char *vid)
{
    CFTypeRef ref;
    
    ref = CFMachPortCreate(kCFAllocatorDefault,
                           NULL, NULL, NULL);
    
    return ref;
}

CFTypeRef get_CFMessagePort(char *vid)
{
    CFTypeRef ref;
    ref = CFMessagePortCreateLocal(NULL, CFSTR("test"), NULL, NULL, NULL);
    return ref;
}

CFTypeRef get_CFPropertyList(char *vid)
{
    CFTypeRef ref;
    CFDataRef		versData = NULL;
    ref = CFPropertyListCreateWithData(kCFAllocatorDefault, versData, 0, NULL, NULL);
    return ref;
}


CFTypeRef get_CFString(char *vid)
{
    CFTypeRef ref;
    ref = CFSTR("aaa");
    return ref;
}

CFTypeRef get_CFTree(char *vid)
{
    CFTypeRef ref;
    ref = CFTreeCreate(NULL, NULL);
    return ref;
}

CFTypeRef get_CFURL(char *vid)
{
    CFTypeRef ref;
    CFURLRef cfUrl = CFURLCreateWithString(kCFAllocatorDefault,
                                           CFSTR("/tmp"),
                                           NULL);
    
    ref = CFURLCreateFilePathURL(kCFAllocatorDefault, cfUrl, NULL);
    
    return ref;
}

CFTypeRef get_CFUUID(char *vid)
{
    CFTypeRef ref;
    
    ref = CFUUIDCreate(NULL);
    
    return ref;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
CFTypeRef get_CFXMLNode(char *vid)
{
    CFTypeRef ref;
    CFXMLNodeTypeCode xmlType = rand()  % 20;
    CFStringRef dataString = get_CFString(vid);
    
    
    ref = CFXMLNodeCreate(kCFAllocatorDefault, xmlType, dataString, 0, 0);
    return ref; 
}

CFTypeRef get_CFXMLParser(char *vid)
{
    CFTypeRef ref;
    
    CFDataRef xmlData = get_CFData(vid);
    
    
    ref = CFXMLParserCreate (kCFAllocatorDefault, (CFDataRef)xmlData, NULL,
                       kCFXMLParserAllOptions,
                       kCFXMLNodeCurrentVersion,
                       0, 0);

    return ref;
}

#pragma clang diagnostic pop

CFTypeRef get_CFTypeRef(char *vid)
{
    // Choose a CF type for value.
    CFTypeRef value;
    // For now only take the serializable ones.
    
    switch (rand() % 10)
    {
        case 1:
        {
            value = get_CFArray(vid);
            break;
        }
        case 3:
        {
            value = get_CFNumber(vid);
            break;
        }
        case 4:
        {
            value = get_CFString(vid);
            break;
        }
        case 5:
        {
            value = get_CFData(vid);
            break;
        }
        default:
        {
            value = get_CFNumber(vid);
            break;
        }
    }
    return value;
}

// Populate a CFDictionary with fuzzed data.
CFDictionaryRef get_CFDictionaryRef(char *vid)
{
    CFDictionaryRef ref;
    
    // Number of elements.
    int elems = get_fuzzed_int16();
    
    ref = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    for (int i = 0; i < elems; i++)
    {
        // Choose a key
        const char *key_name = random_surface_args();
        CFStringRef key = CFStringCreateWithFormat(NULL,NULL,CFSTR("%s"),key_name);
        CFTypeRef value = get_CFTypeRef(vid);
        CFDictionarySetValue(ref, key,value);
    }
    
    CFDictionaryApplyFunction(ref, printKeys, NULL);
    
    return ref;
  }

///////////////// end of CoreFoundation Types //////////////////////////

// get_CFSurfaceInit

///////////////// IOKit Helpers ////////////////////////////////////////

io_service_t get_service(const char *name)
{
    CFDictionaryRef matching;
    io_service_t service;
    
    // Try IOServiceMatching
    matching = IOServiceMatching(name);
    service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    
    if (service == MACH_PORT_NULL)
    {
        // try IOServiceNameMatching
        matching = IOServiceNameMatching(name);
        service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    }
    
    if (service == MACH_PORT_NULL)
    {
        // Try everything and look for a partial name match
        matching = IOServiceMatching("IOService");
        io_iterator_t iterator;
        IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &iterator);
        
        int found_it = 0;
        while ((service == IOIteratorNext(iterator)) != IO_OBJECT_NULL)
        {
            io_name_t object_name;
            IOObjectGetClass(service,object_name);
            if (strstr(object_name,name))
            {
                found_it = 1;
                break;
            }
            IOObjectRelease(iterator);
        }
        IOObjectRelease(iterator);
        
        if (!found_it)
        {
            printf("Could not find any matches for %s\n",name);
            return MACH_PORT_NULL;
        }
    }
    printf("Found %s \n",name);
    
    return service;
}

IONotificationPortRef get_IONotificationPortRef()
{
    IONotificationPortRef ref;
    return ref;
}


#endif /* helpers_h */
