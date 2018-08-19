﻿////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2018, Ilya Kotelnikov (ilya.kotelnikov@gmail.com)
// All rights reserved.
//
// You may use, distribute and modify this code under the terms of GNU GPLv3.
//------------------------------------------------------------------------------

#include <array>
#include <cstring>  // for std::memset
#include <vector>

#include <Plugin.hpp>
#include <PluginSettings.hpp>

#include "config_settings.hpp"
#include "constants.hpp"
#include "dlgbuilderex/plugin_dialog_builder_ex.hpp"
#include "guids.hpp"
#include "localized_strings_ids.hpp"
#include "version.hpp"

namespace {

constexpr intptr_t kNoEditorId = -2;
constexpr intptr_t kCurrentEditorId = -1;

PluginStartupInfo g_info;
FarStandardFunctions g_fsf;
cc_assistant::ConfigSettings g_opt;

const wchar_t* GetMsg(int msg_id) {
  return g_info.GetMsg(&cc_assistant::g_plugin_guid, msg_id);
}

}  // namespace

namespace cc_assistant {

std::wstring GetEditorFileName(intptr_t editor_id) {
  const size_t filename_buffer_size =
       g_info.EditorControl(editor_id, ECTL_GETFILENAME, 0, nullptr);
  if (filename_buffer_size == 0)
    return std::wstring();

  std::wstring filename(filename_buffer_size, 0);
  g_info.EditorControl(editor_id, ECTL_GETFILENAME,
                       filename_buffer_size, filename.data());
  filename.resize(filename_buffer_size - 1);  // wstring adds its own null char.
  return filename;
}

bool MatchEditorFileNameWithFileMasks(intptr_t editor_id,
                                      const std::wstring& file_masks) {
  if (file_masks.empty())  // trivial case.
    return true;

  const std::wstring filename = GetEditorFileName(editor_id);

  return g_info.FSF->ProcessName(
             file_masks.c_str(), const_cast<wchar_t*>(filename.c_str()), 0,
             PN_CMPNAMELIST | PN_SKIPPATH) != 0;
}

bool ValidateFileMasks(const std::wstring& file_masks) {
  return file_masks.empty() ||  // trivial case.
         g_info.FSF->ProcessName(file_masks.c_str(), nullptr, 0,
                                 PN_CHECKMASK | PN_SHOWERRORMESSAGE) != 0;
}

void ActualizePluginSettingsAndRedrawEditor(intptr_t editor_id) {
  PluginSettings far_settings_storage(g_plugin_guid, g_info.SettingsControl);
  g_opt.LoadFromFarStorage(far_settings_storage);

  // Redraw the editor to visualize updated plugin settings.
  if (editor_id != kNoEditorId)
    g_info.EditorControl(editor_id, ECTL_REDRAW, 0, 0);
}

bool ShowConfigDialog() {
  PluginDialogBuilderEx builder(g_info, g_plugin_guid, g_config_dialog_guid,
                                kMConfigTitle, kConfigHelpTopic);

  auto& hlcs = g_opt.highlight_linelimit_column_settings;

  // Add the main setting normally.
  builder.AddCheckbox(kMHighlightLineLimitColumnEnabledOption, &hlcs.enabled);
  builder.AddEmptyLine();  // subitems.push_back(builder.AddSeparatorEx());

  // Add it's subsettings aligned with the main checkbox label.
  std::vector<FarDialogItem*> subitems;

  // Add 'file masks' text field.
  constexpr int kFileMasksEditFieldWidth = 40;
  subitems.push_back(
      builder.AddText(kMHighlightLineLimitColumnFileMasksOption));
  subitems.push_back(builder.AddStringEditField(&hlcs.file_masks,
                                                kFileMasksEditFieldWidth));

  // Add a line with the color format hint and then edits for colors: fore, back
  // and back-if-tabs.
  FarDialogItem* color_format_hint_item = builder.AddText(L"rrggbb");

  FarDialogItem* forecolor_item = builder.AddColorEditField(&hlcs.forecolor);
  subitems.push_back(forecolor_item);
  subitems.push_back(builder.AddTextBefore(
      subitems.back(), kMHighlightLineLimitColumnForecolorOption));

  subitems.push_back(builder.AddColorEditField(&hlcs.backcolor));
  subitems.push_back(builder.AddTextBefore(
      subitems.back(), kMHighlightLineLimitColumnBackcolorOption));

  subitems.push_back(builder.AddColorEditField(&hlcs.backcolor_if_tabs));
  subitems.push_back(builder.AddTextBefore(
      subitems.back(), kMHighlightLineLimitColumnBackcolorIfTabsOption));

  builder.AddEmptyLine();

  // Add line-limit column index option.
  subitems.push_back(builder.AddUIntEditField(&hlcs.column_index, 3));
  subitems.push_back(builder.AddTextBefore(
     subitems.back(), kMHighlightLineLimitColumnIndexOption));

  // Finally shift all the subitems by 4 positions to right, so they are aligned
  // with the main checkbox label.
  for (FarDialogItem* item : subitems) {
    item->X1 += 4;
    item->X2 += 4;
  }

  // Then align the color format label with the color edit fields.
  const int color_format_hint_dx =
      forecolor_item->X1 - color_format_hint_item->X1;
  color_format_hint_item->X1 += color_format_hint_dx;
  color_format_hint_item->X2 += color_format_hint_dx;

  builder.AddOKCancel(kMSave, kMCancel);

  do {
    if (!builder.ShowDialog())
      return false;  // the dialog has been cancelled.

    // Ensure 'file masks' are correct before accepting the settings.
  } while (!ValidateFileMasks(hlcs.file_masks));

  // Save the updated plugin settings to the Far storage.
  PluginSettings far_settings_storage(g_plugin_guid, g_info.SettingsControl);
  g_opt.SaveToFarStorage(&far_settings_storage);
  return true;
}

// Show plugin menu. Returns chosen menu command index (except "Configure" which
// the function handles itself) or -1 if the menu has been cancelled.
int ShowMenuAndReturnChosenMenuIndex() {
  std::array<FarMenuItem, 1> menu_items;
  std::memset(menu_items.data(), 0, menu_items.size() * sizeof(FarMenuItem));
  menu_items[0].Text = GetMsg(kMConfigure);

  while(true) {
    const intptr_t menu_result_code =
        g_info.Menu(&g_plugin_guid, nullptr, -1, -1, 0, FMENU_WRAPMODE,
                    GetMsg(kMTitle), nullptr, kMenuHelpTopic, nullptr, nullptr,
                    menu_items.data(), menu_items.size());

    const int chosen_menu_index = static_cast<int>(menu_result_code);
    if (chosen_menu_index != 0)  // filter "Configure" item index.
      return chosen_menu_index;

    ShowConfigDialog();
  }
  return -1;
}

// Parse a string command passed by a Lua macro then convert to corresponding
// menu command id.
int GetCommandIdForMacroString(const wchar_t* macro_string_value) {
  int result_command_id = -1;

  auto check_known_command =
      [macro_string_value, &result_command_id](
          const wchar_t* known_command_name, int known_command_id) {
        if (lstrcmpi(macro_string_value, known_command_name) != 0)
          return false;

        result_command_id = known_command_id;
        return true;
      };

  check_known_command(L"Configure", 0);
  return result_command_id;
}

void HighlightLineLimitColumnIfEnabled(intptr_t editor_id) {
  auto& hlcs = g_opt.highlight_linelimit_column_settings;

  if (!hlcs.enabled)
    return;

  // TODO: add optimization here.
  if (!MatchEditorFileNameWithFileMasks(editor_id, hlcs.file_masks))
    return;

  EditorInfo editor_info = { sizeof(EditorInfo) };
  g_info.EditorControl(editor_id, ECTL_GETINFO, 0, &editor_info);

  // Optimization: do nothing if the column is out of screen at all.
  if (editor_info.LeftPos > hlcs.column_index ||
      editor_info.LeftPos + editor_info.WindowSizeX < hlcs.column_index) {
    return;
  }

  for (intptr_t i = 0; i < editor_info.WindowSizeY; ++i) {
    const intptr_t curr_visible_line_index = editor_info.TopScreenLine + i;
    if (curr_visible_line_index >= editor_info.TotalLines)
      break;

    // Watch for tabs in the line: ensure the target column is right.
    EditorConvertPos ecp = { sizeof(EditorConvertPos) };
    ecp.StringNumber = curr_visible_line_index;
    ecp.SrcPos = hlcs.column_index;
    g_info.EditorControl(editor_id, ECTL_TABTOREAL, 0, &ecp);
    const intptr_t adjusted_column_index = ecp.DestPos;
    const bool tabs_detected = (adjusted_column_index != hlcs.column_index);

    EditorColor ec = { sizeof(EditorColor) };
    ec.StringNumber = curr_visible_line_index;
    ec.ColorItem = -1;
    ec.StartPos = adjusted_column_index;
    ec.EndPos = adjusted_column_index;
    ec.Priority = EDITOR_COLOR_NORMAL_PRIORITY;
    ec.Flags = ECF_AUTODELETE;
    ec.Color.Flags = 0;
    ec.Color.ForegroundColor = hlcs.forecolor;
    ec.Color.BackgroundColor = (tabs_detected) ? hlcs.backcolor_if_tabs
                                               : hlcs.backcolor; 
    ec.Owner = g_plugin_guid;
    g_info.EditorControl(editor_id, ECTL_ADDCOLOR, 0, &ec);
  }
}

}  // namespace cc_assistant

////////////////////////////////////////////////////////////////////////////////
// Plugin DLL exported routines for calls made by Far.
// 

extern "C" void WINAPI GetGlobalInfoW(GlobalInfo* info) {
  info->StructSize = sizeof(GlobalInfo);
  info->MinFarVersion = FARMANAGERVERSION;
  info->Version = PLUGIN_VERSION;
  info->Guid = cc_assistant::g_plugin_guid;
  info->Title = PLUGIN_NAME;
  info->Description = PLUGIN_DESC;
  info->Author = PLUGIN_AUTHOR;
}

extern "C" void WINAPI GetPluginInfoW(PluginInfo* info) {
  info->StructSize = sizeof(PluginInfo);
  info->Flags = PF_EDITOR | PF_DISABLEPANELS;

  static const wchar_t* eternal_title_string = GetMsg(cc_assistant::kMTitle);
  info->PluginMenu.Guids = &cc_assistant::g_menu_guid;
  info->PluginMenu.Strings = &eternal_title_string;
  info->PluginMenu.Count = 1;

  info->PluginConfig.Guids = &cc_assistant::g_config_dialog_guid;
  info->PluginConfig.Strings = &eternal_title_string;
  info->PluginConfig.Count = 1;
}

extern "C" void WINAPI SetStartupInfoW(const PluginStartupInfo* info) {
  // Use a local copy of |info| as the object will be deleted after the call.
  g_info = (*info);
  g_fsf = (*info->FSF);
  g_info.FSF = &g_fsf;

  // Initialize plugin settings from the Far storage.
  cc_assistant::ActualizePluginSettingsAndRedrawEditor(kNoEditorId);
}

extern "C" intptr_t WINAPI ConfigureW(const ConfigureInfo* info) {
  // As it's an external call, firstly ensure we have the latest plugin
  // settings from the Far storage - they could be changed by another Far
  // instance.
  cc_assistant::ActualizePluginSettingsAndRedrawEditor(kCurrentEditorId);

  return cc_assistant::ShowConfigDialog() ? 1 : 0;
}

extern "C" HANDLE WINAPI OpenW(const OpenInfo* open_info) {
  // As it's an external call, firstly ensure we have the latest plugin
  // settings from the Far storage - they could be changed by another Far
  // instance.
  cc_assistant::ActualizePluginSettingsAndRedrawEditor(kCurrentEditorId);

  // Plugin is called to do an action: chose behaviour depending on where it
  // comes from.
  int command_id = -1;
  switch (open_info->OpenFrom) {
    // An editor wants to show the plugin menu.
    case OPEN_EDITOR:
      command_id = cc_assistant::ShowMenuAndReturnChosenMenuIndex();
      break;

    // Plugin cmd prefix was detected: do handle the command line.
    case OPEN_COMMANDLINE:
      break;

    // A Lua macro has called the plugin (e.g. a key shortcut was triggered). 
    case OPEN_FROMMACRO: {
        OpenMacroInfo* macro_info = (OpenMacroInfo*)open_info->Data;
        if (macro_info->Count > 0) {
          switch (macro_info->Values[0].Type) {
            case FMVT_INTEGER:
              command_id = static_cast<int>(macro_info->Values[0].Integer);
              break;

            case FMVT_DOUBLE:
              command_id = static_cast<int>(macro_info->Values[0].Double);
              break;

            case FMVT_STRING:
              command_id = cc_assistant::GetCommandIdForMacroString(
                  macro_info->Values[0].String);
              break;

            default:
              break;
          }
        }

        // "Configure" is the only macro command we support in the plugin.
        // Here handle it directly as ShowMenuAndReturnChosenMenuIndex() filters
        // it.
        if (command_id == 0) {
          cc_assistant::ShowConfigDialog();
        } else if (command_id < 0) {
          // If macro command id has not been determined then show plugin menu
          // to select a known command by UI.
          command_id = cc_assistant::ShowMenuAndReturnChosenMenuIndex();
        }
      }
      break;

    default:
      break;
  }

  // Handle all possible command ids except "Configure" one.
  // switch(command_id) {
  // }

  return (open_info->OpenFrom == OPEN_FROMMACRO) ? INVALID_HANDLE_VALUE
                                                 : nullptr;
}

extern "C" intptr_t WINAPI ProcessEditorEventW(
    const ProcessEditorEventInfo* info) {
  switch (info->Event) {
    case EE_REDRAW:
      cc_assistant::HighlightLineLimitColumnIfEnabled(info->EditorID);
      break;

    case EE_GOTFOCUS:
      // Actualize plugin settings from the Far storage as another plugin
      // instance (in another Far process) could change them while this
      // instance was in "background".
      cc_assistant::ActualizePluginSettingsAndRedrawEditor(info->EditorID);
      break;

    default:
      break;
  }

  return 0;
}

// Avoid loading into Far2.
extern "C" int WINAPI GetMinFarVersionW() {
  #define MAKEFARVERSION_IN_OLD_FORMAT(major, minor, build) \
      (((build) << 16) | (((major) & 0xff) << 8) | ((minor) & 0xff))

  return MAKEFARVERSION_IN_OLD_FORMAT(FARMANAGERVERSION_MAJOR,
                                      FARMANAGERVERSION_MINOR,
                                      FARMANAGERVERSION_BUILD);
}
