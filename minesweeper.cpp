#include <ncurses.h>
#include <string.h>

#include <cstdlib>
#include <ctime>

#include <vector>
#include <list>

using namespace std;

#define BOMB_CODE 9
#define UNMARKED 0
#define MARKED 1
#define UNKNOWN 2

char empty = '.';
char tile = '#';
char bomb = '@';
char marked = 'X';
char unknown = '?';

char game_title[] = "> MINESWEEPER <\n";
char score_text[] = "BOMBS : ";

struct point {
    int x, y;
};

struct tile {
    char code;
    bool reveal;
    char mark;
};

void move(struct point *p, int y, int x, int h, int w) {
    if (p->y + y < 0 || p->y + y >= h) {
        return;
    }

    if (p->x + x < 0 || p->x + x >= w) {
        return;
    }

    p->y += y;
    p->x += x;
}

char next_mark(char m) {
    switch (m) {
        case UNMARKED:
            return MARKED;
        case MARKED:
            return UNKNOWN;
        default:
            return UNMARKED;
    }
}

void mark(struct tile *t) {
    t->mark = next_mark(t->mark);
}

void reveal(struct tile *t) {
    t->reveal = TRUE;
}

int main() {

    // initialize ncurses screen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    // initialize scene variables
    int scr_height, scr_width;
    int delta_y, delta_x;
    int game_width = 60;
    int game_height = 40;
    srand(time(NULL));

    // initialize game variables
    vector<struct tile> tiles;
    struct point player_pos = {0, 0};
    int num_bombs = rand() % (game_width * game_height / 4) + (game_width * game_height / 4);

    // initialize tiles
    for (int i = 0; i < game_width * game_height; i++) {
        struct tile t;
        t.code = 0;
        t.reveal = FALSE;
        t.mark = UNMARKED;
        tiles.push_back(t);
    }

    // add bombs
    for (int i = 0; i < num_bombs; i++) {
        int j = rand() % tiles.size();

        if (tiles[j].code == BOMB_CODE) {
            i--;
            continue;
        }

        // set the bomb
        tiles[j].code = BOMB_CODE;

        int y = j / game_width;
        int x = j % game_width;

        // update neighbouring tile
        for (int a = y - 1; a <= y + 1; a++)
            for (int b = x - 1; a <= x + 1; b++)
                if (a != y && b != x)
                    if (a >= 0 && a < game_height && b >= 0 && b < game_height)
                        if (tiles[a * game_width + b].code != BOMB_CODE)
                            tiles[a * game_width + b].code++;
    }


    // game itself
    int ch;
    for (;;) {
        // clean the screen
        werase(stdscr);

        //draw the borders
        {
            for (int i = delta_x - 1; i < scr_width - delta_x + 1; i++) {
                mvaddch(delta_y - 1, i, '-');
                mvaddch(delta_y + game_height, i, '-');
            }

            for (int i = delta_y; i < scr_height - delta_y; i++) {
                mvaddch(i, delta_x - 1, '|');
                mvaddch(i, delta_x + game_width, '|');
            }

            mvaddch(delta_y - 1, delta_x - 1, '+');
            mvaddch(delta_y - 1, delta_x + game_width, '+');
            mvaddch(delta_y + game_height, delta_x - 1, '+');
            mvaddch(delta_y + game_height, delta_x + game_width, '+');
        }

        // update scene variables
        {
            getmaxyx(stdscr, scr_height, scr_width);
            delta_y = (scr_height - game_height) / 2;
            delta_x = (scr_width - game_width) / 2;
        }

        // draw title
        {
            int game_name_start = delta_x + game_width / 2 - strlen(game_title) / 2;
            for (int i = 0; i < strlen(game_title); i++) {
                mvaddch(delta_y - 2, game_name_start + i, game_title[i]);
            }
        }

        // draw score bar
        {
            int score_text_start = delta_x + game_width - 20;
            for (int i = 0; i < strlen(score_text); i++) {
                mvaddch(delta_y + game_height + 1, score_text_start + i, score_text[i]);
            }
            for (int i = 1; i < 4; i ++) {
                int num = num_bombs;
                for (int k = 1; k < i; k ++) {
                    num /= 10;
                }
                char c = num % 10 + '0';
                mvaddch(delta_y + game_height + 1, strlen(score_text) + 4 - i , c);
            }
        }

        // draw the board
        for (int i = 0; i < tiles.size(); i++) {
            int y = i / game_width;
            int x = i % game_width;

            // if (y == player_pos.y && x == player_pos.x)
            //     attron(A_BLINK);

            if (tiles[i].reveal) {
                // draw the revealed tiles
                if (tiles[i].code == 0) {
                    mvaddch(delta_y + y, delta_x + x, empty);
                } else if (tiles[i].code == BOMB_CODE){
                    mvaddch(delta_y + y, delta_x + x, bomb);
                } else {
                    char ch = tiles[i].code + '0';
                    mvaddch(delta_y + y, delta_x + x, ch);
                }

            } else {
                // draw the unrevealed tiles
                if (tiles[i].mark == UNMARKED) {
                    mvaddch(delta_y + y, delta_x + x, tile);
                } else if (tiles[i].mark == MARKED) {
                    mvaddch(delta_y + y, delta_x + x, marked);
                } else if (tiles[i].mark == UNKNOWN) {
                    mvaddch(delta_y + y, delta_x + x, unknown);
                }
            }

            mvaddch(delta_y + y, delta_x + x, empty);

            // if (y == player_pos.y && x == player_pos.x)
            //     attron(A_NORMAL);
        }

        // exit condition
        // if (end) {
        //     break;
        // }

        // get input
        if ( (ch = getch()) != ERR) {
            switch (ch) {
                case KEY_UP:
                case 'W':
                case 'w':
                    move(&player_pos, -1, 0, game_height, game_width);
                    break;

                case KEY_RIGHT:
                case 'D':
                case 'd':
                    move(&player_pos, 0, 1, game_height, game_width);
                    break;

                case KEY_LEFT:
                case 'A':
                case 'a':
                    move(&player_pos, 0, -1, game_height, game_width);
                    break;

                case KEY_DOWN:
                case 'S':
                case 's':
                    move(&player_pos, 1, 0, game_height, game_width);
                    break;

                case ' ':
                    mark(&tiles[player_pos.y * game_width + player_pos.x]);
                    break;

                case '\n':
                    reveal(&tiles[player_pos.y * game_width + player_pos.x]);
                    break;
            }
        }
    }

    // clean up the screen before the end screen
    werase(stdscr);

    // // end screen
    // {
    //     // decide upon the text to be printed on the end screen
    //     char *end_text;
    //     if (won) {
    //         end_text = you_won;
    //     } else {
    //         end_text = you_lose;
    //     }
    //
    //     // get window size
    //     getmaxyx(stdscr, scr_height, scr_width);
    //
    //     // print the text
    //     {
    //         int end_text_start = scr_width / 2 - strlen(end_text) / 2;
    //         for (int i = 0; i < strlen(end_text); i++) {
    //             mvaddch(scr_height / 2, end_text_start + i, end_text[i]);
    //         }
    //     }
    //
    //     // print the final score
    //     {
    //         // precalculate the final score
    //         char final_score[] = "00000\n";
    //         for (int i = 1; i < 6; i ++) {
    //             int num = score;
    //             for (int k = 1; k < i; k ++) {
    //                 num /= 10;
    //             }
    //             char c = num % 10 + '0';
    //             final_score[5 - i] = c;
    //         }
    //
    //         // print the resulting text
    //         int final_score_start = scr_width / 2 - (strlen(final_score_text) + strlen(final_score)) / 2;
    //         for (int i = 0; i < strlen(final_score_text) + strlen(final_score); i++) {
    //             if (i >= strlen(final_score_text)) {
    //                 mvaddch(scr_height / 2 + 1, final_score_start + i, final_score[i - strlen(final_score_text)]);
    //             } else {
    //                 mvaddch(scr_height / 2 + 1, final_score_start + i, final_score_text[i]);
    //             }
    //         }
    //     }
    //
    //     // blocking loop
    //     for (;;) {
    //         // awainting for any input to exit
    //         if ( (ch = getch()) != ERR) {
    //             break;
    //         }
    //     }
    // }

    // clean up
    endwin();
    return 0;
}
