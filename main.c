#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define HEIGHT 20
#define WIDTH 40
#define AI 1
#define FPS 24

// Velocity
int vi = 0;
int vj = 1;

// Position
int pi = 0;
int pj = 1;

struct Position {
    int i;
    int j;
};

// Tail
int tail_len = 4;
struct Position *tail;

// Food
int fi;
int fj;

// Score
int score = 0;

void log_buff(char buff[128], FILE *file) {
    fprintf(file, buff);
    fflush(file);
}

void shift(struct Position *array, int len) {
    for (int i = len - 1; i > 0; i--) {
        array[i] = array[i - 1];
    }
}

void shift_left(struct Position *array, int len) {
    for (int i = 0; i < len-1; i++) {
        array[i] = array[i + 1];
    }
}

void shift_left_int(int *array, int len) {
    for (int i = 0; i < len-1; i++) {
        array[i] = array[i + 1];
    }
}

void gen_food() {
    char on_tail;
    do {
        on_tail = 0;
        fi = (rand() % (HEIGHT-1));
        fj = (rand() % (WIDTH-1));
        for (int ti = 0; ti < tail_len; ti++) {
            struct Position pos = tail[ti];
            if (fi == pos.i && fj == pos.j) {
                on_tail = 1;
                break;
            }
        }
    } while (fi == pi || fj == pj || on_tail);
}

void print_game() {
    for (int i = 0; i < HEIGHT+1; i++) {
        for (int j = 0; j < WIDTH+1; j++) {

            char is_tail = 0;
            for (int ti = 0; ti < tail_len; ti++) {
                struct Position pos = tail[ti];
                if (i == pos.i && j == pos.j) {
                    is_tail = 1;
                    break;
                }
            }

            if (i == pi && j == pj)
                printw("G");
            else if (i == fi && j == fj)
                printw("X");
            else if (is_tail)
                printw("-");
            else if (i > HEIGHT - 1 || j > WIDTH - 1)
                printw("*");
            else
                printw(" ");
        }
        printw("\n");
    }

    // Score
    move(5, WIDTH + 5);
    printw("Score: %d", score);
}

char step() {
    for (int i = 1; i < tail_len; i++) {
        struct Position pos = tail[i];
        if (pi == pos.i && pj == pos.j)
            return -1;
    }

    shift(tail, tail_len+1);
    struct Position pos = {pi, pj};
    tail[0] = pos;

    char eaten = 0;
    if (pi == fi && pj == fj) {
        if (tail_len > WIDTH*HEIGHT-2)
            return 2;
        eaten = 1;
        score += 50;
        tail_len += 1;
        gen_food();
    }

    if (eaten)
        return 1;

    pi += vi;
    pj += vj;

    if (pi < 0 || pj < 0 || pi > HEIGHT - 1 || pj > WIDTH - 1) {
        return -1;
    }

    score += 1;
    return 0;
}

void turn_up() {
    if (vi > 0)
        return;
    vi = -1;
    vj = 0;
}

void turn_down() {
    if (vi < 0)
        return;
    vi = 1;
    vj = 0;

}

void turn_left() {
    if (vj > 0)
        return;
    vi = 0;
    vj = -1;
}

void turn_right() {
    if (vj < 0)
        return;
    vi = 0;
    vj = 1;
}

char contains(struct Position *array, int len, struct Position pos) {
    for (int i = 0; i < len; i++) {
        if (array[i].i == pos.i && array[i].j == pos.j)
            return 1;
    }

    return 0;
}

struct Position to_pos(int i, int j) {
    struct Position pos = {i, j};
    return pos;
}

void print_pos_array(struct Position *array, int n) {
    for (int i = 0; i < n; i++) {
        struct Position pos = array[i];
        printw("%d: POS I: %d J: %d\n", i, pos.i, pos.j);
    }
}

// finds path to food
char find_path(struct Position *path, int *len) {
    struct Position *to_visit = malloc(sizeof(struct Position) * (WIDTH) * (HEIGHT));
    int p_to_visit = 0;

    int *to_visit_depth = malloc(sizeof(int) * (WIDTH) * (HEIGHT));

    struct Position *visited = malloc(sizeof(struct Position) * (WIDTH) * (HEIGHT));
    int p_visited = 0;

    // [WIDTH*HEIGHT][2]
    int *map = malloc(sizeof(int) * 2 * WIDTH * HEIGHT);

    // entry point
    to_visit[p_to_visit++] = to_pos(pi, pj);
    to_visit_depth[p_to_visit-1] = 0;

    char found = 0;
    while(p_to_visit > 0) {
        int ci = to_visit[0].i;
        int cj = to_visit[0].j;
        int depth = to_visit_depth[0];

        // Shift
        shift_left(to_visit, p_to_visit);
        shift_left_int(to_visit_depth, p_to_visit);
        p_to_visit--;

        // add to visited
        visited[p_visited++] = to_pos(ci, cj);


        // get positions around
        for (int i = -1; i < 2; i+=2) {
            int j = 0;
            int ai = ci+i;
            int aj = cj+j;

            if (ai < 0 || ai > HEIGHT - 1)
                continue;

            char is_tail = 0;
            //for (int ti = 0; ti < tail_len; ti++) {
            for (int ti = 0; ti < tail_len-depth; ti++) {
                struct Position pos = tail[ti];
                if (ai == pos.i && aj == pos.j) {
                    is_tail = 1;
                    break;
                }
            }

            if (!is_tail) {
                if (!contains(to_visit, p_to_visit, to_pos(ai, aj)) && 
                        !contains(visited, p_visited, to_pos(ai, aj))) {
                    to_visit[p_to_visit++] = to_pos(ai, aj);
                    to_visit_depth[p_to_visit-1] = depth+1;
                    map[ai*WIDTH*2 + aj*2 + 0] = ci;
                    map[ai*WIDTH*2 + aj*2 + 1] = cj;

                    if (ai == fi && aj == fj) {
                        found = 1;
                        break;
                    }
                }
            }
        }
        for (int j = -1; j < 2; j+=2) {
            int i = 0;
            int ai = ci+i;
            int aj = cj+j;

            if (aj < 0 || aj > WIDTH - 1)
                continue;

            char is_tail = 0;
            //for (int ti = 0; ti < tail_len; ti++) {
            for (int ti = 0; ti < tail_len-depth; ti++) {
                struct Position pos = tail[ti];
                if (ai == pos.i && aj == pos.j) {
                    is_tail = 1;
                    break;
                }
            }

            if (!is_tail) {
                if (!contains(to_visit, p_to_visit, to_pos(ai, aj)) && 
                        !contains(visited, p_visited, to_pos(ai, aj))) {
                    to_visit[p_to_visit++] = to_pos(ai, aj);
                    to_visit_depth[p_to_visit-1] = depth+1;
                    map[ai*WIDTH*2 + aj*2 + 0] = ci;
                    map[ai*WIDTH*2 + aj*2 + 1] = cj;

                    if (ai == fi && aj == fj) {
                        found = 1;
                        break;
                    }
                }
            }
        }

    }

    if (found) {
        *len = 0;
        int prev_i = map[fi*WIDTH*2 + fj*2 + 0];
        int prev_j = map[fi*WIDTH*2 + fj*2 + 1];

        path[0] = to_pos(fi, fj);
        *len += 1;

        while (prev_i != pi || prev_j != pj) {
            path[*len] = to_pos(prev_i, prev_j);
            *len += 1;
            
            int temp_i = prev_i;
            int temp_j = prev_j;
            prev_i = map[temp_i*WIDTH*2 + temp_j*2 + 0];
            prev_j = map[temp_i*WIDTH*2 + temp_j*2 + 1];
        }
    }


    // Cleaning
    free(to_visit_depth);
    free(to_visit);
    free(visited);
    free(map);

    if (!found)
        return 0;
    else
        return 1;
}

char rel_pos_to_key(struct Position pos) {
    int i = pos.i;
    int j = pos.j;

    if (i == pi + 1)
        // S
        return 115;
    else if (i == pi - 1)
        // W
        return 119;
    else if (j == pj + 1)
        // D
        return 100;
    else if (j == pj - 1)
        // A
        return 97;

    return ERR;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    // Log file
    //FILE *log_file = fopen("game_log.txt", "a");

    // Ncurses INIT
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    curs_set(0);

    tail = malloc(sizeof(struct Position) * WIDTH * HEIGHT);
    gen_food();

    char is_holding = 0;
    char key;
    char res;
    char found;

    char playing = 1;

    struct Position *path = malloc(sizeof(struct Position) * WIDTH * HEIGHT);
    int *len_path = malloc(sizeof(int));

    found = find_path(path, len_path);
    while (playing) {

        clear();
        print_game();

        // Try to find path to food again
        if (!found) {
            found = find_path(path, len_path);
        }

        if (AI) {
            if (found) {
                *len_path -= 1;
                key = rel_pos_to_key(path[*len_path]);
            } else {
                key = ERR;
            }
        } else {
            key = getch();
        }

        if (key != ERR && is_holding < 3) {
            if (!AI) {
                is_holding += 1;
            }

            // W
            if (key == 119) {
                turn_up();
            }
            // S
            else if (key == 115) {
                turn_down();
            }
            // A
            else if (key == 97) {
                turn_left();
            }
            // D
            else if (key == 100) {
                turn_right();
            }
        } else {
            is_holding = 0;
        }

        res = step();

        if (res == -1) {
            //char buff[128];
            //sprintf(buff, "LOST AT PI: %d, PJ: %d\n", pi, pj);
            //log_buff(buff, log_file);

            clear();
            printw("YOU LOST WITH SCORE %d\nPRESS ANY KEY TO EXIT", score);
            refresh();
            nodelay(stdscr, FALSE);
            getch();
            playing = 0;
            break;
        } else if (res == 1) {
            //char buff[128];
            //sprintf(buff, "EATEN AT PI: %d, PJ: %d NEXT AT FI: %d FJ: %d\n", pi, pj, fi, fj);
            //log_buff(buff, log_file);

            found = find_path(path, len_path);

            //sprintf(buff, "COMPUTED PATH FOUND: %d\n", found);
            //log_buff(buff, log_file);
        } else if (res == 2) {
            clear();
            printw("YOU WON WITH SCORE %d\nPRESS ANY KEY TO EXIT", score);
            refresh();
            nodelay(stdscr, FALSE);
            getch();
            playing = 0;
            break;
        }

        refresh();
        usleep(1000000/FPS);
    }

    // Cleaning
    //fclose(log_file);
    free(path);
    free(len_path);
    free(tail);
    endwin();
    return 0;
}
