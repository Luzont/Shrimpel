/****************************************************************************************
 
   Copyright (C) 2026 Autodesk, Inc.
   All rights reserved.
 
   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.
 
****************************************************************************************/

/** \file fbxstdcompliant.h
* Macros to properly support the CRT secure functions. */
#ifndef _FBXSDK_CORE_ARCH_STDCOMPLIANT_H_
#define _FBXSDK_CORE_ARCH_STDCOMPLIANT_H_

#include <fbxsdk/fbxsdk_def.h>

#include <fbxsdk/fbxsdk_nsbegin.h>

#if defined(FBXSDK_ENV_WIN)
    #define FBXSDK_printf                            printf_s
    #define FBXSDK_fprintf                           fprintf_s
    inline int FBXSDK_sprintf(char* dst, size_t dstsize, const char* format, ...){ va_list vl; va_start(vl, format); int ret = vsprintf_s(dst, dstsize, format, vl); va_end(vl); return ret; }
    inline int FBXSDK_snprintf(char* dst, size_t dstsize, const char* format, ...){ va_list vl; va_start(vl, format); int ret = vsnprintf_s(dst, dstsize, _TRUNCATE, format, vl); va_end(vl); return ret; }
    inline int FBXSDK_vsprintf(char* dst, size_t dstsize, const char* format, va_list vl){ return vsprintf_s(dst, dstsize, format, vl); }
    inline int FBXSDK_vsnprintf(char* dst, size_t dstsize, const char* format, va_list vl){ return vsnprintf_s(dst, dstsize, _TRUNCATE, format, vl); }
    #define FBXSDK_stricmp(dst, src)                _stricmp(dst, src)
    #define FBXSDK_strnicmp(dst, src, count)        _strnicmp(dst, src, count)
    #define FBXSDK_strcpy(dst, size, src)            strcpy_s(dst, size, src)
    #define FBXSDK_strncpy(dst, size, src, count)    strncpy_s(dst, size, src, count)
    #define FBXSDK_strcat(dst, size, src)            strcat_s(dst, size, src)
    #define FBXSDK_strtok(str, delim, ctx)           strtok_s(str, delim, ctx)
    #define FBXSDK_wcscpy(dst, size, src)            wcscpy_s(dst, size, src)
    #define FBXSDK_wcscat(dst, size, src)            wcscat_s(dst, size, src)
#if !defined(FBXSDK_ENV_WINSTORE)
    #define FBXSDK_getpid                           _getpid
    #define FBXSDK_getcwd                           _getcwd
#else
    inline int FBXSDK_getpid(){ return 0; }
    inline char* FBXSDK_getcwd(char*,int){ return NULL; }
#endif
    #define FBXSDK_localtime(ptm, time)              { struct tm tms; ptm = &tms; localtime_s(ptm, time); }
    #define FBXSDK_gmtime(ptm, time)                 { struct tm tms; ptm = &tms; gmtime_s(ptm, time); }
    #define FBXSDK_fopen(fp, name, mode)             fopen_s(&fp, name, mode)

#elif defined(FBXSDK_ENV_MAC) || defined(FBXSDK_ENV_LINUX)
    #define FBXSDK_printf                            printf
    #define FBXSDK_fprintf                           fprintf
    inline int FBXSDK_sprintf(char* dst, size_t dstsize, const char* format, ...){ va_list vl; va_start(vl, format); int ret = vsnprintf(dst, dstsize, format, vl); va_end(vl); return ret; }
    inline int FBXSDK_snprintf(char* dst, size_t dstsize, const char* format, ...){ va_list vl; va_start(vl, format); int ret = vsnprintf(dst, dstsize, format, vl); va_end(vl); return ret; }
    inline int FBXSDK_vsprintf(char* dst, size_t dstsize, const char* format, va_list vl){ return vsnprintf(dst, dstsize, format, vl); }
    inline int FBXSDK_vsnprintf(char* dst, size_t dstsize, const char* format, va_list vl){ return vsnprintf(dst, dstsize, format, vl); }

    #define FBXSDK_stricmp(dst, src)                stricmp(dst, src)
    #define FBXSDK_strnicmp(dst, src, count)        strnicmp(dst, src, count)

    inline char* FBXSDK_strcpy(char* dst, size_t size, const char* src)
    {
        if (!dst || !src || size==0) return dst;
        memset(dst, 0, size);
        char* ret=strncpy(dst, src, size-1);
        dst[size-1]='\0';
        return ret;
    }

    inline char* FBXSDK_strncpy(char* dst, size_t size, const char* src, size_t count)
    {
        if (!dst || !src || size==0 || count==0) return dst;
        if (count >= size) { count = size - 1; }
        char* ret=strncpy(dst, src, count); 
        dst[count]='\0';
        return ret;
    }

    inline char* FBXSDK_strcat(char* dst, size_t size, const char* src)
    {
        if (!dst || !src || size==0) return dst;
        /* Bounded scan: strlen(dst) is unsafe if dst has no NUL within the buffer. */
        size_t dst_len = strnlen(dst, size);
        if (dst_len >= size) return nullptr;
        size_t avail = size - dst_len - 1;
        size_t src_len = strlen(src);
        if (src_len > avail) return nullptr;
        memcpy(dst + dst_len, src, src_len + 1);
        return dst;
    }

    inline wchar_t* FBXSDK_wcscpy(wchar_t* dst, size_t size, const wchar_t* src)
    {
        if (!dst || !src || size == 0) return dst;
        wmemset(dst, 0, size);
        wcsncpy(dst, src, size - 1);
        dst[size - 1] = L'\0';
        return dst;
    }

    inline wchar_t* FBXSDK_wcscat(wchar_t* dst, size_t size, const wchar_t* src)
    {
        if (!dst || !src || size == 0) return dst;
        size_t dst_len = wcsnlen(dst, size);
        if (dst_len >= size) return nullptr;
        size_t avail = size - dst_len - 1;
        size_t src_len = wcslen(src);
        if (src_len > avail) return nullptr;
        wmemcpy(dst + dst_len, src, src_len + 1);
        return dst;
    }

    #define FBXSDK_strtok(str, delim, ctx)           strtok_r(str, delim, ctx)
    #define FBXSDK_getpid                            getpid    
    #define FBXSDK_getcwd                            getcwd
    #define FBXSDK_localtime(tm, time)               tm=localtime(time)
    #define FBXSDK_gmtime(tm, time)                  tm=gmtime(time)
    #define FBXSDK_fopen(fp, name, mode)             fp=fopen(name, mode)

#else
    #error Unsupported platform!
#endif

#if defined(FBXSDK_ENV_WIN)
    #define FBXSDK_ftell64(fp)             _ftelli64(fp)
    #define FBXSDK_fseek64(fp, offs, mode) _fseeki64(fp, offs, mode)
#elif defined(FBXSDK_ENV_LINUX)  && !defined(FBXSDK_ENV_ANDROID)
    #define FBXSDK_ftell64(fp)              ftello64(fp)
    #define FBXSDK_fseek64(fp, offs, mode)  fseeko64(fp, offs, mode)
#else
    #define FBXSDK_ftell64(fp)              ftello(fp)
    #define FBXSDK_fseek64(fp, offs, mode)  fseeko(fp, (off_t)offs, mode)
#endif

#define FBXSDK_strdup                                FbxStrDup

//The scanf family functions cannot easily be used in both secure and non-secure versions because
//Microsoft's secure version expects the size of the string/char* arguments following their address.
//On Unix machines the scanf family functions do not have this behavior and trying to use the same
//calls would result in compiler errors because the arguments would not match the format string.
//Using the following macros in the code will simply desable the warning at compile time.
#if defined(FBXSDK_COMPILER_MSC) && (_MSC_VER >= 1300)
    #define FBXSDK_CRT_SECURE_NO_WARNING_BEGIN\
    {\
        __pragma(warning(push))\
        __pragma(warning(disable : 4996))\
    }
    
    #define FBXSDK_CRT_SECURE_NO_WARNING_END\
    {\
        __pragma(warning(pop))\
    }
#else
    #define FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
    #define FBXSDK_CRT_SECURE_NO_WARNING_END
#endif

#include <fbxsdk/fbxsdk_nsend.h>

#endif /* _FBXSDK_CORE_ARCH_STDCOMPLIANT_H_ */
