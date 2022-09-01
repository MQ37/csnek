#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int HEIGHT = 20;
int WIDTH = 20;

// Velocity
int vi = 0;
int vj = 1;

// Position
int pi = 0;
int pj = 0;

struct Position {
  int i;
  int j;
};

// Tail
int tail_len = 4;
struct Position tail[200];

// Food
int fi;
int fj;

void shift(struct Position *tail, int len) {
  for (int i = len - 1; i > 0; i--) {
    tail[i] = tail[i - 1];
  }
}

void print_game() {
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {

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
      else
        printw(" ");
    }
    printw("\n");
  }
}

void step() {
  shift(tail, tail_len);
  struct Position pos = {pi, pj};
  tail[0] = pos;

  if (pi == fi && pj == fj) {
    tail_len += 1;
    fi = rand() % HEIGHT;
    fj = rand() % WIDTH;
  }

  pi += vi;
  pj += vj;
}

int main(int argc, char *argv[]) {
  initscr();

  cbreak();
  noecho();
  nodelay(stdscr, TRUE);

  scrollok(stdscr, TRUE);

    fi = rand() % HEIGHT;
    fj = rand() % WIDTH;
  while (1) {
    clear();
    refresh();
    print_game();
    step();

    char key = getch();

    if (key != ERR) {
      // W
      if (key == 119) {
        vi = -1;
        vj = 0;
      }
      // S
      else if (key == 115) {
        vi = 1;
        vj = 0;
      }
      // A
      else if (key == 97) {
        vi = 0;
        vj = -1;
      }
      // D
      else if (key == 100) {
        vi = 0;
        vj = 1;
      }
    } else
      usleep(250000);
    refresh();
  }
  endwin();
  return 0;
}
