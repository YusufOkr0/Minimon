// ui/cli.h

#ifndef CLI_H
#define CLI_H

#include "../utils/config.h"

void print_cpu_stats(CPUStats *cpu);
void print_mem_stats(MemStats *mem);
void print_net_stats(NetStats *netstats);
void print_disk_stats(DiskStatsList *disk_list);
void print_connectivity_stats( ConnectivityInfo *conn_info);
void print_process_list( ProcessList *plist);
 void print_top_processes(TopProcessList *top_list);






#endif
