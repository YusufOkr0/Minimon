#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "interface/collector.h"
#include "interface/reporter.h"
#include "interface/analyzer.h"
#include "ui/cli.h"
#include <signal.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"

#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_MAGENTA "\033[35m"

void print_help() {

    printf("\033[2J\033[H");

    printf("%s╔═════════════════════════════════════════════════════════════════════════════════════╗%s\n", COLOR_BOLD, COLOR_RESET);
    printf("║ %s%s📊  MiniMon System Monitor  📊%s%s                                                      ║\n", COLOR_GREEN, COLOR_BOLD, COLOR_RESET, COLOR_BOLD);
    printf("╚═════════════════════════════════════════════════════════════════════════════════════╝%s\n", COLOR_RESET);

    printf("\n%s%s📋 Usage:%s ./minimon [command]%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET, COLOR_RESET);
    printf("%s══════════════════════════════════════════════════════════════════════════════════════%s\n", COLOR_CYAN, COLOR_RESET);

    printf("\n%s%s🔧 Available Commands:%s\n", COLOR_YELLOW, COLOR_BOLD, COLOR_RESET);
    printf("%s──────────────────────────────────────────────────────────────────────────────────────%s\n", COLOR_YELLOW, COLOR_RESET);

    printf("  %s%-15s%s - Display CPU usage statistics\n", COLOR_MAGENTA, "cpu", COLOR_RESET);
    printf("  %s%-15s%s - Show memory usage details\n", COLOR_MAGENTA, "mem", COLOR_RESET);
    printf("  %s%-15s%s - Display disk usage information\n", COLOR_MAGENTA, "disk", COLOR_RESET);
    printf("  %s%-15s%s - List network interfaces\n", COLOR_MAGENTA, "net", COLOR_RESET);
    printf("  %s%-15s%s - Show active TCP connections\n", COLOR_MAGENTA, "connect", COLOR_RESET);
    printf("  %s%-15s%s - List all running processes\n", COLOR_MAGENTA, "proc", COLOR_RESET);
    printf("  %s%-15s%s - Show top N CPU-consuming processes\n", COLOR_MAGENTA, "top N", COLOR_RESET);
    printf("  %s%-15s%s - Start background reporting task\n", COLOR_MAGENTA, "report", COLOR_RESET);
    printf("  %s%-15s%s - Stop background reporting\n", COLOR_MAGENTA, "stop-report", COLOR_RESET);

    printf("\n%s%sℹ️  Tip:%s Run './minimon --help' to display this menu anytime!%s\n", COLOR_GREEN, COLOR_BOLD, COLOR_RESET, COLOR_RESET);
    printf("\n%s%s👥 Developed by:%s\n", COLOR_GREEN, COLOR_BOLD, COLOR_RESET);
    printf("  %s%-20s%s - %s\n", COLOR_MAGENTA, "Yusuf Okur", COLOR_RESET, "B221202045");
    printf("  %s%-20s%s - %s\n", COLOR_MAGENTA, "Serhat Turgut", COLOR_RESET, "B221202056");
    printf("%s══════════════════════════════════════════════════════════════════════════════════════%s\n", COLOR_GREEN, COLOR_RESET);
}



#define PID_FILE "minimon_report.pid"

int main(int argc, char *argv[]) {
    SystemStats stats;

    if (argc < 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "./minimon") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "cpu") == 0) {
        if (read_cpu_stats(&stats.cpu) != 0) {
            printf("Failed to retrieve CPU data.\n");
            return 1;
        }
        print_cpu_stats(&stats.cpu);
    }
    else if (strcmp(argv[1], "mem") == 0) {
        if (read_mem_stats(&stats.mem) != 0) {
            printf("Failed to retrieve memory data.\n");
            return 1;
        }
        print_mem_stats(&stats.mem);
    }
    else if (strcmp(argv[1], "net") == 0) {
        if (read_net_stats(&stats.net) != 0) {
            printf("Failed to retrieve network data.\n");
            return 1;
        }
        print_net_stats(&stats.net);
    }
    else if (strcmp(argv[1], "disk") == 0) {
        if (read_disk_stats(&stats.disks) != 0) {
            printf("Failed to retrieve disk data.\n");
            return 1;
        }
        print_disk_stats(&stats.disks);
    }
    else if (strcmp(argv[1], "connect") == 0) {
        if (read_connectivity_stats(&stats.conn) != 0) {
            printf("Failed to retrieve connectivity data.\n");
            return 1;
        }
        print_connectivity_stats(&stats.conn);
    }
    else if (strcmp(argv[1], "proc") == 0) {
        ProcessList plist = {0};
        if (read_process_list(&plist) != 0) {
            printf("Failed to retrieve process information.\n");
            return 1;
        }
        print_process_list(&plist);
        free(plist.processes);
    }
    else if (strcmp(argv[1], "top") == 0) {
        if (argc < 3) {
            printf("Usage: %s top N\n", argv[0]);
            return 1;
        }

        int n = atoi(argv[2]);
        if (n <= 0) {
            printf("Please enter a valid N value.\n");
            return 1;
        }

        ProcessList plist = {0};
        if (read_process_list(&plist) != 0) {
            printf("Failed to retrieve process list.\n");
            return 1;
        }

        TopProcessList top = find_top_cpu_processes(&plist, n);
        print_top_processes(&top);

        free(plist.processes);
        free(top.processes);
    }
    else if (strcmp(argv[1], "report") == 0) {
        FILE *pidf = fopen(PID_FILE, "r");
        if (pidf != NULL) {
            int existing_pid;
            if (fscanf(pidf, "%d", &existing_pid) == 1 && kill(existing_pid, 0) == 0) {
                printf("Reporting is already running (PID: %d).\n", existing_pid);
                fclose(pidf);
                return 1;
            }
            fclose(pidf);
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid > 0) {
            printf("Reporting started in background. PID: %d\n", pid);
            printf("NOTE: Reporting runs every 2 minutes.\n");
            printf("To stop, please use './minimon stop-report' command.\n");

            FILE *f = fopen(PID_FILE, "w");
            if (f) {
                fprintf(f, "%d\n", pid);
                fclose(f);
            } else {
                perror("Failed to write PID file");
            }
            return 0;
        }

        setsid();

        struct stat st = {0};
        if (stat("reports", &st) == -1) {
            mkdir("reports", 0700);
        }

        while (1) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char filename[256];
            strftime(filename, sizeof(filename), "reports/system_report_%Y%m%d_%H%M%S.json", t);

            ProcessList plist = {0};
            read_process_list(&plist);

            TopProcessList top = find_top_cpu_processes(&plist, 5);

            if (read_cpu_stats(&stats.cpu) != 0 ||
                read_mem_stats(&stats.mem) != 0 ||
                read_net_stats(&stats.net) != 0 ||
                read_disk_stats(&stats.disks) != 0 ||
                read_connectivity_stats(&stats.conn) != 0) {
                // Could log errors here if needed
            } else {
                stats.processes= top;
                save_system_report_to_json(&stats, filename);
            }

            sleep(120);
        }
    }
    else if (strcmp(argv[1], "stop-report") == 0) {
        FILE *f = fopen(PID_FILE, "r");
        if (!f) {
            printf("Reporting process is not running.\n");
            return 1;
        }

        int pid;
        if (fscanf(f, "%d", &pid) != 1) {
            printf("Failed to read PID.\n");
            fclose(f);
            return 1;
        }
        fclose(f);

        if (kill(pid, SIGTERM) != 0) {
            perror("Failed to terminate process");
            return 1;
        }

        remove(PID_FILE);
        printf("Reporting process stopped (PID: %d).\n", pid);
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        printf("Usage: %s [cpu|mem|net|disk|top N|report|connect|proc]\n", argv[0]);
        return 1;
    }

    return 0;
}
