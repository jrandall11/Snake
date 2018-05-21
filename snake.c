//
//  snake.c
//  lab10.c
//
//  Created by Josh Randall on 4/20/17.
//  Copyright © 2017 Josh Randall. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

WINDOW *w; //global window
int delay = 750000; //global timer
char dir;

struct Snake {
    struct Snake *fwd;
    struct Snake *bwd;
    char symbol;
    int snakesize;
    int dir;
    int x_pos;
    int y_pos;
};

struct Snake snake;

struct Food{
    int f_posx;
    int f_posy;
    int value;
};

struct Food food;

void snakefood(){
    
    struct Snake *sn = snake.bwd; // head of snake
    
    int foodcol, foodrow, snkfood;
    srand((unsigned int)time(NULL));
    foodcol = rand()%(COLS-1)+1;
    srand((unsigned int)time(NULL));
    foodrow = rand()%(LINES-1)+1;
    srand((unsigned int)time(NULL));
    snkfood = rand()%9+1;
    
    while (sn){
        if (foodrow==sn->y_pos && foodcol==sn->x_pos){
            srand((unsigned int)time(NULL));
            foodcol = rand()%(COLS-1)+1;
	    srand((unsigned int)time(NULL));
            foodrow = rand()%(LINES-1)+1;
            sn = snake.bwd;
        }
        sn = sn->bwd;
    }
    
    food.f_posx = foodcol;
    food.f_posy = foodrow;
    food.value  = snkfood;
    
    //mvaddch(foodrow, foodcol, snkfood + '0');
    //refresh();
    
}

int set_ticker(int n_msecs){
    struct itimerval new_timeset;
    long n_sec, n_usecs;
    
    n_sec = n_msecs / 1000;         // int part
    n_usecs = (n_msecs%1000)*100L;  //remainder
    
    new_timeset.it_interval.tv_sec = n_sec; //set reload
    new_timeset.it_interval.tv_usec = n_usecs; // new ticker value
    new_timeset.it_value.tv_sec = n_sec;        // store this
    new_timeset.it_value.tv_usec = n_usecs;     // and this
    
    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

// INITIALIZES SNAKE TAIL
void initializeList(struct Snake *tail){
    tail->fwd = NULL;
    tail->bwd = NULL;
    tail->symbol = '#';
    tail->snakesize = 0;
    tail->x_pos = 35;
    tail->y_pos = 12;
}

int insertLast(struct Snake *root, int y, int x, char dir)
{
    struct Snake *newsnake;
    
    /* Allocate and initialize a new snake, if failure return null */
    newsnake = (struct Snake *)malloc(sizeof(struct Snake));
    if (newsnake == NULL)
        return 0;
    newsnake->symbol = '#';
    newsnake->dir   = dir;
    newsnake->x_pos = x;
    newsnake->y_pos = y;
    snake.snakesize++;
    
    /* For a previously empty list */
    if (root->fwd == NULL)
    {
        root->fwd = newsnake;
        root->bwd = newsnake;
        newsnake->fwd = NULL;
        newsnake->bwd = NULL;
    }
    
    /* For a list with at least one snake */
    else
    {
        /* previous first snake now new snake's successor */
        root->bwd->fwd = newsnake;
        newsnake->bwd = root->bwd;
        /* no predecessor to the new first snake */
        newsnake->fwd = NULL;
        /* root points to this new first */
        root->bwd = newsnake;
    }
    
    
    return 1;
}

void traverseForward(struct Snake s)
{
    struct Snake *q = s.fwd;
    
    while (q)
    {
        move(q->y_pos, q->x_pos);
        addch(q->symbol);
        refresh();
        q = q->fwd;
    }
}

void deleteFirst( struct Snake *tail )
{
    struct Snake *temp;
    snake.snakesize--;
    
    /* if list is empty do nothing */
    if (tail->fwd==NULL)
        return;
    
    /* if list is a single node delete the node */
    if (tail->fwd->fwd==NULL && tail->bwd->bwd==NULL)
    {
        temp = tail->fwd; /* node to be delted */
        tail->fwd = tail->fwd->fwd;
        tail->bwd = tail->bwd->bwd;
        free(temp);
    }
    
    else
    {
        temp = tail->fwd; /* node to be deleted */
        tail->fwd->bwd = tail->bwd;
        tail->fwd->fwd->bwd = NULL;
        tail->fwd = tail->fwd->fwd;
        
        free(temp);
    }
    
}

/* deletes all the nodes in the linked-list */
void deleteAll(struct Snake *root)
{
    struct Snake *temp;
    
    while (root->fwd != NULL)
    {
        temp = root; /* node to be deleted */
        temp = NULL;
        free(temp);
        root->fwd = root->fwd->fwd; /* advance pointer */
        root->bwd = root->bwd->bwd; /* advance pointer */
        
    }
}

void set_up(){
    int i, srow, scol;
    char dir = 0;
    
    // Get random direction
    static int initialized = 0;
    int intdir;
    
    if (!initialized){
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    intdir = rand()%4+1;
    
    if(intdir==1) dir = '>';
    if(intdir==2) dir = '<';
    if(intdir==3) dir = '^';
    if(intdir==4) dir = 'v';
    
    // tail always starts in same position.
    initializeList(&snake);
    snake.snakesize = 0;
    
    srow = snake.y_pos;
    scol = snake.x_pos;
    
    printw("dir = %d, srow: %d, scol: %d", intdir, srow, scol);
    
    // Build the rest of the snake
    for (i=0; i<1; i++){
        insertLast(&snake, srow, scol, dir);
        if(intdir==1) scol++; // >
        if(intdir==2) scol--; // <
        if(intdir==3) srow--; // ^
        if(intdir==4) srow++; // v
    }
    
    // draw snake
    //signal(SIGINT, SIG_IGN);
    traverseForward(snake);
    //signal( SIGALRM, ball_move);
    //set_ticker(1000/TICKS_PER_SEC);
    
}

void game_over(WINDOW *w){
    clear();
    box(w, 0, 0);
    mvaddstr(LINES/2, (COLS-9)/2, "GAME OVER" );
    nodelay(w, FALSE);
    while(getch()!=' ');
    
    clear();
    deleteAll(&snake);
    delwin(w);
    endwin();
    refresh();
    
    exit(0);
}

void game_won(WINDOW *w){
    clear();
    box(w, 0, 0);
    mvaddstr(LINES/2, (COLS-7)/2, "YOU WIN" );
    nodelay(w, FALSE);
    while(getch()!=' ');
    
    clear();
    deleteAll(&snake);
    delwin(w);
    endwin();
    refresh();
    
    exit(0);
}


void snake_grow(int srow, int scol, int dir){
    insertLast(&snake, srow, scol, dir);
    traverseForward(snake);
    if (dir=='^') srow--;
    if (dir=='>') scol++;
    if (dir=='<') scol--;
    if (dir=='v') srow++;
}

void snake_move(){
    char curdir = 0;
    int scol=0, srow=0, i, c;
    
    struct Snake *sn = snake.bwd;
    
    curdir = sn->dir;
    srow = sn->y_pos;
    scol = sn->x_pos;
    
    if (curdir==dir){
        if (curdir=='^') srow--;
        if (curdir=='>') scol++;
        if (curdir=='<') scol--;
        if (curdir=='v') srow++;
    }
    else {
        if (dir=='^') srow--;
        if (dir=='>') scol++;
        if (dir=='<') scol--;
        if (dir=='v') srow++;
    }
    
    if(srow==0||srow==LINES-1||scol==0||scol==COLS-1) game_over(w);
    
    while (sn){
        if (srow==sn->y_pos && scol==sn->x_pos) game_over(w);
        sn = sn->bwd;
    }
    
    //printw(" dir = %c, row: %d, col: %d", curdir, srow, scol);
    
    signal(SIGINT, SIG_IGN);
    
    if (srow==food.f_posy && scol==food.f_posx) {
        delay = delay/1.1;
        for (i=0; i<food.value; i++){
            c = getch();
            
            if (c==KEY_UP && snake.bwd->dir != 'v') dir='^';
            else if (c==KEY_DOWN && snake.bwd->dir != '^') dir='v';
            else if (c==KEY_RIGHT && snake.bwd->dir != '<') dir='>';
            else if (c==KEY_LEFT && snake.bwd->dir != '>') dir='<';
            
            // GAME OVER CONDITIONS
            else if (c==KEY_UP && snake.bwd->dir == 'v') game_over(w);
            else if (c==KEY_DOWN && snake.bwd->dir == '^') game_over(w);
            else if (c==KEY_RIGHT && snake.bwd->dir == '<') game_over(w);
            else if (c==KEY_LEFT && snake.bwd->dir == '>') game_over(w);
            
            snake_grow(srow, scol, dir);
        }
        snakefood(); // new snakefood position
    }

    insertLast(&snake, srow, scol, dir);
    deleteFirst(&snake);
    traverseForward(snake);
    mvaddch(food.f_posy, food.f_posx, food.value + '0');
    refresh();
    usleep(delay);
}

int main(int argc, const char * argv[]) {
    
    int c, go = 0;
    
    w = initscr();
    noecho();
    curs_set(0);
    nodelay(w, TRUE);
    keypad(w, TRUE);
    
    box(w, 0, 0);
    wrefresh(w);
    
    // get into position
    set_up();
    snakefood();
    
    while((c = getch()) != 'q' && go ==0){
        clear();
        box(w, 0, 0);
        
        if (c==KEY_UP && snake.bwd->dir != 'v') dir='^';
        else if (c==KEY_DOWN && snake.bwd->dir != '^') dir='v';
        else if (c==KEY_RIGHT && snake.bwd->dir != '<') dir='>';
        else if (c==KEY_LEFT && snake.bwd->dir != '>') dir='<';
        
        // GAME OVER CONDITIONS
        else if (c==KEY_UP && snake.bwd->dir == 'v') go =1;
        else if (c==KEY_DOWN && snake.bwd->dir == '^') go=1;
        else if (c==KEY_RIGHT && snake.bwd->dir == '<') go=1;
        else if (c==KEY_LEFT && snake.bwd->dir == '>') go=1;
        
        // IF DIRECTION IS NOT CHANGED
        else dir = snake.bwd->dir;
        
        // debugging
        //printw("you entered: %c, dir = %c", c, dir);    // debugging
        //printw("snakesize: %d", snake.snakesize);
        
        snake_move();
        
        if (snake.snakesize>= ((COLS+LINES)/2)) game_won(w);
        
        wrefresh(w);
    }
    
    if (go==1) game_over(w);
    
    clear();
    deleteAll(&snake);
    delwin(w);
    endwin();
    refresh();
    
    
    return 0;
}
