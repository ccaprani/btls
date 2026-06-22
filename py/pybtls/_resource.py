"""
Best-effort host-memory helpers shared by the traffic loaders and the GPU
engine. Recorded traffic and garage pools are materialized in full, so a very
large input file can exhaust RAM; this module estimates that and warns.
"""

import os
import sys


def available_host_memory():
    """Free host RAM available now, in bytes. Respects a cgroup v2 memory limit
    if one is set (so it does not read the host's free memory inside a
    container), falling back to the physical free pages, then to a conservative
    default if the OS will not report it."""
    try:
        free = os.sysconf("SC_AVPHYS_PAGES") * os.sysconf("SC_PAGE_SIZE")
    except (ValueError, OSError, AttributeError):
        free = 8 * 1024 ** 3
    try:  # cgroup v2: (limit - current usage) of this process's own cgroup
        rel = ""
        with open("/proc/self/cgroup") as fh:
            for ln in fh:
                parts = ln.strip().split(":")
                if len(parts) == 3 and parts[0] == "0":
                    rel = parts[2]
                    break
        for base in ("/sys/fs/cgroup" + rel, "/sys/fs/cgroup"):
            try:
                with open(base + "/memory.max") as fh:
                    lim = fh.read().strip()
                if lim == "max":
                    break
                with open(base + "/memory.current") as fh:
                    used = int(fh.read().strip())
                free = min(free, max(0, int(lim) - used))
                break
            except (OSError, ValueError):
                continue
    except OSError:
        pass
    return free


# A traffic/garage record (~50-100 B of text on disk) becomes a C++ vehicle plus
# a Python wrapper plus the per-lane list (plus, for the CPU engine, a second C++
# copy at simulation time), so the in-memory footprint is roughly 10x the file
# size (measured ~5x for the load alone; the rest covers the simulation copy).
_MEM_PER_FILE_BYTE = 10


def _human(n):
    return f"{n / 1e9:.1f} GB" if n >= 1e9 else f"{n / 1e6:.0f} MB"


def warn_if_file_too_large(file_path, what="traffic file"):
    """Print a terminal warning (without blocking) if loading the whole file is
    likely to exhaust RAM. The recorded-traffic loader and the garage pool
    materialize the entire file in memory, so a very large file can OOM and the
    simulation cannot run. Generated traffic, by contrast, is streamed by the GPU
    engine in bounded memory."""
    try:
        file_bytes = os.path.getsize(file_path)
    except OSError:
        return
    avail = available_host_memory()
    estimate = file_bytes * _MEM_PER_FILE_BYTE
    if estimate > 0.8 * avail:
        print(
            f"*** WARNING: the {what} is {_human(file_bytes)} on disk and is "
            f"loaded entirely into memory (~{_human(estimate)} estimated, vs "
            f"~{_human(avail)} available) — this may exhaust RAM and fail. "
            f"Recorded traffic / garage pools cannot be streamed; use a shorter "
            f"file. (Generated traffic IS streamed in bounded memory.)",
            file=sys.stderr,
        )
