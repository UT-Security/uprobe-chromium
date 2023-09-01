# Custom uprobe for chrome
The code in this repo shows an example of attaching a custom handler (uprobe) to the `Math.sqrt()` function in V8. It can be used to precisely measure the hardware perf events for some browser workload, or repurposed for low-level instrumentation in general. 

To build and run the code:

1. $ make uprobe

2. $ sudo ./uprobe

3. Then (the order matters), run chrome with `/path/to/debugbuild/chrome --no-sandbox` (we need symbol info so debug build; our instrumentation violates the seccomp sandbox)

Expected behavior:
If you call `Math.sqrt` from JavaScript, you are expected to see the `uprobe` program's handler `handle_event` getting fired 

## FAQ
- Q: I want to use my special chromium build. What is the specific `gn` build flag required for the symbol info so I can modify the build configuration myself?

    A: `v8_symbol_level = 2` should work. Note you can also modify `symbol_level` which is a superset that implies `v8_symbol_level` if the latter is not specified.  

- Q: I see a V8 fatal error when I start chromium, what is this error and how can I fix it? 

    A: It's probably a debug-only check in V8 that will terminate the process when V8 detects instrumentation. You can safely disable it by commenting out the corresponding `#ifdef DEBUG` block in `v8/src/execution/isolate.cc` and recompile chromium. Or, note that the build flag for a target is stored in `{out_dir}/obj/{target}.ninja` so you may change the `defines` there and build again. 

- Q: What should I modify if I want to attach to other code points rather than `Math.sqrt()`?

    A: If you want to change the attached code point, change `SEC("uprobe//home/utsec/chromium/src/out/Debug/libv8.so:Builtins_MathSqrt")` in the BPF program `uprobe.bpf.c`.

# Reference

The base infra is from [`libbpf-bootstrap`](https://github.com/libbpf/libbpf-bootstrap), and [this blog](https://nakryiko.com/) can help a lot if you want to learn more about the libbpf details.


