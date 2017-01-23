#include "main.h"
#include <string.h>

static const char *SIZES[] = { "B", "kB", "MB", "GB" };
static char buffer[32];
static void format_size(char *out, size_t size, char *suffix) {
    size_t div = 0;
    size_t rem = 0;

    while (size >= 1000 && div < (sizeof(SIZES) / sizeof(*SIZES))) {
        rem = (size % 1000);
        div++;
        size /= 1000;
    }

    sprintf(out, "%.2f %s%s", (float)size + (float)rem / 1000.0, SIZES[div], suffix);
}

static void draw_border(WINDOW *window, char *name, int width) {
    wborder(window, '|', '|', '-', '-', '+', '+', '+', '+');
    int name_len = strlen(name);
    int title_start = (width - name_len) / 2;

    wattron(window, A_BOLD);
    wattron(window, COLOR_PAIR(CPAIR_TITLE));
    mvwprintw(window, 0, title_start, name);
    wrefresh(window);
    wattroff(window, COLOR_PAIR(CPAIR_TITLE));
    wattroff(window, A_BOLD);
}

static void clear_area(WINDOW *window, int y1, int x1, int y2, int x2) {
    int x, y;
    for (x = x1; x < x2; x++) {
        for (y = y1; y < y2; y++) {
            mvwaddstr(window, y, x, " ");
        }
    }
    wrefresh(window);
}

static void clear_line(WINDOW *window, int y, int x) {
    int my, mx;
    getmaxyx(window, my, mx);
    clear_area(window, y, x, y+1, mx-1);    // Take 1 for Border
}

void init_interface_window(INTERFACE_WINDOW *window, int x, int y, int width, int height) {
    window->window = newwin(height, width, y, x);
    window->width = width;
    window->height = height;

    parse_interfaces(window);

    box(window->window, 0, 0);
    draw_border(window->window, window->interface_name, window->width);

    wattron(window->window, A_BOLD);

    wattron(window->window, COLOR_PAIR(CPAIR_HEADING));
    mvwprintw(window->window, 2, 3, "Status:");

    mvwprintw(window->window, 4, 3, "Mode:");

    mvwprintw(window->window, 6, 3, "Primary:");
    mvwprintw(window->window, 7, 3, "Active:");

    wrefresh(window->window);
    wattroff(window->window, A_BOLD);

    int i, x2 = 0, y2 = 0, cols, lines;
    getbegyx(window->window, y2, x2);
    y2 += 9; x2 += 2;
    getmaxyx(window->window, lines, cols);
    cols -= 2;
    int itf_count = window->interfaces_count;
    int width2 = cols / itf_count;
    int height2 = lines - y2 + 2;
    for (i = 0; i < itf_count; i++) {
        SLAVE_INTERFACE *itf = &window->interfaces[i];
        itf->width = width2; itf->height = height2;
        itf->window = subwin(window->window, height2, width2, y2, x2);
        x2 += width2;

        wattron(itf->window, A_BOLD);
        wattron(itf->window, COLOR_PAIR(CPAIR_HEADING));
        mvwprintw(itf->window, 2, 3, "Status:");

        mvwprintw(itf->window, 4, 3, "Speed:");
        mvwprintw(itf->window, 5, 3, "Duplex:");

        mvwprintw(itf->window, 7, 3, "MAC Addr:");

        mvwprintw(itf->window, 9, 3, "RX:");
        mvwprintw(itf->window, 10, 3, "TX:");
        wattroff(itf->window, A_BOLD);

        draw_border(itf->window, itf->if_name, itf->width);
        wrefresh(itf->window);
    }

    post_parse_interfaces(window);
}

void tick_interface_window(INTERFACE_WINDOW *window) {
    parse_interfaces(window);
    clear_line(window->window, 2, 13);
    clear_line(window->window, 4, 13);
    clear_line(window->window, 6, 13);
    clear_line(window->window, 7, 13);

    mvwprintw(window->window, 4, 13, window->bonding_mode);
    mvwprintw(window->window, 6, 13, window->primary_if);
    mvwprintw(window->window, 7, 13, window->active_if);

    int status = 0; // 0 = OK, else = FAULT
    int i;
    for (i = 0; i < window->interfaces_count; i++) {
        SLAVE_INTERFACE *itf = &window->interfaces[i];
        if (itf->status) status += 1;

        clear_line(itf->window, 2, 13);
        clear_line(itf->window, 4, 13);
        clear_line(itf->window, 5, 13);
        clear_line(itf->window, 7, 13);
        clear_line(itf->window, 9, 9);
        clear_line(itf->window, 10, 9);

        wattron(itf->window, A_BOLD);
        if (itf->status == 0) {
            wattron(itf->window, COLOR_PAIR(CPAIR_OK));
            mvwprintw(itf->window, 2, 13, "UP");
            wattroff(itf->window, COLOR_PAIR(CPAIR_OK));
        } else {
            wattron(itf->window, COLOR_PAIR(CPAIR_ERROR));
            mvwprintw(itf->window, 2, 13, "DOWN");
            wattroff(itf->window, COLOR_PAIR(CPAIR_ERROR));
        }
        wattroff(itf->window, A_BOLD);
        mvwprintw(itf->window, 4, 13, itf->speed);
        mvwprintw(itf->window, 5, 13, itf->duplex);
        mvwprintw(itf->window, 7, 13, itf->hw_addr);

        int rx_rate = itf->cur_rx_bytes - itf->last_rx_bytes;
        int tx_rate = itf->cur_tx_bytes - itf->last_tx_bytes;

        format_size(&buffer[0], rx_rate, "/s");
        mvwprintw(itf->window, 9, 9, &buffer[0]);
        format_size(&buffer[0], itf->cur_rx_bytes, "");
        mvwprintw(itf->window, 9, 23, &buffer[0]);

        format_size(&buffer[0], tx_rate, "/s");
        mvwprintw(itf->window, 10, 9, &buffer[0]);
        format_size(&buffer[0], itf->cur_tx_bytes, "");
        mvwprintw(itf->window, 10, 23, &buffer[0]);

        wrefresh(itf->window);
    }

    wattron(window->window, A_BOLD);
    if (status == 0) {
        wattron(window->window, COLOR_PAIR(CPAIR_OK));
        mvwprintw(window->window, 2, 13, "OK");
        wattroff(window->window, COLOR_PAIR(CPAIR_OK));
    } else {
        wattron(window->window, COLOR_PAIR(CPAIR_ERROR));
        if (status == window->interfaces_count)
            mvwprintw(window->window, 2, 13, "DOWN");
        else
            mvwprintw(window->window, 2, 13, "FAULT");
        wattroff(window->window, COLOR_PAIR(CPAIR_ERROR));
    }
    wattroff(window->window, A_BOLD);
    wrefresh(window->window);
    post_parse_interfaces(window);
}