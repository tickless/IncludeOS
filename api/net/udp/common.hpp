
#pragma once

#include <delegate>
#include <net/addr.hpp>
#include <net/packet.hpp>
#include <net/error.hpp>
#include <net/checksum.hpp>

namespace net {
  // temp
  class PacketUDP;
}
namespace net::udp
{
  using addr_t = net::Addr;
  using port_t = uint16_t;

  using sendto_handler    = delegate<void()>;
  using error_handler     = delegate<void(const Error&)>;

  // temp
  using Packet_ptr = std::unique_ptr<PacketUDP, std::default_delete<net::Packet>>;

  template <typename View4>
  uint16_t calculate_checksum4(const View4& packet)
  {
    constexpr uint8_t Proto_UDP = 17;
    uint16_t length = packet.udp_length();
    const auto ip_src = packet.ip4_src();
    const auto ip_dst = packet.ip4_dst();
    // Compute sum of pseudo-header
    uint32_t sum =
          (ip_src.whole >> 16)
        + (ip_src.whole & 0xffff)
        + (ip_dst.whole >> 16)
        + (ip_dst.whole & 0xffff)
        + (Proto_UDP << 8)
        + htons(length);

    // Compute sum of header and data
    const char* buffer = (char*) &packet.udp_header();
    return net::checksum(sum, buffer, length);
  }

  template <typename View6>
  uint16_t calculate_checksum6(const View6& packet)
  {
    constexpr uint8_t Proto_UDP = 17;
    const uint16_t length = packet.udp_length();
    const auto ip_src = packet.ip6_src();
    const auto ip_dst = packet.ip6_dst();
    // Compute sum of pseudo-header
    uint32_t sum = 0;

    for(int i = 0; i < 4; i++)
    {
      uint32_t part = ip_src.template get_part<uint32_t>(i);
      sum += (part >> 16);
      sum += (part & 0xffff);

      part = ip_dst.template get_part<uint32_t>(i);
      sum += (part >> 16);
      sum += (part & 0xffff);
    }

    sum += (Proto_UDP << 8) + htons(length);

    // Compute sum of header and data
    const char* buffer = (char*) &packet.udp_header();
    return net::checksum(sum, buffer, length);
  }

}
