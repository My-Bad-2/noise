#include "log.hpp"
#include "memory/memory.hpp"
#include "memory/physical.hpp"

#include <string.h>

#include <cstdint>
#include <new>
#include <utility>

#include "limine.h"

namespace memory {
size_t PhysicalMemoryManager::addr_to_idx(uintptr_t addr) const {
  return (addr - this->memory_base) / PageSize4KiB;
}

uintptr_t PhysicalMemoryManager::idx_to_addr(size_t idx) const {
  return this->memory_base + (idx * PageSize4KiB);
}

size_t PhysicalMemoryManager::get_idx(size_t idx, int order) const {
  return idx ^ (1 << order);
}

void PhysicalMemoryManager::init_node(Node* node) {
  node->next = node;
  node->prev = node;
}

void PhysicalMemoryManager::add_node(Node* new_node, Node* head) {
  new_node->next = head->next;
  new_node->prev = head;

  head->next->prev = new_node;
  head->next = new_node;
}

void PhysicalMemoryManager::remove_node(Node* node) {
  node->next->prev = node->prev;
  node->prev->next = node->next;

  node->next = nullptr;
  node->prev = nullptr;
}

bool PhysicalMemoryManager::is_list_empty(const Node* head) {
  return head->next == head;
}

void PhysicalMemoryManager::deallocate_internal(size_t idx, int order) {
  while (order < (MAX_ORDERS - 1)) {
    size_t buddy_idx = this->get_idx(idx, order);
    Page* buddy_page = &this->page_metadata_arr[buddy_idx];

    if (buddy_page->in_use || buddy_page->order != order) {
      break;
    }

#if NOISE_DEBUG
    debug("Merging order %d blocks at indices %zu and %zu", order, idx,
          buddy_idx);
#endif

    this->remove_node(&buddy_page->node);

    if (buddy_idx < idx) {
      idx = buddy_idx;
    }

    order++;
  }

  Page* page = &this->page_metadata_arr[idx];
  page->in_use = false;
  page->order = order;
  this->add_node(&page->node, &free_lists[order]);
}

void* PhysicalMemoryManager::allocate_internal(int order) {
  if (order >= MAX_ORDERS) {
    return nullptr;
  }

  int curr_order = 0;

  for (curr_order = order; curr_order < MAX_ORDERS; ++curr_order) {
    if (!this->is_list_empty(&free_lists[curr_order])) {
      break;
    }
  }

  if (curr_order == MAX_ORDERS) {
    return nullptr;
  }

  Node* block_head = this->free_lists[curr_order].next;
  this->remove_node(block_head);

  Page* block_page = reinterpret_cast<Page*>(
      reinterpret_cast<char*>(block_head) - offsetof(Page, node));
  size_t block_idx = block_page - this->page_metadata_arr;

  while (curr_order > order) {
    curr_order--;
    size_t buddy_idx = block_idx + (1 << curr_order);

#ifdef NOISE_DEBUG
    debug("Splitting block at index %zu (order %d) into %zu and %zu", block_idx,
          curr_order + 1, block_idx, buddy_idx);
#endif

    Page* buddy_page = &this->page_metadata_arr[buddy_idx];
    buddy_page->in_use = false;
    buddy_page->order = curr_order;

    this->add_node(&block_page->node, &this->free_lists[curr_order]);
  }

  block_page->in_use = true;
  block_page->order = order;

  return reinterpret_cast<void*>(this->idx_to_addr(block_idx));
}

void* PhysicalMemoryManager::allocate(size_t size, bool clear) {
  if (size == 0) {
    return nullptr;
  }

  size_t pages_needed = div_roundup(size, std::to_underlying(PageSize4KiB));
  int order = 0;
  size_t block_size_in_pages = 1;

  while (block_size_in_pages < pages_needed) {
    block_size_in_pages <<= 1;
    order++;
  }

#ifdef NOISE_DEBUG
  debug("Request for %zu bytes -> %zu pages -> order %d", size, pages_needed,
        order);
#endif

  void* ret = this->allocate_internal(order);

  if (clear) {
    memset(ret, 0, pages_needed * PageSize4KiB);
  }

  return ret;
}

void PhysicalMemoryManager::deallocate(void* ptr) {
  if (ptr == nullptr) {
    return;
  }

  uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

  if (is_aligned(addr, std::to_underlying(PageSize4KiB))) {
#ifdef NOISE_DEBUG
    error("Physical Deallocation called with non-page-aligned address 0x%lx",
          addr);
#endif
    return;
  }

  size_t idx = this->addr_to_idx(addr);
  Page* page = &this->page_metadata_arr[idx];

  if (!page->in_use) {
#ifdef NOISE_DEBUG
    error("Double free or corruption detected at address %p", addr);
#endif
    return;
  }

  int order = page->order;
  this->deallocate_internal(idx, order);
}

void PhysicalMemoryManager::initialize(
    limine_memmap_response* memmap_response) {
  uintptr_t highest_addr = 0;
  size_t total_usable_mem = 0;

  if(memmap_response == nullptr) {
    #ifdef NOISE_DEBUG
      warning("Limine memory map Response is null");
    #endif

    return;
  }

  for (size_t i = 0; i < memmap_response->entry_count; ++i) {
    auto* entry = memmap_response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      uintptr_t top = entry->base + entry->length;

      if (top > highest_addr) {
        highest_addr = top;
      }

      total_usable_mem += entry->length;
    }
  }

  this->memory_base = 0;
  this->highest_memory_addr = highest_addr;

  size_t total_pages = this->highest_memory_addr / PageSize4KiB;
  size_t metadata_size = total_pages * sizeof(Page);

#ifdef NOISE_DEBUG
  info("Highest address: 0x%lx", highest_addr);
  info("Total usable memory: %lu MiB", total_usable_mem / 1024 / 1024);
  info("Total pages to manage: %zu", total_pages);
  info("Required metadata size: %zu KiB", metadata_size / 1024);
#endif

  for (size_t i = 0; i < memmap_response->entry_count; ++i) {
    auto* entry = memmap_response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= metadata_size) {
      this->page_metadata_arr = to_higher_half(reinterpret_cast<Page*>(entry->base));

#ifdef NOISE_DEBUG
      debug("Placing metadata at physical address 0x%lx", entry->base);
#endif

      entry->base += metadata_size;
      entry->length -= metadata_size;
      break;
    }
  }

  if (this->page_metadata_arr == nullptr) {
#ifdef NOISE_DEBUG
    panic(
        "Could not find a suitable memory region for Physical Allocator "
        "metadata.\n");
#endif

    return;
  }

  for (size_t i = 0; i < total_pages; ++i) {
    new (&this->page_metadata_arr[i]) Page();
  }

  for (int i = 0; i < MAX_ORDERS; ++i) {
    this->init_node(&this->free_lists[i]);
  }

  for (size_t i = 0; i < memmap_response->entry_count; ++i) {
    auto* entry = memmap_response->entries[i];

    if (entry->type == LIMINE_MEMMAP_USABLE) {
      uintptr_t base = align_up(entry->base, std::to_underlying(PageSize4KiB));

      size_t length = align_down(entry->base + entry->length - base,
                                 std::to_underlying(PageSize4KiB));

      uintptr_t curr_addr = base;

      while (length > 0) {
        int order = MAX_ORDERS - 1;

        while (order > 0) {
          size_t block_size = (1ull << order) * PageSize4KiB;

          if (block_size <= length) {
            break;
          }

          order--;
        }

        size_t idx = this->addr_to_idx(curr_addr);

#ifdef NOISE_DEBUG
        debug(
            "Freeing initial block of order %d at address 0x%lx (length "
            "0x%lx)\n",
            order, curr_addr, (1ull << order) * PageSize4KiB);
#endif

        this->deallocate_internal(idx, order);

        size_t freed_size = (1ull << order) * PageSize4KiB;
        curr_addr += freed_size;
        length -= freed_size;
      }
    }
  }
}

void PhysicalMemoryManager::print() const {
  printf("\n--- Physical Allocator Free Lists ---\n");

  for (int i = 0; i < MAX_ORDERS; ++i) {
    int count = 0;

    for (auto curr = this->free_lists[i].next; curr != &this->free_lists[i];
         curr = curr->next) {
      count++;
    }

    if (count > 0) {
      printf("\tOrder %2d (%6lu KiB): %d blocks\n", i,
             ((1ul << i) * PageSize4KiB) / 1024, count);
    }
  }

  printf("----------------------------------\n\n");
}
}  // namespace memory
