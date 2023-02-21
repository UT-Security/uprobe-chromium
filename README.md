# Custom uprobe for chrome
1. $ make uprobe

2. $ sudo ./uprobe

3. Then (the order matters), run chrome with `/path/to/debugbuild/chrome --no-sandbox` (we need symbol info so debug build; our instrumentation violates the seccomp sandbox)

Expected:
If you call `Math.sqrt` in the browser, you are expected to see the `uprobe` program's handler `handle_event` got fired 

If you see a V8 fatal error, it's probably a debug-only check in V8 and you can safely disable it by comment out the corresponding `#ifdef DEBUG` block in `v8/src/execution/isolate.cc` and recompile chrome. It shouldn't take a long time.  

If you want to change the attached code point, change `SEC("uprobe//home/utsec/chromium/src/out/Debug/libv8.so:Builtins_MathSqrt")` in the BPF program.

# Reference

The base infra is from [`libbpf-bootstrap`](https://github.com/libbpf/libbpf-bootstrap), and [this blog](https://nakryiko.com/) helps a lot.


