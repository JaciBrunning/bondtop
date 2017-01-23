#include "main.h"

#include <stdlib.h>
#include <string.h>

static char buffer[192];

static void cpytrim(char *dest, char *source) {
    strcpy(dest, source);
    int len = strlen(dest);
    if (len > 0 && dest[len-1] == '\n') dest[len-1] = '\0';
}

static int parse_line(char *line, const char *start, char *remaining) {
    if (strncmp(line, start, strlen(start)) == 0) {
        if (remaining) {
            char *ptr = line + strlen(start) + 1;
            cpytrim(remaining, ptr);
        }
        return 1;
    }
    return 0;
}

void parse_interfaces(INTERFACE_WINDOW *window) {
    fseek(window->bond_file, 0, SEEK_SET);

    ssize_t read;
    size_t len = 0;
    char *line = NULL;
    int interface = 0;

    while ((read = getline(&line, &len, window->bond_file)) != -1) {
        parse_line(line, "Bonding Mode:", window->bonding_mode);
        parse_line(line, "Primary Slave:", window->primary_if);
        parse_line(line, "Currently Active Slave:", window->active_if);

        if (parse_line(line, "Slave Interface:", &buffer[0])) {
            cpytrim(window->interfaces[interface].if_name, &buffer[0]);
            interface++;
        }
        if (interface != 0) {
            SLAVE_INTERFACE *itf = &window->interfaces[interface - 1];
            if (parse_line(line, "MII Status:", &buffer[0])) {
                itf->status = strncmp(&buffer[0], "up", 2) != 0;
            } else if (parse_line(line, "Speed:", &buffer[0])) {
                cpytrim(itf->speed, &buffer[0]);
            } else if (parse_line(line, "Duplex:", &buffer[0])) {
                cpytrim(itf->duplex, &buffer[0]);
            } else if (parse_line(line, "Permanent HW addr:", &buffer[0])) {
                cpytrim(itf->hw_addr, &buffer[0]);
            }
        }
    }
    window->interfaces_count = interface;

    int i;
    for (i = 0; i < interface; i++) {
        SLAVE_INTERFACE *itf = &window->interfaces[i];
        strcpy(&buffer[0], "/sys/class/net/");
        strcpy(&buffer[15], itf->if_name);
        strcpy(&buffer[15+strlen(itf->if_name)], "/statistics/rx_bytes");
        itf->file_rx = fopen(&buffer[0], "r");
        strcpy(&buffer[15+strlen(itf->if_name)], "/statistics/tx_bytes");
        itf->file_tx = fopen(&buffer[0], "r");

        fscanf(itf->file_rx, "%ld", &itf->cur_rx_bytes);
        fscanf(itf->file_tx, "%ld", &itf->cur_tx_bytes);
        fclose(itf->file_rx);
        fclose(itf->file_tx);
    }
}

void post_parse_interfaces(INTERFACE_WINDOW *window) {
    int i;
    for (i = 0; i < window->interfaces_count; i++) {
        SLAVE_INTERFACE *itf = &window->interfaces[i];
        itf->last_rx_bytes = itf->cur_rx_bytes;
        itf->last_tx_bytes = itf->cur_tx_bytes;
    }
}