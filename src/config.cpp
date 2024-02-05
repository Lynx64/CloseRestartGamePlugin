#include "config.h"
#include "globals.hpp"
#include "config/WUPSConfigItemCheckbox.h"
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <sysapp/launch.h>
#include <nn/act/client_cpp.h>
#include <coreinit/title.h>
#include <string_view>

WUPS_USE_STORAGE("CloseRestartGamePlugin");

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

void initConfig()
{
    // Open storage to read values
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        //failed to open storage - use default values
    } else {
        // Try to get value from storage
        if (WUPS_GetBool(nullptr, "gPressToClose", &gPressToClose) != WUPS_STORAGE_ERROR_SUCCESS) {
            // Add the value to the storage if it's missing
            gPressToClose = false;
            WUPS_StoreBool(nullptr, "gPressToClose", gPressToClose);
        }

        if (WUPS_GetBool(nullptr, "gHoldToRestart", &gHoldToRestart) != WUPS_STORAGE_ERROR_SUCCESS) {
            gHoldToRestart = true;
            WUPS_StoreBool(nullptr, "gHoldToRestart", gHoldToRestart);
        }

        // Close storage
        WUPS_CloseStorage();
    }
}

void checkboxItemChanged(ConfigItemCheckbox *item, bool newValue)
{
    if (item && item->configId) {
        if (std::string_view(item->configId) == "sCloseNow") {
            sCloseNow = newValue;
        } else if (std::string_view(item->configId) == "sRestartNow") {
            sRestartNow = newValue;
        } else if (std::string_view(item->configId) == "sSwitchUsers") {
            sSwitchUsers = newValue;
        }
    }
}

void boolItemCallback(ConfigItemBoolean *item, bool newValue)
{
    if (item && item->configId) {
        if (std::string_view(item->configId) == "gPressToClose") {
            gPressToClose = newValue;
            WUPS_StoreBool(nullptr, item->configId, gPressToClose);
        } else if (std::string_view(item->configId) == "gHoldToRestart") {
            gHoldToRestart = newValue;
            WUPS_StoreBool(nullptr, item->configId, gHoldToRestart);
        }
    }
}

WUPS_GET_CONFIG()
{
    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "Close/Restart Game");

    // Open the storage
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        //failed to open storage
        WUPSConfigCategoryHandle errorInfo1;
        WUPSConfig_AddCategoryByNameHandled(config, "Error opening storage", &errorInfo1);

        WUPSConfigCategoryHandle errorInfo2;
        WUPSConfig_AddCategoryByNameHandled(config, "Try deleting plugins/config/CloseRestartGamePlugin.json", &errorInfo2);
        return config;
    }

    //check that current title is not in blacklist
    bool validTitle = true;
    uint64_t titleId = OSGetTitleID();
    for (auto &blackId : TITLE_ID_BLACKLIST) {
        if (titleId == blackId) {
            validTitle = false;
            break;
        }
    }

    if (validTitle) {
        // Category: Game
        WUPSConfigCategoryHandle gameOptions;
        WUPSConfig_AddCategoryByNameHandled(config, "Game", &gameOptions);

        WUPSConfigItemCheckbox_AddToCategoryHandled(config, gameOptions, "sRestartNow", "Restart \ue08e", sRestartNow, &checkboxItemChanged);

        WUPSConfigItemCheckbox_AddToCategoryHandled(config, gameOptions, "sCloseNow", "Close \ue098", sCloseNow, &checkboxItemChanged);

        WUPSConfigItemCheckbox_AddToCategoryHandled(config, gameOptions, "sSwitchUsers", "Close and switch users \ue098", sSwitchUsers, &checkboxItemChanged);
    }

    // Category: HOME Menu settings
    WUPSConfigCategoryHandle homeMenuSettings;
    WUPSConfig_AddCategoryByNameHandled(config, "\ue073 Menu settings", &homeMenuSettings);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, homeMenuSettings, "gPressToClose", "Press \ue002 to close", gPressToClose, &boolItemCallback);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, homeMenuSettings, "gHoldToRestart", "Hold \ue002 to restart", gHoldToRestart, &boolItemCallback);

    return config;
}

WUPS_CONFIG_CLOSED()
{
    // Save all changes
    WUPS_CloseStorage();

    if (sCloseNow) {
        SYSLaunchMenu();
    } else if (sRestartNow) {
        SYSRelaunchTitle(0, 0);
    } else if (sSwitchUsers) {
        nn::act::SlotNo slot = nn::act::GetSlotNo();
        _SYSLaunchMenuWithCheckingAccount(slot);
    }
    sCloseNow = false;
    sRestartNow = false;
    sSwitchUsers = false;
}
