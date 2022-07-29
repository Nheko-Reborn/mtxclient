#pragma once

/// @file
/// @brief Image packs from [MSC2545](https://github.com/matrix-org/matrix-doc/pull/2545)

#include <bitset>
#include <map>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events.hpp"

namespace mtx {
namespace events {

//! Custom emotes and stickers MSC
namespace msc2545 {
//! How a pack or image is intended to be used.
enum PackUsage : uint8_t
{
    Sticker,
    Emoji,
};

//! A single image in a pack
struct PackImage
{
    //! The mxc uri of this image.
    std::string url;
    //! The body to be sent in a sticker. May be empty.
    std::string body;

    //! Info as used in stickers about size, etc.
    std::optional<mtx::common::ImageInfo> info;

    //! What the images are used for. Indexed by PackUsage
    std::bitset<2> usage;

    //! If this overrides the pack level usage definition.
    bool overrides_usage() const { return usage.any(); }

    //! If this can be used as an emoji/emojicon.
    bool is_emoji() const { return usage.test(PackUsage::Emoji); }
    //! If this can be used as a sticker.
    bool is_sticker() const { return usage.test(PackUsage::Sticker); }
};

//! A pack of stickers and/or emoticons.
struct ImagePack
{
    //! The stickers/emoticons in this pack. Indexed by slug.
    std::map<std::string, PackImage> images;

    //! Information about an image pack.
    struct PackDescription
    {
        //! The name of this pack to be presented to the user.
        std::string display_name;
        //! An optional avatar/preview of the pack.
        std::string avatar_url;
        //! Attribution for this pack, i.e. where it is from originally.
        std::string attribution;

        //! What the images are used for. Indexed by PackUsage
        std::bitset<2> usage;

        //! If this can be used as an emoji/emojicon.
        bool is_emoji() const { return usage.none() || usage.test(PackUsage::Emoji); }
        //! If this can be used as a sticker.
        bool is_sticker() const { return usage.none() || usage.test(PackUsage::Sticker); }
    };

    //! Information about this pack
    std::optional<PackDescription> pack;

    friend void from_json(const nlohmann::json &obj, ImagePack &content);
    friend void to_json(nlohmann::json &obj, const ImagePack &content);
};

//! The image packs from rooms enabled by this user to be available globally.
struct ImagePackRooms
{
    // A map from room_id to state key to an arbitrary object (which currently is unused).
    std::map<std::string, std::map<std::string, std::string>> rooms;

    friend void from_json(const nlohmann::json &obj, ImagePackRooms &content);
    friend void to_json(nlohmann::json &obj, const ImagePackRooms &content);
};

} // namespace msc2545
} // namespace events
} // namespace mtx
