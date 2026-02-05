module;

#include <raylib.h>
#include <cstdlib>
#include <expected>
#include <string>

export module base;

export constexpr float RADIUS = 5.0f;
export constexpr size_t BOUNDING_BOX_PADDING = 15;

export {
    using Position = Vector2; 
    using Rect     = Rectangle;
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f32 = float;
    using f64 = double;
    using uid = i32;

    template<typename T>
    using Res = std::expected<T, std::string>;
    
    constexpr std::unexpected<std::string> Err(std::string msg) {return std::unexpected<std::string>(msg);};
};

export {
    constexpr Color PORCELEAN = Color{254, 254, 250, 255};
    constexpr Color LAVENDER_MIST = Color{248, 244, 255, 255};

    constexpr Color SOFT_BLUSH = Color{255, 229, 236, 255};     // BG
    constexpr Color CHERRY_BLOSSOM = Color{255, 179, 198, 255}; // ACTIVE
    constexpr Color COTTON_CANDY = Color{255, 143, 171, 255};   // HOT
    constexpr Color PETAL_ROUGE = Color{251, 111, 146, 255};    // IDLE
};
