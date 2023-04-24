/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once
#include <AzCore/IPC/SharedMemory.h>
#include <AzCore/std/containers/deque.h>
#include <AzCore/std/containers/queue.h>
#include <AzCore/std/containers/map.h>
#include <AzCore/std/function/function_template.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>
//#include <AzCore/std/containers/vector.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/parallel/thread.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/Memory/OSAllocator.h>

#ifndef WINAPI
#define WINAPI __stdcall
#endif

#ifndef SERVER_ID
#define SERVER_ID 1
#endif

namespace Ipc
{
    namespace Internal
    {
        struct RingData;
    } // namespace Internal

    constexpr auto IPC_MAX_PID      = 10;       // Max size of process ID table
    constexpr auto IPC_MAX_MSGS     = 100;      // Max number of messages stored in queue
    constexpr auto IPC_MAX_MSG_SIZE = 0x8000;   // Max size of of a message (32KB)
    constexpr auto IPC_MAX_MAP      = 20;       // Max number of simultaneous extended maps;
    constexpr auto IPC_MIN_MAP_SIZE = 0x100000; // min map size (1MB);
    constexpr auto MAP_PAGE_SIZE    = 4096;

    enum IPC_MSG_TYPE
    {
        IPC_MSG_JSON,               // normal messages
        IPC_MSG_SHM                 // Message is SHM map related
                                    // 
    };

    // Table of processes that uses this IPC object
    struct IpcIdTable
    {
        AZ::u64 pid[IPC_MAX_PID];
        AZ::s64 i64Poll[IPC_MAX_PID];
        AZ::u32 uiPrevMsgSequence[IPC_MAX_PID];
    };

    struct IpcMapTable
    {
        // a table of available maps in the SHM
        AZ::u64 id[IPC_MAX_MAP];       // owner? if 0 means it's free
        AZ::u64 mapID[IPC_MAX_MAP];    // a randomly generated ID, it could be a uuid or current tick.
        AZ::u64 uSize[IPC_MAX_MAP];    // map size; there should be no similar size. I don't think there will
                                       // be two users that will use a map at the same time
    };

    struct IpcMapSortArray
    {
        AZ::u32 arraySize;              // number of created SHM
        AZ::u32 sortArray[IPC_MAX_MAP]; // the actual sort array containing indexes mapping to IpcMapTable
                                        // sort order is ascending
    };

    // Structure of an element in the IPC_MESSAGE_TABLE
    struct IPCMessage
    {
        IPCMessage() = default;
        IPCMessage(AZ::u64 pID, AZ::u64 sID, AZ::u32 pType, AZ::u64 uSize, AZ::u32 uMsgSec = 0, AZ::s64 sTimeStamp = 0)
        {
            pid = pID;
            senderID = sID;
            uType = pType;
            uiMsgSize = uSize;
            uiMsgSequence = uMsgSec;
            i64Timestamp = sTimeStamp;
        }

        AZ::u64 pid = 0;
        AZ::u64 senderID = 0;
        AZ::u32 uType = 0;
        AZ::u32 uiMsgSequence = 0;
        AZ::u64 uiMsgSize = 0;
        AZ::s64 i64Timestamp = 0;
        AZ::u8 ucMsg[IPC_MAX_MSG_SIZE] = { 0 };
    };

    #define IPC_MESSAGE_DATA_SIZE 40 // 64+64+32+64+32+64

    // Table of messages to be processed by IPC
    struct IpcMessageTable
    {
        IPCMessage message[IPC_MAX_MSGS];
        // AZ::u32		crc[BRIPC_MAX_MESSAGES];
    };

    /*class IpcHandler
    {
    public:
        virtual void HandleMessage(AZ::u32 pType, AZ::u8* pData, AZ::u32 uSize) = 0;
    };*/

    /**
     * Same implementation as SharedMemoryRingBuffer but with a public m_info
     */
    class SHMRingBuffer : public AZ::SharedMemory
    {
        bool m_isSetup;

        SHMRingBuffer(const SHMRingBuffer& rhs);
        SHMRingBuffer& operator=(const SHMRingBuffer&);

    public:
        SHMRingBuffer();

        // If return true if we are create
        bool Create(const char* name, unsigned int size, bool openIfCreated = false);

        /// Maps to the created map. If size == 0 it will map the whole memory.
        bool Map(AccessMode mode = ReadWrite, unsigned int size = 0);
        bool UnMap();

        /// IMPORTANT: All functions below are UNSAFE. Don't forget to Lock/Unlock before/after using them.

        /// Returns true is we wrote the data, false if the free memory is insufficient.
        bool Write(const void* data, unsigned int dataSize);
        /// Reads data up to the maxDataSize, returns number of bytes red.
        unsigned int Read(void* data, unsigned int maxDataSize);

        unsigned int Read(void** data, unsigned int maxDataSize);

        /// Get number of bytes to read.
        unsigned int DataToRead() const;
        /// Get maximum data we can write.
        unsigned int MaxToWrite() const;
        /// Clears the ring buffer data and reset it to initial condition.
        void Clear();

        Internal::RingData* m_info;
    };

    class Ipc
    {
    public:
        typedef AZ::u32 IPCHandleType;
        typedef AZStd::deque<IPCMessage> IPCMessageQueue;
        typedef AZStd::map<AZ::u64, IPCMessageQueue> IPCMessageQueueMap;
        typedef long (*IPCHandler)(AZ::u64, const char*, AZ::u64);
        //typedef AZStd::unordered_map<AZ::u64, IPCHandler> IPCHandlerContainer;
        typedef AZStd::map<AZ::u32, AZ::u32> IPCMsgSequence;
        typedef AZStd::vector<AZ::u8> ByteStream;
        typedef AZStd::map<AZ::u64, SHMRingBuffer*> SHMMap;

        Ipc();
        virtual ~Ipc();

        void Initialize(AZ::u64 id = SERVER_ID, IPCHandler handler = nullptr);
        void Uninitialize();

        void SendMsg(AZ::u32 pType, const AZ::u8* pData, AZ::u64 uSize, AZ::u64 id = SERVER_ID);
        bool IsInitialized()
        {
            return m_success;
        }

        AZ::u64 CheckForMessage();
        bool ReadMessage(char* buffer, AZ::u64 length);

        // Map Functions
        AZ::u64 RequestSHM(AZ::u64 uSize);
        bool OpenSHM(AZ::u64 mapId);
        bool ReadSHM(AZ::u64 mapId, void** address, AZ::u64* length);
        void WriteSHM(AZ::u64 mapId, const char* source, const AZ::u64 length);
        void ClearSHM(AZ::u64 mapId);

        bool IsServerRunning();
        bool IsPeerAttached();
        
        static Ipc* GetInstance();
        static void DestroyInstance()
        {
            if (m_Instance != NULL)
            {
                delete m_Instance;
                m_Instance = NULL;
            }
        }

        void ProcessThread();

    protected:
    private:
        bool IsAttachedProcessId(AZ::u64 pid, bool bMutexLocked);
        void AddMessage(AZ::u64 pID, AZ::u32 pType, const AZ::u8* pData, AZ::u64 uSize, bool bMutexLocked);
        AZ::u32 GetPIDIdx(AZ::u64 pID);
        void PerformServerCleanup(AZStd::sys_time_t ts, AZStd::sys_time_t tLastCleanup);
        void PerformClientCheck(AZStd::sys_time_t ts);
        void ProcessOverlowQueue(AZStd::sys_time_t ts);
        void PollForMessages(IPCMsgSequence& mapMsgSequence);
        bool CreateSHMRingBuffer(AZ::u64 mapId, AZ::u64 uSize);

        static Ipc* m_Instance;

        bool m_bServer = false;
        bool m_success = false;
        AZStd::atomic_bool m_ShutdownThread = false;
        AZ::u64 m_uID = 0;
        AZ::u32 m_uIDIdx = 0;
        AZ::u32 m_uServerPID = 0;
        AZ::u32 m_uiPrevMsgSequence = 0; // The index of the IPC message that was just accomplished
        AZ::u32 m_uMsgAddIdx = 5;
        AZ::s64 m_uLastCmdTime = 0;

        AZ::SharedMemory m_SharedMem;   // IPC map
        SHMMap m_SharedMemMap;          // contains the large allocation of SHM.
                                        // server: eventually handles the management of all SHMs
                                        // client: if no available map for the requested size it will create it's own SHM and will not close it until the server has sent a message back to the client.

        AZStd::thread m_PollThread;

        IpcIdTable* m_ProcessIDs;       // Pointer to an instance of IPC_PID_TABLE
        IpcMessageTable* m_MsgTable;    // Pointer to an instance of IPC_MESSAGE_TABLE
        IpcMapTable* m_MapTable;        // Point to an instance of IPC_MAP_TABLE
        IpcMapSortArray* m_MapSortArray;
        IPCMessageQueueMap m_OverflowQueue; // Extra tables should the m_MsgTable be filled
        //IPCHandlerContainer m_Handlers;
        IPCHandler m_handler = nullptr;

        struct WaitingIPCMsg
        {
            AZ::u64 m_id;
            AZ::u32 m_type;
            ByteStream m_content;

            WaitingIPCMsg(AZ::u64 id, AZ::u32 msgType, const ByteStream& content)
                : m_id(id)
                , m_type(msgType)
                , m_content(content)
            {
            }
        };

        typedef AZStd::queue<WaitingIPCMsg> MessageContainer;
        MessageContainer m_MessagesWaitingToExecute;
        AZStd::mutex m_MessageListMutex;
    };

}

