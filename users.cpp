#pragma comment(lib, "netapi32.lib")

#include <napi.h>
#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <string>
#include <lm.h>

Napi::Array getUserNames(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Array userNames;

    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTmpBuf;
    DWORD dwLevel = 0;
    DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;
    LPTSTR pszServerName = NULL;

    do
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
                    assert(pTmpBuf != NULL);

                    if (pTmpBuf == NULL)
                    {
                        fprintf(stderr, "An access violation has occurred\n");
                        break;
                    }
                    pTmpBuf++;
                    dwTotalCount++;
                }
                userNames = Napi::Array::New(env, dwTotalCount);
                pTmpBuf = pBuf;
                for (i = 0; (i < dwEntriesRead); i++)
                {
                    std::wstring userNameWstr(pTmpBuf->usri0_name);
                    std::u16string userNameU16str(wstr.begin(), wstr.end());
                    Napi::String userName = Napi::String::New(env, userNameU16str);
                    userNames[i] = userName;
                    pTmpBuf++;
                }
            }
        }
        else
            fprintf(stderr, "A system error has occurred: %d\n", nStatus);
        if (pBuf != NULL)
        {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    } while (nStatus == ERROR_MORE_DATA);
    if (pBuf != NULL)
        NetApiBufferFree(pBuf);
    return userNames;
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "getUserNames"), Napi::Function::New(env, getUserNames));
    return exports;
};

NODE_API_MODULE(users, init);