
#pragma once
#define USE_BRIDGE
#ifdef USE_BRIDGE
#include <Bridge.h>
#else
#ifndef SERVER_ID
#define SERVER_ID 1
#endif

typedef unsigned long long u64;
typedef unsigned int u32;

typedef long (*HandlerCallback)(u64, const char*, u64);
#endif
namespace GeomNodes
{
    namespace API
    {
        bool Init(u64 id, HandlerCallback cb);
        void Uninitialize();
        void SendMsg(const char* data, u64 length, u64 id);
        bool ReadMsg(char* buffer, u64 length);

        // map related
        bool OpenSHM(u64 mapId);
        bool ReadSHM(u64 uId, void** address, u64* length);
        void ClearSHM(u64 uId);
    } // namespace API
} // namespace GeomNodes