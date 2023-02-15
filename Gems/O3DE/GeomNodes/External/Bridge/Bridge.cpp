#include "Bridge.h"
#include "Ipc.h"

bool Init(u64 id, HandlerCallback cb)
{
    Ipc::Ipc::GetInstance()->Initialize(id, cb);
    return Ipc::Ipc::GetInstance()->IsInitialized();
}

void Uninitialize()
{
    Ipc::Ipc::GetInstance()->Uninitialize();
}

void SendMsg(const char* data, u64 length, u64 id)
{
    Ipc::Ipc::GetInstance()->SendMsg(Ipc::IPC_MSG_JSON, reinterpret_cast<const AZ::u8*>(data), length, id);
}

bool IsConnected()
{
    return Ipc::Ipc::GetInstance()->IsInitialized() && Ipc::Ipc::GetInstance()->IsServerRunning();
}

u64 CheckForMsg()
{
    return Ipc::Ipc::GetInstance()->CheckForMessage();
}

bool ReadMsg(char* buffer, u64 length)
{
    return Ipc::Ipc::GetInstance()->ReadMessage(buffer, length);
}

u64 RequestSHM(u64 uSize)
{
    return Ipc::Ipc::GetInstance()->RequestSHM(uSize);
}

bool OpenSHM(u64 mapId)
{
    return Ipc::Ipc::GetInstance()->OpenSHM(mapId);
}

bool ReadSHM(u64 uId, void** address, u64* length)
{
    return Ipc::Ipc::GetInstance()->ReadSHM(uId, address, length);
}

void WriteSHM(u64 uId, const char* source, const u64 length)
{
    Ipc::Ipc::GetInstance()->WriteSHM(uId, source, length);
}

void ClearSHM(u64 uId)
{
    Ipc::Ipc::GetInstance()->ClearSHM(uId);
}
