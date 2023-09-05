#pragma once

namespace HoudiniEngine
{
    class IHoudiniAsset;

    class AssetContainer
    {
    public:
        IHoudiniAsset* asset = nullptr;
    };

    Q_DECLARE_METATYPE(AssetContainer)
} // namespace HoudiniEngine
