// Downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/

#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include <string.h>
#include <stdio.h>
#include "units_conv.h"

#if IBM
	#include <windows.h>
#endif
#if LIN
#include <GL/gl.h>
#endif
#if __GNUC__ && APL
#include <OpenGL/gl.h>
#endif
#if __GNUC__ && IBM
#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif


// Log buffer
char myValue_buffer[256];

//  Add dataref to DRE message
#define MSG_ADD_DATAREF 0x01000000

//  Our custom dataref
static XPLMDataRef altInMeters = NULL;
static XPLMDataRef pilotAltFeet = NULL;

//  Our custom dataref's value
float   altInMetersValue;
float   altInFeet;

// float   calc_altimeter(float elapsedMe, float elapsedSim, int counter, void* refcon );
float   RegaltInMetersInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon);  //  Declare callback to register dataref

float   getAltVal(void* inRefcon);
void    setAltVal(void* inRefcon, float outValue);


PLUGIN_API int XPluginStart(
        char *        outName,
        char *        outSig,
        char *        outDesc)
{

    // Plugin Info
    strcpy(outName, "P-180 Avanti II");
    strcpy(outSig, "tselios.acft.avanti");
    strcpy(outDesc, "Aircraft Plugin for Avanti");

    //  Create our custom integer dataref
    altInMeters = XPLMRegisterDataAccessor("avanti/FM/pilotAltMeters",
                                           xplmType_Float,         // The types we support
                                           1,                      // Writable
                                           NULL, NULL,             // Integer accessors
                                           getAltVal, setAltVal,   // Float accessors
                                           NULL, NULL,             // Doubles accessors
                                           NULL, NULL,             // Int array accessors
                                           NULL, NULL,             // Float array accessors
                                           NULL, NULL,             // Raw data accessors
                                           NULL, NULL);            // Refcons not used

// Find and intialize our Counter dataref
    altInMeters = XPLMFindDataRef ("avanti/FM/pilotAltMeters");
    pilotAltFeet = XPLMFindDataRef("sim/cockpit2/gauges/indicators/altitude_ft_pilot");
    altInFeet = XPLMGetDataf(pilotAltFeet);
    XPLMSetDataf(altInMeters, altInFeet * 0.3048);


//time stamp and print to Log.txt
    sprintf(myValue_buffer, "[P-180 Avanti II]: Plugin loaded!\n");
    XPLMDebugString(myValue_buffer);

//register the callback
    XPLMRegisterFlightLoopCallback(RegaltInMetersInDRE, 1, NULL);   // This FLCB will register our custom dataref in DRE
// XPLMRegisterFlightLoopCallback(calc_altimeter, 1, NULL);
    XPLMPluginID PluginID = XPLMFindPluginBySignature("xplanesdk.examples.DataRefEditor");

    units_conv_init();

    if (PluginID != XPLM_NO_PLUGIN_ID)
    {
        XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)"avanti/FM/pilotAltMeters");
        XPLMSendMessageToPlugin(PluginID, MSG_ADD_DATAREF, (void*)"avanti/gauges/pilot/plt_airspeed_kmh");
    }
    return 1;
}


PLUGIN_API void     XPluginStop(void)
{
    XPLMUnregisterDataAccessor(altInMeters);
    XPLMUnregisterFlightLoopCallback(RegaltInMetersInDRE, NULL);	 //  Don't forget to unload this callback.
//      XPLMUnregisterFlightLoopCallback(calc_altimeter,  NULL);

}

PLUGIN_API void XPluginDisable(void)
{
//     XPLMUnregisterFlightLoopCallback(calc_altimeter, NULL);	 //  Don't forget to unload this callback.
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}


PLUGIN_API void XPluginReceiveMessage(XPLMPluginID    inFromWho,
                                      long             inMessage,
                                      void *           inParam)
{
}


float getAltVal(void* inRefcon)
{
    return altInMetersValue;
}

void setAltVal(void* inRefcon, float inValue)
{
    altInMetersValue = inValue;
}

//  This single shot FLCB registers our custom dataref in DRE
float RegaltInMetersInDRE(float elapsedMe, float elapsedSim, int counter, void * refcon)
{
    float p_alt = XPLMGetDataf(pilotAltFeet);
    float alt_conv = (p_alt * 0.3048);
    XPLMSetDataf(altInMeters, alt_conv);

    units_conv_update();

    return 0.1f;

}
