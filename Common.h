#ifndef SOCKET_APP_COMMON_H
#define SOCKET_APP_COMMON_H

#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <format>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cs_sockets.h"

using i8    = std::int8_t;
using u8    = std::uint8_t;
using i32   = std::int32_t;
using u32   = std::uint32_t;
using i64   = std::int16_t;
using u16   = std::uint16_t;
using usize = std::size_t;

enum class CommandPacket : u8
{
    None,
    Disconnect,
    Shell,
    Message
};

template <typename HeaderType>
    requires(std::is_enum_v<HeaderType>)
struct PacketHeader
{
    HeaderType id = HeaderType{};
    usize      size;
};

template <typename HeaderType>
    requires(std::is_enum_v<HeaderType>)
struct Packet
{
public:
    PacketHeader<HeaderType> header = PacketHeader<HeaderType>{};
    std::vector<u8>          buffer;

public:
    inline Packet() noexcept = default;
    inline Packet(const HeaderType id) noexcept { this->header.id = id; }
    inline Packet(const HeaderType id, const std::vector<u8>& buffer) noexcept
    {
        this->header.id   = id;
        this->buffer      = buffer;
        this->header.size = this->buffer.size();
    }

public:
    inline auto operator<<(const std::vector<u8>& buffer) noexcept -> Packet<HeaderType>&
    {
        this->buffer.insert(buffer.end(), buffer.begin(), buffer.end());
        this->header.size = this->buffer.size();
        return *this;
    }
    inline auto operator<<(const std::string_view str) noexcept -> Packet<HeaderType>&
    {
        this->buffer.insert(this->buffer.end(), str.begin(), str.end());
        this->header.size = this->buffer.size();
        return *this;
    }
};

using SocketDisposer = decltype(Socket_Dispose)*;
using ScopedSocket   = std::unique_ptr<Socket, SocketDisposer>;
/*template <typename PacketHeader>
    requires(std::is_enum_v<PacketHeader>)
using ScopedPacket = std::unique_ptr<Packet<PacketHeader>>;
*/

#endif // SOCKET_APP_COMMON_H
