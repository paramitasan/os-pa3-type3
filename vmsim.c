/*
 * vmsim.c -- Programming Assignment 3, the file YOU edit.
 *
 * Out of the box this runs FIFO, which is given to you complete as a worked
 * example and is also the baseline every topic is compared against. Run it,
 * read it, check its numbers against the published answer in README.md, and
 * then replace the body of choose_victim() with your own algorithm.
 *
 * You write two functions:
 *
 *     choose_victim()      the replacement decision. Usually under 15 lines.
 *     print_topic_extra()  your topic's own extra output.
 *
 * You should not need to change anything else in the kit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmlib.h"

/* Put your algorithm's name here. It appears in every heading. */
const char *ALGORITHM_NAME = "SECOND CHANCE";

/* ===================================================================== */
/*  THE WORKED EXAMPLE: FIFO                                              */
/* ===================================================================== */

/*
 * FIFO throws out whichever resident page has been in memory the longest,
 * regardless of whether it is still being used. "Oldest arrival" means the
 * smallest load_time, and load_time was set to the reference index at which
 * the page was brought in.
 *
 * This is the whole algorithm. Four lines.
 *
 * Leave this function here or delete it, but do not submit it as your answer:
 * implementing your own algorithm is the assignment.
 */
__attribute__((unused))
static int fifo_victim(VmSim *sim)
{
    int victim = 0;
    for (int i = 1; i < sim->nframes; i++)
        if (sim->frame[i].load_time < sim->frame[victim].load_time)
            victim = i;
    return victim;
}

/* ===================================================================== */
/*  YOUR ALGORITHM                                                        */
/* ===================================================================== */

/*
 * Choose which frame to throw out, and return its index.
 *
 * The library calls this ONLY when a page fault has happened and every frame
 * is already full, so you never have to look for a free frame. The page being
 * brought in is sim->ref[sim->t].
 *
 * What you have to work with, for each frame i in 0 .. sim->nframes-1:
 *
 *     sim->frame[i].page        the page number resident there
 *     sim->frame[i].load_time   when it was brought in        -> FIFO
 *     sim->frame[i].last_used   when it was last referenced   -> LRU
 *     sim->frame[i].uses        how many times referenced     -> LFU, MFU
 *     sim->frame[i].ref_bit     reference bit                 -> Second Chance
 *     sim->frame[i].dirty_bit   modify bit                    -> Enhanced S.C.
 *     sim->frame[i].age         8-bit shift register          -> Aging
 *     sim->hand                 the clock hand, yours to move -> Second Chance
 *
 * and these helpers:
 *
 *     next_use(sim, page, from) when `page` is referenced again, or sim->nrefs
 *     find_page(sim, page)      which frame holds a page, or -1
 *
 * TIE-BREAKING: whenever two frames are equally good victims, choose the
 * LOWER FRAME INDEX, unless your topic's ASSIGNMENT.md says otherwise.
 * Every answer in the key is computed that way, so a different tie-break will
 * not match and you will not be able to reproduce your own results.
 */

 /* ALGORITHM EXPLANATION: 
    1. loop forever
    2. if the frame the hand is on has ref_bit == 0, the sim hand will be recorded inside victim 
    3. advance the sim hand using the given formula: sim->hand = (sim->hand + 1) % sim->nframes 
    4. return the victim
    5. else condition where the frame the hand is on has ref_bit == 1, clear that frame's ref bit
    6. then advance the hand using the given formula: sim->hand = (sim->hand + 1) % sim->nframes 
*/

/* attributes for print extra output purposes */
static int hand_from[MAX_REFS];
static int chances_at[MAX_REFS];
int choose_victim(VmSim *sim)
{   
    int starting_hand = sim->hand;
    int chances = 0;

    while (1) {
        if (sim->frame[sim->hand].ref_bit == 0) {
            int victim = sim->hand;
            sim->hand = (sim->hand + 1) % sim->nframes;

            /* (for print extra output purposes)
            more codes to record the eviction's starting hand and second chances granted
            by using the current reference index (sim->t) as the array slot. */
            if (sim->recording) {
                hand_from[sim->t] = starting_hand;
                chances_at[sim->t] = chances;
            }

            return victim;
        } else {
            sim->frame[sim->hand].ref_bit = 0;
            sim->hand = (sim->hand + 1) % sim->nframes;
            
            /* (for print extra output purposes)
            since second chances is granted, therefore chances + 1 */
            chances++;
        }
    }
}

/* ===================================================================== */
/*  YOUR TOPIC'S EXTRA OUTPUT                                            */
/* ===================================================================== */

/*
 * Called once, after the run, with the simulation state as it finished.
 *
 * The two comparisons below are REQUIRED FOR EVERY TOPIC and are one line
 * each. Your ASSIGNMENT.md then names one extra thing your topic must print;
 * add it where the TODO is, BEFORE the two comparisons.
 *
 * Why before? print_frame_sweep() re-runs your algorithm six more times to
 * fill in its table. If your extra output collects a log while the run
 * happens, print it before the sweep, or guard the collecting with
 * sim->recording, which the library sets to 0 for those extra runs.
 */
static void rule(char c)
{
    for (int i = 0; i < 59; i++) putchar(c);
    putchar('\n');
}
void print_topic_extra(VmSim *sim)
{
    int total_chances = 0;

    rule('=');
    printf("SECOND CHANCES GRANTED, AND WHERE THE HAND WAS\n");
    rule('=');
    printf("'chances' is how many frames the hand passed over at this\n"
           "eviction because their reference bit was set. Each one had\n"
           "its reference bit cleared and survived the round.\n");
    rule('-');
    printf("%-6s %-10s %-12s %-10s %-12s\n",
           "t", "page in", "hand from", "chances", "victim frame");
    rule('-');

    /* loop through every request index */
    for (int t = 0; t < sim->nrefs; t++) {
        /* hits and faults using an empty frame are not evictions. */
        if (sim->victim_at[t] == -1) continue;

        printf("%-6d %-10d f%-11d %-10d f%d\n",
               t, sim->ref[t], hand_from[t], chances_at[t],
               sim->victim_at[t]);
        total_chances += chances_at[t];
    }

    rule('-');
    printf("Evictions              : %d\n", sim->evictions);
    printf("Second chances granted : %d\n", total_chances);
    printf("Hand ends at           : f%d\n", sim->hand);
    putchar('\n');

    print_vs_fifo(sim, ALGORITHM_NAME);
    print_frame_sweep(sim, ALGORITHM_NAME);
}

/* ===================================================================== */
/*  main: given, you do not need to change it                            */
/* ===================================================================== */

static void usage(const char *prog)
{
    fprintf(stderr,
            "usage: %s <reference-string-file> [-f FRAMES] [-v]\n"
            "  -f FRAMES   number of page frames (default 3, max %d)\n"
            "  -v          print what happens at every reference\n",
            prog, MAX_FRAMES);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    const char *path = argv[1];
    int nframes = 3;
    int verbose = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            nframes = atoi(argv[++i]);
            if (nframes < 1 || nframes > MAX_FRAMES) {
                fprintf(stderr, "error: frames must be between 1 and %d\n",
                        MAX_FRAMES);
                return 1;
            }
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    VmSim *sim = malloc(sizeof *sim);
    if (sim == NULL) {
        fprintf(stderr, "error: out of memory\n");
        return 1;
    }

    if (load_refstring(sim, path) != 0) {
        free(sim);
        return 1;
    }

    vm_reset(sim, nframes);
    sim->verbose = verbose;

    print_header(sim, ALGORITHM_NAME);
    if (verbose) {
        printf("Trace of every reference\n\n");
    }

    run_simulation(sim);

    if (verbose) printf("\n");
    print_frame_table(sim);
    print_frames_now(sim);
    print_summary(sim);
    print_eat(sim);
    print_topic_extra(sim);

    if (write_csv(sim, ALGORITHM_NAME, "paging_report.csv") == 0)
        printf("Wrote paging_report.csv\n");

    free(sim);
    return 0;
}
