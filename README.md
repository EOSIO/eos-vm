# EOS-VM

## A Low-Latency, High Performance and Extensible WebAssembly Backend Library

![EOSIO Labs](https://img.shields.io/badge/EOSIO-Labs-5cb3ff.svg)

With WebAssembly (WASM) becoming ever more ubiquitous, there is a greater need for a succinct implementation of a WASM backend.  We implemented __EOS-VM__ because all existing backends we evaluated fell short in meeting all of our needs for a WASM backend best suited for use in a public blockchain environment. __EOS-VM__ was designed to satisfy five focal tenets.  
   1) Satisfying the needs of a blockchain.
   2) Security built into the framework.
   3) Performance centric design.
   4) Light weight and easy to integrate solution.
   5) Effortless extendability.

# About EOSIO Labs

EOSIO Labs repositories are experimental.  Developers in the community are encouraged to use EOSIO Labs repositories as the basis for code and concepts to incorporate into their applications. Community members are also welcome to contribute and further develop these repositories. Since these repositories are not supported by Block.one, we may not provide responses to issue reports, pull requests, updates to functionality, or other requests from the community, and we encourage the community to take responsibility for these.

## EOS-VM A VM for Blockchain
- Given that all executions on the blockchain have to be deterministic, floating point operations are of particular interest to us.  Because of the non-deterministic nature of rounding modes, NaNs and denormals, special care has to be made to ensure a deterministic environment on all supported platforms.  This comes in the form of "softfloat", a software implementation of IEEE-754 float point arithmetic, which is constrained further to ensure determinism.  If this determinism is not required, hardware based floating point operations are still available through a compile time define.

- The ability to ensure that execution doesn't overbound CPU time that is allotted for a given run is a central component of a resource limited blockchain.  This is satisfied by the watchdog timer system (as mentioned below, this mechanism is also useful for general security).

- The fact that any secondary limits/constraints (i.e. stack size, call depth, etc.) can cause consensus failures if these restrictions do not match any previous backend that was in place, __EOS-VM__ has all of these constraints user definable through either a compile-time system or run-time based on the use case and data type involved.

## EOS-VM Baked-in Security
- The fundamental data types that make up __EOS-VM__ are built with certain invariants designed into them from the onset.  This means that explicit checks and validations, which can be error-prone because of programmer forgetfulness, are not needed as the data types themselves maintain these invariants and kill the execution if these are violated.  

- In addition to these core data types, some of the special purpose allocators utilize the security of the CPU and core OS to satisfy that memory is properly sandboxed (a guard paging mechanism).  

- Two mechanisms are available to the user to bound the execution of WASM:
  1) A simple instruction counter based bounding, this incurs a performance penalty, but doesn't require multi-threading.
  2) A watchdog timer solution that incurs no noticeable overhead during WASM execution.

- Because of the utilization of guard paging for the memory operations, host function calls that execute natively don't have to explicitly validate pointers that are passed into these functions if access outside of the sandboxed memory occurs, please note special care should be made to ensure that the host function can fail hard, i.e. not call destructors and have no negative impact.

- At no point during parsing or evaluation does EOS-VM use unbounded recursion or loops, everything is tightly bound to limit or eliminate the ability for a bad or corrupt WASM to cause a crash or infinitely hang the machine.

- All of these solutions are transparent to the developer and allow for more succinct functions that are not cluttered with external checks and only the core logic is needed in most places.  

## EOS-VM High-Performance Execution
 - Certain design decisions were made to maximize the performance of interpreter implementation.  As mentioned above, __EOS-VM__ has custom allocators and memory management that fits the needs and use cases for different access patterns and allocation requirements.  These allocators are used to back the core data types (fast vector, WASM stack, fast variant, WASM module), and as such do not "own" the memory that they use for their operations.  These non-owning data structures allow for the ability to use the memory cleanly and not have to concern the data type with destructing when going out of scope, which can increase the performance for certain areas EOS-VM without loss of generality for the developer.  Since the data is held by these allocators and have lifetimes that match that of a WASM module, no copies of these heavyweight data types is ever needed.  Once an element in an EOS-VM is constructed, that is its home and final resting place for the lifetime of the WASM module.  

 - A fast `variant` or discriminating union type is the fundamental data type that represents a WASM opcode or a WASM stack element.  This allows for a clean interface to "visit" each WASM opcode without any loss of performance.  This visiting is statically derivable and not dynamically dispatched like more classical approaches that use the object-oriented visitor pattern.  In addition to a `visit` function that acts the same as `std::visit`, a custom dispatcher is defined that allows for a similar interface but with __EOS-VM__ specific optimizations and assumptions.

 - Because of the utilization of native paging mechanisms, almost all memory operations are very close to native if not at parity with native memory operations.

 - Host functions are callable through a thin layer that doesn't incur heavy performance penalties.

 - Because of the high compaction and linear nature of the builtin allocators, this allows for a very cache friendly environment and further allows for high performance execution.

## EOS-VM Effortless Integration
 - With the exception of the softfloat library, which is an external dependency, __EOS-VM__ is a header only implementation.

 - Given the needs of the end user, integration can be as simple as pointing to the include directory.

 - __EOS-VM__ utilizes __CMake__ which allows integration into a project to be as little as adding `eos-vm` to the list of targets in the `target_link_libraries`.

 - If the need is only single-threaded a self-contained backend type is defined for the user to encapsulate all the components needed, which allows for source code integration to be constructing an instance of that type and adding "host functions" to the `registered_host_functions`.  Registering the host functions is as easy as calling a function with the function/member pointer and supplying the WASM module name and function name.

 - If multi-threaded execution is needed (i.e. multiple backends running at once), then the above integration is needed and the user will have to also construct thread specific watchdog timers and linear memory allocators.  These are also designed to be effortlessly registered to a particular WASM backend.  

## EOS-VM Highly Extensible Design
- Given the __EOS-VM__ variant type and visitor system, new backends with custom logic can be easily defined and allows the same level of flexibility and code reuse as a much more heavyweight OOP __Visitor__ or __Listener__ design.

- Since the design of __EOS-VM__ is component based, with each component being very self-contained, new backends or tools for WASM can be crafted from previously defined components while only needing to define the logic for the extended functionality that is needed, with very little, to no, boilerplate needed.

- Extensions to WASM itself can be made by simply defining the new section (aka C++ class field) for the module and the function needed to parse an element of that section.  This will allow for tooling to be constructed at a rapid pace for custom WASMs for a multitude of needs (debugging, profiling, etc.).

## Contributing

[Contributing Guide](./CONTRIBUTING.md)

[Code of Conduct](./CONTRIBUTING.md#conduct)

## Important

Refer to [LICENSE](./LICENSE) for copyright notice.  Block.one makes its contribution on a voluntary basis as a member of the EOSIO community and is not responsible for ensuring the overall performance of the software or any related applications.  We make no representation, warranty, guarantee or undertaking in respect of the software or any related documentation, whether expressed or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall we be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or documentation or the use or other dealings in the software or documentation. Any test results or performance figures are indicative and will not reflect performance under all conditions.  Any reference to any third party or third-party product, service or other resource is not an endorsement or recommendation by Block.one.  We are not responsible, and disclaim any and all responsibility and liability, for your use of or reliance on any of these resources. Third-party resources may be updated, changed or terminated at any time, so the information here may be out of date or inaccurate.  Any person using or offering this software in connection with providing software, goods or services to third parties shall advise such third parties of these license terms, disclaimers and exclusions of liability.  Block.one, EOSIO, EOSIO Labs, EOS, the heptahedron and associated logos are trademarks of Block.one.
