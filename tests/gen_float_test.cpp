/*
 * This program generates contracts that test NaN propagation in floating point instructions.
 *
 * It uses the eos-vm interpreter to generate the reference values.
 *
 * On failure, the contract prints the arguments that caused the failure
 * as a hex blob.
 */

#include <iostream>
#include <vector>
#include <eosio/vm/backend.hpp>
#include "utils.hpp"

using namespace eosio::vm;

std::vector<uint64_t> f64_values = {
   0x0000'0000'0000'0000, // 0
   0x7FF0'0000'0000'0000, // inf
   0x7FF0'0000'0000'0001, // SNaN
   0x7FF8'0000'0000'0000, // QNaN
   0x7FF4'0000'0000'0000, // SNaN
   0x7FFC'0000'0000'0000, // QNaN
   0x0000'0000'0000'0001, // denorm min
   0x000F'FFFF'FFFF'FFFF, // largest denormal
   0x7FEF'FFFF'FFFF'FFFF, // max
};

uint64_t calc_f64_unop_result(uint64_t x, unsigned char op) {
   std::vector<uint8_t> f64_unop_wasm = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60,
      0x01, 0x7c, 0x01, 0x7e, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x01, 0x02,
      0x66, 0x6e, 0x00, 0x00, 0x0a, 0x08, 0x01, 0x06, 0x00, 0x20, 0x00, op,
      0xbd, 0x0b
   };
   backend<nullptr_t> bkend{f64_unop_wasm};
   return bkend.call_with_return(nullptr, "env", "fn", bit_cast<double>(x))->to_ui64();
}

std::vector<unsigned char> f64_unop_prefix1{
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x04, 0x60,
   0x02, 0x7f, 0x7f, 0x00, 0x60, 0x00, 0x00, 0x60, 0x02, 0x7c, 0x7c, 0x00,
   0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x02, 0x1c, 0x02, 0x03, 0x65, 0x6e,
   0x76, 0x08, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x68, 0x65, 0x78, 0x00, 0x00,
   0x03, 0x65, 0x6e, 0x76, 0x05, 0x61, 0x62, 0x6f, 0x72, 0x74, 0x00, 0x01,
   0x03, 0x03, 0x02, 0x02, 0x03, 0x05, 0x03, 0x01, 0x00, 0x01, 0x07, 0x09,
   0x01, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x79, 0x00, 0x03, 0x0a
};
std::vector<unsigned char> f64_unop_prefix2{
   /*0x42,*/ 0x02,
   0x29, 0x00, 0x20, 0x00
};
std::vector<unsigned char> f64_unop_prefix3{
   /*0x9f*/ 0xbd, 0x20, 0x01, 0xbd, 0x52, 0x04, 0x40,
   0x41, 0x00, 0x20, 0x00, 0x39, 0x03, 0x00, 0x41, 0x00, 0x41, 0x08, 0x10,
   0x00, 0x41, 0x00, 0x20, 0x01, 0x39, 0x03, 0x00, 0x41, 0x00, 0x41, 0x08,
   0x10, 0x00, 0x10, 0x01, 0x0b, 0x0b
};

// 20 bytes
void write_one_f64_unop_test(std::streambuf* out, uint64_t arg0, uint64_t result) {
   out->sputc(0x44);
   out->sputn(reinterpret_cast<char*>(&arg0), sizeof(arg0));
   out->sputc(0x44);
   out->sputn(reinterpret_cast<char*>(&result), sizeof(result));
   out->sputc(0x10);
   out->sputc(0x02);
}

void write_f64_unop_prefix(std::streambuf* out, std::size_t tests_size, unsigned char op) {
   varuint<32> fn_size(static_cast<uint32_t>(tests_size + 2));
   varuint<32> code_size(static_cast<uint32_t>(tests_size + fn_size.size() + 0x42 - 20 - 1));
   out->sputn((char*)f64_unop_prefix1.data(), f64_unop_prefix1.size());
   code_size.to(out);
   out->sputn((char*)f64_unop_prefix2.data(), f64_unop_prefix2.size());
   out->sputc((char)op);
   out->sputn((char*)f64_unop_prefix3.data(), f64_unop_prefix3.size());
   fn_size.to(out);
   out->sputc((char)0);
}

void make_f64_unop_tests(std::streambuf* out, unsigned char op) {
   write_f64_unop_prefix(out, 20*2*f64_values.size(), op);
   for(uint64_t signbitx : {0ull, (1ull << 63)}) {
      for(uint64_t x : f64_values) {
         x |= signbitx;
         uint64_t r = calc_f64_unop_result(x, op);
         write_one_f64_unop_test(out, x, r);
      }
   }
   out->sputc((char)0x0b);
}

uint64_t calc_f64_binop_result(uint64_t x, uint64_t y, unsigned char op) {
   std::vector<uint8_t> f64_binop_wasm = {
      0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x60,
      0x02, 0x7c, 0x7c, 0x01, 0x7e, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x01,
      0x02, 0x66, 0x6e, 0x00, 0x00, 0x0a, 0x0a, 0x01, 0x08, 0x00, 0x20, 0x00,
      0x20, 0x01, op, 0xbd, 0x0b
   };
   backend<nullptr_t> bkend{f64_binop_wasm};
   return bkend.call_with_return(nullptr, "env", "fn", bit_cast<double>(x), bit_cast<double>(y))->to_ui64();
}

std::vector<unsigned char> f64_binop_prefix1{
   0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x15, 0x04, 0x60,
   0x02, 0x7f, 0x7f, 0x00, 0x60, 0x00, 0x00, 0x60, 0x03, 0x7c, 0x7c, 0x7c,
   0x00, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x02, 0x1c, 0x02, 0x03, 0x65,
   0x6e, 0x76, 0x08, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x68, 0x65, 0x78, 0x00,
   0x00, 0x03, 0x65, 0x6e, 0x76, 0x05, 0x61, 0x62, 0x6f, 0x72, 0x74, 0x00,
   0x01, 0x03, 0x03, 0x02, 0x02, 0x03, 0x05, 0x03, 0x01, 0x00, 0x01, 0x07,
   0x09, 0x01, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x79, 0x00, 0x03, 0x0a
};

std::vector<unsigned char> f64_binop_prefix2{
  0x02, 0x2b, 0x00, 0x20, 0x00, 0x20, 0x01
};

std::vector<unsigned char> f64_binop_prefix3{
  0xbd, 0x20, 0x02, 0xbd,
  0x52, 0x04, 0x40, 0x41, 0x00, 0x20, 0x00, 0x39, 0x03, 0x00, 0x41, 0x00,
  0x41, 0x08, 0x10, 0x00, 0x41, 0x00, 0x20, 0x01, 0x39, 0x03, 0x00, 0x41,
  0x00, 0x41, 0x08, 0x10, 0x00, 0x10, 0x01, 0x0b, 0x0b
};

// 29 bytes
void write_one_f64_binop_test(std::streambuf* out, uint64_t arg0, uint64_t arg1, uint64_t result) {
   out->sputc(0x44);
   out->sputn(reinterpret_cast<char*>(&arg0), sizeof(arg0));
   out->sputc(0x44);
   out->sputn(reinterpret_cast<char*>(&arg1), sizeof(arg1));
   out->sputc(0x44);
   out->sputn(reinterpret_cast<char*>(&result), sizeof(result));
   out->sputc(0x10);
   out->sputc(0x02);
}

void write_f64_binop_prefix(std::streambuf* out, std::size_t tests_size, unsigned char op) {
   varuint<32> fn_size(static_cast<uint32_t>(tests_size + 2));
   varuint<32> code_size(static_cast<uint32_t>(tests_size + fn_size.size() + 0x4d - 29 - 1));
   out->sputn((char*)f64_binop_prefix1.data(), f64_binop_prefix1.size());
   code_size.to(out);
   out->sputn((char*)f64_binop_prefix2.data(), f64_binop_prefix2.size());
   out->sputc((char)op);
   out->sputn((char*)f64_binop_prefix3.data(), f64_binop_prefix3.size());
   fn_size.to(out);
   out->sputc((char)0);
}

void make_f64_binop_tests(std::streambuf* out, unsigned char op) {
   write_f64_binop_prefix(out, 29*4*f64_values.size()*f64_values.size(), op);
   for(uint64_t signbitx : {0ull, (1ull << 63)}) {
      for(uint64_t x : f64_values) {
         for(uint64_t signbity : {0ull, (1ull << 63)}) {
            for(uint64_t y : f64_values) {
               x |= signbitx;
               y |= signbity;
               uint64_t r = calc_f64_binop_result(x, y, op);
               write_one_f64_binop_test(out, x, y, r);
            }
         }
      }
   }
   out->sputc((char)0x0b);
}

int main(int argc, const char** argv) {
   if(argc != 2) {
      std::cerr << "Usage: gen_float_test <instr name>\n";
      return 2;
   }
   std::string instr{argv[1]};
   if(instr == "f64.ceil") {
      make_f64_unop_tests(std::cout.rdbuf(), 0x9b);
   } else if(instr == "f64.floor") {
      make_f64_unop_tests(std::cout.rdbuf(), 0x9c);
   } else if(instr == "f64.trunc") {
      make_f64_unop_tests(std::cout.rdbuf(), 0x9d);
   } else if(instr == "f64.nearest") {
      make_f64_unop_tests(std::cout.rdbuf(), 0x9e);
   } else if(instr == "f64.sqrt") {
      make_f64_unop_tests(std::cout.rdbuf(), 0x9f);
   } else if(instr == "f64.add") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa0);
   } else if(instr == "f64.sub") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa1);
   } else if(instr == "f64.mul") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa2);
   } else if(instr == "f64.div") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa3);
   } else if(instr == "f64.min") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa4);
   } else if(instr == "f64.max") {
      make_f64_binop_tests(std::cout.rdbuf(), 0xa5);
   }
}
