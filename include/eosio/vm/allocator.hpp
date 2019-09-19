#pragma once

#include <eosio/vm/constants.hpp>
#include <eosio/vm/exceptions.hpp>

#include <cassert>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>

namespace eosio { namespace vm {
   class bounded_allocator {
    public:
      bounded_allocator(size_t size) {
         mem_size = size;
         raw      = std::unique_ptr<uint8_t[]>(new uint8_t[mem_size]);
      }
      template <typename T>
      T* alloc(size_t size = 1) {
         EOS_VM_ASSERT((sizeof(T) * size) + index <= mem_size, wasm_bad_alloc, "wasm failed to allocate native");
         T* ret = (T*)(raw.get() + index);
         index += sizeof(T) * size;
         return ret;
      }

      template <typename T>
      void reclaim(const T* ptr, size_t size=0) { /* noop for now */ }

      void free() {
         EOS_VM_ASSERT(index > 0, wasm_double_free, "double free");
         index = 0;
      }
      void                       reset() { index = 0; }
      size_t                     mem_size;
      std::unique_ptr<uint8_t[]> raw;
      size_t                     index = 0;
   };

   class contiguous_allocator {
      public:
         template<std::size_t align_amt>
         static constexpr size_t align_offset(size_t offset) { return (offset + align_amt - 1) & ~(align_amt - 1); }

         static std::size_t align_to_page(std::size_t offset) {
            std::size_t pagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
            return (offset + pagesize - 1) & ~(pagesize - 1);
         }

         contiguous_allocator(size_t size) {
            _size = align_to_page(size);
            _base = (char*)mmap(NULL, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            EOS_VM_ASSERT(_base != MAP_FAILED, wasm_bad_alloc, "mmap failed.");
         }
         ~contiguous_allocator() { munmap(_base, align_to_page(_size)); }

         template <typename T>
         T* alloc(size_t size = 0) {
            _offset = align_offset<alignof(T)>(_offset);
            size_t aligned = (sizeof(T) * size) + _offset;
            if (aligned > _size) {
               size_t new_size = align_to_page(aligned);
               char* new_base = (char*)mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
               EOS_VM_ASSERT(new_base != MAP_FAILED, wasm_bad_alloc, "mmap failed.");
               memcpy(new_base, _base, _size);
               munmap(_base, _size);
               _size = new_size;
               _base = new_base;
            }
            T* ptr = (T*)(_base + _offset);
            _offset = aligned;
            return ptr;
         }
         template <typename T>
         void reclaim(const T* ptr, size_t size=0) { /* noop for now */ }
         void free() { /* noop for now */ }

      private:
         size_t _offset = 0;
         size_t _size   = 0;
         char*  _base;
   };

   class growable_allocator {
    public:
      static constexpr size_t max_memory_size = 1024 * 1024 * 1024; // 1GB
      static constexpr size_t chunk_size      = 128 * 1024;         // 128KB
      template<std::size_t align_amt>
      static constexpr size_t align_offset(size_t offset) { return (offset + align_amt - 1) & ~(align_amt - 1); }

      static std::size_t align_to_page(std::size_t offset) {
         std::size_t pagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
         assert(max_memory_size % page_size == 0);
         return (offset + pagesize - 1) & ~(pagesize - 1);
      }

      // size in bytes
      growable_allocator(size_t size) {
         EOS_VM_ASSERT(size <= max_memory_size, wasm_bad_alloc, "Too large initial memory size");
         _base = (char*)mmap(NULL, max_memory_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
         EOS_VM_ASSERT(_base != MAP_FAILED, wasm_bad_alloc, "mmap failed.");
         if (size != 0) {
            size_t chunks_to_alloc = (align_offset<chunk_size>(size) / chunk_size);
            _size += (chunk_size * chunks_to_alloc);
            mprotect((char*)_base, _size, PROT_READ | PROT_WRITE);
         }
      }

      ~growable_allocator() { munmap(_base, max_memory_size); }

      // TODO use Outcome library
      template <typename T>
      T* alloc(size_t size = 0) {
         static_assert(max_memory_size % alignof(T) == 0, "alignment must divide max_memory_size.");
         _offset = align_offset<alignof(T)>(_offset);
         // Evaluating the inequality in this form cannot cause integer overflow.
         // Once this assertion passes, the rest of the function is safe.
         EOS_VM_ASSERT ((max_memory_size - _offset) / sizeof(T) >= size, wasm_bad_alloc, "Allocated too much memory");
         size_t aligned = (sizeof(T) * size) + _offset;
         if (aligned > _size) {
            size_t chunks_to_alloc = align_offset<chunk_size>(aligned - _size) / chunk_size;
            mprotect((char*)_base + _size, (chunk_size * chunks_to_alloc), PROT_READ | PROT_WRITE);
            _size += (chunk_size * chunks_to_alloc);
         }

         T* ptr  = (T*)(_base + _offset);
         _offset = aligned;
         return ptr;
      }

      void * start_code() {
         _offset = align_to_page(_offset);
         return _base + _offset;
      }
      template<bool IsJit>
      void end_code(void * code_base) {
         assert((char*)code_base >= _base);
         assert((char*)code_base <= (_base+_offset));
         _offset = align_to_page(_offset);
         _code_base = (char*)code_base;
         _code_size = _offset - ((char*)code_base - _base);
         enable_code(IsJit);
      }

      // Sets protection on code pages to allow them to be executed.
      void enable_code(bool is_jit) {
         mprotect(_code_base, _code_size, is_jit?PROT_EXEC:PROT_READ);
      }
      // Make code pages unexecutable
      void disable_code() {
         mprotect(_code_base, _code_size, PROT_NONE);
      }

      /* different semantics than free,
       * the memory must be at the end of the most recently allocated block.
       */
      template <typename T>
      void reclaim(const T* ptr, size_t size=0) {
         EOS_VM_ASSERT( _offset / sizeof(T) >= size, wasm_bad_alloc, "reclaimed too much memory" );
         EOS_VM_ASSERT( size == 0 || (char*)(ptr + size) == (_base + _offset), wasm_bad_alloc, "reclaiming memory must be strictly LIFO");
         if ( size != 0 )
            _offset = ((char*)ptr - _base);
      }
      void free() { EOS_VM_ASSERT(false, wasm_bad_alloc, "unimplemented"); }

      void reset() { _offset = 0; }

      size_t _offset = 0;
      size_t _size   = 0;
      char*  _base;
      char*  _code_base = nullptr;
      size_t _code_size = 0;
   };

   template <typename T>
   class fixed_stack_allocator {
    private:
      T*     raw      = nullptr;
      size_t max_size = 0;

    public:
      template <typename U>
      void free() {
         munmap(raw, max_memory);
      }
      fixed_stack_allocator(size_t max_size) : max_size(max_size) {
         raw = (T*)mmap(NULL, max_memory, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
         EOS_VM_ASSERT( raw != MAP_FAILED, wasm_bad_alloc, "mmap failed to alloca pages" );
         mprotect(raw, max_size * sizeof(T), PROT_READ | PROT_WRITE);
      }
      inline T* get_base_ptr() const { return raw; }
   };

   class wasm_allocator {
    private:
      char*   raw       = nullptr;
      int32_t page      = 0;

    public:
      template <typename T>
      void alloc(size_t size = 1 /*in pages*/) {
         if (size == 0) return;
         EOS_VM_ASSERT(size != -1, wasm_bad_alloc, "require memory to allocate");
         EOS_VM_ASSERT(size <= max_pages - page, wasm_bad_alloc, "exceeded max number of pages");
         int err = mprotect(raw + (page_size * page), (page_size * size), PROT_READ | PROT_WRITE);
         EOS_VM_ASSERT(err == 0, wasm_bad_alloc, "mprotect failed");
         T* ptr    = (T*)(raw + (page_size * page));
         memset(ptr, 0, page_size * size);
         page += size;
      }
      void free() {
         std::size_t syspagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
         munmap(raw - syspagesize, max_memory + 2*syspagesize);
      }
      wasm_allocator() {
         std::size_t syspagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
         raw  = (char*)mmap(NULL, max_memory + 2*syspagesize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
         EOS_VM_ASSERT( raw != MAP_FAILED, wasm_bad_alloc, "mmap failed to alloca pages" );
         int err = mprotect(raw, syspagesize, PROT_READ);
         EOS_VM_ASSERT(err == 0, wasm_bad_alloc, "mprotect failed");
         raw += syspagesize;
         page = 0;
      }
      void reset(uint32_t new_pages) {
         if (page != -1) {
            memset(raw, '\0', page_size * page); // zero the memory
         } else {
            std::size_t syspagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
            int err = mprotect(raw - syspagesize, syspagesize, PROT_READ);
            EOS_VM_ASSERT(err == 0, wasm_bad_alloc, "mprotect failed");
         }
         // no need to mprotect if the size hasn't changed
         if (new_pages != page && page > 0) {
            int err = mprotect(raw, page_size * page, PROT_NONE); // protect the entire region of memory
            EOS_VM_ASSERT(err == 0, wasm_bad_alloc, "mprotect failed");
         }
         page = 0;
      }
      // Signal no memory defined
      void reset() {
         if (page != -1) {
            std::size_t syspagesize = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
            memset(raw, '\0', page_size * page); // zero the memory
            int err = mprotect(raw - syspagesize, page_size * page + syspagesize, PROT_NONE);
            EOS_VM_ASSERT(err == 0, wasm_bad_alloc, "mprotect failed");
         }
         page = -1;
      }
      template <typename T>
      inline T* get_base_ptr() const {
         return reinterpret_cast<T*>(raw);
      }
      template <typename T>
      inline T* create_pointer(uint32_t offset) { return reinterpret_cast<T*>(raw + offset); }
      inline int32_t get_current_page() const { return page; }
      bool is_in_region(char* p) { return p >= raw && p < raw + max_memory; }
   };
}} // namespace eosio::vm
