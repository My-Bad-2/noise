#ifndef MEMORY_PHYSICAL_HPP
#define MEMORY_PHYSICAL_HPP 1

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_ORDERS 11

namespace memory {
class PhysicalMemoryManager {
 public:
  PhysicalMemoryManager() = default;
  ~PhysicalMemoryManager() = default;

  static PhysicalMemoryManager& instance() {
    static PhysicalMemoryManager inst;
    return inst;
  }

  void initialize(limine_memmap_response* memmap_response);
  void* allocate(size_t size, bool clear = false);
  void deallocate(void* ptr);

  template <typename T = void*>
  T allocate(size_t size, bool clear = false) {
    return reinterpret_cast<T>(allocate(size, clear));
  }

  void deallocate(auto ptr) {
    this->deallocate(reinterpret_cast<void*>(ptr));
  }

  void print() const;

 private:
  struct Node {
    Node* next = nullptr;
    Node* prev = nullptr;
  };

  struct Page {
    bool in_use = true;
    int order = -1;
    Node node;
  };

 private:
  PhysicalMemoryManager(const PhysicalMemoryManager&) = delete;
  PhysicalMemoryManager(PhysicalMemoryManager&&) = delete;

  PhysicalMemoryManager& operator=(const PhysicalMemoryManager&) = delete;
  PhysicalMemoryManager& operator=(PhysicalMemoryManager&&) = delete;

  size_t addr_to_idx(uintptr_t addr) const;
  uintptr_t idx_to_addr(size_t idx) const;
  size_t get_idx(size_t idx, int order) const;

  void deallocate_internal(size_t idx, int order);
  void* allocate_internal(int order);

  bool is_list_empty(const Node* head);
  void add_node(Node* new_node, Node* head);
  void remove_node(Node* node);
  void init_node(Node* node);

 private:
  Page* page_metadata_arr = nullptr;
  uintptr_t memory_base = 0;
  size_t highest_memory_addr = 0;
  Node free_lists[MAX_ORDERS] = {};
};
}  // namespace memory

#endif  // MEMORY_PHYSICAL_HPP
