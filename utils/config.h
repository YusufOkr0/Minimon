// utils/config.h

#ifndef CONFIG_H
#define CONFIG_H
#define MAX_NET_INTERFACES 16
#define MAX_IF_NAME_LEN 32
#define MAX_DISK_NAME_LEN 32
#define MAX_DISKS 32
#define MAX_PROCESSES 1024
#define MAX_CMDLINE_LEN 256
#define MAX_PROC_NAME_LEN 64
#define MAX_CONNECTIONS 32

#define MAX_CORES 8


typedef struct {
    float user;
    float system;
    float idle;
    float total;
    float core_usage[MAX_CORES];
} CPUStats;



typedef struct {
    float total_gb;
    float used_gb;
    float swap_used_gb;
    float swap_total_gb;
} MemStats;



typedef struct {
    char name[MAX_IF_NAME_LEN];
    unsigned long rx_bytes;
    unsigned long tx_bytes;
} NetInterfaceStats;


typedef struct {
    NetInterfaceStats interfaces[MAX_NET_INTERFACES];
    int interface_count;
} NetStats;



typedef struct {
    char name[MAX_DISK_NAME_LEN];
    unsigned long long read_bytes;
    unsigned long long write_bytes;
} DiskStats;


typedef struct {
    DiskStats disks[MAX_DISKS];  // MAX_DISKS uygun bir makro olsun, örn 16-32
    int disk_count;
} DiskStatsList;


typedef struct {
    char local_address[64];
    int local_port;
    char remote_address[64];
    int remote_port;
    char state[32]; // e.g. "LISTEN", "ESTABLISHED"
} ConnectionInfo;

typedef struct {
    ConnectionInfo connections[MAX_CONNECTIONS];
    int connection_count;
} ConnectivityInfo;



typedef struct {
    int pid;
    char name[256];
    char cmdline[1024];
} ProcessInfo;

typedef struct {
    ProcessInfo *processes;  // Dinamik dizi
    int count;               // Listedeki süreç sayısı
    int capacity;            // Dizinin kapasitesi (büyütmek için)
} ProcessList;

typedef struct {
    int pid;            // Süreç ID
    char name[256];     // Süreç adı
    float cpu_usage;    // CPU kullanım yüzdesi (örneğin 14.3)
} TopProcess;

typedef struct {
    TopProcess *processes; // Dinamik dizi olarak top süreçler
    int count;             // Listedeki süreç sayısı (örneğin 5)
    int capacity;          // Dizinin kapasitesi (gerektiğinde genişletmek için)
} TopProcessList;







typedef struct {
    CPUStats cpu;
    MemStats mem;
    NetStats net;
    DiskStatsList disks;
    TopProcessList processes;
    ConnectivityInfo conn;
} SystemStats;












#endif
