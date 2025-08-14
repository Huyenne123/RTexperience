// movement_rapid_trigger.c
// Rapid trigger for WASD + arrow keys only
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/time.h>

static void write_event(__u16 type, __u16 code, __s32 value) {
    struct input_event ev;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ev.time = tv;
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(STDOUT_FILENO, &ev, sizeof(ev));
}

static void syn_report(void) {
    write_event(EV_SYN, SYN_REPORT, 0);
}

// Keys we care about: WASD + arrow keys
static int is_movement_key(int code) {
    return code == KEY_W || code == KEY_A || code == KEY_S || code == KEY_D ||
           code == KEY_UP || code == KEY_LEFT || code == KEY_DOWN || code == KEY_RIGHT;
}

#define MAX_KEYS 512
static bool phys[MAX_KEYS]; // track physical holds

int main(void) {
    struct input_event ev;
    int active = 0; // currently active movement key

    while (read(STDIN_FILENO, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type != EV_KEY) {
            write(STDOUT_FILENO, &ev, sizeof(ev));
            continue;
        }

        // If this is not a movement key, just pass it through
        if (!is_movement_key(ev.code)) {
            write(STDOUT_FILENO, &ev, sizeof(ev));
            continue;
        }

        if (ev.value == 1) { // press
            phys[ev.code] = true;
            if (active != ev.code) {
                if (active != 0) { // release previous movement key
                    write_event(EV_KEY, active, 0);
                    syn_report();
                }
                write_event(EV_KEY, ev.code, 1);
                syn_report();
                active = ev.code;
            }
            continue;
        }

        if (ev.value == 0) { // release
            phys[ev.code] = false;
            if (active == ev.code) {
                write_event(EV_KEY, ev.code, 0);
                syn_report();
                active = 0;
                // restore previous held movement key if any
                for (int k = 1; k < MAX_KEYS; k++) {
                    if (phys[k] && is_movement_key(k)) {
                        write_event(EV_KEY, k, 1);
                        syn_report();
                        active = k;
                        break;
                    }
                }
            }
            continue;
        }

        if (ev.value == 2) { // repeat
            if (active == ev.code)
                write(STDOUT_FILENO, &ev, sizeof(ev));
            continue;
        }
    }
    return 0;
}
