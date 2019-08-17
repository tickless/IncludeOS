
#include <cstdio>
#include <cstring>

#include <common>
#include <fs/common.hpp>
#include <fs/memdisk.hpp>
#include <statman>


extern char _DISK_START_;
extern char _DISK_END_;

namespace fs {

  MemDisk::MemDisk() noexcept
    : MemDisk(&_DISK_START_, &_DISK_END_)
  {
    INFO("Memdisk", "Initializing start=%p end=%p", image_start_, image_end_);
  }

  MemDisk::MemDisk(const char* start, const char* end) noexcept
    : Block_device(),
      image_start_ { start },
      image_end_   { end },
      stat_read( Statman::get().create(
               Stat::UINT64, device_name() + ".reads").get_uint64() )
  {
    Expects(image_start_ <= image_end_);
  }

  MemDisk::buffer_t MemDisk::read_sync(block_t blk, size_t cnt)
  {
    stat_read++;

    auto start_loc = image_start_ + blk * block_size();
    auto end_loc = start_loc + cnt * block_size();

    // Disallow reading memory past disk image
    if (UNLIKELY(end_loc > image_end_))
      return nullptr;

    return fs::construct_buffer(start_loc, end_loc);
  }

  MemDisk::block_t MemDisk::size() const noexcept {
    // we are NOT going to round up to "support" unevenly sized
    // disks that are not created as multiples of sectors
    return (image_end_ - image_start_) / block_size();
  }

  void MemDisk::deactivate() {}

} //< namespace fs
