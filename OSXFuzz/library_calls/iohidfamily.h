//
//  IOHIDFamily.c
//  OSXFuzz
//
//

#ifndef iohidfamily_calls_h
#define iohidfamily_calls_h

#include <IOKit/hidsystem/IOHIDShared.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "logger.h"
#include "object_database.h"

const char * hid_properties[] = {
    "HIDKind",
    "HIDInterfaceID",
    "HIDSubinterfaceID",
    "HIDOriginalSubinterfaceID",
    "HIDParameters",
    "HIDVirtualDevice",
    "HIDKeyRepeat",
    "HIDInitialKeyMapping",
    "HIDKeyMapping",
    "HIDResetKeyboard",
    "HIDKeyboardModifierMappingPairs",
    "HIDKeyboardModifierMappingSrc",
    "HIDKeyboardModifierMappingDst",
    "HIDKeyboardCapsLockDoesLock",
    "HIDKeyboardSupportsF12Eject",
    "HIDKeyboardSupportedModifers",
    "HIDPointerResolution",
    "HIDPointerResetPointer",
    "HIDPointerConvertAbsolute",
    "HIDPointerContactToMove",
    "HIDPointerPressureToClick",
    "HIDPointerButtonMode",
    "HIDPointerButtonCount",
    "HIDPointerAcceleration",
    "HIDPointerAccelerationSettings",
    "HIDPointerAccelerationTable",
    "HIDScrollReset",
    "HIDScrollResolution",
    "HIDScrollReportRate",
    "HIDScrollAcceleration",
    "HIDScrollAccelerationType",
    "HIDScrollAccelerationTable",
    "HIDScrollResolutionX",
    "HIDScrollResolutionY",
    "HIDScrollResolutionZ",
    "HIDScrollAccelerationTableX",
    "HIDScrollAccelerationTableY",
    "HIDScrollAccelerationTableZ",
    "HIDScrollMouseButton",
    "HIDScrollZoomModiferMask",
    "HIDTrackpadAcceleration",
    "HIDMouseAcceleration",
    "HIDClickTime",
    "HIDClickSpace",
    "HIDWaitCursorFrameInterval",
    "HIDAutoDimThreshold",
    "HIDAutoDimState",
    "HIDAutoDimTime",
    "HIDIdleTime",
    "HIDBrightness",
    "HIDAutoDimBrightness",
    "HIDFkeyMode",
    "HIDStickyKeysDisabled",
    "HIDStickyKeysOn",
    "HIDStickyKeysShiftToggles",
    "HIDMouseKeysOptionToggles",
    "HIDSlowKeysDelay",
    "HIDF12EjectDelay",
    "HIDMouseKeysOn",
    "HIDUseKeyswitch",
    "HIDDisallowRemappingOfPrimaryClick",
    "HIDMouseKeysEnablesVirtualNumPad",
    "HIDResetLEDs",
    "HIDAccelCurves",
    "HIDTrackingAccelCurves",
    "HIDScrollAccelCurves",
    "HIDAccellCurvesDebug",
    "HIDScrollAccelCurvesDebug",
    "HIDAccelGainLinear",
    "HIDAccelGainParabolic",
    "HIDAccelGainCubic",
    "HIDAccelGainQuartic",
    "HIDAccelTangentSpeedLinear",
    "HIDAccelTangentSpeedParabolicRoot",
    "HIDAccelTangestSpeedCubicRoot",
    "HIDAccelTangentSpeedQuarticRoot",
    "HIDAccelIndex",
    "HIDScrollCountMaxTimeDeltaBetween",
    "HIDScrollCountMaxTimeDeltaToSustain",
    "HIDScrollCountMinDeltaToSustain",
    "HIDScrollCountIgnoreMomentumScrolls",
    "HIDScrollCountMouseCanReset",
    "HIDScrollCountMax",
    "HIDScrollCountAccelerationFactor",
    "HIDScrollCountZero",
    "HIDScrollCountBootDefault",
    "HIDScrollCountReset",
};


// IOHIDParamUserClient
void init_hid_userclient_methods() {
    static const userclient_method __unused methods[] = {
        {3,"extPostEvent",0,sizeof(struct evioLLEvent) + sizeof(int),{"evioLLEvent"},0,0},
        {4,"extSetMouseLocation",0,sizeof(int32_t)*3,0,0,0},
        {5,"extGetStateForSelector",0,0,0,0},
        {6,"extSetStateForSelector",0,0,0,0},
        {7,"extRegisterVirtualDisplay",0,0,0,0},
        {8,"extUnregisterVirtualDisplay",0,0,0,0},
        {9,"extSetVirtualDisplayBounds",0,0,0,0},
        {10,"extGetUserHidActivityState",0,0,0,0},
        {10,"setContinuousCursorEnable",0,0,0,0},
    };
}

void BH_IOHIDCreateSharedMemory()
{
    BH_Object h_iohid_system_createschmem = {0};
    
    // There's two versions (3 and 4).
    int version = 4;
    kern_return_t ret = -1;
    
    // The handle
    char vid[16];
    sprintf(vid,"%u",get_time_in_ms()+rand());
    logger("io_connect_t h_iohid_system_createschmem%s = 0;",vid);
    
    h_iohid_system_createschmem = get_random_object_by_name("IOHIDSystem");
    logger("h_iohid_system_createschmem%s = get_specific_object(%d);",vid,h_iohid_system_createschmem.index);
    
    logger("//[Library Call]: BH_IOHIDCreateSharedMemory");
    logger("IOHIDCreateSharedMemory(get_specific_object(%d),%d);",h_iohid_system_createschmem.index,version);
    
    ret = IOHIDCreateSharedMemory((io_connect_t)h_iohid_system_createschmem.value, version);
    return_logger("IOHIDCreateSharedMemory", ret);
    
}

#endif 
