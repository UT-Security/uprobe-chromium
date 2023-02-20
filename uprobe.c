// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
#include <argp.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "uprobe.h"
#include "uprobe.skel.h"

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

uint64_t get_time(void)
{
	uint64_t cycles;
	asm volatile("rdtscp\n\t"
				 "shl $32, %%rdx\n\t"
				 "or %%rdx, %0\n\t"
				 : "=a"(cycles)
				 :
				 : "rcx", "rdx", "memory");
	return cycles;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	fprintf(stderr, "RDTSCP: %lu\n", get_time());

	return 0;
}

int main(int argc, char **argv)
{
	struct uprobe_bpf *skel;
	struct ring_buffer *rb = NULL;
	int err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Load and verify BPF program */
	skel = uprobe_bpf__open_and_load();
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	err = uprobe_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to auto-attach BPF skeleton: %d\n", err);
		goto cleanup;
	}

	/* Prepare a handler to sync with the BPF program */
	rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
	if (!rb) {
		err = -1;
		fprintf(stderr, "Failed to create ring buffer\n");
		goto cleanup;
	}

	while (!exiting) {
		err = ring_buffer__poll(rb, -1);
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			printf("Error polling bpf ring buffer: %d\n", err);
			break;
		}
	}

cleanup:
	uprobe_bpf__destroy(skel);
	return -err;
}
