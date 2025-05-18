#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../interface/analyzer.h"


typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
} CpuTimes;

typedef struct {
    int pid;
    unsigned long utime;
    unsigned long stime;
} ProcCpuTime;


// Sistem CPU zamanlarını oku
static int read_system_cpu_times(CpuTimes *cpu) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -1;

    char line[512];
    if (fgets(line, sizeof(line), f) == NULL) {
        fclose(f);
        return -1;
    }

    sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu",
           &cpu->user, &cpu->nice, &cpu->system, &cpu->idle,
           &cpu->iowait, &cpu->irq, &cpu->softirq);

    fclose(f);
    return 0;
}

// Process CPU zamanını oku
static int read_process_cpu_time(int pid, ProcCpuTime *proc_time) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char buf[1024];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return -1;
    }
    fclose(f);

    char *start = strchr(buf, '(');
    char *end = strrchr(buf, ')');
    if (!start || !end || end < start) return -1;

    char state;
    int ret = sscanf(end + 2,
                     "%c %*d %*d %*d %*d %*d "
                     "%*u %*u %*u %*u %*u "
                     "%lu %lu",
                     &state, &proc_time->utime, &proc_time->stime);

    if (ret != 3) return -1;

    proc_time->pid = pid;
    return 0;
}


TopProcessList find_top_cpu_processes(ProcessList *process_list, int n) {
    TopProcessList top_list = {0};
    if (process_list->count == 0 || n <= 0) {
        top_list.processes = NULL;
        top_list.count = 0;
        top_list.capacity = 0;
        return top_list;
    }

    int proc_count = process_list->count;
    ProcCpuTime *times_before = malloc(sizeof(ProcCpuTime) * proc_count);
    ProcCpuTime *times_after = malloc(sizeof(ProcCpuTime) * proc_count);
    if (!times_before || !times_after) {
        free(times_before);
        free(times_after);
        top_list.processes = NULL;
        top_list.count = 0;
        top_list.capacity = 0;
        return top_list;
    }

    CpuTimes cpu_before, cpu_after;
    if (read_system_cpu_times(&cpu_before) != 0) goto cleanup;
    for (int i = 0; i < proc_count; i++) {
        read_process_cpu_time(process_list->processes[i].pid, &times_before[i]);
    }

    sleep(1);

    if (read_system_cpu_times(&cpu_after) != 0) goto cleanup;
    for (int i = 0; i < proc_count; i++) {
        read_process_cpu_time(process_list->processes[i].pid, &times_after[i]);
    }

    unsigned long long total_cpu_before =
        cpu_before.user + cpu_before.nice + cpu_before.system + cpu_before.idle +
        cpu_before.iowait + cpu_before.irq + cpu_before.softirq;
    unsigned long long total_cpu_after =
        cpu_after.user + cpu_after.nice + cpu_after.system + cpu_after.idle +
        cpu_after.iowait + cpu_after.irq + cpu_after.softirq;

    unsigned long long total_diff = total_cpu_after - total_cpu_before;
    if (total_diff == 0) total_diff = 1;


    top_list.processes = malloc(sizeof(TopProcess) * proc_count);
    if (!top_list.processes) goto cleanup;

    for (int i = 0; i < proc_count; i++) {
        unsigned long proc_before = times_before[i].utime + times_before[i].stime;
        unsigned long proc_after = times_after[i].utime + times_after[i].stime;
        unsigned long diff = proc_after > proc_before ? proc_after - proc_before : 0;

        float cpu_pct = ((float)diff * 100.0f) / (float)total_diff;
        if (cpu_pct < 0.0f) cpu_pct = 0.0f;

        top_list.processes[i].pid = process_list->processes[i].pid;
        strncpy(top_list.processes[i].name, process_list->processes[i].name, 255);
        top_list.processes[i].name[255] = '\0';
        top_list.processes[i].cpu_usage = cpu_pct;
    }

    top_list.count = proc_count;
    top_list.capacity = proc_count;

    // Sırala azalan CPU kullanımına göre
    int cmp(const void *a, const void *b) {
        const TopProcess *pa = a;
        const TopProcess *pb = b;
        if (pb->cpu_usage > pa->cpu_usage) return 1;
        if (pb->cpu_usage < pa->cpu_usage) return -1;
        return 0;
    }
    qsort(top_list.processes, top_list.count, sizeof(TopProcess), cmp);

    // Eğer n < count ise kısalt
    if (top_list.count > n) top_list.count = n;

cleanup:
    free(times_before);
    free(times_after);
    return top_list;
}
