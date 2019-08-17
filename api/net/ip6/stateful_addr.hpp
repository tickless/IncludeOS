#pragma once

#include "detail/stateful_addr.hpp"
#include <rtc>

namespace net::ip6
{
  class Stateful_addr {
  public:
    using Impl = net::ip6::detail::Stateful_addr<RTC>;
    static constexpr uint32_t infinite_lifetime = Impl::infinite_lifetime;

    Stateful_addr(ip6::Addr addr, uint8_t prefix,
                  uint32_t preferred_lifetime = infinite_lifetime,
                  uint32_t valid_lifetime     = infinite_lifetime)
      : impl{std::move(addr), prefix, preferred_lifetime, valid_lifetime}
    {}

    const ip6::Addr& addr() const noexcept
    { return impl.addr(); }

    ip6::Addr& addr() noexcept
    { return impl.addr(); }

    uint8_t prefix() const noexcept
    { return impl.prefix(); }

    bool preferred() const noexcept
    { return impl.preferred(); }

    bool valid() const noexcept
    { return impl.valid(); }

    bool always_valid() const noexcept
    { return impl.always_valid(); }

    uint32_t remaining_valid_time()
    { return impl.remaining_valid_time(); }

    void update_preferred_lifetime(uint32_t preferred_lifetime)
    { impl.update_preferred_lifetime(preferred_lifetime); }

    void update_valid_lifetime(uint32_t valid_lifetime)
    { impl.update_valid_lifetime(valid_lifetime); }

    auto valid_ts() const noexcept
    { return impl.valid_ts(); }

    auto preferred_ts() const noexcept
    { return impl.preferred_ts(); }

    std::string to_string() const
    { return impl.to_string(); }

    bool match(const ip6::Addr& other) const noexcept
    { return impl.match(other); }

  private:
    Impl impl;

  };

}
