#include <stdio.h>
#include "../interface/reporter.h"

int save_system_report_to_json(SystemStats *stats, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Dosya açılamadı");
        return -1;
    }

    fprintf(fp, "{\n");

    // CPU
    fprintf(fp, "  \"cpu\": {\n");
    fprintf(fp, "    \"usage\": %.2f,\n", stats->cpu.total - stats->cpu.idle);
    fprintf(fp, "    \"user\": %.2f,\n", stats->cpu.user);
    fprintf(fp, "    \"system\": %.2f,\n", stats->cpu.system);
    fprintf(fp, "    \"idle\": %.2f\n", stats->cpu.idle);
    fprintf(fp, "  },\n");

    // Memory
    fprintf(fp, "  \"memory\": {\n");
    fprintf(fp, "    \"total_gb\": %.2f,\n", stats->mem.total_gb);
    fprintf(fp, "    \"used_gb\": %.2f,\n", stats->mem.used_gb);
    fprintf(fp, "    \"swap_used_gb\": %.2f,\n", stats->mem.swap_used_gb);
    fprintf(fp, "    \"swap_total_gb\": %.2f\n", stats->mem.swap_total_gb);
    fprintf(fp, "  },\n");

    // Network
    fprintf(fp, "  \"net\": [\n");
    for (int i = 0; i < stats->net.interface_count; i++) {
        NetInterfaceStats *iface = &stats->net.interfaces[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"name\": \"%s\",\n", iface->name);
        fprintf(fp, "      \"rx_bytes\": %lu,\n", iface->rx_bytes);
        fprintf(fp, "      \"tx_bytes\": %lu\n", iface->tx_bytes);
        fprintf(fp, "    }%s\n", (i == stats->net.interface_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");

    // Disks
    fprintf(fp, "  \"disks\": [\n");
    for (int i = 0; i < stats->disks.disk_count; i++) {
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"name\": \"%s\",\n", stats->disks.disks[i].name);
        fprintf(fp, "      \"read_bytes\": %llu,\n", stats->disks.disks[i].read_bytes);
        fprintf(fp, "      \"write_bytes\": %llu\n", stats->disks.disks[i].write_bytes);
        fprintf(fp, "    }%s\n", (i == stats->disks.disk_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");

    // Connectivity
    fprintf(fp, "  \"connectivity\": [\n");
    for (int i = 0; i < stats->conn.connection_count; i++) {
        ConnectionInfo *c = &stats->conn.connections[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"local_address\": \"%s\",\n", c->local_address);
        fprintf(fp, "      \"local_port\": %d,\n", c->local_port);
        fprintf(fp, "      \"remote_address\": \"%s\",\n", c->remote_address);
        fprintf(fp, "      \"remote_port\": %d,\n", c->remote_port);
        fprintf(fp, "      \"state\": \"%s\"\n", c->state);
        fprintf(fp, "    }%s\n", (i == stats->conn.connection_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ],\n");


    fprintf(fp, "  \"active top 5 processes\": [\n");
    int process_limit = stats->processes.count < 5 ? stats->processes.count : 5; // Limit to top 5
    for (int i = 0; i < process_limit; i++) {
        TopProcess *p = &stats->processes.processes[i];
        fprintf(fp, "    {\n");
        fprintf(fp, "      \"pid\": %d,\n", p->pid);
        fprintf(fp, "      \"name\": \"%s\",\n", p->name);
        fprintf(fp, "      \"cpu usage\": \"%f\"\n", p->cpu_usage);
        fprintf(fp, "    }%s\n", (i == process_limit - 1) ? "" : ",");
    }
    fprintf(fp, "  ]\n");

    // Close JSON
    fprintf(fp, "}\n");

    fclose(fp);
    return 0;
}
