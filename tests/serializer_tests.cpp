#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdlib>
#include <fstream>
#include <string>

#include <iostream>

#include "utils.hpp"

#include <eosio/vm/serializer.hpp>

#include <catch2/catch.hpp>

using namespace eosio;
using namespace eosio::vm;

growable_allocator allocator(100000);

template<typename T>
void check_encode(const T& input, const std::vector<uint8_t>& expected_result) {
   serializer s(allocator);
   auto result = s.encode(input);
   std::vector<uint8_t> actual_result (result.data.raw(), result.data.raw() + result.idx);
   CHECK(actual_result == expected_result);
}

std::vector<uint8_t> codeToStdVec(const code_t& code) {
   return { code.data.raw(), code.data.raw() + code.idx };
}

void check_result(const code_t& result, const std::vector<uint8_t>& expected_result) {
   std::vector<uint8_t> actual_result (result.data.raw(), result.data.raw() + result.idx);
   CHECK(actual_result == expected_result);
}

template <typename T>
void set_guarded_vector(guarded_vector<T>& to, std::vector<T>&& from) {
   to.copy(from.data(), from.size());
}

TEST_CASE("magic version test", "[magic_version_test]") {
   serializer s(allocator);

   std::vector<uint8_t> expected_magic { 0x00, 0x61, 0x73, 0x6D };
   auto actual_magic = codeToStdVec( s.encode_magic() );
   CHECK(actual_magic == expected_magic);

   std::vector<uint8_t> expected_version { 0x01, 0x00, 0x00, 0x00 };
   auto actual_version = codeToStdVec( s.encode_version() );
   CHECK(actual_version == expected_version);
}

TEST_CASE("encode varuint1 test", "[encode_varuint1_test]") {
   serializer s(allocator);

   std::vector<uint8_t> expected_1 { 0x00 };
   auto actual_1 = codeToStdVec( s.encode_varuint1( 0x00 ) );
   CHECK(actual_1 == expected_1);

   std::vector<uint8_t> expected_2{ 0x01 };
   auto actual_2 = codeToStdVec( s.encode_varuint1(0x01) );
   CHECK(actual_2 == expected_2);
}

TEST_CASE("encode varuint32 test", "[encode_varuint32_test]") {
   serializer s(allocator);

   // 624485 in binary 10011000011101100101
   std::vector<uint8_t> expected { 0xE5, 0x8E, 0x26 };
   auto actual = codeToStdVec( s.encode_varuint32( 624485 ) );
   CHECK( actual == expected );
}

TEST_CASE("encode varint32 test", "[encode_varint32_test]") {
   serializer s(allocator);

   // -123456 in binary 111100001110111000000
   std::vector<uint8_t> expected { 0xC0, 0xBB, 0x78 };
   auto actual = codeToStdVec( s.encode_varint32( -123456 ) );
   CHECK( actual == expected );

   std::vector<uint8_t> expected_1 { 0x9B, 0xF1, 0x59 };
   auto actual_1 = codeToStdVec( s.encode_varint32( -624485 ) );
   CHECK( actual_1 == expected_1 );
}

TEST_CASE("encode varint64 test", "[encode_varint64_test]") {
   serializer s(allocator);

   std::vector<uint8_t> expected_1 { 0xC0, 0xBB, 0x78 };
   auto actual_1 = codeToStdVec( s.encode_varint64( -123456 ) );
   CHECK( actual_1 == expected_1 );

   std::vector<uint8_t> expected_2 { 0x9B, 0xF1, 0x59 };
   auto actual_2 = codeToStdVec( s.encode_varint64( -624485 ) );
   CHECK( actual_2 == expected_2 );

   std::vector<uint8_t> expected_3 { 0xE5, 0x8E, 0x26 };
   auto actual_3 = codeToStdVec( s.encode_varint64( 624485 ) );
   CHECK( actual_3 == expected_3 );
}

TEST_CASE("encode utf8 string test", "[encode_utf8_string_test]") {
   serializer s(allocator);

   std::vector<uint8_t> expected { 0x05, 0x10, 0x11, 0x12, 0x13, 0x14 }; // string of 5
   std::vector<uint8_t> in { 0x10, 0x11, 0x12, 0x13, 0x14 };
   guarded_vector<uint8_t> input { allocator, in.size() };
   input.copy(in.data(), in.size());
   auto actual = codeToStdVec( s.encode_utf8_string( input ) );
   CHECK( actual == expected );
}

TEST_CASE("encode uint32_t test", "[encode_uint32_t_test]") {
   uint32_t uint32 = 624485; 

   check_encode( uint32, { 0xE5, 0x8E, 0x26 } );
}

// type:num_of_parms:parm_types:num_returns:return_types
TEST_CASE("encode func type test", "[encode_func_type_test]") {
   struct func_type ft = { 0x60, { allocator, 2 }, 1, 0x7f };
   set_guarded_vector(ft.param_types, { 0x7c, 0x7e  });

   check_encode(ft, { 0x60, 0x02, 0x7c, 0x7e, 0x01, 0x7f } );
}

// import ::= mod:name nm:name d:importdesc
// importdesc ::= 0x00 x:typeidx
// (import "foo" "bar" (func (param f32)))
// 03                   ; string length
// 66 6f 6f             foo  ; import module name
// 03                   ; string length
// 62 61 72             bar  ; import field name
// 00                   ; import kind
// 01                   ; import signature indexo

TEST_CASE("encode import test", "[encode_import_entry_test]") {
   struct import_entry ie = { { allocator, 3 }, { allocator, 3 }, external_kind::Function };
   set_guarded_vector(ie.module_str, {0x66, 0x6f, 0x6f});
   set_guarded_vector(ie.field_str, {0x62, 0x61, 0x72});
   ie.type.func_t = 0x01;

   check_encode(ie, { 0x03, 0x66, 0x6f, 0x6f, 0x03, 0x62, 0x61, 0x72, 0x00, 0x01} );
}

// (table 0 1 anyfunc)
// 70                                        ; funcref
// 01                                        ; limits: flags
// 00                                        ; limits: initial
// 0xE5, 0x8E, 0x26                          ; limits: max
TEST_CASE("encode table type test", "[encode_table_type_test]") {
   struct table_type tt_1 = { 0x70, {true, 0, 624485}, { allocator, 1 }};
   check_encode(tt_1, { 0x70, 0x01, 0x00, 0xE5, 0x8E, 0x26 });

   struct table_type tt_2 = { 0x70, {false, 624485}, { allocator, 1 }};
   check_encode(tt_2, { 0x70, 0x00, 0xE5, 0x8E, 0x26 });
}

TEST_CASE("encode init expr test", "[encode_init_expr_test]") {
   serializer s(allocator);

   init_expr ie_1 = { opcodes::i32_const };
   ie_1.value.i32 = 0x0A;
   check_result(s.encode_init_expr(ie_1), {opcodes::i32_const, 0x0A, opcodes::end});

   init_expr ie_2 = { opcodes::i64_const };
   ie_2.value.i64 = 0x3A;
   check_result(s.encode_init_expr(ie_2), {opcodes::i64_const, 0x3A, opcodes::end});

   init_expr ie_3 = { opcodes::f32_const };
   ie_3.value.f32 = 0x01020304;
   check_result(s.encode_init_expr(ie_3), {opcodes::f32_const, 0x04, 0x03, 0x02, 0x01, opcodes::end});
}

TEST_CASE("encode memory type test", "[encode_memory_type_test]") {
   memory_type mt_1 = {{ true, 0x10, 0x30 }};
   check_encode(mt_1, { 0x01, 0x10, 0x30 });

   memory_type mt_2 = {{ false, 0x01}};
   check_encode(mt_2, {0x00, 0x01});
}

TEST_CASE("encode global variable test", "[encode_global_variable_test]") {
   struct global_variable gv = {{types::i32, true}, { opcodes::i32_const }};
   gv.init.value.i32 = 0x0B;
   check_encode(gv, { 0x7F, 0x01, opcodes::i32_const, 0x0B, opcodes::end });
}

TEST_CASE("encode export entry test", "[encode_export_entry_test]") {
   export_entry ee = {{ allocator, 3 }, external_kind::Function, 624485};
   set_guarded_vector(ee.field_str, {0x66, 0x6f, 0x6f});
   check_encode(ee, { 0x03, 0x66, 0x6f, 0x6f, 0x00, 0xE5, 0x8E, 0x26 });
}

TEST_CASE("encode elem segment test", "[encode_elem_segment_test]") {
   elem_segment ee = {1, { opcodes::i32_const }, { allocator, 3 }};
   ee.offset.value.i32 = 0x0A;
   ee.elems[0] = 1;
   ee.elems[1] = 2;
   ee.elems[2] = 624485;

   check_encode(ee, { 0x01, opcodes::i32_const, 0x0A, opcodes::end, 0x03, 0x01, 0x02, 0xE5, 0x8E, 0x26 });
}

TEST_CASE("encode local entry test", "[encode_local_entry_test]") {
   local_entry le = {500, 0x7C};

   // 500 in LEB128 is 0xF4 0x03
   check_encode(le, {0xF4, 0x03, 0x7C});
}

TEST_CASE("encode local vector test", "[encode_local_vector_test]") {
   serializer s(allocator);

   guarded_vector<local_entry> lv { allocator, 3 };
   set_guarded_vector(lv, {{1, 0x7E}, {500, 0x7C}, {624485, 0x7D}});

   check_result(s.encode_function_locals(lv), {0x03,  0x01, 0x7E,  0xF4, 0x03, 0x7C,  0xE5, 0x8E, 0x26, 0x7D} );
}

TEST_CASE("encode control instructions test", "[encode_control_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = unreachable_t{};
   check_result( s.encode_function_code(fb.raw(), size), {0x00} );

   fb[0]= nop_t{};         
   check_result( s.encode_function_code(fb.raw(), size), {0x01} );

   fb[0] = block_t{};       
   check_result( s.encode_function_code(fb.raw(), size), {0x02} );

   fb[0] = loop_t{};        
   check_result( s.encode_function_code(fb.raw(), size), {0x03} );

   fb[0] = if_t{};          
   check_result( s.encode_function_code(fb.raw(), size), {0x04} );

   fb[0] = else_t{};        
   check_result( s.encode_function_code(fb.raw(), size), {0x05} );

   fb[0] = end_t{};         
   check_result( s.encode_function_code(fb.raw(), size), {0x0B} );

   fb[0] = return_t{};         
   check_result( s.encode_function_code(fb.raw(), size), {0x0F} );

   fb[0] = br_t{500};  // set data field
   check_result( s.encode_function_code(fb.raw(), size), {0x0C, 0xF4, 0x03} );

   fb[0] = br_if_t{500};
   check_result( s.encode_function_code(fb.raw(), size), {0x0D, 0xF4, 0x03} );
}

TEST_CASE("encode parametric ops instructions test", "[encode_parametric_ops_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = drop_t{};
   check_result( s.encode_function_code(fb.raw(), 1), {0x1A} );

   fb[0] = select_t{};
   check_result( s.encode_function_code(fb.raw(), 1), {0x1B} );
}

TEST_CASE("encode variable access ops instructions test", "[encode_variable_access_ops_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = get_local_t{1};  // set index, test different size of uint32_t
   check_result( s.encode_function_code(fb.raw(), 1), {0x20, 0x01} );

   fb[0] = set_local_t{500};  // set index
   check_result( s.encode_function_code(fb.raw(), 1), {0x21, 0xF4, 0x03} );

   fb[0] = tee_local_t{11};  // set index
   check_result( s.encode_function_code(fb.raw(), 1), {0x22, 0x0B} );

   fb[0] = get_global_t{10};  // set index
   check_result( s.encode_function_code(fb.raw(), 1), {0x23, 0x0A} );

   fb[0] = set_global_t{624485};  // set index
   check_result( s.encode_function_code(fb.raw(), 1), {0x24, 0xE5, 0x8E, 0x26} );
}

TEST_CASE("encode memory load test", "[encode_memory_load_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_load_t { 1, 10 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x28, 0x01, 0x0A} );

   fb[0] = i64_load_t { 1, 624485 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x29, 0x01, 0xE5, 0x8E, 0x26} );

   fb[0] = f32_load_t { 1, 10 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2A, 0x01, 0x0A} );

   fb[0] = f64_load_t { 1, 624485 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2B, 0x01, 0xE5, 0x8E, 0x26} );

   fb[0] = i32_load8_s_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2C, 0x05, 0xF4, 0x03} );

   fb[0] = i32_load8_u_t { 3, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2D, 0x03, 0xF4, 0x03} );

   fb[0] = i32_load16_s_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2E, 0x05, 0xF4, 0x03} );

   fb[0] = i32_load16_u_t { 3, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x2F, 0x03, 0xF4, 0x03} );

   fb[0] = i64_load8_s_t { 6, 6 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x30, 0x06, 0x06} );

   fb[0] = i64_load8_u_t { 3, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x31, 0x03, 0xF4, 0x03} );

   fb[0] = i64_load16_s_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x32, 0x05, 0xF4, 0x03} );

   fb[0] = i64_load16_u_t { 3, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x33, 0x03, 0xF4, 0x03} );

   fb[0] = i64_load32_s_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x34, 0x05, 0xF4, 0x03} );

   fb[0] = i64_load32_u_t { 3, 5 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x35, 0x03, 0x05} );
}

TEST_CASE("encode memory store test", "[encode_memory_store_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_store_t { 1, 10 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x36, 0x01, 0x0A} );

   fb[0] = i64_store_t { 1, 624485 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x37, 0x01, 0xE5, 0x8E, 0x26} );

   fb[0] = f32_store_t { 1, 10 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x38, 0x01, 0x0A} );

   fb[0] = f64_store_t { 1, 624485 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x39, 0x01, 0xE5, 0x8E, 0x26} );

   fb[0] = i32_store8_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x3A, 0x05, 0xF4, 0x03} );

   fb[0] = i32_store16_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x3B, 0x05, 0xF4, 0x03} );

   fb[0] = i64_store8_t { 6, 6 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x3C, 0x06, 0x06} );

   fb[0] = i64_store16_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x3D, 0x05, 0xF4, 0x03} );

   fb[0] = i64_store32_t { 5, 500 };  // set align and offset, each uint32_t. 
   check_result( s.encode_function_code(fb.raw(), size), {0x3E, 0x05, 0xF4, 0x03} );
}

TEST_CASE("encode memory misc test", "[encode_memory_misc_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = current_memory_t { };
   check_result( s.encode_function_code(fb.raw(), size), {0x3F, 0x00} );

   fb[0] = grow_memory_t { };
   check_result( s.encode_function_code(fb.raw(), size), {0x40, 0x00} );
}

TEST_CASE("encode const instructions test", "[encode_const_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_const_t { 500 };
   check_result( s.encode_function_code(fb.raw(), size), {0x41, 0xF4, 0x03} );

   fb[0] = i64_const_t { (int64_t)624485 };
   check_result( s.encode_function_code(fb.raw(), size), {0x42, 0xE5, 0x8E, 0x26} );
}

TEST_CASE("encode i32 comparison instructions test", "[encode_i32_comparison_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_eqz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x45} );

   fb[0] = i32_eq_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x46} );

   fb[0] = i32_ne_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x47} );

   fb[0] = i32_lt_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x48} );

   fb[0] = i32_lt_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x49} );

   fb[0] = i32_gt_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4A} );

   fb[0] = i32_gt_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4B} );

   fb[0] = i32_le_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4C} );

   fb[0] = i32_le_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4D} );

   fb[0] = i32_ge_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4E} );

   fb[0] = i32_ge_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x4F} );
}

TEST_CASE("encode i64 comparison instructions test", "[encode_i32_comparison_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i64_eqz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x50} );

   fb[0] = i64_eq_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x51} );

   fb[0] = i64_ne_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x52} );

   fb[0] = i64_lt_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x53} );

   fb[0] = i64_lt_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x54} );

   fb[0] = i64_gt_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x55} );

   fb[0] = i64_gt_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x56} );

   fb[0] = i64_le_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x57} );

   fb[0] = i64_le_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x58} );

   fb[0] = i64_ge_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x59} );

   fb[0] = i64_ge_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5A} );
}

TEST_CASE("encode f32 comparison instructions test", "[encode_i32_comparison_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f32_eq_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5B} );

   fb[0] = f32_ne_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5C} );

   fb[0] = f32_lt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5D} );

   fb[0] = f32_gt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5E} );

   fb[0] = f32_le_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x5F} );

   fb[0] = f32_ge_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x60} );
}

TEST_CASE("encode f64 comparison instructions test", "[encode_i32_comparison_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f64_eq_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x61} );

   fb[0] = f64_ne_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x62} );

   fb[0] = f64_lt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x63} );

   fb[0] = f64_gt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x64} );

   fb[0] = f64_le_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x65} );

   fb[0] = f64_ge_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x66} );
}

TEST_CASE("encode i32 arithmetic instructions test", "[encode_i32_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_clz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x67} );

   fb[0] = i32_ctz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x68} );

   fb[0] = i32_popcnt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x69} );

   fb[0] = i32_add_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6A} );

   fb[0] = i32_sub_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6B} );

   fb[0] = i32_mul_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6C} );

   fb[0] = i32_div_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6D} );

   fb[0] = i32_div_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6E} );

   fb[0] = i32_rem_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x6F} );

   fb[0] = i32_rem_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x70} );

   fb[0] = i32_and_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x71} );

   fb[0] = i32_or_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x72} );

   fb[0] = i32_xor_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x73} );

   fb[0] = i32_shl_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x74} );

   fb[0] = i32_shr_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x75} );

   fb[0] = i32_shr_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x76} );

   fb[0] = i32_rotl_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x77} );

   fb[0] = i32_rotr_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x78} );
}

TEST_CASE("encode i64 arithmetic instructions test", "[encode_i64_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i64_clz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x79} );

   fb[0] = i64_ctz_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7A} );

   fb[0] = i64_popcnt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7B} );

   fb[0] = i64_add_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7C} );

   fb[0] = i64_sub_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7D} );

   fb[0] = i64_mul_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7E} );

   fb[0] = i64_div_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x7F} );

   fb[0] = i64_div_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x80} );

   fb[0] = i64_rem_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x81} );

   fb[0] = i64_rem_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x82} );

   fb[0] = i64_and_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x83} );

   fb[0] = i64_or_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x84} );

   fb[0] = i64_xor_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x85} );

   fb[0] = i64_shl_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x86} );

   fb[0] = i64_shr_s_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x87} );

   fb[0] = i64_shr_u_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x88} );

   fb[0] = i64_rotl_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x89} );

   fb[0] = i64_rotr_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8A} );
}


TEST_CASE("encode f32 arithmetic instructions test", "[encode_f32_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f32_abs_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8B} );

   fb[0] = f32_neg_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8C} );

   fb[0] = f32_ceil_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8D} );

   fb[0] = f32_floor_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8E} );

   fb[0] = f32_trunc_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x8F} );

   fb[0] = f32_nearest_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x90} );

   fb[0] = f32_sqrt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x91} );

   fb[0] = f32_add_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x92} );

   fb[0] = f32_sub_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x93} );

   fb[0] = f32_mul_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x94} );

   fb[0] = f32_div_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x95} );

   fb[0] = f32_min_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x96} );

   fb[0] = f32_max_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x97} );

   fb[0] = f32_copysign_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x98} );
}


TEST_CASE("encode f64 arithmetic instructions test", "[encode_f64_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f64_abs_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x99} );

   fb[0] = f64_neg_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9A} );

   fb[0] = f64_ceil_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9B} );

   fb[0] = f64_floor_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9C} );

   fb[0] = f64_trunc_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9D} );

   fb[0] = f64_nearest_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9E} );

   fb[0] = f64_sqrt_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0x9F} );

   fb[0] = f64_add_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA0} );

   fb[0] = f64_sub_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA1} );

   fb[0] = f64_mul_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA2} );

   fb[0] = f64_div_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA3} );

   fb[0] = f64_min_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA4} );

   fb[0] = f64_max_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA5} );

   fb[0] = f64_copysign_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA6} );
}


TEST_CASE("encode i32 conversion instructions test", "[encode_i32_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_wrap_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA7} );

   fb[0] = i32_trunc_s_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA8} );

   fb[0] = i32_trunc_u_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xA9} );

   fb[0] = i32_trunc_s_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAA} );

   fb[0] = i32_trunc_u_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAB} );
}

TEST_CASE("encode i64 conversion instructions test", "[encode_i64_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i64_extend_s_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAC} );

   fb[0] = i64_extend_u_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAD} );

   fb[0] = i64_trunc_s_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAE} );

   fb[0] = i64_trunc_u_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xAF} );

   fb[0] = i64_trunc_s_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB0} );

   fb[0] = i64_trunc_u_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB1} );
}

TEST_CASE("encode f32 conversion instructions test", "[encode_f32_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f32_convert_s_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB2} );

   fb[0] = f32_convert_u_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB3} );

   fb[0] = f32_convert_s_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB4} );

   fb[0] = f32_convert_u_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB5} );

   fb[0] = f32_demote_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB6} );
}

TEST_CASE("encode f64 conversion instructions test", "[encode_f64_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = f64_convert_s_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB7} );

   fb[0] = f64_convert_u_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB8} );

   fb[0] = f64_convert_s_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xB9} );

   fb[0] = f64_convert_u_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBA} );

   fb[0] = f64_promote_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBB} );
}


TEST_CASE("encode reinterpret conversion instructions test", "[encode_reinterpret_arithmetic_instructions_test]") {
   serializer s(allocator);
   size_t size = 1;
   guarded_vector<opcode> fb {allocator, size};

   fb[0] = i32_reinterpret_f32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBC} );

   fb[0] = i64_reinterpret_f64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBD} );

   fb[0] = f32_reinterpret_i32_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBE} );

   fb[0] = f64_reinterpret_i64_t {};
   check_result( s.encode_function_code(fb.raw(), size), {0xBF} );
}


TEST_CASE("encode multiple instructions test", "[encode_multiple_instructions_test]") {
   serializer s(allocator);

   size_t size = 0;
   guarded_vector<opcode> fb {allocator, 100};

   fb[size++] = unreachable_t{};
   fb[size++] = nop_t{};         
   fb[size++] = block_t{};       
   fb[size++] = loop_t{};        
   fb[size++] = if_t{};          
   fb[size++] = else_t{};        
   fb[size++] = end_t{};         
   fb[size++] = return_t{};         
   check_result( s.encode_function_code(fb.raw(), size), {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x0B, 0x0F} );
}
