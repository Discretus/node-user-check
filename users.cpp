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
    Napi::Array userNames; // Массив в который будут заноситься все имена пользователей

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

    // Вызывааем NetUserEnum чтобы получить локальные учетные записи
    nStatus = NetUserEnum((LPCWSTR)pszServerName,
                          dwLevel,
                          FILTER_NORMAL_ACCOUNT,
                          (LPBYTE *)&pBuf,
                          dwPrefMaxLen,
                          &dwEntriesRead,
                          &dwTotalEntries,
                          &dwResumeHandle);
    // Если вызов успешен
    if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
    {
        if ((pTmpBuf = pBuf) != NULL)
        {
            userNames = Napi::Array::New(env, dwEntriesRead); // Инициализируем массив длины равной количеству полученных пользователей
            for (i = 0; (i < dwEntriesRead); i++)             // Пробегаем по всем полученым пользователям
            {
                assert(pTmpBuf != NULL);

                if (pTmpBuf == NULL)
                {
                    fprintf(stderr, "An access violation has occurred\n");
                    break;
                }
                // Так как Napi::String::New не принимает wchar и wstring, то сначала переводим в wstring, а потом в u16string
                std::wstring userNameWstr(pTmpBuf->usri0_name);
                std::u16string userNameU16str(userNameWstr.begin(), userNameWstr.end());
                Napi::String userName = Napi::String::New(env, userNameU16str);
                userNames[i] = userName; // Добавляем пользователя в массив
                pTmpBuf++;
            }
        }
    }
    // Иначе выводим системную ошибку
    else
        fprintf(stderr, "A system error has occurred: %d\n", nStatus);
    // Освобождаем выделенный буфер
    if (pBuf != NULL)
    {
        NetApiBufferFree(pBuf);
        pBuf = NULL;
    }
    return userNames;
}

Napi::Object init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "getUserNames"), Napi::Function::New(env, getUserNames));
    return exports;
};

NODE_API_MODULE(users, init);