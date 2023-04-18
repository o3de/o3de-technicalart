/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SERVER_ID
#define SERVER_ID 1
#endif

typedef unsigned long long u64;
typedef unsigned int u32;

typedef long (*HandlerCallback)(u64, const char*, u64);


#define BRIDGE_EXPORT __declspec(dllexport)

__declspec(dllexport) bool Init(u64 id, HandlerCallback cb);
__declspec(dllexport) void Uninitialize();
__declspec(dllexport) void SendMsg(const char* data, u64 length, u64 id);
__declspec(dllexport) bool IsConnected();

// mostly for python use since callback doesn't work atm
__declspec(dllexport) u64 CheckForMsg();
__declspec(dllexport) bool ReadMsg(char* buffer, u64 length);

// map related
__declspec(dllexport) u64 RequestSHM(u64 uSize);
__declspec(dllexport) bool OpenSHM(u64 mapId);
__declspec(dllexport) bool ReadSHM(u64 uId, void** address, u64* length);
__declspec(dllexport) void WriteSHM(u64 uId, const char* source, const u64 length);
__declspec(dllexport) void ClearSHM(u64 uId);


#ifdef __cplusplus
}
#endif
