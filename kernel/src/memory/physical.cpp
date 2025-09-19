#include "log.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"

#include <string.h>

#include <utility>

namespace memory {
static PhysicalMemoryManager pmm_instance;

PhysicalMemoryManager& PhysicalMemoryManager::instance() {
  return pmm_instance;
}

// Compute smallest order (2^order pages) that can satisfy 'size' bytes.
uint8_t PhysicalMemoryManager::size_to_order(size_t size) const {
  if (size == 0) {
    size = 1;
  }

  size_t num_pages = div_roundup(size, std::to_underlying(PageSize4KiB));

  if (num_pages <= 1) {
    return 0;
  }

  size_t ret = sizeof(long long) * 8 - __builtin_clzll(num_pages - 1);

  return ret;
}

void PhysicalMemoryManager::insert_block(uintptr_t addr, uint8_t order) {
  // Insert a block at 'addr' (phys) into the free list of 'order'.
  FreeBlockNode* node = reinterpret_cast<FreeBlockNode*>(to_higher_half(addr));

  node->next = this->free_lists[order].next;
  node->prev = &this->free_lists[order];

  // Set node to be the head
  this->free_lists[order].next->prev = node;
  this->free_lists[order].next = node;

  this->set_page_metadata(addr, order, true);
}

void PhysicalMemoryManager::remove_block(uintptr_t addr, uint8_t order) {
  // Remove the block at 'addr' from the free list of 'order'.
  FreeBlockNode* node = reinterpret_cast<FreeBlockNode*>(to_higher_half(addr));

  node->prev->next = node->next;
  node->next->prev = node->prev;
}

uintptr_t PhysicalMemoryManager::get_buddy_address(uintptr_t addr,
                                                   uint8_t order) {
  // Buddy address differs by the block size of this order.
  size_t block_size = PageSize4KiB << order;
  return addr ^ block_size;
}

void PhysicalMemoryManager::set_page_metadata(uintptr_t base_addr,
                                              uint8_t order, bool is_free) {
  // Mark '1 << order' pages starting at 'base_addr' as free/used.
  size_t num_pages = (1 << order);
  size_t start_page_idx = base_addr / std::to_underlying(PageSize4KiB);

  if ((num_pages + start_page_idx) > this->total_pages) {
    panic(
        "Metadata access out of bounds!\n\t"
        "Address: 0x%lx, Order: %u, Pages: %lu\n\t"
        "Start Index: %lu, Total Pages: %lu",
        base_addr, order, num_pages, start_page_idx, total_pages);
  }

  for (size_t i = 0; i < num_pages; ++i) {
    PageMetadata& page = this->page_metadata[start_page_idx + i];

    page.is_free = is_free;
    page.order = order;
  }
}

void* PhysicalMemoryManager::allocate(size_t bytes, bool clear) {
  // Allocate a block large enough to cover 'bytes'. Optionally zero.
  libs::LockGuard guard(this->lock);

  uint8_t order = this->size_to_order(bytes);
  if (order > MAX_ORDER) {
    err("[PMM][ALLOC] Request too large: bytes=0x%lx order=%u", bytes, order);
    return nullptr;
  }

  uint8_t curr_order = order;
  while (curr_order <= MAX_ORDER) {
    const FreeBlockNode* node = &this->free_lists[curr_order];

    if (node->next != node) {
      break;  // Found a suitable block
    }

    curr_order++;
  }

  if (curr_order > MAX_ORDER) {
    panic("[PMM][ALLOC] Out of memory (request=0x%lx)", bytes);
    return nullptr;
  }

  // const size_t before_free = this->usable_memory;
  FreeBlockNode* block_node = this->free_lists[curr_order].next;
  uintptr_t block_addr =
      reinterpret_cast<uintptr_t>(from_higher_half(block_node));
  this->remove_block(block_addr, curr_order);

  // Split down to requested order; buddies go back to their free lists.
  while (curr_order > order) {
    curr_order--;
    uintptr_t buddy_addr = get_buddy_address(block_addr, curr_order);
    this->insert_block(buddy_addr, curr_order);
  }

  this->set_page_metadata(block_addr, order, false);
  this->usable_memory -= (PageSize4KiB << order);

  void* ret = reinterpret_cast<void*>(block_addr);
  if (clear) {
    memset(ret, 0, bytes);
  }

  // debug(
  // "[PMM][ALLOC] bytes=0x%lx order=%u addr=0x%lx free_before=0x%lx "
  // "free_after=0x%lx",
  // bytes, order, block_addr, before_free, this->usable_memory);
  return ret;
}

void PhysicalMemoryManager::deallocate(void* ptr) {
  // Free a previously allocated block, coalescing with free buddies.
  if (ptr == nullptr) {
    return;
  }

  libs::LockGuard guard(this->lock);

  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
  const size_t page_bytes = std::to_underlying(PageSize4KiB);

  if (!is_aligned(addr, page_bytes)) {
    err("[PMM][FREE] Address %p is not page-aligned", ptr);
    return;
  }

  const size_t page_idx = addr / page_bytes;

  // The order of the block being freed is stored in its first page's metadata.
  uint8_t order = this->page_metadata[page_idx].order;
  const uint8_t orig_order = order;
  // const size_t before_free = this->usable_memory;

  // Coalesce (merge) with buddies while possible.
  while (order < MAX_ORDER) {
    const uintptr_t buddy_addr = this->get_buddy_address(addr, order);
    const size_t buddy_page_idx = buddy_addr / page_bytes;

    // Check if buddy is within bounds, is free, and has the same order.
    if ((buddy_page_idx >= this->total_pages) ||
        !this->page_metadata[buddy_page_idx].is_free ||
        (this->page_metadata[buddy_page_idx].order != order)) {
      break;  // Buddy not mergeable
    }

    // Buddy is available: remove it from free list and merge.
    this->remove_block(buddy_addr, order);

    // Merged block starts at the lower address.
    addr = (addr <= buddy_addr) ? addr : buddy_addr;
    order++;
  }

  // Insert (possibly merged) block back into free lists.
  this->insert_block(addr, order);

  // Important: Increase free memory only by the size of the originally freed
  // block. The buddies we removed were already counted in usable_memory.
  this->usable_memory += (PageSize4KiB << orig_order);

  // debug(
  // "[PMM][FREE] addr=0x%lx orig_order=%u final_order=%u free_before=0x%lx "
  // "free_after=0x%lx",
  // reinterpret_cast<uintptr_t>(ptr), orig_order, order, before_free,
  // this->usable_memory);
}

void PhysicalMemoryManager::initialize(
    limine_memmap_response* memmap_response) {
  const size_t memmap_count = memmap_response->entry_count;

  // Initialize all free lists (sentinel nodes point to themselves).
  for (int i = MIN_ORDER; i <= MAX_ORDER; ++i) {
    FreeBlockNode* node = &this->free_lists[i];
    node->next = node->prev = node;
  }

  // Step 1: Find the highest memory address to determine total size.
  for (size_t i = 0; i < memmap_count; ++i) {
    const limine_memmap_entry* entry = memmap_response->entries[i];
    const uintptr_t top = entry->base + entry->length;

    if ((entry->type != LIMINE_MEMMAP_USABLE) &&
        (entry->type != LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) &&
        (entry->type != LIMINE_MEMMAP_EXECUTABLE_AND_MODULES)) {
      continue;
    }

    this->highest_addr = (this->highest_addr >= top) ? this->highest_addr : top;
  }

  this->total_pages =
      div_roundup(this->highest_addr, std::to_underlying(PageSize4KiB));
  this->total_memory = this->total_pages * PageSize4KiB;

  // Step 2: Find a home for the page_metadata array (reserve from a usable
  // region).
  size_t metadata_size = align_up(total_pages * sizeof(PageMetadata),
                                  std::to_underlying(PageSize4KiB));

  for (size_t i = 0; i < memmap_count; ++i) {
    limine_memmap_entry* entry = memmap_response->entries[i];

    if ((entry->type == LIMINE_MEMMAP_USABLE) &&
        (entry->length >= metadata_size)) {
      this->page_metadata =
          reinterpret_cast<PageMetadata*>(to_higher_half(entry->base));

      // Reserve the space by adjusting the entry itself.
      entry->base += metadata_size;
      entry->length -= metadata_size;
      break;
    }
  }

  if (this->page_metadata == nullptr) {
    panic("[PMM][INIT] No space for physical page metadata");
  }

  // Step 3: Initialize all metadata to "used" as a fail-safe.
  memset(this->page_metadata, 0, total_pages * sizeof(PageMetadata));

  // Step 4: Free all usable memory regions.
  for (size_t i = 0; i < memmap_count; ++i) {
    limine_memmap_entry* entry = memmap_response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      const uintptr_t start =
          align_up(entry->base, std::to_underlying(PageSize4KiB));
      const uintptr_t end = align_down(entry->base + entry->length,
                                       std::to_underlying(PageSize4KiB));

      if (start >= end) {
        continue;
      }

      // Greedily add memory chunks to the free lists.
      uintptr_t curr_addr = start;

      while (curr_addr < end) {
        size_t remaining_bytes = end - curr_addr;
        uint8_t order = MAX_ORDER;

        // Find the largest order that fits and is naturally aligned.
        while (order > MIN_ORDER) {
          const size_t block_size = PageSize4KiB << order;
          if ((block_size <= remaining_bytes) &&
              is_aligned(curr_addr, block_size)) {
            break;
          }
          order--;
        }

        const size_t block_size = (PageSize4KiB << order);
        this->insert_block(curr_addr, order);
        this->usable_memory += block_size;

        curr_addr += block_size;
      }
    }
  }

  debug("Page Metadata address = %p", this->page_metadata);
  info(
      "[PMM][INIT]\n\tHighest Address: 0x%lx\n\t"
      "Total Pages: %lu\n\t"
      "Total Memory: %lu MiB\n\t"
      "Metadata Size: %lu KiB\n\t"
      "Metadata VA: %p\n\t"
      "Usable (free) Memory: %lu MiB",
      this->highest_addr, this->total_pages, this->total_memory / 1024 / 1024,
      metadata_size / 1024, this->page_metadata,
      this->usable_memory / 1024 / 1024);
}

void PhysicalMemoryManager::print() const {
  libs::LockGuard guard(this->lock);

  printf("---------- Physical Memory Free Block ----------\n");
  printf("Total Memory: %lu MiB | Free Memory: %lu MiB\n",
         this->get_total_memory() / 1024 / 1024,
         this->get_free_memory() / 1024 / 1024);
  printf("================================================\n");

  for (int order = MIN_ORDER; order <= MAX_ORDER; ++order) {
    const FreeBlockNode* head = &this->free_lists[order];
    const FreeBlockNode* curr = head->next;

    if (curr == head) {
      // Skip empty lists
      continue;
    }

    size_t block_size = PageSize4KiB << order;
    const char* unit = "B";
    size_t size_in_unit = block_size;

    if (block_size >= 1024 * 1024) {
      size_in_unit = block_size / (1024 * 1024);
      unit = "MiB";
    } else if (block_size >= 1024) {
      size_in_unit = block_size / 1024;
      unit = "KiB";
    }

    printf("Order %2d (%4lu %s blocks):\n", order, size_in_unit, unit);

    int count = 0;

    while ((curr != head) && curr != nullptr) {
      uintptr_t phys_addr = reinterpret_cast<uintptr_t>(from_higher_half(curr));
      printf("  -> Block at 0x%016lx\n", phys_addr);

      curr = curr->next;
      count++;
    }

    printf("   (Total: %d blocks)\n", count);
  }

  printf("================================================\n");
}
}  // namespace memory
