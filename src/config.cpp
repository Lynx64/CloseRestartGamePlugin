#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include "config/WUPSConfigItemCheckbox.h"
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <sysapp/launch.h>
#include <nn/act/client_cpp.h>
#include <coreinit/title.h>
#include <string_view>

WUPS_USE_STORAGE("CloseRestartGamePlugin");

extern "C" void _SYSLaunchMenuFromHBM(void);
extern "C" void _SYSLaunchMenuWithCheckingAccountFromHBM(uint8_t slot);

const uint64_t TITLE_ID_BLACKLIST[] = {0x0005001010045000, // System Updater JPN
                                       0x0005001010045100, // System Updater USA
                                       0x0005001010045200, // System Updater EUR
                                       0x0005001010047000, // System Settings JPN
                                       0x0005001010047100, // System Settings USA
                                       0x0005001010047200, // System Settings EUR
                                       0x0005001010049000, // User Settings JPN
                                       0x0005001010049100, // User Settings USA
                                       0x0005001010049200, // User Settings EUR
                                       0x000500101004A000, // Mii Maker JPN
                                       0x000500101004A100, // Mii Maker USA
                                       0x000500101004A200, // Mii Maker EUR
                                       0x000500101004B000, // Account Settings JPN
                                       0x000500101004B100, // Account Settings USA
                                       0x000500101004B200, // Account Settings EUR
                                       0x0005001010062000, // Software Data Transfer JPN
                                       0x0005001010062100, // Software Data Transfer USA
                                       0x0005001010062200, // Software Data Transfer EUR
                                       0x0005001010040000, // Wii U Menu JPN
                                       0x0005001010040100, // Wii U Menu USA
                                       0x0005001010040200  // Wii U Menu EUR
                                       };

static bool sCloseNow = false;
static bool sRestartNow = false;
static bool sSwitchUsers = false;

void checkboxItemChanged(ConfigItemCheckbox *item, bool newValue)
{
    if (item && item->identifier) {
        if (std::string_view("sCloseNow") == item->identifier) {
            sCloseNow = newValue;
        } else if (std::string_view("sRestartNow") == item->identifier) {
            sRestartNow = newValue;
        } else if (std::string_view("sSwitchUsers") == item->identifier) {
            sSwitchUsers = newValue;
        }
    }
}

void boolItemCallback(ConfigItemBoolean *item, bool newValue)
{
    if (item && item->identifier) {
        if (std::string_view(PRESS_TO_CLOSE_CONFIG_ID) == item->identifier) {
            gPressToClose = newValue;
            WUPSStorageAPI::Store(item->identifier, gPressToClose);
        } else if (std::string_view(HOLD_TO_RESTART_CONFIG_ID) == item->identifier) {
            gHoldToRestart = newValue;
            WUPSStorageAPI::Store(item->identifier, gHoldToRestart);
        } else if (std::string_view(LAUNCH_MENU_DIRECT_CONFIG_ID) == item->identifier) {
            gLaunchMenuDirect = newValue;
            WUPSStorageAPI::Store(item->identifier, gLaunchMenuDirect);
        }
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        //check that current title is not in blacklist
        bool validTitle = true;
        uint64_t titleId = OSGetTitleID();
        for (auto &blackId : TITLE_ID_BLACKLIST) {
            if (titleId == blackId) {
                validTitle = false;
                break;
            }
        }

        // Category: Root
        if (validTitle) {
            root.add(WUPSConfigItemCheckbox::Create("sRestartNow",
                                                    "Restart \ue08e",
                                                    false,
                                                    sRestartNow,
                                                    &checkboxItemChanged));

            root.add(WUPSConfigItemCheckbox::Create("sCloseNow",
                                                    "Close \ue098",
                                                    false,
                                                    sCloseNow,
                                                    &checkboxItemChanged));

            root.add(WUPSConfigItemCheckbox::Create("sSwitchUsers",
                                                    "Close and switch users \ue098",
                                                    false,
                                                    sSwitchUsers,
                                                    &checkboxItemChanged));
        }
        
        // Category: HOME Menu settings
        auto homeMenuSettings = WUPSConfigCategory::Create("\ue073 Menu settings");

        homeMenuSettings.add(WUPSConfigItemBoolean::Create(PRESS_TO_CLOSE_CONFIG_ID,
                                                           "Press \ue002 to close",
                                                           DEFAULT_PRESS_TO_CLOSE_VALUE,
                                                           gPressToClose,
                                                           &boolItemCallback));

        homeMenuSettings.add(WUPSConfigItemBoolean::Create(HOLD_TO_RESTART_CONFIG_ID,
                                                           "Hold \ue002 to restart",
                                                           DEFAULT_HOLD_TO_RESTART_VALUE,
                                                           gHoldToRestart,
                                                           &boolItemCallback));

        // Category: Advanced
        auto advancedSettings = WUPSConfigCategory::Create("Advanced");

        advancedSettings.add(WUPSConfigItemBoolean::Create(LAUNCH_MENU_DIRECT_CONFIG_ID,
                                                           "Prefer \"LaunchMenuFromHBM\" functions",
                                                           DEFAULT_LAUNCH_MENU_DIRECT_VALUE,
                                                           gLaunchMenuDirect,
                                                           &boolItemCallback));

        if (validTitle) {
            auto settings = WUPSConfigCategory::Create("Settings");
            settings.add(std::move(homeMenuSettings));
            settings.add(std::move(advancedSettings));
            root.add(std::move(settings));
        } else {
            root.add(std::move(homeMenuSettings));
            root.add(std::move(advancedSettings));
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Exception: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
    // Save all changes
    WUPSStorageAPI::SaveStorage();

    if (sCloseNow) {
        if (gLaunchMenuDirect) {
            _SYSLaunchMenuFromHBM();
        } else {
            SYSLaunchMenu();
        }
    } else if (sRestartNow) {
        SYSRelaunchTitle(0, 0);
    } else if (sSwitchUsers) {
        if (gLaunchMenuDirect) {
            _SYSLaunchMenuWithCheckingAccountFromHBM(nn::act::GetSlotNo());
        } else {
            _SYSLaunchMenuWithCheckingAccount(nn::act::GetSlotNo());
        }
    }
    sCloseNow = false;
    sRestartNow = false;
    sSwitchUsers = false;
}

void initConfig()
{
    WUPSConfigAPIOptionsV1 configOptions = {.name = "Close/Restart Game"};
    WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);
    
    WUPSStorageAPI::GetOrStoreDefault(PRESS_TO_CLOSE_CONFIG_ID, gPressToClose, DEFAULT_PRESS_TO_CLOSE_VALUE);

    WUPSStorageAPI::GetOrStoreDefault(HOLD_TO_RESTART_CONFIG_ID, gHoldToRestart, DEFAULT_HOLD_TO_RESTART_VALUE);

    WUPSStorageAPI::GetOrStoreDefault(LAUNCH_MENU_DIRECT_CONFIG_ID, gLaunchMenuDirect, DEFAULT_LAUNCH_MENU_DIRECT_VALUE);
}
