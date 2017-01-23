#include "main.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <curses.h>
#include <dirent.h>
#include <string.h>

WINDOW * mainwin;
char interfaces[16][16];
INTERFACE_WINDOW windows[16];
int interfaces_found = 0;

int main() {
    init_interfaces();
    if ((mainwin = initscr()) == NULL) {
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(1);
    }
    if (has_colors() == TRUE) {
        start_color();
        init_pair(CPAIR_TITLE, COLOR_CYAN, COLOR_BLACK);
        init_pair(CPAIR_HEADING, COLOR_WHITE, COLOR_BLACK);
        init_pair(CPAIR_OK, COLOR_GREEN, COLOR_BLACK);
        init_pair(CPAIR_ERROR, COLOR_RED, COLOR_BLACK);
    }
    curs_set(0);

    refresh();
    char tmp[80];
    const char *base = "/proc/net/bonding/";
    int i, x = 1, y = 3, cols = COLS - 2;
    int interface_count = bond_interfaces_count();
    int width = cols / interface_count;
    int height = LINES - y;
    for (i = 0; i < interface_count; i++) {
        windows[i].interface_name = (char *)interfaces[i];
        strcpy(&tmp[0], base);
        strcpy(&tmp[strlen(base)], interfaces[i]);
        windows[i].bond_file = fopen(tmp, "r");
        init_interface_window(&windows[i], x, y, width, height);
        x += width;
    }

    while (1) {
        for (i = 0; i < interface_count; i++) {
            tick_interface_window(&windows[i]);
        }
        sleep(1);
    }

    quit();
    return 0;
}

WINDOW *window_main() {
    return mainwin;
}

void init_interfaces() {
    DIR *dir;
    struct dirent *ent;
    int i = 0;
    if ((dir = opendir ("/proc/net/bonding")) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) {
                strcpy(interfaces[i++], ent->d_name);
            }
        }
        closedir(dir);
    } else {
        fprintf(stderr, "/proc/net/bonding does not exist.\n");
        exit(1);
    }
    interfaces_found = i;
}

char *bond_interface(int i) {
    return interfaces[i];
}

int bond_interfaces_count() {
    if (interfaces_found == 0) init_interfaces();
    return interfaces_found;
}

void quit() {
    int i;
    for (i = 0; i < bond_interfaces_count(); i++) {
        delwin(windows[i].window);
        fclose(windows[i].bond_file);
    }
    delwin(mainwin);
    endwin();
    refresh();
}