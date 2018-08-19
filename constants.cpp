﻿////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018, Ilya Kotelnikov (ilya.kotelnikov@gmail.com)
// All rights reserved.
//
// You may use, distribute and modify this code under the terms of GNU GPLv3.
//------------------------------------------------------------------------------

#include "constants.hpp"

////////////////////////////////////////////////////////////////////////////////
// Help topics.
//
// Note: keep consistent with "distrib/Options*.hlf" files' contents.
// 
const wchar_t kMenuHelpTopic[] = L"Contents";
const wchar_t kConfigHelpTopic[] = L"Configure";

////////////////////////////////////////////////////////////////////////////////
// Names and default values for plugin settings.
//
const wchar_t kHighlightLineLimitColumnEnabledSettingName[] =
    L"HighlightLineLimitColumnEnabled";
const wchar_t kHighlightLineLimitColumnIndexSettingName[] =
    L"HighlightLineLimitColumnIndex";
const wchar_t kHighlightLineLimitColumnForecolorSettingName[] =
    L"HighlightLineLimitColumnForecolor";
const wchar_t kHighlightLineLimitColumnBackcolorSettingName[] =
    L"HighlightLineLimitColumnBackcolor";
const wchar_t kHighlightLineLimitColumnBackcolorIfTabsSettingName[] =
    L"HighlightLineLimitColumnBackcolorIfTabs";
const wchar_t kHighlightLineLimitColumnFileMasksSettingName[] =
    L"HighlightLineLimitColumnFileMasks";

const wchar_t kHighlightLineLimitColumnFileMasksSettingDefault[] =
    L"*.c;*.cc;*.cpp;*.h;*.hh;*.hpp;";
