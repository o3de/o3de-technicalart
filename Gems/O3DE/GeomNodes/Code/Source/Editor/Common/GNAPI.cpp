#include <Editor/Common/GNAPI.h>

namespace GeomNodes
{
    namespace API
    {
        bool Init(u64 id, HandlerCallback cb)
        {
#ifdef USE_BRIDGE
            return ::Init(id, cb);
#else
            (void)id;
            (void)cb;
            return false;
#endif
        }

        void Uninitialize()
        {
#ifdef USE_BRIDGE
            ::Uninitialize();
#endif
        }

        void SendMsg(const char* data, u64 length, u64 id)
        {
#ifdef USE_BRIDGE
            ::SendMsg(data, length, id);
#else
            (void)data;
            (void)length;
            (void)id;
#endif
        }

        bool ReadMsg(char* buffer, u64 length)
        {
#ifdef USE_BRIDGE
            return ::ReadMsg(buffer, length);
#else
            (void)buffer;
            (void)length;
            return false;
#endif
        }

        bool OpenSHM(u64 mapId)
        {
#ifdef USE_BRIDGE
            return ::OpenSHM(mapId);
#else
            (void)mapId;
            return false;
#endif
        }

        bool ReadSHM(u64 uId, void** address, u64* length)
        {
#ifdef USE_BRIDGE
            return ::ReadSHM(uId, address, length);
#else
            (void)uId;
            (void)address;
            (void)length;
            return false;
#endif
        }

        void ClearSHM(u64 uId)
        {
#ifdef USE_BRIDGE
            ::ClearSHM(uId);
#else
            (void)uId;
#endif
        }
    } // namespace API
} // namespace GeomNodes