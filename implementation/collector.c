// core/collector.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>      // isdigit
#include <unistd.h>     // sysconf
#include <dirent.h>     // DIR, opendir, readdir, closedir

#include "../interface/collector.h"


// CPU 
int read_cpu_stats(CPUStats *cpu_stats) {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("Failed to open /proc/stat");
        return -1;
    }

    char line[256];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        perror("Failed to read from /proc/stat");
        return -1;
    }

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    int parsed = sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
                        &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

    if (parsed < 8) {
        fclose(fp);
        fprintf(stderr, "Failed to parse /proc/stat\n");
        return -1;
    }

    unsigned long long idle_all_time = idle + iowait;
    unsigned long long system_all_time = system + irq + softirq;
    unsigned long long total_time = user + nice + system_all_time + idle_all_time + steal;

    cpu_stats->user = (float)user * 100.0 / total_time;
    cpu_stats->system = (float)system_all_time * 100.0 / total_time;
    cpu_stats->idle = (float)idle_all_time * 100.0 / total_time;
    cpu_stats->total = 100.0;

    fclose(fp);
    return 0;
}


// MEMORY
int read_mem_stats(MemStats *mem_stats) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) {
        perror("Failed to open /proc/meminfo");
        return -1;
    }

    char line[256];
    unsigned long mem_total_kb = 0;
    unsigned long mem_free_kb = 0;
    unsigned long buffers_kb = 0;
    unsigned long cached_kb = 0;
    unsigned long swap_total_kb = 0;
    unsigned long swap_free_kb = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %lu kB", &mem_total_kb) == 1) continue;
        if (sscanf(line, "MemFree: %lu kB", &mem_free_kb) == 1) continue;
        if (sscanf(line, "Buffers: %lu kB", &buffers_kb) == 1) continue;
        if (sscanf(line, "Cached: %lu kB", &cached_kb) == 1) continue;
        if (sscanf(line, "SwapTotal: %lu kB", &swap_total_kb) == 1) continue;
        if (sscanf(line, "SwapFree: %lu kB", &swap_free_kb) == 1) continue;
    }

    fclose(fp);

    unsigned long used_kb = mem_total_kb - mem_free_kb - buffers_kb - cached_kb;
    unsigned long swap_used_kb = swap_total_kb - swap_free_kb;

    mem_stats->total_gb = mem_total_kb / 1024.0 / 1024.0;
    mem_stats->used_gb = used_kb / 1024.0 / 1024.0;
    mem_stats->swap_total_gb = swap_total_kb / 1024.0 / 1024.0;
    mem_stats->swap_used_gb = swap_used_kb / 1024.0 / 1024.0;

    return 0;
}




// NET 
int read_net_stats(NetStats *netstats) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;

    char line[256];
    int line_no = 0;
    netstats->interface_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_no++;
        if (line_no <= 2) continue;  // İlk 2 satır başlık

        // format: "  eth0: 12345 0 0 0 0 0 0 0 67890 0 0 0 0 0 0 0"
        char iface_name[MAX_IF_NAME_LEN];
        unsigned long rx_bytes, tx_bytes;
        // sscanf formatı arayüz ismi iki nokta üst üste ile biter, sonra gelen rx bytes, sonra diğer veriler gelir
        int res = sscanf(line, " %[^:]: %lu %*u %*u %*u %*u %*u %*u %*u %lu", 
                         iface_name, &rx_bytes, &tx_bytes);
        if (res == 3) {
            if (netstats->interface_count < MAX_NET_INTERFACES) {
                strcpy(netstats->interfaces[netstats->interface_count].name, iface_name);
                netstats->interfaces[netstats->interface_count].rx_bytes = rx_bytes;
                netstats->interfaces[netstats->interface_count].tx_bytes = tx_bytes;
                netstats->interface_count++;
            }
        }
    }

    fclose(fp);
    return 0;
}



#define SECTOR_SIZE 512  // Sektör boyutu (genelde 512 bayt)

int read_disk_stats(DiskStatsList *disk_list) {
    FILE *fp = fopen("/proc/diskstats", "r");
    if (!fp) {
        perror("Failed to open /proc/diskstats");
        return -1;
    }

    char line[256];
    disk_list->disk_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (disk_list->disk_count >= MAX_DISKS)
            break;

        unsigned int major, minor;
        char dev_name[MAX_DISK_NAME_LEN];
        unsigned long long reads_completed, reads_merged, sectors_read, time_reading;
        unsigned long long writes_completed, writes_merged, sectors_written, time_writing;
        int ret = sscanf(line, " %u %u %31s %llu %llu %llu %llu %llu %llu %llu %llu",
                         &major, &minor, dev_name,
                         &reads_completed, &reads_merged, &sectors_read, &time_reading,
                         &writes_completed, &writes_merged, &sectors_written, &time_writing);

        if (ret < 11) continue;

        // Sadece gerçek diskleri al (örn: sd, hd, nvme ile başlayanlar)
        if (strncmp(dev_name, "sd", 2) != 0 && 
            strncmp(dev_name, "hd", 2) != 0 &&
            strncmp(dev_name, "nvme", 4) != 0)
            continue;

        DiskStats *d = &disk_list->disks[disk_list->disk_count++];
        strncpy(d->name, dev_name, MAX_DISK_NAME_LEN);

        // Sektör sayısını bayta çeviriyoruz
        d->read_bytes = sectors_read * SECTOR_SIZE;
        d->write_bytes = sectors_written * SECTOR_SIZE;
    }

    fclose(fp);
    return 0;
}


int read_connectivity_stats(ConnectivityInfo *conn_info) {
    FILE *fp = popen("netstat -tunap", "r");
    if (!fp) {
        perror("netstat çalıştırılamadı");
        return -1;
    }

    char line[256];
    int line_no = 0;
    conn_info->connection_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_no++;
        if (line_no <= 2) continue; // başlıkları atla

        char proto[16], local[64], remote[64], state[32] = "-";
        int parsed = sscanf(line, "%15s %*s %*s %63s %63s %31s", proto, local, remote, state);
        if (parsed < 3) continue;

        if (conn_info->connection_count >= MAX_CONNECTIONS)
            break;

        ConnectionInfo *conn = &conn_info->connections[conn_info->connection_count++];

        // IP ve portu ayır
        char *colon;
        colon = strrchr(local, ':');
        if (colon) {
            *colon = '\0';
            strcpy(conn->local_address, local);
            conn->local_port = atoi(colon + 1);
        }

        colon = strrchr(remote, ':');
        if (colon) {
            *colon = '\0';
            strcpy(conn->remote_address, remote);
            conn->remote_port = atoi(colon + 1);
        }

        strncpy(conn->state, state, sizeof(conn->state));
    }

    pclose(fp);
    return 0;
}


int read_process_list(ProcessList *plist) {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("Failed to open /proc");
        return -1;
    }

    if (!plist->processes) {
        plist->capacity = 64;
        plist->count = 0;
        plist->processes = malloc(plist->capacity * sizeof(ProcessInfo));
        if (!plist->processes) {
            perror("malloc failed");
            closedir(proc_dir);
            return -1;
        }
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        char *dname = entry->d_name;
        int is_pid = 1;
        for (int i = 0; dname[i] != '\0'; i++) {
            if (!isdigit(dname[i])) {
                is_pid = 0;
                break;
            }
        }
        if (!is_pid) continue;

        int pid = atoi(dname);

        // Process name oku
        char status_path[256];
        snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);

        FILE *fp_status = fopen(status_path, "r");
        if (!fp_status) continue;

        char line[512];
        char process_name[256] = "";
        while (fgets(line, sizeof(line), fp_status)) {
            if (sscanf(line, "Name: %255s", process_name) == 1) break;
        }
        fclose(fp_status);

        if (process_name[0] == '\0') continue;

        // cmdline oku
        char cmdline_path[256];
        snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%d/cmdline", pid);

        FILE *fp_cmd = fopen(cmdline_path, "r");
        if (!fp_cmd) continue;

        char cmdline[1024] = "";
        size_t len = fread(cmdline, 1, sizeof(cmdline) - 1, fp_cmd);
        fclose(fp_cmd);

        for (size_t i = 0; i < len; i++) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }

        // Listeye ekle (dinamik dizi yönetimi)
        if (plist->count >= plist->capacity) {
            int new_capacity = plist->capacity * 2;
            ProcessInfo *new_arr = realloc(plist->processes, new_capacity * sizeof(ProcessInfo));
            if (!new_arr) {
                perror("realloc failed");
                closedir(proc_dir);
                return -1;
            }
            plist->processes = new_arr;
            plist->capacity = new_capacity;
        }

        plist->processes[plist->count].pid = pid;
        strncpy(plist->processes[plist->count].name, process_name, sizeof(plist->processes[plist->count].name));
        plist->processes[plist->count].name[sizeof(plist->processes[plist->count].name)-1] = '\0';

        strncpy(plist->processes[plist->count].cmdline, cmdline, sizeof(plist->processes[plist->count].cmdline));
        plist->processes[plist->count].cmdline[sizeof(plist->processes[plist->count].cmdline)-1] = '\0';

        plist->count++;
    }

    closedir(proc_dir);
    return 0;
}




