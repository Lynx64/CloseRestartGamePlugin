#include "WUPSConfigItemCheckbox.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <wups.h>

static int32_t WUPSConfigItemCheckbox_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemCheckbox *) context;
    snprintf(out_buf, out_size, "%s", item->value ? item->trueValue : item->falseValue);
    return 0;
}

static void WUPSConfigItemCheckbox_onCloseCallback(void *context) {
    auto *item = (ConfigItemCheckbox *) context;
    if (item->valueAtCreation != item->value && item->valueChangedCallback != nullptr) {
        ((CheckboxValueChangedCallback) (item->valueChangedCallback))(item, item->value);
    }
}

static void WUPSConfigItemCheckbox_onInput(void *context, WUPSConfigSimplePadData input) {
    auto *item = (ConfigItemCheckbox *) context;
    if (input.buttons_d & WUPS_CONFIG_BUTTON_A) {
        item->value = !item->value;
    }
}

static int32_t WUPSConfigItemCheckbox_getCurrentValueSelectedDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemCheckbox *) context;
    if (item->value) {
        snprintf(out_buf, out_size, "(Exit to confirm) %s", item->trueValue);
    } else {
        snprintf(out_buf, out_size, "%s", item->falseValue);
    }
    return 0;
}

static void WUPSConfigItemCheckbox_restoreDefault(void *context) {
    auto *item  = (ConfigItemCheckbox *) context;
    item->value = item->defaultValue;
}

static void WUPSConfigItemCheckbox_Cleanup(ConfigItemCheckbox *item) {
    if (!item) {
        return;
    }
    free((void *) item->identifier);
    free(item);
}

static void WUPSConfigItemCheckbox_onDelete(void *context) {
    WUPSConfigItemCheckbox_Cleanup((ConfigItemCheckbox *) context);
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemCheckbox_CreateEx(const char *identifier,
                                const char *displayName,
                                bool defaultValue,
                                bool currentValue,
                                CheckboxValueChangedCallback callback,
                                const char *trueValue,
                                const char *falseValue,
                                WUPSConfigItemHandle *outHandle) {
    if (outHandle == nullptr) {
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }
    auto *item = (ConfigItemCheckbox *) malloc(sizeof(ConfigItemCheckbox));
    if (item == nullptr) {
        return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
    }

    if (identifier != nullptr) {
        item->identifier = strdup(identifier);
    } else {
        item->identifier = nullptr;
    }

    item->defaultValue         = defaultValue;
    item->value                = currentValue;
    item->valueAtCreation      = currentValue;
    item->valueChangedCallback = (void *) callback;
    snprintf(item->trueValue, sizeof(item->trueValue), "%s", trueValue);
    snprintf(item->falseValue, sizeof(item->falseValue), "%s", falseValue);

    WUPSConfigAPIItemCallbacksV2 callbacks = {
            .getCurrentValueDisplay         = &WUPSConfigItemCheckbox_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &WUPSConfigItemCheckbox_getCurrentValueSelectedDisplay,
            .onSelected                     = nullptr,
            .restoreDefault                 = &WUPSConfigItemCheckbox_restoreDefault,
            .isMovementAllowed              = nullptr,
            .onCloseCallback                = &WUPSConfigItemCheckbox_onCloseCallback,
            .onInput                        = &WUPSConfigItemCheckbox_onInput,
            .onInputEx                      = nullptr,
            .onDelete                       = &WUPSConfigItemCheckbox_onDelete};

    WUPSConfigAPIItemOptionsV2 options = {
            .displayName = displayName,
            .context     = item,
            .callbacks   = callbacks};

    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(options, &item->handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        WUPSConfigItemCheckbox_Cleanup(item);
        return err;
    }

    *outHandle = item->handle;
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemCheckbox_AddToCategoryEx(WUPSConfigCategoryHandle cat,
                                       const char *identifier,
                                       const char *displayName,
                                       bool defaultValue,
                                       bool currentValue,
                                       CheckboxValueChangedCallback callback,
                                       const char *trueValue,
                                       const char *falseValue) {
    WUPSConfigItemHandle itemHandle;
    WUPSConfigAPIStatus res;
    if ((res = WUPSConfigItemCheckbox_CreateEx(identifier,
                                               displayName,
                                               defaultValue,
                                               currentValue,
                                               callback,
                                               trueValue,
                                               falseValue,
                                               &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return res;
    }

    if ((res = WUPSConfigAPI_Category_AddItem(cat, itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        WUPSConfigAPI_Item_Destroy(itemHandle);
        return res;
    }
    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemCheckbox_AddToCategory(WUPSConfigCategoryHandle cat,
                                     const char *identifier,
                                     const char *displayName,
                                     bool defaultValue,
                                     bool currentValue,
                                     CheckboxValueChangedCallback callback) {
    return WUPSConfigItemCheckbox_AddToCategoryEx(cat, identifier, displayName, defaultValue, currentValue, callback, "\u25C9", "\u25CB");
}
