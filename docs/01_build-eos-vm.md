## Building EOS-VM
To build __EOS-VM__ you will need a fully C++17 compliant toolchain, since **CMake** is not perfect some toolchains will appear to **CMake** as 17 compliant but are not, we will do
our best to special case these as they arise.

Since __EOS-VM__ is designed to be a header only library (with the exception of softfloat, until the new header only softfloat library is finished), building __EOS-VM__ is not necessary
to use __EOS-VM__ in a C++ project. But, if you would like to use the softfloat capabilities, build the example tools or the tests, then all that is needed is to create a build directory
and execute the command `cmake ..` and then `make` after this all tools and tests will be built.

## Using The Example Tools
Once you have built __EOS-VM__ you will notice 3 tools in the directory **build/tools**, you can run your test WASMs by executing the command `eos-vm-interp <path>/<wasm name>.wasm` this
will then run all exported functions within that WASM.  You can also run `bench-interp <path>/<wasm name>.wasm` and get two times in nanoseconds; the time to parse and instaniate your WASM
and the time to execute your WASM.  The last tool is `hello-driver`, this has a prebaked in helloworld WASM and uses user input to bound the number of loops the printing occurs and whether
it should assert, this tool is an example of how to setup a fully integrated solution with host functions.

Both of these are designed to be modified by the end-user and are simply there to show how to easily integrate __EOS-VM__ into your own project.