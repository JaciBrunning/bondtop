#ifndef MAIN_H
#define MAIN_H

#include <curses.h>

#define CPAIR_TITLE 1
#define CPAIR_HEADING 2

#define CPAIR_OK 10
#define CPAIR_ERROR 11

typedef struct {
    char if_name[32];
    int status;     // 0 = up, 1 = down
    char speed[32]; // Mbps or Unknown
    char duplex[32];
    char hw_addr[32];


    FILE *file_rx, *file_tx;
    size_t last_tx_bytes;
    size_t last_rx_bytes;
    size_t cur_tx_bytes;
    size_t cur_rx_bytes;

    WINDOW *window;
    int width, height;
} SLAVE_INTERFACE;

typedef struct {
    WINDOW *window;
    char *interface_name;
    FILE *bond_file;

    int width, height;

    SLAVE_INTERFACE interfaces[4];
    int interfaces_count;
    char bonding_mode[64];
    char active_if[16];
    char primary_if[16];
} INTERFACE_WINDOW;

void quit();
WINDOW *window_main();
void init_interfaces();
char *bond_interface(int i);
int bond_interfaces_count();

void init_interface_window(INTERFACE_WINDOW *window, int x, int y, int width, int height);
void tick_interface_window(INTERFACE_WINDOW *window);

void parse_interfaces(INTERFACE_WINDOW *window);
void post_parse_interfaces(INTERFACE_WINDOW *window);

#endif