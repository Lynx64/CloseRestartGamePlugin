#include "WUPSConfigItemCheckbox.h"

std::optional<WUPSConfigItemCheckbox> WUPSConfigItemCheckbox::CreateEx(std::optional<std::string> identifier, std::string_view displayName, bool defaultValue, bool currentValue, CheckboxValueChangedCallback callback, std::string_view trueValue, std::string_view falseValue, WUPSConfigAPIStatus &err) noexcept {
    WUPSConfigItemHandle itemHandle;
    if ((err = WUPSConfigItemCheckbox_CreateEx(identifier ? identifier->data() : nullptr,
                                              displayName.data(),
                                              defaultValue, currentValue,
                                              callback,
                                              trueValue.data(),
                                              falseValue.data(),
                                              &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return std::nullopt;
    }
    return WUPSConfigItemCheckbox(itemHandle);
}

WUPSConfigItemCheckbox WUPSConfigItemCheckbox::CreateEx(std::optional<std::string> identifier, std::string_view displayName, bool defaultValue, bool currentValue, CheckboxValueChangedCallback callback, std::string_view trueValue, std::string_view falseValue) {
    WUPSConfigAPIStatus err = WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    auto result             = CreateEx(std::move(identifier), displayName, defaultValue, currentValue, callback, trueValue, falseValue, err);
    if (!result) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemCheckbox: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*result);
}

std::optional<WUPSConfigItemCheckbox> WUPSConfigItemCheckbox::Create(std::optional<std::string> identifier, std::string_view displayName, bool defaultValue, bool currentValue, CheckboxValueChangedCallback callback, WUPSConfigAPIStatus &err) noexcept {
    return CreateEx(std::move(identifier), displayName, defaultValue, currentValue, callback, "\u25C9", "\u25CB", err);
}

WUPSConfigItemCheckbox WUPSConfigItemCheckbox::Create(std::optional<std::string> identifier, std::string_view displayName, bool defaultValue, bool currentValue, CheckboxValueChangedCallback callback) {
    WUPSConfigAPIStatus err = WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    auto result             = Create(std::move(identifier), displayName, defaultValue, currentValue, callback, err);
    if (!result) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemCheckbox: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*result);
}
