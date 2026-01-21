#pragma  once

#include <expected>
#include <string>
#include <raylib.h>


using Position = Vector2; 
using  s8 = signed char;
using  u8 = unsigned char;
using s16 = signed short;
using u16 = unsigned short;
using s32 = signed int;
using u32 = unsigned int;
using s64 = signed long long;
using u64 = unsigned long long;
using uid = s32;


template<typename T>
using Res = std::expected<T, std::string>;

constexpr std::unexpected<std::string> Err(std::string msg) {return std::unexpected<std::string>(msg);};
