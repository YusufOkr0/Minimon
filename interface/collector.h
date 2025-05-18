// core/collector.h

#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "../utils/config.h"

int read_cpu_stats(CPUStats *cpu_stats);
int read_mem_stats(MemStats *mem_stats);
int read_net_stats(NetStats *netstats);
int read_disk_stats(DiskStatsList *disk_list);
int read_connectivity_stats(ConnectivityInfo *conn_info);
int read_process_list(ProcessList *plist);



#endif
