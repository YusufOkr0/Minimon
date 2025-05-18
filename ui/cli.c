#include <stdio.h>
#include "cli.h"

// Basit renklendirme için ANSI escape kodları (terminal destekliyorsa)
#define COLOR_RESET   "\033[0m"
#define COLOR_TITLE   "\033[1;34m"
#define COLOR_HEADER  "\033[1;33m"
#define COLOR_VALUE   "\033[0;37m"
#define COLOR_WARN    "\033[1;31m"

void print_cpu_stats(CPUStats *cpu) {
    float usage = cpu->total - cpu->idle;
    printf(COLOR_TITLE "=== CPU Usage ===\n" COLOR_RESET);
    printf("Total Usage: " COLOR_VALUE "%.1f%%\n" COLOR_RESET, usage);
    printf("User:        " COLOR_VALUE "%.1f%%\n" COLOR_RESET, cpu->user);
    printf("System:      " COLOR_VALUE "%.1f%%\n" COLOR_RESET, cpu->system);
    printf("Idle:        " COLOR_VALUE "%.1f%%\n\n" COLOR_RESET, cpu->idle);
}

void print_mem_stats(MemStats *mem) {
    printf(COLOR_TITLE "=== Memory Usage ===\n" COLOR_RESET);
    printf("Total RAM:   " COLOR_VALUE "%.2f GB\n" COLOR_RESET, mem->total_gb);
    printf("Used RAM:    " COLOR_VALUE "%.2f GB\n" COLOR_RESET, mem->used_gb);
    printf("Swap Used:   " COLOR_VALUE "%.2f GB\n" COLOR_RESET, mem->swap_used_gb);
    printf("Swap Total:  " COLOR_VALUE "%.2f GB\n\n" COLOR_RESET, mem->swap_total_gb);
}

void print_net_stats(NetStats *netstats) {
    printf(COLOR_TITLE "=== Network Interfaces ===\n" COLOR_RESET);
    printf(COLOR_HEADER "%-15s %15s %15s\n" COLOR_RESET, "Interface", "RX Bytes", "TX Bytes");
    printf("-------------------------------------------------------\n");
    for (int i = 0; i < netstats->interface_count; i++) {
        printf("%-15s %15lu %15lu\n", 
               netstats->interfaces[i].name,
               netstats->interfaces[i].rx_bytes,
               netstats->interfaces[i].tx_bytes);
    }
    printf("\n");
}

void print_size(unsigned long long bytes) {
    const double KB = 1024.0;
    const double MB = KB * 1024;
    const double GB = MB * 1024;

    if (bytes >= GB) {
        printf("%.2f GB", bytes / GB);
    } else if (bytes >= MB) {
        printf("%.2f MB", bytes / MB);
    } else if (bytes >= KB) {
        printf("%.2f KB", bytes / KB);
    } else {
        printf("%llu B", bytes);
    }
}

void print_disk_stats(DiskStatsList *disk_list) {
    printf(COLOR_TITLE "=== Disk Statistics ===\n" COLOR_RESET);
    printf(COLOR_HEADER "%-8s  %-15s  %-15s\n" COLOR_RESET, "Disk", "Read", "Write");
    printf("-------------------------------------------------------\n");

    for (int i = 0; i < disk_list->disk_count; i++) {
        DiskStats *d = &disk_list->disks[i];
        printf("%-8s  ", d->name);
        print_size(d->read_bytes);
        printf("      ");
        print_size(d->write_bytes);
        printf("\n");
    }
    printf("\n");
}

void print_connectivity_stats(ConnectivityInfo *conn_info) {
    printf(COLOR_TITLE "=== Active Connections (%d) ===\n" COLOR_RESET, conn_info->connection_count);
    printf(COLOR_HEADER "%-22s %-7s %-22s %-7s %-12s\n" COLOR_RESET,
           "Local Address", "L.Port", "Remote Address", "R.Port", "State");
    printf("-------------------------------------------------------------------------------\n");

    for (int i = 0; i < conn_info->connection_count; ++i) {
        const ConnectionInfo *c = &conn_info->connections[i];
        printf("%-22s %-7d %-22s %-7d %-12s\n",
               c->local_address, c->local_port,
               c->remote_address, c->remote_port,
               c->state);
    }
    printf("\n");
}

void print_process_list(ProcessList *plist) {
    if (plist == NULL || plist->count == 0) {
        printf(COLOR_WARN "No processes to show.\n\n" COLOR_RESET);
        return;
    }

    printf(COLOR_TITLE "=== Process List ===\n" COLOR_RESET);
    printf(COLOR_HEADER "%-8s %-25s %s\n" COLOR_RESET, "PID", "Process Name", "Command Line");
    printf("-------------------------------------------------------------------------------\n");

    for (int i = 0; i < plist->count; i++) {
        const ProcessInfo *p = &plist->processes[i];
        printf("%-8d %-25s %.80s\n", p->pid, p->name, p->cmdline);
    }
    printf("\n");
}

void print_top_processes(TopProcessList *toplist) {
    if (!toplist || toplist->count == 0) {
        printf(COLOR_WARN "No top processes found.\n\n" COLOR_RESET);
        return;
    }

    printf(COLOR_TITLE "=== Top CPU-consuming Processes ===\n" COLOR_RESET);
    printf(COLOR_HEADER "%-8s %-25s %s\n" COLOR_RESET, "PID", "Process Name", "CPU Usage (%)");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < toplist->count; ++i) {
        TopProcess *p = &toplist->processes[i];
        printf("%-8d %-25s %.2f\n", p->pid, p->name, p->cpu_usage);
    }
    printf("\n");
}
