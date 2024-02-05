#include "main.h"
#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include <wups.h>
#include <vpad/input.h>
#include <padscore/kpad.h>
#include <sysapp/launch.h>
#include <coreinit/dynload.h>
#include <nn/acp/title.h>
#include <coreinit/systeminfo.h>

// Mandatory plugin info
WUPS_PLUGIN_NAME("Close/Restart Game");
WUPS_PLUGIN_DESCRIPTION(DESCRIPTION);
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Lynx64");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();

// Called when exiting the plugin loader
INITIALIZE_PLUGIN()
{
    initConfig();
}

extern "C" uint32_t VPADGetButtonProcMode(VPADChan chan);

#define HBM_HOLD_TO_RESTART_FRAMES 45

static OSDynLoad_Module sysappModule                                 = nullptr;
static void (*dyn_SYSLaunchMenu)()                                   = nullptr;
static void (*dyn__SYSLaunchTitleDirect)(uint64_t titleId)           = nullptr;

static OSDynLoad_Module acpModule                                    = nullptr;
static ACPResult (*dyn_ACPGetTitleIdOfMainApplication)(ACPTitleId *) = nullptr;

static uint32_t sHeldXForFramesGamePad = 0;
static uint32_t sHeldXForFramesKPAD[4] = {0, 0, 0, 0};

static void restartGameFromHBM()
{
    //load sysapp
    if (OSDynLoad_Acquire("sysapp.rpl", &sysappModule) == OS_DYNLOAD_OK) {
        if (OSDynLoad_FindExport(sysappModule, OS_DYNLOAD_EXPORT_FUNC, "_SYSLaunchTitleDirect", (void**) &dyn__SYSLaunchTitleDirect) == OS_DYNLOAD_OK) {
            if (OSDynLoad_Acquire("nn_acp.rpl", &acpModule) == OS_DYNLOAD_OK) {
                if (OSDynLoad_FindExport(acpModule, OS_DYNLOAD_EXPORT_FUNC, "ACPGetTitleIdOfMainApplication", (void**) &dyn_ACPGetTitleIdOfMainApplication) == OS_DYNLOAD_OK) {
                    uint64_t titleId;
                    if (dyn_ACPGetTitleIdOfMainApplication(&titleId) == ACP_RESULT_SUCCESS) {
                        dyn__SYSLaunchTitleDirect(titleId);
                    }
                }
                OSDynLoad_Release(acpModule);
            }
        }
        OSDynLoad_Release(sysappModule);
    }
}

static void closeGameFromHBM()
{
    //load sysapp
    if (OSDynLoad_Acquire("sysapp.rpl", &sysappModule) == OS_DYNLOAD_OK) {
        if (OSDynLoad_FindExport(sysappModule, OS_DYNLOAD_EXPORT_FUNC, "SYSLaunchMenu", (void**) &dyn_SYSLaunchMenu) == OS_DYNLOAD_OK) {
            dyn_SYSLaunchMenu();
        }
        OSDynLoad_Release(sysappModule);
    }
}

DECL_FUNCTION(int32_t, VPADRead, VPADChan chan, VPADStatus *buffer, uint32_t bufferSize, VPADReadError *error)
{
    VPADReadError realError = VPAD_READ_UNINITIALIZED;
    int32_t result = real_VPADRead(chan, buffer, bufferSize, &realError);
    uint32_t end = 1;
    if (VPADGetButtonProcMode(chan) == 1) {
        end = bufferSize;
    }

    if (result > 0 && realError == VPAD_READ_SUCCESS) {
        bool foundX = false;
        for (uint32_t i = 0; i < end; i++) {
            if (buffer[i].hold & VPAD_BUTTON_X) {
                foundX = true;
                break;
            }
        }
        if (foundX) {
            if (sHeldXForFramesGamePad == HBM_HOLD_TO_RESTART_FRAMES && gHoldToRestart && OSIsHomeButtonMenuEnabled()) {
                OSEnableHomeButtonMenu(FALSE);
                restartGameFromHBM();
            }
            sHeldXForFramesGamePad++;
        } else if (sHeldXForFramesGamePad > 0) {
            if (sHeldXForFramesGamePad <= HBM_HOLD_TO_RESTART_FRAMES && gPressToClose && OSIsHomeButtonMenuEnabled()) {
                OSEnableHomeButtonMenu(FALSE);
                closeGameFromHBM();
            }
            sHeldXForFramesGamePad = 0;
        }
    }
    if (error) {
        *error = realError;
    }
    return result;
}

DECL_FUNCTION(int32_t, KPADReadEx, KPADChan chan, KPADStatus *data, uint32_t size, KPADError *error)
{
    KPADError realError = KPAD_ERROR_UNINITIALIZED;
    int32_t result = real_KPADReadEx(chan, data, size, &realError);
    if (result > 0 && realError == KPAD_ERROR_OK && 
        chan >= 0 && chan < 4 && 
        (data[0].extensionType == WPAD_EXT_CLASSIC || data[0].extensionType == WPAD_EXT_PRO_CONTROLLER)) {
        
        if (data[0].classic.hold & WPAD_CLASSIC_BUTTON_X) {
            if (sHeldXForFramesKPAD[chan] == HBM_HOLD_TO_RESTART_FRAMES && gHoldToRestart && OSIsHomeButtonMenuEnabled()) {
                OSEnableHomeButtonMenu(FALSE);
                restartGameFromHBM();
            }
            sHeldXForFramesKPAD[chan]++;
        } else if (sHeldXForFramesKPAD[chan] > 0) {
            if (sHeldXForFramesKPAD[chan] <= HBM_HOLD_TO_RESTART_FRAMES && gPressToClose && OSIsHomeButtonMenuEnabled()) {
                OSEnableHomeButtonMenu(FALSE);
                closeGameFromHBM();
            }
            sHeldXForFramesKPAD[chan] = 0;
        }
    }
    if (error) {
        *error = realError;
    }
    return result;
}

WUPS_MUST_REPLACE_FOR_PROCESS(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead, WUPS_FP_TARGET_PROCESS_HOME_MENU);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(KPADReadEx, 0x2e4507F4, 0x0e4507F4, WUPS_FP_TARGET_PROCESS_HOME_MENU);
