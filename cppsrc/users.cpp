#pragma comment(lib, "netapi32.lib")

#include <napi.h>
#include <iostream>
#include <windows.h>
#include <string>
#include <lm.h>

Napi::Error newException(Napi::Env env, std::string text)
{
    std::cout << "Addon error. " + text << std::endl;
    return Napi::Error::New(env, text);
}

Napi::Boolean checkUserExists(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    if (info.Length() != 1) // Проверяем количество аргументов, если != 1 то выкидываем исключение
    {
        throw newException(env, "Wrong number of arguments.");
    }
    if (!info[0].IsString()) // Если в качестве аргумента была дана не строка, то выкидываем исключение
    {
        throw newException(env, "First argument must be a string.");
    }

    // Полученный из функции аргумент переводим в wchar_t*
    std::string nameStr = std::string(info[0].ToString());
    std::wstring nameWstr = std::wstring(nameStr.begin(), nameStr.end());
    const WCHAR *name = nameWstr.c_str();

    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTmpBuf;
    DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    NET_API_STATUS nStatus;
    LPTSTR pszServerName = NULL;

    do // Пока у нас есть пользователи, заносим их в буфер и проходим по нему
    {
        nStatus = NetUserEnum((LPCWSTR)pszServerName,
                              dwLevel,
                              FILTER_NORMAL_ACCOUNT,
                              (LPBYTE *)&pBuf,
                              dwPrefMaxLen,
                              &dwEntriesRead,
                              &dwTotalEntries,
                              &dwResumeHandle);
        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
        {
            if ((pTmpBuf = pBuf) != NULL)
            {
                for (i = 0; (i < dwEntriesRead); i++)
                {
                    if (pTmpBuf == NULL)
                    {
                        throw newException(env, "An access violation has occurred.");
                    }
                    if (std::wcscmp(pTmpBuf->usri0_name, name) == 0) // Если имя пользователя найдено, возвращаем true
                    {
                        return Napi::Boolean::New(env, true);
                    }
                    pTmpBuf++;
                }
            }
        }
        else
            throw newException(env, "A system error has occurred.");
        if (pBuf != NULL)
        {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    } while (nStatus == ERROR_MORE_DATA);
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);
    return Napi::Boolean::New(env, false);
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "checkUserExists"), Napi::Function::New(env, checkUserExists));
    return exports;
};

NODE_API_MODULE(users, init);