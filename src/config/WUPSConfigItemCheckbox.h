#pragma once

#include <wups.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigItemCheckbox {
    const char *identifier;
    WUPSConfigItemHandle handle;
    bool defaultValue;
    bool valueAtCreation;
    bool value;
    char trueValue[32];
    char falseValue[32];
    void *valueChangedCallback;
} ConfigItemCheckbox;

typedef void (*CheckboxValueChangedCallback)(ConfigItemCheckbox *, bool);

WUPSConfigAPIStatus
WUPSConfigItemCheckbox_CreateEx(const char *identifier,
                                const char *displayName,
                                bool defaultValue,
                                bool currentValue,
                                CheckboxValueChangedCallback callback,
                                const char *trueValue,
                                const char *falseValue,
                                WUPSConfigItemHandle *outHandle);

/**
 * @brief Adds a checkbox configuration item to the specified category.
 *
 * This function adds a checkbox configuration item to the given category. The item is displayed with a specified display name.
 * The default value and current value of the item are set to the provided values. A callback function is called whenever
 * the value of the item changes.
 *
 * @param cat The handle of the category to add the item to.
 * @param identifier Optional identifier for the item. Can be NULL.
 * @param displayName The display name of the item.
 * @param defaultValue The default value of the item.
 * @param currentValue The current value of the item.
 * @param callback A callback function that will be called when the config menu closes and the value of the item has been changed.
 * @return WUPSCONFIG_API_RESULT_SUCCESS if the item was added successfully.
 */
WUPSConfigAPIStatus
WUPSConfigItemCheckbox_AddToCategory(WUPSConfigCategoryHandle cat,
                                     const char *identifier,
                                     const char *displayName,
                                     bool defaultValue,
                                     bool currentValue,
                                     CheckboxValueChangedCallback callback);

/**
 * @brief Adds a checkbox configuration item to the specified category.
 *
 * This function adds a checkbox configuration item to the given category. The item is displayed with a specified display name.
 * The default value and current value of the item are set to the provided values. A callback function is called whenever
 * the value of the item changes.
 *
 * @param cat The handle of the category to add the item to.
 * @param identifier Optional identifier for the item. Can be NULL.
 * @param displayName The display name of the item.
 * @param defaultValue The default value of the item.
 * @param currentValue The current value of the item.
 * @param callback A callback function that will be called when the config menu closes and the value of the item has been changed.
 * @param trueValue The string representation of the true value.
 * @param falseValue The string representation of the false value.
 * @return WUPSCONFIG_API_RESULT_SUCCESS if the item was successfully added to the category.
 */
WUPSConfigAPIStatus
WUPSConfigItemCheckbox_AddToCategoryEx(WUPSConfigCategoryHandle cat,
                                       const char *identifier,
                                       const char *displayName,
                                       bool defaultValue,
                                       bool currentValue,
                                       CheckboxValueChangedCallback callback,
                                       const char *trueValue,
                                       const char *falseValue);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <optional>
#include <stdexcept>
#include <string>

class WUPSConfigItemCheckbox : public WUPSConfigItem {
public:
    static std::optional<WUPSConfigItemCheckbox> CreateEx(std::optional<std::string> identifier,
                                                          std::string_view displayName,
                                                          bool defaultValue,
                                                          bool currentValue,
                                                          CheckboxValueChangedCallback callback,
                                                          std::string_view trueValue,
                                                          std::string_view falseValue,
                                                          WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemCheckbox CreateEx(std::optional<std::string> identifier,
                                           std::string_view displayName,
                                           bool defaultValue,
                                           bool currentValue,
                                           CheckboxValueChangedCallback callback,
                                           std::string_view trueValue,
                                           std::string_view falseValue);

    static std::optional<WUPSConfigItemCheckbox> Create(std::optional<std::string> identifier,
                                                        std::string_view displayName,
                                                        bool defaultValue,
                                                        bool currentValue,
                                                        CheckboxValueChangedCallback callback,
                                                        WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemCheckbox Create(std::optional<std::string> identifier,
                                         std::string_view displayName,
                                         bool defaultValue,
                                         bool currentValue,
                                         CheckboxValueChangedCallback callback);

private:
    explicit WUPSConfigItemCheckbox(WUPSConfigItemHandle itemHandle) : WUPSConfigItem(itemHandle) {
    }
};

#endif
