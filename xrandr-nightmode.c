/*
  This is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Redshift is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Redshift.  If not, see <http://www.gnu.org/licenses/>.

  Copyright (c) 2013  Zoltan Padrah <zoltan_padrah@users.sf.net>

  Copyright (c) 2017  Adrien Guatto <adrien@guatto.org>
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>

#include "gamma_randr.h"

void die(char *message) {
    perror(message);
    exit(1);
}

volatile sig_atomic_t do_disable = 0;
volatile sig_atomic_t do_switch = 0;

static void sigdisable_handler(int signo) {
    do_disable = 1;
}

static void sigswitch_handler(int signo) {
    do_switch = 1;
}

int nightmode(randr_state_t *state);

void setup_signals() {
    struct sigaction sigact;
    sigset_t sigset;
    int r;
    sigemptyset(&sigset);

    /* Install signal handler for INT and TERM signals */
    sigact.sa_handler = sigdisable_handler;
    sigact.sa_mask = sigset;
    sigact.sa_flags = 0;

    r = sigaction(SIGINT, &sigact, NULL);
    if (r < 0)
        die("sigaction");

    r = sigaction(SIGTERM, &sigact, NULL);
    if (r < 0)
        die("sigaction");

    /* Install signal handler for USR1 signal */
    sigact.sa_handler = sigswitch_handler;
    sigact.sa_mask = sigset;
    sigact.sa_flags = 0;

    r = sigaction(SIGUSR1, &sigact, NULL);
    if (r < 0)
        die("sigaction");
}

char lock_path[256];

int is_already_running() {
    int running = 0, pid_file, rc;
    const char *homedir;

    homedir = getenv("HOME");
    if (homedir == NULL)
        die("getenv");

    snprintf(lock_path, sizeof lock_path, "%s/.xrandr-nightmode.lock", homedir);
    pid_file = open(lock_path, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (pid_file < 0)
        die("open");

    rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if (rc < 0) {
        if (errno == EWOULDBLOCK)
            running = 1;
        else
            die("flock");
    }

    return running;
}

int main(int argc, const char *argv[]){
    int enabled = 0;
    randr_state_t state;

    if (randr_init(&state) < 0){
        fprintf(stderr, "error while init\n");
        return 1;
    }
    if (randr_start(&state) < 0){
        fprintf(stderr, "error while starting randr\n");
        return 1;
    }

    if (is_already_running()) {
        printf("xrandr-nightmode: already running, exiting\n");
        return 1;
    }
    printf("xrandr-nightmode: switching to background\n");

    setup_signals();

    daemon(0, 1);

    for (;;) {
        pause();

        if (do_switch) {
            if (enabled)
                randr_restore(&state);
            else
                if (nightmode(&state) < 0) {
                    fprintf(stderr, "error while setting night mode\n");
                    do_disable = 1;
                }
            enabled = !enabled;
            do_switch = 0;
        }

        if (do_disable) {
            printf("xrandr-nightmode: quitting\n");
            randr_restore(&state);
            break;
        }
    }

    randr_free(&state);
    remove(lock_path);

    return 0;
}

int nightmode_for_crtc(randr_state_t *state, int crtc_num);

int nightmode(randr_state_t *state){
    int r;

    /* If no CRTC number has been specified,
       set night mode on all CRTCs. */
    if (state->crtc_num < 0) {
        for (unsigned int i = 0; i < state->crtc_count; i++) {
            r = nightmode_for_crtc(state, i);
            if (r < 0) return -1;
        }
    } else {
        return nightmode_for_crtc(state, state->crtc_num);
    }

    return 0;
}

void copy_nightmode_ramps(const randr_crtc_state_t *crtc_status,
                          uint16_t *r, uint16_t *g, uint16_t *b);

int nightmode_for_crtc(randr_state_t *state, int crtc_num){
    xcb_generic_error_t *error;

    if (crtc_num >= (int)state->crtc_count || crtc_num < 0) {
        fprintf(stderr, "CRTC %d does not exist.",  state->crtc_num);
        if (state->crtc_count > 1) {
            fprintf(stderr, "Valid CRTCs are [0-%d].\n", state->crtc_count-1);
        } else {
            fprintf(stderr, "Only CRTC 0 exists.\n");
        }

        return -1;
    }

    xcb_randr_crtc_t crtc = state->crtcs[crtc_num].crtc;
    unsigned int ramp_size = state->crtcs[crtc_num].ramp_size;

    /* Create new gamma ramps */
    uint16_t *gamma_ramps = malloc(3*ramp_size*sizeof(uint16_t));
    if (gamma_ramps == NULL)
        die("malloc");

    uint16_t *gamma_r = &gamma_ramps[0*ramp_size];
    uint16_t *gamma_g = &gamma_ramps[1*ramp_size];
    uint16_t *gamma_b = &gamma_ramps[2*ramp_size];

    copy_nightmode_ramps(&(state->crtcs[crtc_num]), gamma_r, gamma_g, gamma_b);

    /* Set new gamma ramps */
    xcb_void_cookie_t gamma_set_cookie =
        xcb_randr_set_crtc_gamma_checked(state->conn, crtc,
                                         ramp_size, gamma_r,
                                         gamma_g, gamma_b);
    error = xcb_request_check(state->conn, gamma_set_cookie);

    free(gamma_ramps);

    if (error) {
        fprintf(stderr, "`%s' returned error %d\n", "RandR Set CRTC Gamma", error->error_code);
        return -1;
    }
    return 0;
}

void copy_nightmode_ramps(const randr_crtc_state_t *crtc_status,
                         uint16_t *r, uint16_t *g, uint16_t *b){
    unsigned int ramp_size = crtc_status->ramp_size;
    unsigned i;
    for(i = 0; i < ramp_size; i++)
        r[ramp_size - 1 - i] = crtc_status->saved_ramps[i + 0 * ramp_size];
    for(i = 0; i < ramp_size; i++)
        g[ramp_size - 1 - i] = 0;
    for(i = 0; i < ramp_size; i++)
        b[ramp_size - 1 - i] = 0;
}
