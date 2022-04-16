# phi
Inject periodic control handovers into WebAssembly modules.

WebAssembly is a binary code format which is designed to run in a secure sandboxenvironment. 
The security features of this standard make it very useful for facilitatingthe execution of untrusted code as part of running systems. 
However, even when thecode is effectively prevented from causing any harm, there is still one problem to besolved - the control loss problem for the host. 
When the host calls the guest code, what prevents the guest from just running indefinitely, holding on to system resources?

Phi is a runtime-independent, Wasm-level, solution to the host's control-loss problem, using [Binaryen](https://github.com/WebAssembly/binaryen). 
Phi transforms the Wasm module's code such that it is guaranteed to periodically call an imported function supplied by the host. 
It does that by injecting those calls in the necessary points in the code as to prevent an infinite loop which does not contain a control handover.

Phi adds code to the Wasm module which comes at some performance cost when running the treated Wasm module.
