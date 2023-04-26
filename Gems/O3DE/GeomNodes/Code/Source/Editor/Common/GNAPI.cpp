/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <Editor/Common/GNAPI.h>

namespace GeomNodes
{
    namespace API
    {
        bool Init([[maybe_unused]] u64 id, [[maybe_unused]] HandlerCallback cb)
        {
#ifdef USE_BRIDGE
            return ::Init(id, cb);
#else
            return false;
#endif
        }

        void Uninitialize()
        {
#ifdef USE_BRIDGE
            ::Uninitialize();
#endif
        }

        void SendMsg([[maybe_unused]] const char* data, [[maybe_unused]] u64 length, [[maybe_unused]] u64 id)
        {
#ifdef USE_BRIDGE
            ::SendMsg(data, length, id);
#endif
        }

        bool ReadMsg([[maybe_unused]] char* buffer, [[maybe_unused]] u64 length)
        {
#ifdef USE_BRIDGE
            return ::ReadMsg(buffer, length);
#else
            return false;
#endif
        }

        bool OpenSHM([[maybe_unused]] u64 mapId)
        {
#ifdef USE_BRIDGE
            return ::OpenSHM(mapId);
#else
            return false;
#endif
        }

        bool ReadSHM([[maybe_unused]] u64 uId, [[maybe_unused]] void** address, [[maybe_unused]] u64* length)
        {
#ifdef USE_BRIDGE
            return ::ReadSHM(uId, address, length);
#else
            return false;
#endif
        }

        void ClearSHM([[maybe_unused]] u64 uId)
        {
#ifdef USE_BRIDGE
            ::ClearSHM(uId);
#endif
        }
    } // namespace API
} // namespace GeomNodes