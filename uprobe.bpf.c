// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "uprobe.h"

/* license is mandatory */
char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 256 * 1024);
} rb SEC(".maps");

/* Our BPF program */
SEC("uprobe//home/utsec/chromium/src/out/Debug/libv8.so:Builtins_MathSqrt")
int BPF_KPROBE(uprobe_mathsqrt, int a)
{
	struct task_struct *task;
	struct event *e;

	e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
	if (!e)
		return 0;

	bpf_ringbuf_submit(e, 0);
	return 0;
}



