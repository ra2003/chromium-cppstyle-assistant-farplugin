﻿////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018, Ilya Kotelnikov (ilya.kotelnikov@gmail.com)
// All rights reserved.
//
// You may use, distribute and modify this code under the terms of GNU GPLv3.
//------------------------------------------------------------------------------

#pragma once

#include <windows.h>  // for COLORREF and RGB().

namespace cc_assistant {

////////////////////////////////////////////////////////////////////////////////
// Help topics.
//
extern const wchar_t kMenuHelpTopic[];
extern const wchar_t kConfigHelpTopic[];

////////////////////////////////////////////////////////////////////////////////
// Names and default values for plugin settings.
//
extern const wchar_t kFileMasksSettingName[];
extern const wchar_t kHighlightLineLimitColumnEnabledSettingName[];
extern const wchar_t kHighlightLineLimitColumnIndexSettingName[];
extern const wchar_t kHighlightLineLimitColumnForecolorSettingName[];
extern const wchar_t kHighlightLineLimitColumnBackcolorSettingName[];
extern const wchar_t kHighlightLineLimitColumnBackcolorIfTabsSettingName[];
extern const wchar_t kHighlightWhitespacesAtLineEndEnabledSettingName[];
extern const wchar_t kHighlightWhitespacesAtLineEndBackcolorSettingName[];

extern const wchar_t kFileMasksSettingDefault[];
constexpr int kHighlightLineLimitColumnEnabledSettingDefault = 1;
constexpr unsigned int kHighlightLineLimitColumnIndexSettingDefault = 80U;
// Define colors in hex as 0x00bbggrr dword value or use RGB() macro.
constexpr COLORREF kHighlightLineLimitColumnForecolorSettingDefault =
    RGB(0xff, 0xff, 0xff);  // White.
constexpr COLORREF kHighlightLineLimitColumnBackcolorSettingDefault =
    RGB(0x00, 0x00, 0x00);  // Black.
constexpr COLORREF kHighlightLineLimitColumnBackcolorIfTabsSettingDefault =
    RGB(0xff, 0x00, 0x00);  // Red.
constexpr int kHighlightWhitespacesAtLineEndEnabledSettingDefault = 1;
constexpr COLORREF kHighlightWhitespacesAtLineEndBackcolorSettingDefault =
    RGB(0xff, 0x00, 0xff);  // Magenta.

}  // namespace cc_assistant
