#pragma once

#include <eosio/vm/stack_elem.hpp>

class vm_backend {
   public:
      auto& get_ref() {
         return *_backend;
      }
      void initialize();
      std::optional<eosio::vm::stack_elem> call(
   private:
      template <typename Host>
      eosio::vm::backend;

      std::unique_ptr<eosio::vm::backend<nullptr_t>> _backend;
};
