#pragma once

#include <eosio/vm/allocator.hpp>
#include <eosio/vm/constants.hpp>
#include <eosio/vm/base_visitor.hpp>
#include <eosio/vm/variant.hpp>
#include <eosio/vm/exceptions.hpp>
#include <eosio/vm/leb128.hpp>
#include <eosio/vm/options.hpp>
#include <eosio/vm/sections.hpp>
#include <eosio/vm/types.hpp>
#include <eosio/vm/utils.hpp>

#include <iostream>

namespace eosio { namespace vm {

   // Common types and funtions used by serializer and
   // serializer_visitor classes

   struct code_t {
      guarded_vector<uint8_t> data;
      size_t idx = 0;
   };

   void append(code_t& to, const code_t& from ) {
      if ( to.idx + from.idx > to.data.size()) {
         to.data.resize( to.idx + from.idx );
      }

      memcpy( to.data.raw() + to.idx, from.data.raw(), from.idx );
      to.idx += from.idx;
   }

   void append(code_t& to, const guarded_vector<uint8_t>& from ) {
      if ( to.idx + from.size() > to.data.size()) {
         to.data.resize( to.idx + from.size() );
      }

      memcpy( to.data.raw() + to.idx, from.raw(), from.size() );
      to.idx += from.size();
   }

   template<typename T, size_t N>
   code_t encode_varuint(growable_allocator& alloc, T val) {
      varuint<N> val_leb128(val);
      code_t result = {{alloc, val_leb128.size()}, val_leb128.size()};
      memcpy(result.data.raw(), val_leb128.get_storage().data(), val_leb128.size());
      return result;
   }

   template<typename T, size_t N>
   code_t encode_varint(growable_allocator& alloc, T val) {
      varint<N> val_leb128(val);
      code_t result = { { alloc, val_leb128.size() }, val_leb128.size() };
      memcpy(result.data.raw(), val_leb128.get_storage().data(), val_leb128.size());
      return result;
   }

   class serializer {
   public:
      explicit serializer(growable_allocator& alloc):
         _allocator(alloc) {}

       inline code_t serialize_module(const module& mod) {
          code_t code {{_allocator, constants::initial_module_size}};
          serialize_module(mod, code);
          return code;
       }

       void serialize_module(const module& mod, code_t& code) {
          // Preamble
          append(code, encode_magic());
          append(code, encode_version());

          // Sections.
          //
          // We serialize each section based on the type and
          // appendenate them together.
          append(code, serialize_section(mod.types, section_id::type_section));
          append(code, serialize_section(mod.imports, section_id::import_section));
          append(code, serialize_section(mod.functions, section_id::function_section));
          append(code, serialize_section(mod.tables, section_id::table_section));
          append(code, serialize_section(mod.memories, section_id::memory_section));
          append(code, serialize_section(mod.globals, section_id::global_section));
          append(code, serialize_section(mod.exports, section_id::export_section));
          append(code, serialize_section(mod.elements, section_id::element_section));
          append(code, serialize_section(mod.code, section_id::code_section));
          append(code, serialize_section(mod.data, section_id::data_section));
       }

       template<typename T>
       code_t encode_raw(T raw) {
          static_assert(std::is_arithmetic_v<std::decay_t<T>>, "Can only serialize raw builtin types");
          code_t result = {{ _allocator, sizeof( T ) }, sizeof( T )};
          uint8_t* from = reinterpret_cast<uint8_t*>( &raw );
          memcpy(result.data.raw(), from, sizeof( T ));
          return result;
       }

       inline code_t encode_varuint1(uint8_t val) {
          return encode_varuint<uint8_t, 1>(_allocator, val);
       }

       inline code_t encode_varuint32(uint32_t val) {
          return encode_varuint<uint32_t, 32>(_allocator, val);
       }


       inline code_t encode_varint32(int32_t val) {
          return encode_varint<int32_t, 32>(_allocator, val);
       }

       inline code_t encode_varint64(int64_t val) {
          return encode_varint<int64_t, 64>(_allocator, val);
       }

       code_t encode_utf8_string(const guarded_vector<uint8_t>& str) {
          uint32_t len = str.size();
          code_t result = {{ _allocator, bytes_needed<32>() + len }};

          append( result, encode_varuint32(len) );
          append( result, str );
          return result;
       }

       template<typename T>
       code_t encode_vector(const guarded_vector<T>& val) {
          code_t result = {{ _allocator, constants::initial_module_size }};

          uint32_t num_eles = val.size();
          append(result, encode(num_eles));

          for (size_t i = 0; i < num_eles; ++i) {
             auto ele = encode(val[i]);
             append(result, ele);
          }

          //std::cout << "idx: " << result.idx << std::endl;
          return result;
       }

       inline code_t encode_magic() {
          return encode_raw<uint32_t>(constants::magic);
       }

       inline code_t encode_version() {
          return encode_raw<uint32_t>(constants::version);
       }

       // The following functions use the same name "encode"
       // but different type of input argument
       // so that they can be called by encode_vector

       inline code_t encode(uint32_t val) {
          return encode_varuint32(val);
       }

       // Serialize Type section
       code_t encode(const func_type& ft) {
          auto max_len = 3 + bytes_needed<32>() + ft.param_types.size(); // 3 for from, return_count and return_type
          code_t result = {{ _allocator, max_len }};

          result.data[result.idx++] = ft.form;

          uint32_t num_param_types = ft.param_types.size();
          append(result, encode_varuint32(num_param_types));
          for (size_t i = 0; i < num_param_types; ++i) {
             result.data[result.idx++] = ft.param_types[i];
          }

          result.data[result.idx++] = ft.return_count;

          if (ft.return_count > 0) {
             result.data[result.idx++] = ft.return_type;
          }

          return result;
       }

       // Serialize Import section
       code_t encode(const import_entry& ie) {
          auto max_len = 1 + (3 * bytes_needed<32>()) + ie.module_str.size() + ie.field_str.size(); // 1 for kind; bytes_needed for type, module_str length, and field_str length
          code_t result = {{ _allocator, max_len }};

          append( result, encode_varuint32(ie.module_str.size()) );
          append( result, ie.module_str );
          append( result, encode_varuint32(ie.field_str.size()) );
          append( result, ie.field_str);

          result.data[result.idx++] = ie.kind;
          switch ((uint8_t)ie.kind) {
             case external_kind::Function: {
                append( result, encode_varuint32(ie.type.func_t) );
                break;
             }
             default: EOS_VM_ASSERT(false, wasm_unsupported_import_exception, "only function imports are supported");
          }

          return result;
       }


       // Serialize Table section
       // tabletype ::= et:elemtype lim:limits
       // eletype   ::= 0x70
       code_t encode(const table_type& tt) {
          EOS_VM_ASSERT(tt.element_type == types::anyfunc, wasm_parse_exception, "table must have type anyfunc");

          size_t max_len = 2 + (2 * bytes_needed<32>()); // 1 for element_type, 1 for flags, bytes_needed for initial and for maximum 
          code_t result = {{ _allocator, max_len }};

          result.data[result.idx++] = tt.element_type;
          result.data[result.idx++] = tt.limits.flags; // TBD Assume 1 byte flag as in latest spec
          append( result, encode_varuint32(tt.limits.initial) );
          if (tt.limits.flags) {
             append( result, encode_varuint32(tt.limits.maximum));
          }

          return result;
       }

       // Serialize init expression
       // ?? Didn't find this in the latest spec
       constexpr static size_t init_expr_max_len = 2 + bytes_needed<64>(); // 1 for opcode, 1 for opcodes::end, bytes_needed<64> for i64
       code_t encode_init_expr(const init_expr& ie) {
          //size_t max_len = 2 + bytes_needed<64>(); // 1 for opcode, 1 for opcodes::end, bytes_needed<64>  for i64
          code_t result = {{ _allocator, init_expr_max_len }};

          result.data[result.idx++] = ie.opcode;
          switch (ie.opcode) {
             case opcodes::i32_const:
                append( result, encode_varint32(ie.value.i32));
                break;
             case opcodes::i64_const:
                append( result, encode_varint64(ie.value.i64));
                break;
             case opcodes::f32_const:
                append( result, encode_raw<uint32_t>(ie.value.f32));
                break;
             case opcodes::f64_const:
                append( result, encode_raw<uint64_t>(ie.value.f64));
                break;
             default:
                EOS_VM_ASSERT(false, wasm_parse_exception,
                              "initializer expression can only acception i32.const, i64.const, f32.const and f64.const");
          }
          result.data[result.idx++] = opcodes::end;

          return result;
       }


       // Serialize Memory section
       code_t encode(const memory_type& mt) {
          size_t max_len = 1 + (2 * bytes_needed<32>()); // 1 for flags, bytes_needed<32>() for initial and for maximum
          code_t result = {{ _allocator, max_len }};

          result.data[result.idx++] = mt.limits.flags; // TBD Assume 1 byte flag as in latest spec

          // WASM specification
          EOS_VM_ASSERT(mt.limits.initial <= 65536u, wasm_parse_exception, "initial memory out of range");
          append( result, encode_varuint32(mt.limits.initial) );
          if (mt.limits.flags) {
             EOS_VM_ASSERT(mt.limits.maximum >= mt.limits.initial, wasm_parse_exception, "maximum must be at least minimum");
             EOS_VM_ASSERT(mt.limits.maximum <= 65536u, wasm_parse_exception, "maximum memory out of range");
             append( result, encode_varuint32(mt.limits.maximum ));
          }

          return result;
       }

       // Serialize Global section
       code_t encode(const global_variable& gv) {
          size_t max_len = 2 + init_expr_max_len; // 1 for content_type, 1 for mutability
          code_t result = {{ _allocator, max_len }};

          EOS_VM_ASSERT(gv.type.content_type == types::i32 || gv.type.content_type == types::i64 || gv.type.content_type == types::f32 || gv.type.content_type == types::f64,
                        wasm_parse_exception, "invalid global content type");
          result.data[result.idx++] = gv.type.content_type;
          append( result, encode_varuint1( gv.type.mutability) );
          append( result, encode_init_expr(gv.init) );

          return result;
       }

       // Serialize Export section
       code_t encode(const export_entry& entry) {
          size_t max_len = 1 + 2 * bytes_needed<32>() + entry.field_str.size(); // 1 for kind, bytes_needed<32> for index and size for field_str
          code_t result = {{ _allocator, max_len }};

          append( result, encode_utf8_string(entry.field_str) );
          result.data[result.idx++] = entry.kind;
          append( result, encode_varuint32(entry.index) );

          return result;
       }

       // Serialize Element section
       code_t encode(const elem_segment& es) {
          size_t max_len = 2 * bytes_needed<32>() + init_expr_max_len + bytes_needed<32>() * es.elems.size(); // 2*bytes_needed<32> for index and size field of elems vector, init_expr_max_len for offset, elems is a vector of uint32_t
          code_t result = {{ _allocator, max_len }};

          append( result, encode_varuint32(es.index) );
          append( result, encode_init_expr(es.offset) );
          append( result, encode_vector(es.elems) );

          return result;
       }

       // Encode a single local entry
       code_t encode(const local_entry& le) {
          size_t max_len = 1 + bytes_needed<32>(); // bytes_needed<32>() for count, 1 for type
          code_t result = {{ _allocator, max_len }};

          append( result, encode_varuint32(le.count) );
          result.data[result.idx++] = le.type;

          return result;
       }

       // Serialize locals in a function
       code_t encode_function_locals(const guarded_vector<local_entry>& locals) {
          size_t max_len = bytes_needed<32>() + 6 * locals.size(); // bytes_needed for size of locals, 6 for each local_entry 
          code_t result = {{ _allocator, max_len }};

          append( result, encode_vector(locals) );

          return result;
       }

       // pull in serializer_visitor definition
#include  <eosio/vm/serializer_visitor.hpp>

       code_t encode_function_code(const opcode* code, uint32_t size) {
          code_t result = {{ _allocator, size }};
          serializer_visitor opc_visitor(_allocator);

          for (auto i = 0; i < size; ++i) {
             append(result, eosio::vm::visit(opc_visitor, code[i]));
          }

          return result;
       }

       // Serialize Code section
       code_t encode(const function_body& fb) {
          code_t result = {{ _allocator, fb.size }};

          append( result, encode_varuint32(fb.size) );
          append( result, encode_function_locals(fb.locals) );
          append( result, encode_function_code(fb.code, fb.size) );

          return result;
       }

       // Serialize Data section
       code_t encode(const data_segment& ds) {
          size_t max_len = bytes_needed<32>() + init_expr_max_len + ds.data.size(); // bytes_needed<32>() for index, init_expr_max_len for offset which is init_expr
          code_t result = {{ _allocator, max_len }};

          append( result, encode_varuint32(ds.index) );
          append( result, encode_init_expr(ds.offset) );
          append( result, ds.data );

          return result;
       }

       template <typename T>
       code_t serialize_section(const guarded_vector<T>& sec, section_id id) {
          code_t result = {{ _allocator, constants::initial_module_size }};

          result.data[result.idx++] = id;
          append(result, encode_vector(sec));

          return result;
       }

     private:
        growable_allocator& _allocator;
   };
}} // namespace eosio::vm
