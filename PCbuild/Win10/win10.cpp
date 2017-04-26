#include "pch.h"
#include "win10.h"
#include <string>

using namespace Windows::Storage;
using namespace Windows::Foundation;

std::wstring win32ConvertUtf8ToW(const std::string &text, bool *resultSuccessful /* = NULL*/)
{
  if (text.empty())
  {
    if (resultSuccessful != NULL)
      *resultSuccessful = true;
    return L"";
  }
  if (resultSuccessful != NULL)
    *resultSuccessful = false;

  int bufSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1, NULL, 0);
  if (bufSize == 0)
    return L"";
  wchar_t *converted = new wchar_t[bufSize];
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1, converted, bufSize) != bufSize)
  {
    delete[] converted;
    return L"";
  }

  std::wstring Wret(converted);
  delete[] converted;

  if (resultSuccessful != NULL)
    *resultSuccessful = true;
  return Wret;
}

std::string win32ConvertWToUtf8(const std::wstring &text, bool *resultSuccessful /*= NULL*/)
{
  if (text.empty())
  {
    if (resultSuccessful != NULL)
      *resultSuccessful = true;
    return "";
  }
  if (resultSuccessful != NULL)
    *resultSuccessful = false;

  int bufSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.c_str(), -1, NULL, 0, NULL, NULL);
  if (bufSize == 0)
    return "";
  char * converted = new char[bufSize];
  if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.c_str(), -1, converted, bufSize, NULL, NULL) != bufSize)
  {
    delete[] converted;
    return "";
  }

  std::string ret(converted);
  delete[] converted;

  if (resultSuccessful != NULL)
    *resultSuccessful = true;
  return ret;
}


int putenv(const char* k, const char* v)
{
  if (k == NULL || v == NULL)
  {
    return -1;
  }

  bool result;
  std::string name(k);
  std::string value(v);

  std::wstring Wname(win32ConvertUtf8ToW(name, &result));
  if (!result)
    return -1;

  if (Wname.empty() || name.find('=') != std::wstring::npos)
    return -1;

  std::wstring Wvalue(win32ConvertUtf8ToW(value, &result));
  if (!result)
    return -1;

  ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
  auto values = localSettings->Values;

  Platform::String^ keyString = ref new Platform::String(Wname.c_str());
  Platform::String^ valueString = ref new Platform::String(Wvalue.c_str());

  return values->Insert(keyString, valueString) ? 0 : -1;
}

char* win10_getenv(const char* n)
{
  static std::string result;
  bool success;

  if (n == NULL)
  {
    return NULL;
  }

  std::string name(n);

  // check key
  if (!name.empty())
  {
    std::wstring Wname(win32ConvertUtf8ToW(name, &success));
    if (success)
    {
      Platform::String^ key = ref new Platform::String(Wname.c_str());

      ApplicationDataContainer^ localSettings = ApplicationData::Current->LocalSettings;
      auto values = localSettings->Values;

      if (values->HasKey(key))
      {
        auto value = safe_cast<Platform::String^>(values->Lookup(key));
        result = win32ConvertWToUtf8(std::wstring(value->Data()), &success);
        if (success)
        {
          return (char*)(result.c_str());
        }
      }
    }
  }

  return NULL;
}
