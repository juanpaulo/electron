// Copyright (c) 2014 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "atom/app/atom_content_client.h"

#include <string>
#include <vector>

#include "atom/common/atom_version.h"
#include "atom/common/chrome_version.h"
#include "atom/common/options_switches.h"
#include "base/command_line.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "content/public/common/content_constants.h"
#include "content/public/common/pepper_plugin_info.h"
#include "content/public/common/user_agent.h"
#include "ppapi/shared_impl/ppapi_permissions.h"
#include "url/url_constants.h"

namespace atom {

namespace {

content::PepperPluginInfo CreatePepperFlashInfo(const base::FilePath& path,
                                                const std::string& version) {
  content::PepperPluginInfo plugin;

  plugin.is_out_of_process = true;
  plugin.name = content::kFlashPluginName;
  plugin.path = path;
  plugin.permissions = ppapi::PERMISSION_ALL_BITS;

  std::vector<std::string> flash_version_numbers = base::SplitString(
      version, ".", base::TRIM_WHITESPACE, base::SPLIT_WANT_NONEMPTY);
  if (flash_version_numbers.size() < 1)
    flash_version_numbers.push_back("11");
  // |SplitString()| puts in an empty string given an empty string. :(
  else if (flash_version_numbers[0].empty())
    flash_version_numbers[0] = "11";
  if (flash_version_numbers.size() < 2)
    flash_version_numbers.push_back("2");
  if (flash_version_numbers.size() < 3)
    flash_version_numbers.push_back("999");
  if (flash_version_numbers.size() < 4)
    flash_version_numbers.push_back("999");
  // E.g., "Shockwave Flash 10.2 r154":
  plugin.description = plugin.name + " " + flash_version_numbers[0] + "." +
      flash_version_numbers[1] + " r" + flash_version_numbers[2];
  plugin.version = base::JoinString(flash_version_numbers, ".");
  content::WebPluginMimeType swf_mime_type(
      content::kFlashPluginSwfMimeType,
      content::kFlashPluginSwfExtension,
      content::kFlashPluginSwfDescription);
  plugin.mime_types.push_back(swf_mime_type);
  content::WebPluginMimeType spl_mime_type(
      content::kFlashPluginSplMimeType,
      content::kFlashPluginSplExtension,
      content::kFlashPluginSplDescription);
  plugin.mime_types.push_back(spl_mime_type);

  return plugin;
}

void ConvertStringWithSeparatorToVector(std::vector<std::string>* vec,
                                        const char* separator,
                                        const char* cmd_switch) {
  auto command_line = base::CommandLine::ForCurrentProcess();
  auto string_with_separator = command_line->GetSwitchValueASCII(cmd_switch);
  if (!string_with_separator.empty())
    *vec = base::SplitString(string_with_separator, separator,
                             base::TRIM_WHITESPACE,
                             base::SPLIT_WANT_NONEMPTY);
}

}  // namespace

AtomContentClient::AtomContentClient() {
}

AtomContentClient::~AtomContentClient() {
}

std::string AtomContentClient::GetProduct() const {
  return "Chrome/" CHROME_VERSION_STRING;
}

std::string AtomContentClient::GetUserAgent() const {
  return content::BuildUserAgentFromProduct(
      "Chrome/" CHROME_VERSION_STRING " "
      ATOM_PRODUCT_NAME "/" ATOM_VERSION_STRING);
}

void AtomContentClient::AddAdditionalSchemes(
    std::vector<url::SchemeWithType>* standard_schemes,
    std::vector<std::string>* savable_schemes) {
  std::vector<std::string> schemes;
  ConvertStringWithSeparatorToVector(&schemes, ",",
                                     switches::kRegisterStandardSchemes);
  if (!schemes.empty()) {
    for (const std::string& scheme : schemes)
      standard_schemes->push_back({scheme.c_str(), url::SCHEME_WITHOUT_PORT});
  }
  standard_schemes->push_back({"chrome-extension", url::SCHEME_WITHOUT_PORT});
}

void AtomContentClient::AddPepperPlugins(
    std::vector<content::PepperPluginInfo>* plugins) {
  auto command_line = base::CommandLine::ForCurrentProcess();
  auto flash_path = command_line->GetSwitchValuePath(
      switches::kPpapiFlashPath);
  if (flash_path.empty())
    return;

  auto flash_version = command_line->GetSwitchValueASCII(
      switches::kPpapiFlashVersion);

  plugins->push_back(
      CreatePepperFlashInfo(flash_path, flash_version));
}

void AtomContentClient::AddServiceWorkerSchemes(
    std::set<std::string>* service_worker_schemes) {
  std::vector<std::string> schemes;
  ConvertStringWithSeparatorToVector(&schemes, ",",
                                     switches::kRegisterServiceWorkerSchemes);
  if (!schemes.empty()) {
    for (const std::string& scheme : schemes)
      service_worker_schemes->insert(scheme);
  }
  service_worker_schemes->insert(url::kFileScheme);
}

}  // namespace atom
