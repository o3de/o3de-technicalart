/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include "Ipc.h"
#include <AzCore/std/time.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/containers/set.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/Entity.h> // we just use this for create a random u64 id
#include <AzCore/std/parallel/spin_mutex.h>

#define SHMEM_NAME "GNIPCSharedMemory"

namespace Ipc
{
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    // Shared Memory ring buffer
    //////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////
    namespace Internal
    {
        struct RingData
        {
            AZ::u32 m_readOffset;
            AZ::u32 m_writeOffset;
            AZ::u32 m_startOffset;
            AZ::u32 m_endOffset;
            AZ::u32 m_dataToRead;
            AZ::u8 m_pad[32 - sizeof(AZStd::spin_mutex)];
        };
    } // namespace Internal

    //=========================================================================
    // SHMRingBuffer
    // [4/29/2011]
    //=========================================================================
    SHMRingBuffer::SHMRingBuffer()
        : m_info(nullptr)
    {
    }

    //=========================================================================
    // Create
    // [4/29/2011]
    //=========================================================================
    bool SHMRingBuffer::Create(const char* name, unsigned int size, bool openIfCreated)
    {
        return SharedMemory::Create(name, size + sizeof(Internal::RingData), openIfCreated) != SharedMemory::CreateFailed;
    }

    //=========================================================================
    // Map
    // [4/28/2011]
    //=========================================================================
    bool SHMRingBuffer::Map(AccessMode mode, unsigned int size)
    {
        if (SharedMemory::Map(mode, size))
        {
            MemoryGuard l(*this);
            m_info = reinterpret_cast<Internal::RingData*>(m_data);
            m_data = m_info + 1;
            m_dataSize -= sizeof(Internal::RingData);
            if (m_info->m_endOffset == 0) // if endOffset == 0 we have never set the info structure, do this only once.
            {
                m_info->m_startOffset = 0;
                m_info->m_readOffset = m_info->m_startOffset;
                m_info->m_writeOffset = m_info->m_startOffset;
                m_info->m_endOffset = m_info->m_startOffset + m_dataSize;
                m_info->m_dataToRead = 0;
            }
            return true;
        }

        return false;
    }

    //=========================================================================
    // UnMap
    // [4/28/2011]
    //=========================================================================
    bool SHMRingBuffer::UnMap()
    {
        m_info = nullptr;
        return SharedMemory::UnMap();
    }

    //=========================================================================
    // Write
    // [4/28/2011]
    //=========================================================================
    bool SHMRingBuffer::Write(const void* data, unsigned int dataSize)
    {
        AZ_Warning(
            "AZSystem",
            !Platform::IsWaitFailed(),
            "You are writing the ring buffer %s while the Global lock is NOT locked! This can lead to data corruption!",
            m_name);
        AZ_Assert(m_info != nullptr, "You need to Create and Map the buffer first!");
        if (m_info->m_writeOffset >= m_info->m_readOffset)
        {
            unsigned int freeSpace = m_dataSize - (m_info->m_writeOffset - m_info->m_readOffset);
            // if we are full or don't have enough space return false
            if (m_info->m_dataToRead == m_dataSize || freeSpace < dataSize)
            {
                return false;
            }
            unsigned int copy1MaxSize = m_info->m_endOffset - m_info->m_writeOffset;
            unsigned int dataToCopy1 = AZStd::GetMin(copy1MaxSize, dataSize);
            if (dataToCopy1)
            {
                memcpy(reinterpret_cast<char*>(m_data) + m_info->m_writeOffset, data, dataToCopy1);
            }
            unsigned int dataToCopy2 = dataSize - dataToCopy1;
            if (dataToCopy2)
            {
                memcpy(
                    reinterpret_cast<char*>(m_data) + m_info->m_startOffset,
                    reinterpret_cast<const char*>(data) + dataToCopy1,
                    dataToCopy2);
                m_info->m_writeOffset = m_info->m_startOffset + dataToCopy2;
            }
            else
            {
                m_info->m_writeOffset += dataToCopy1;
            }
        }
        else
        {
            unsigned int freeSpace = m_info->m_readOffset - m_info->m_writeOffset;
            if (freeSpace < dataSize)
            {
                return false;
            }
            memcpy(reinterpret_cast<char*>(m_data) + m_info->m_writeOffset, data, dataSize);
            m_info->m_writeOffset += dataSize;
        }
        m_info->m_dataToRead += dataSize;

        return true;
    }

    //=========================================================================
    // Read
    // [4/28/2011]
    //=========================================================================
    unsigned int SHMRingBuffer::Read(void* data, unsigned int maxDataSize)
    {
        AZ_Warning(
            "AZSystem",
            !Platform::IsWaitFailed(),
            "You are reading the ring buffer %s while the Global lock is NOT locked! This can lead to data corruption!",
            m_name);

        if (m_info->m_dataToRead == 0)
        {
            return 0;
        }

        AZ_Assert(m_info != nullptr, "You need to Create and Map the buffer first!");
        unsigned int dataRead;
        if (m_info->m_writeOffset > m_info->m_readOffset)
        {
            unsigned int dataToRead = AZStd::GetMin(m_info->m_writeOffset - m_info->m_readOffset, maxDataSize);
            if (dataToRead)
            {
                memcpy(data, reinterpret_cast<char*>(m_data) + m_info->m_readOffset, dataToRead);
            }
            m_info->m_readOffset += dataToRead;
            dataRead = dataToRead;
        }
        else
        {
            unsigned int dataToRead1 = AZStd::GetMin(m_info->m_endOffset - m_info->m_readOffset, maxDataSize);
            if (dataToRead1)
            {
                maxDataSize -= dataToRead1;
                memcpy(data, reinterpret_cast<char*>(m_data) + m_info->m_readOffset, dataToRead1);
            }
            unsigned int dataToRead2 = AZStd::GetMin(m_info->m_writeOffset - m_info->m_startOffset, maxDataSize);
            if (dataToRead2)
            {
                memcpy(reinterpret_cast<char*>(data) + dataToRead1, reinterpret_cast<char*>(m_data) + m_info->m_startOffset, dataToRead2);
                m_info->m_readOffset = m_info->m_startOffset + dataToRead2;
            }
            else
            {
                m_info->m_readOffset += dataToRead1;
            }
            dataRead = dataToRead1 + dataToRead2;
        }
        m_info->m_dataToRead -= dataRead;

        return dataRead;
    }

    unsigned int SHMRingBuffer::Read(void** data, unsigned int maxDataSize)
    {
        AZ_Warning(
            "AZSystem",
            !Platform::IsWaitFailed(),
            "You are reading the ring buffer %s while the Global lock is NOT locked! This can lead to data corruption!",
            m_name);

        if (m_info->m_dataToRead == 0)
        {
            return 0;
        }

        AZ_Assert(m_info != nullptr, "You need to Create and Map the buffer first!");
        unsigned int dataRead = 0;
        if (m_info->m_writeOffset > m_info->m_readOffset)
        {
            unsigned int dataToRead = AZStd::GetMin(m_info->m_writeOffset - m_info->m_readOffset, maxDataSize);
            if (dataToRead)
            {
                *data = reinterpret_cast<char*>(m_data) + m_info->m_readOffset;
            }
            m_info->m_readOffset += dataToRead;
            dataRead = dataToRead;
        }
        AZ_Assert(m_info->m_writeOffset >= m_info->m_readOffset, "SHMRingBuffer: not enough data to read");

        m_info->m_dataToRead -= dataRead;

        return dataRead;
    }

    //=========================================================================
    // Read
    // [4/28/2011]
    //=========================================================================
    unsigned int SHMRingBuffer::DataToRead() const
    {
        return m_info ? m_info->m_dataToRead : 0;
    }

    //=========================================================================
    // Read
    // [4/28/2011]
    //=========================================================================
    unsigned int SHMRingBuffer::MaxToWrite() const
    {
        return m_info ? (m_dataSize - m_info->m_dataToRead) : 0;
    }

    //=========================================================================
    // Clear
    // [4/19/2013]
    //=========================================================================
    void SHMRingBuffer::Clear()
    {
        SharedMemory::Clear();
        if (m_info)
        {
            m_info->m_readOffset = m_info->m_startOffset;
            m_info->m_writeOffset = m_info->m_startOffset;
            m_info->m_dataToRead = 0;
        }
    }

    Ipc* Ipc::m_Instance = nullptr;
    
    Ipc::Ipc()
    {
        m_OverflowQueue.clear();
    }

    Ipc::~Ipc()
    {
        Uninitialize();
    }

    void Ipc::Initialize(AZ::u64 id, IPCHandler handler)
    {
        //MessageBox(NULL, L"Debug-WMain-Install", L"Debug", MB_OK);

        m_uID = id;
        m_handler = handler;

        bool bServer = id == SERVER_ID;

        if (m_handler == nullptr && m_bServer)
        {
            AZ_Warning("App", false, "GNIPC: Callback is not set. You'll miss messages..");
        }

        if (!m_success)
        {
            m_success = true;

            AZ::u32 memory_size = sizeof(IpcIdTable) + sizeof(IpcMessageTable) + sizeof(IpcMapTable) + sizeof(IpcMapSortArray);
            if (m_SharedMem.Create(SHMEM_NAME, memory_size, true) == AZ::SharedMemory::CreateFailed)
            {
                AZ_Warning("App", false, "GNIPC: Could not Open IPC buffer, open files will not work.");
                m_success = false;
            }

            if ((m_success) && (!m_SharedMem.Map()))
            {
                AZ_Warning("App", false, "GNIPC: Could not Map IPC buffer, open files will not work.");
                m_success = false;
                m_SharedMem.Close();
            }

            m_ProcessIDs = (IpcIdTable*)m_SharedMem.Data();
            m_MsgTable = (IpcMessageTable*)((AZ::u8*)m_ProcessIDs + sizeof(IpcIdTable));
            m_MapTable = (IpcMapTable*)((AZ::u8*)m_MsgTable + sizeof(IpcMessageTable));
            m_MapSortArray = (IpcMapSortArray*)((AZ::u8*)m_MapTable + sizeof(IpcMapTable));

            {
                m_SharedMem.lock();
                if (bServer && (m_ProcessIDs->pid[0] == 0))
                {
                    // we are the server
                    m_ProcessIDs->pid[0] = m_uID;
                    m_ProcessIDs->i64Poll[0] = AZStd::GetTimeNowSecond();
                    m_bServer = true;
                    m_uIDIdx = 0;
                }
                else
                {
                    // we are some other process (client);
                    for (AZ::s32 i = 1; i < IPC_MAX_PID; ++i)
                    {
                        if ((m_ProcessIDs->pid[i] == 0) || (m_ProcessIDs->pid[i] == m_uID) ||
                            !IsAttachedProcessId(m_ProcessIDs->pid[i], true))
                        {
                            m_ProcessIDs->pid[i] = m_uID;
                            m_ProcessIDs->i64Poll[i] = AZStd::GetTimeNowSecond();
                            m_uIDIdx = i;
                            break;
                        }
                    }
                }

                if (m_uIDIdx < IPC_MAX_PID)
                    m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx] = 0;

                m_SharedMem.unlock();
            }

            if (!m_bServer && (m_uIDIdx == 0))
            {
                AZ_Warning("App", false, "GNIPC: Unable to initialize IPC; PID table is full!");
                m_success = false;
                m_SharedMem.Close();
            }

            if (m_success)
            {
                // start the poll thread
                m_PollThread = AZStd::thread(AZStd::bind(&Ipc::ProcessThread, this));
            }
        }
    }

    void Ipc::Uninitialize()
    {
        m_ShutdownThread = true;
        if (m_success)
        {
            m_PollThread.join();

            {
                m_SharedMem.lock();
                // remove ourself from the table
                if (m_ProcessIDs->pid[m_uIDIdx] == m_uID)
                {
                    m_ProcessIDs->pid[m_uIDIdx] = 0;
                    m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx] = 0;
                    m_ProcessIDs->i64Poll[m_uIDIdx] = 0;
                }

                // remove all pending messages for us in the table
                for (AZ::u32 j = 0; j < IPC_MAX_MSGS; ++j)
                {
                    IPCMessage* message = &m_MsgTable->message[j];
                    if (message->pid == m_uID)
                    {
                        message->pid = 0;
                        // memset(message, 0, BRIPC_MESSAGE_DATA_SIZE);
                    }
                }

                m_SharedMem.unlock();
            }

            m_SharedMem.UnMap();
            m_SharedMem.Close();
        }

        m_handler = nullptr;
        m_OverflowQueue.clear();
        m_success = false;

        // free the SHMs
        for (auto it = m_SharedMemMap.begin(); it != m_SharedMemMap.end(); ++it)
        {
            if (it->second)
            {
                it->second->UnMap();
                it->second->Close();
                delete it->second;
            }
        }

        m_SharedMemMap.clear();
    }

    void Ipc::SendMsg(AZ::u32 pType, const AZ::u8* pData, AZ::u64 uSize, AZ::u64 id)
    {
        if (m_success)
        {
            if (uSize <= IPC_MAX_MSG_SIZE)
            {
                bool bMutexLocked = m_SharedMem.try_lock();
                bMutexLocked = m_bServer ? bMutexLocked : (m_uIDIdx != 0 ? bMutexLocked : false);
                AddMessage(id, pType, pData, uSize, bMutexLocked);
                
                if (bMutexLocked)
                {
                    m_SharedMem.unlock();
                }
            }
        }
    }

    AZ::u64 Ipc::CheckForMessage()
    {
        AZStd::unique_lock<AZStd::mutex> lock(m_MessageListMutex);
        AZ::u64 length = 0;
        if (m_MessagesWaitingToExecute.size())
        {
            auto msg = m_MessagesWaitingToExecute.front();
            length = msg.m_content.size();
            AZ_Assert(length > 0, "There should be no empty message");
            if (length == 0)
                m_MessagesWaitingToExecute.pop(); // pop empty message so there's no chance that queue gets stuck
        }
        return length;
    }

    bool Ipc::ReadMessage(char* buffer, AZ::u64 length)
    {
        // https://stackoverflow.com/questions/26277322/passing-arrays-with-ctypes
        //TODO: need to allocate memory and have another function to release the allocated memory.
        // or do the second method where we allocate the buffer in python.
        // modify CheckForMessage that will return the length of the needed buffer.
        AZStd::unique_lock<AZStd::mutex> lock(m_MessageListMutex);
        auto msg = m_MessagesWaitingToExecute.front();

        AZ_Assert(msg.m_content.size() == length, "Different message.");
        memcpy(buffer, msg.m_content.data(), length);
        m_MessagesWaitingToExecute.pop();
        return true;
    }

    // Map functions
    void sortBySize(AZ::u32* indexes, AZ::u64* sizes, AZ::u32 n)
    {
        std::sort(
            indexes,
            indexes + n,
            [&](AZ::u32 i, AZ::u32 j)
            {
                return sizes[i] < sizes[j];
            });
    }

    bool Ipc::CreateSHMRingBuffer(AZ::u64 mapId, AZ::u64 uSize)
    {
        auto shmInstance = aznew SHMRingBuffer;
        bool bSuccess = true;
        AZStd::string shmName = AZStd::string::format("%llu", mapId);
        if (!shmInstance->Create(shmName.c_str(), (unsigned int)uSize, true))
        {
            AZ_Warning("Ipc", false, "Could not Open SHM, open files will not work.!");
            bSuccess = false;
        }

        if ((m_success) && (!shmInstance->Map()))
        {
            AZ_Warning("App", false, "Could not Map SHM, open files will not work.");
            bSuccess = false;
            shmInstance->Close();
        }

        if (bSuccess)
        {
            // add the opened map in the SHM map
            m_SharedMemMap.insert(AZStd::make_pair(mapId, shmInstance));
        }

        return bSuccess;
    }

    AZ::u64 Ipc::RequestSHM(AZ::u64 uSize)
    {
        AZ::u64 mapId = 0;
        if (m_success && !m_bServer) // client only
        {
            {
                m_SharedMem.lock();
                // we always get one MAP_PAGE_SIZE bigger than uSize's whole number.
                auto alignedSize = ((uSize / MAP_PAGE_SIZE) + 1) * MAP_PAGE_SIZE;

                // look for a free SHM or create one
                for (AZ::u32 i = 0; i < m_MapSortArray->arraySize; i++)
                {
                    auto idx = m_MapSortArray->sortArray[i];
                    if (m_MapTable->id[idx] == 0 && alignedSize <= m_MapTable->uSize[idx])
                    {
                        mapId = m_MapTable->mapID[idx];
                        m_MapTable->id[idx] = m_uID; // set the id to the this instance m_uID to assign it.
                        break;
                    }
                }
                
                if (mapId == 0) // if we are still zero here we create one
                {
                    mapId = (AZ::u64)AZ::Entity::MakeId();

                    auto mapTblidx = m_MapSortArray->arraySize; // 
                    m_MapTable->id[mapTblidx] = m_uID;
                    m_MapTable->mapID[mapTblidx] = mapId;
                    m_MapTable->uSize[mapTblidx] = alignedSize;
                    m_MapSortArray->sortArray[mapTblidx] = mapTblidx; // add the map table index to the sort Array
                    m_MapSortArray->arraySize += 1;

                    AZ_Assert(m_MapSortArray->arraySize <= IPC_MAX_MAP, "Map table is more than IPC_MAX_MAP(%i)", IPC_MAX_MAP);
                    // sort the array
                    sortBySize(m_MapSortArray->sortArray, m_MapTable->uSize, m_MapSortArray->arraySize);
                }

                m_SharedMem.unlock();

                if (mapId > 0) // create or open the SHM
                {
                    CreateSHMRingBuffer(mapId, alignedSize);
                }
                else
                {
                    AZ_Error("Ipc", false, "Wasn't able to create or open an existing Map!");
                }
            }
            /*else
            {
                AZ_Error("Ipc", false, "RequestSHM: failed to lock the SHM!");
            }*/
        }

        AZ_Assert(mapId != 0, "RequestSHM : there should be no zero map Id!");

        return mapId; //  if we get zero that means there's an error
    }

    bool Ipc::OpenSHM(AZ::u64 mapId)
    {
        AZ_Assert(mapId != 0, "Opening an SHM with a zero map Id.");
        bool bSuccess = true;
        if (m_success && m_bServer)
        {
            {
                m_SharedMem.lock();
                AZ::u64 uMapSize = 0;
                // look for the mapId and get the map size
                for (AZ::u32 i = 0; i < m_MapSortArray->arraySize; i++)
                {
                    if (m_MapTable->mapID[i] == mapId) // look for the mapID
                    {
                        uMapSize = m_MapTable->uSize[i];
                        break;
                    }
                }
                m_SharedMem.unlock();

                AZ_Assert(uMapSize != 0, "Opening an SHM with a zero sized map");

                bSuccess = CreateSHMRingBuffer(mapId, uMapSize);
            }
        }

        return bSuccess;
    }

    bool Ipc::ReadSHM(AZ::u64 mapId, void** address, AZ::u64* length)
    {
        auto shmIter = m_SharedMemMap.find(mapId);
        if (shmIter != m_SharedMemMap.end())
        {
            auto shmInstance = shmIter->second;
            {
                AZ::SharedMemory::MemoryGuard g(*shmInstance);
                int chunkSize;
                const int headerSize = sizeof(int);
                shmInstance->Read(&chunkSize, headerSize);
                if (chunkSize > 0)
                {
                    auto dataRead = shmInstance->Read(address, chunkSize - headerSize);
                    
                    if (length)
                        *length = dataRead;

                    return true;
                }
                else
                {
                    *length = 0;
                }
            }
        }
        return false;
    }

    void Ipc::WriteSHM(AZ::u64 mapId, const char* source, const AZ::u64 length)
    {
        auto shmIter = m_SharedMemMap.find(mapId);
        if (shmIter != m_SharedMemMap.end())
        {
            auto shmInstance = shmIter->second;
            {
                AZ::SharedMemory::MemoryGuard g(*shmInstance);
                const int headerSize = sizeof(int);
                const int chunkSize = (int)length + headerSize;

                shmInstance->Write(&chunkSize, headerSize);
                shmInstance->Write(source, (AZ::u32)length);
            }
        }
    }

    void Ipc::ClearSHM(AZ::u64 mapId)
    {
        if (m_success)
        {
            auto shmIter = m_SharedMemMap.find(mapId);
            if (shmIter != m_SharedMemMap.end())
            {
                auto shmInstance = shmIter->second;
                if (!m_bServer)
                {
                    shmInstance->UnMap();
                    shmInstance->Close();

                    m_SharedMem.lock();
                    for (AZ::u32 i = 0; i < m_MapSortArray->arraySize; i++)
                    {
                        if (m_MapTable->mapID[i] == mapId) // look for the mapID
                        {
                            m_MapTable->id[i] = 0; // clear the Id so it can be seen as "free"
                            break;
                        }
                    }
                    m_SharedMem.unlock();
                    m_SharedMemMap.erase(shmIter);
                }
                else
                {
                    shmInstance->lock();
                    shmInstance->Clear();
                    shmInstance->unlock();
                }
            }
        }
    }

    bool Ipc::IsServerRunning()
    {
        if (m_bServer)
            return true;

        if (m_ProcessIDs)
        {
            AZ::s64 tNow = AZStd::GetTimeNowSecond();
            if (tNow - m_ProcessIDs->i64Poll[0] < 10)
                return true;
        }

        return false;
    }

    bool Ipc::IsPeerAttached()
    {
        if (m_bServer)
        {
            if (m_ProcessIDs)
            {
                for (AZ::u32 i = 1; i < IPC_MAX_PID; ++i)
                {
                    if ((AZStd::GetTimeNowSecond() - m_ProcessIDs->i64Poll[i]) < 15)
                    {
                        if (m_ProcessIDs->i64Poll[i] != 0)
                            return true;
                    }
                }
            }
            return false;
        }
        else
            return IsServerRunning();
    }

    Ipc* Ipc::GetInstance()
    {
        if (m_Instance == NULL)
        {
            m_Instance = new Ipc;
        }
        return m_Instance;
    }

    void Ipc::PerformServerCleanup(AZStd::sys_time_t ts, AZStd::sys_time_t tLastCleanup)
    {
        if ((ts - tLastCleanup) >= 15)
        {
            tLastCleanup = ts;

            AZStd::set<AZ::u64> pidForCleanup;
            if (m_ProcessIDs)
            {
                for (AZ::u32 i = 1; i < IPC_MAX_PID; ++i)
                {
                    if ((m_ProcessIDs->i64Poll[i] != 0) && ((ts - m_ProcessIDs->i64Poll[i]) > 15))
                    {
                        // clean up the data if stagnant
                        pidForCleanup.insert(m_ProcessIDs->pid[i]);
                        m_ProcessIDs->pid[i] = 0;
                        m_ProcessIDs->i64Poll[i] = 0;
                    }
                }
            }

            if (m_MsgTable)
            {
                // remove all stagnant messages
                for (AZ::u32 j = 0; j < IPC_MAX_MSGS; ++j)
                {
                    IPCMessage* message = &m_MsgTable->message[j];
                    if ((pidForCleanup.find(message->pid) != pidForCleanup.end()) ||
                        ((message->pid > 0) && (message->i64Timestamp != 0) && ((ts - message->i64Timestamp) > 30)))
                    {
                        message->pid = 0;
                    }
                }
            }

            if (m_MapTable)
            {
                // clean the SHMs as well they could still be assigned to dead processes
                for (AZ::u32 j = 0; j < IPC_MAX_MAP; ++j)
                {
                    auto id = m_MapTable->id[j];
                    if ((pidForCleanup.find(id) != pidForCleanup.end()))
                    {
                        m_MapTable->id[j] = 0;
                    }
                }
            }
        }
    }

    void Ipc::PerformClientCheck(AZStd::sys_time_t ts)
    {
        if (m_ProcessIDs->pid[m_uIDIdx] == 0)
        {
            m_ProcessIDs->pid[m_uIDIdx] = m_uID; // reassert our ownership of the slot
        }
        else if (m_ProcessIDs->pid[m_uIDIdx] != m_uID)
        {
            // someone took our slot; we probably fell asleep too long
            m_uIDIdx = 0;
            {
                m_SharedMem.lock();
                for (int32_t i = 1; i < IPC_MAX_PID; ++i)
                {
                    if ((m_ProcessIDs->pid[i] == 0) || (m_ProcessIDs->pid[i] == m_uID) || !IsAttachedProcessId(m_ProcessIDs->pid[i], true))
                    {
                        m_ProcessIDs->pid[i] = m_uID;
                        m_ProcessIDs->i64Poll[i] = ts;
                        m_uIDIdx = i;
                        break;
                    }
                }
                m_SharedMem.unlock();
            }
        }
    }

    void Ipc::ProcessOverlowQueue(AZStd::sys_time_t ts)
    {
        if (!m_OverflowQueue.empty())
        {
            {
                m_SharedMem.lock();
                // process overflow queue here
                bool bAdded(false);
                AZ::u32 pIDx, nMsgIndex, nInitialIdx(m_uMsgAddIdx);
                AZ::u64 uPID;

                // iterate through each queue present in the map
                IPCMessageQueueMap::iterator mapIter = m_OverflowQueue.begin();
                while (mapIter != m_OverflowQueue.end())
                {
                    IPCMessageQueue& tQueue = mapIter->second;

                    while (!tQueue.empty())
                    {
                        if (!IsAttachedProcessId(tQueue.front().pid, true))
                        {
                            // this message is for an inactive PID; discard
                            tQueue.pop_front();
                            continue;
                        }

                        pIDx = GetPIDIdx(tQueue.front().pid);
                        bAdded = false;
                        nMsgIndex = (pIDx * 10) + m_uMsgAddIdx;
                        uPID = m_MsgTable->message[nMsgIndex].pid;

                        if ((uPID == 0) || !IsAttachedProcessId(uPID, true) || ((ts - m_MsgTable->message[nMsgIndex].i64Timestamp) > 30))
                        {
                            // Place the queued msg into the shared message table...
                            IPCMessage tOverflowMsg = tQueue.front();
                            AZ::u32 idIdx = GetPIDIdx(tOverflowMsg.pid);
                            tOverflowMsg.uiMsgSequence = m_ProcessIDs->uiPrevMsgSequence[idIdx] + 1;
                            tOverflowMsg.i64Timestamp = ts;

                            IPCMessage* tMsg = &m_MsgTable->message[nMsgIndex];
                            memcpy(tMsg, &tOverflowMsg, IPC_MESSAGE_DATA_SIZE + tOverflowMsg.uiMsgSize);

                            ++m_ProcessIDs->uiPrevMsgSequence[idIdx];

                            tQueue.pop_front();
                            bAdded = true;

                            // NOTE:  Don't use AddMessage() here because it will push right back onto the overflow queue (infinite loop).
                        }

                        ++m_uMsgAddIdx;
                        if (m_uMsgAddIdx > IPC_MAX_PID - 1)
                            m_uMsgAddIdx = 0;

                        if (m_uMsgAddIdx == nInitialIdx)
                        {
                            AZ_Warning(
                                "App",
                                false,
                                "GNIPC: Unable to find room for message from overflow queue; remaining queued messages[%u]!\n",
                                tQueue.size());
                            break;
                        }

                        if (bAdded)
                            nInitialIdx = m_uMsgAddIdx;
                    }

                    if (tQueue.empty())
                    {
                        mapIter = m_OverflowQueue.erase(mapIter); // empty ID
                    }
                    else
                    {
                        ++mapIter;
                    }
                    
                }

                m_SharedMem.unlock();
            }
        }
    }

    void Ipc::PollForMessages(IPCMsgSequence& mapMsgSequence)
    {
        IpcMessageTable* tMsgList = m_MsgTable;
        for (AZ::u32 i = 0; i < IPC_MAX_MSGS; ++i)
        {
            IPCMessage* tMsg = &tMsgList->message[i];
            if (tMsg->pid == m_uID)
            {
                mapMsgSequence[tMsg->uiMsgSequence] = i;
            }
        }

        if (mapMsgSequence.empty())
        {
            // process table says we have a new message, but PID entry doesn't display any message for us
            for (AZ::u32 i = 0; i < IPC_MAX_MSGS; ++i)
            {
                IPCMessage* tMsg = &tMsgList->message[i];
                if ((tMsg->uiMsgSequence <= m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx]) && (tMsg->uiMsgSequence > m_uiPrevMsgSequence))
                {
                    mapMsgSequence[tMsg->uiMsgSequence] = i;
                }
            }
        }
    }

    void Ipc::ExecuteIPCHandlers()
    {
        /*MessageContainer batch;
        {
            AZStd::lock_guard<AZStd::recursive_mutex> guard(m_MessageListMutex);
            batch = AZStd::move(m_MessagesWaitingToExecute);
        }*/

        // iterate through the messages
        AZStd::unique_lock<AZStd::mutex> lock(m_MessageListMutex);

        while (!m_MessagesWaitingToExecute.empty())
        {
            if (!m_handler) break; // no handlers

            auto msg = m_MessagesWaitingToExecute.front();
            if (!m_bServer)
            {
                m_handler(0, "", 0);
            }
            else
            {
                m_handler(msg.m_id, (const char*)msg.m_content.data(), msg.m_content.size());
            }

            m_MessagesWaitingToExecute.pop();
        }
    }

    void Ipc::ProcessThread()
    {
        // AZ::u32 tCounter = 0;
        auto tLastCleanup = AZStd::GetTimeNowSecond();
        while (!m_ShutdownThread)
        {
            auto ts = AZStd::GetTimeNowSecond();

            if (m_bServer)
            {
                PerformServerCleanup(ts, tLastCleanup);
            }
            else
            {
                PerformClientCheck(ts);

                if (m_uIDIdx == 0)
                {
                    // we failed to recover a slot to use; sleep for now
                    AZ_Warning("App", false, "GNIPC: Unable to recover client slot; sleeping\n");
                    AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(10));
                    continue;
                }
            }

            m_ProcessIDs->i64Poll[m_uIDIdx] = ts;

            ProcessOverlowQueue(ts);

            // don't poll if the message index hasn't changed
            if (m_ProcessIDs && (m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx] > m_uiPrevMsgSequence))
            {
                m_uLastCmdTime = AZStd::GetTimeNowSecond();

                // poll here
                IPCMsgSequence mapMsgSequence;
                PollForMessages(mapMsgSequence);

                if (!mapMsgSequence.empty())
                {
                    IpcMessageTable* tMsgList = m_MsgTable;
                    ByteStream contents;
                    AZ::u64 uBuffSize;

                    IPCMsgSequence::iterator iter;
                    for (iter = mapMsgSequence.begin(); iter != mapMsgSequence.end(); ++iter)
                    {
                        IPCMessage* tMsg = &tMsgList->message[iter->second];
                        uBuffSize = AZStd::min(tMsg->uiMsgSize, (size_t)IPC_MAX_MSG_SIZE);
                        if (uBuffSize > 0)
                        {
                            contents.resize_no_construct(uBuffSize);
                            memcpy(contents.data(), tMsg->ucMsg, uBuffSize);

                            {
                                AZStd::unique_lock<AZStd::mutex> lock(m_MessageListMutex);
                                m_MessagesWaitingToExecute.push(WaitingIPCMsg(tMsg->senderID, tMsg->uType, contents));
                            }
                        }

                        // remove the message
                        tMsg->pid = 0; // should be faster than memset
                        tMsg->senderID = 0;

                        ++m_uiPrevMsgSequence;
                    }

                    m_uiPrevMsgSequence = m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx];

                    {
                        AZStd::unique_lock<AZStd::mutex> lock(m_MessageListMutex);

                        while (!m_MessagesWaitingToExecute.empty())
                        {
                            if (!m_handler)
                                break; // no handlers

                            auto msg = m_MessagesWaitingToExecute.front();
                            if (!m_bServer)
                            {
                                m_handler(SERVER_ID, "", 0);
                            }
                            else
                            {
                                m_handler(msg.m_id, (const char*)msg.m_content.data(), msg.m_content.size());
                            }

                            m_MessagesWaitingToExecute.pop();
                        }
                    }
                    
                    //ExecuteIPCHandlers();
                    //EBUS_QUEUE_FUNCTION(AZ::SystemTickBus, &Ipc::ExecuteIPCHandlers, this);
                }
                else
                {
                    // still empty; wait for the shared memory segment to refresh(we might be hammerring it)
                    AZ_Warning(
                        "App",
                        false,
                        "GNIPC: Unable to find message going to seq:%u; sleeping...\n",
                        m_ProcessIDs->uiPrevMsgSequence[m_uIDIdx]);
                    AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(10));
                }
            }
            /*else
            {
                if (!IsPeerAttached() || ((AZStd::GetTimeNowSecond() - m_uLastCmdTime) > 10))
                    AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(10));

                AZStd::this_thread::sleep_for(AZStd::chrono::milliseconds(10));
            }*/
        }
    }

    bool Ipc::IsAttachedProcessId(AZ::u64 pid, bool bMutexLocked)
    {
        if (pid == 0)
            return false;

        if (m_ProcessIDs)
        {
            for (AZ::u32 i = 0; i < IPC_MAX_PID; ++i)
            {
                if (m_ProcessIDs->pid[i] == pid)
                {
                    auto ticks = AZStd::GetTimeNowSecond();
                    if ((ticks - m_ProcessIDs->i64Poll[i]) < 15)
                    {
                        return true;
                    }
                    else
                    {
                        if (bMutexLocked && m_bServer)
                        {
                            // clean up the data if stagnant
                            m_ProcessIDs->pid[i] = 0;
                            m_ProcessIDs->i64Poll[i] = 0;

                            for (AZ::u32 j = 0; j < IPC_MAX_MSGS; ++j)
                            {
                                IPCMessage* message = &m_MsgTable->message[j];
                                if (message->pid == pid)
                                {
                                    message->pid = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
    void Ipc::AddMessage(AZ::u64 pID, AZ::u32 pType, const AZ::u8* pData, AZ::u64 uSize, bool bMutexLocked)
    {
        bool bAdded = false;
        if (uSize > IPC_MAX_MSG_SIZE)
            uSize = IPC_MAX_MSG_SIZE;

        // Attempt to find an empty spot in table for our message to go.
        if (bMutexLocked && (m_OverflowQueue.find(pID) == m_OverflowQueue.end()))
        {
            AZ::s64 tNow = AZStd::GetTimeNowSecond();
            AZ::u64 dwToSendPid;
            AZ::u32 i, initialIdx = m_uMsgAddIdx, pIDx = GetPIDIdx(pID);

            while (!bAdded)
            {
                i = (pIDx * 10) + m_uMsgAddIdx;
                dwToSendPid = m_MsgTable->message[i].pid;

                if (dwToSendPid == 0
                    || ((tNow - m_MsgTable->message[i].i64Timestamp) > 30))
                {
                    IPCMessage tMsg(pID, m_uID, pType, uSize, m_ProcessIDs->uiPrevMsgSequence[pIDx] + 1, tNow);
                    IPCMessage* tIPCMsg = &m_MsgTable->message[i];
                    if ((uSize > 0) && (pData != NULL)) // copy the data
                        memcpy(tIPCMsg->ucMsg, pData, uSize);
                    memcpy(tIPCMsg, &tMsg, IPC_MESSAGE_DATA_SIZE);

                    ++m_ProcessIDs->uiPrevMsgSequence[pIDx];
                    bAdded = true;
                }

                ++m_uMsgAddIdx;
                if (m_uMsgAddIdx > IPC_MAX_PID - 1)
                    m_uMsgAddIdx = 0;

                if (m_uMsgAddIdx == initialIdx)
                {
                    //AZ_Warning("App", false, "GNIPC: Unable to find room for new message; defaulting to overflow!");
                    break;
                }
            }
        }

        // table was FULL/mutex failed to acquire lock.  Make sure and put this message into the overflow queue so it can be handled
        // as soon as we have the room
        if (!bAdded)
        {
            IPCMessageQueue& pidQueue = m_OverflowQueue[pID];
            // NOTE:  Theoretically we could queue an infinite number of messages here.  Don't hose the users
            // machine.  Make sure this queue is no bigger than 100 messages.
            if (pidQueue.size() > IPC_MAX_MSGS)
            {
                AZ_Warning("App", false, "GNIPC: Overflow queue for pID[%u] has more than 100 messages!\n", pID);
            }

            IPCMessage tIPCMsg(pID, m_uID, pType, uSize);
            if ((uSize > 0) && (pData != NULL))
                memcpy(tIPCMsg.ucMsg, pData, uSize);

            pidQueue.push_back(tIPCMsg);
        }
    }

    AZ::u32 Ipc::GetPIDIdx(AZ::u64 pID)
    {
        if (m_ProcessIDs)
        {
            for (AZ::u32 i = 0; i < IPC_MAX_PID; ++i)
            {
                if (m_ProcessIDs->pid[i] == pID)
                    return i;
            }
        }
        return (IPC_MAX_PID - 1);
    }
} // namespace Ipc
