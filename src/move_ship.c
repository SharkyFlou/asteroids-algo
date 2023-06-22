#include "asteroids.h"
#include <stdlib.h>
#include <stdio.h>
//#include <curses.h>

//allow to have a state for the ship with the Y position
// could be used to store more information about the ship
struct state{
    int ship_row;
};

typedef struct state state;

//define how hard the ship want to stay in the middle
//the higher the value, the more the ship will try to stay in the middle
//value can be between 0 and 10 included
#define MIDDLE_COEF 3.0

int find_path_dumb(int field[][FIELD_WIDTH], int ship_row);
state *find_ship(int field[][FIELD_WIDTH]);
int find_first_asteroid(int field[][FIELD_WIDTH], int row, int start_col);
float get_coef_from_row(int row);
int find_path_smart(int field[][FIELD_WIDTH], int ship_row);
int max(int up, int mid, int down);
int search(int field[][FIELD_WIDTH], int ship_row, int ship_col, int tab_already_seen[][FIELD_WIDTH]);
int find_path_rec(int field[][FIELD_WIDTH], int ship_row, int ship_col, int tab_already_seen[][FIELD_WIDTH]);
int return_true_direction(int field[][FIELD_WIDTH], int ship_row, int tab_already_seen[][FIELD_WIDTH]);


//find the ship in the field, return the state with its Y position
//return NULL if not found
state *find_ship(int field[][FIELD_WIDTH]){
    state *ship = malloc(sizeof(state));

    for (int i=0;i<FIELD_HEIGHT;i++){
        if (field[i][0] == SHIP_VAL){
            ship->ship_row = i;
            return ship;
        }
    }
    return NULL;
}

//a very dumb path finding algorithm, just go down or up
// is not used in the final version
int find_path_dumb(int field[][FIELD_WIDTH], int ship_row){
    if(ship_row<10){
        return MOVE_DOWN;
    }
    if(ship_row>=10){
        return MOVE_UP;
    }

    return  MOVE_NO;
}


//find the first asteroid in a row, starting from a the column start_col
int find_first_asteroid(int field[][FIELD_WIDTH], int row, int start_col){
    for (int i=start_col;i<FIELD_WIDTH;i++){
        if (field[row][i] == ASTEROID_VAL){
            return i;
        }
    }
    return FIELD_WIDTH;
}

//get a "coefficient" for a row, the coef gets higher when the row is close to the middle
float get_coef_from_row(int row){
    float coef = MIDDLE_COEF - (abs(row - (FIELD_HEIGHT/2.0))/ ((FIELD_HEIGHT/2.0)/MIDDLE_COEF) );
    return coef;
}

//find the row where the ship can go the furthest
// is influenced by the Y level too, the ship will try to stay in the middle
// can't calculate complicated path, either it goes up, down or stay, no zigzag
// this algorithm is not perfect, but still manage to go up to 29 000 000 +
// is not used in the final version
int find_path_smart(int field[][FIELD_WIDTH], int ship_row){
    int max_length = -1;
    int y_max_length = ship_row;
    
    //start to check from the ship's row to the top
    int start_y = 0;
    for(int i = ship_row; i>=0; i--){
        int length = find_first_asteroid(field, i, start_y);

        //if the path is locked, then we can't go up anymore
        if(length-start_y < 3){
            break;
        }

        //if the row is more interseting than the previous one (the coef is higher/ it can go further) then we keep it
        if(get_coef_from_row(i) + length > max_length + get_coef_from_row(y_max_length)){
            max_length = length;
            y_max_length = i;
        }

        //for the next row, we start to check from one column further (because the ship go right after each iteration)
        start_y++;
    }

    //start to check from the ship's row plus one to the bottom
    start_y = 1;
    for(int i = ship_row+1; i<FIELD_HEIGHT; i++){
        int length = find_first_asteroid(field, i, start_y);

        //if the path is locked, then we can't go down anymore
        if(length-start_y < 3){
            break;
        }
        if(get_coef_from_row(i) + length > max_length + get_coef_from_row(y_max_length)){
            max_length = length;
            y_max_length = i;
        }

        start_y++;
    }

    //is used for debugging purpose
    // only works when asteroids.c is modified to not clear all the terminal
    //print_an_int(y_max_length, 23, 0);
    //print_an_int(max_length, 24, 0);
    //print_a_float(get_coef_from_row(y_max_length), 25, 0);
    
    //if the ship is already on the row where it can go the furthest, then it stays
    //if the ship can go further (or closer to middle) by going up, then it goes up
    //if the ship can go further (or closer to middle) by going down, then it goes down
    if(y_max_length < ship_row){
        return MOVE_UP;
    }
    if(y_max_length > ship_row){
        return MOVE_DOWN;
    }

    return  MOVE_NO;
}

int max(int up, int mid, int down){
    if(up>mid){
        if(up>down){
            return up;
        }
        return down;
    }
    if(mid>down){
        return mid;
    }
    return down;
}

int search(int field[][FIELD_WIDTH], int ship_row, int ship_col, int tab_already_seen[][FIELD_WIDTH]){
    if(ship_row < FIELD_HEIGHT && ship_row >= 0){
        int first_astro = find_first_asteroid(field, ship_row, ship_col);
        if(first_astro - ship_col > SHIP_WIDTH-1){
            return find_path_rec(field, ship_row, ship_col, tab_already_seen);
        }
    }
    return -1;
}

//a recurcive path finding algorithm
// find the path that goes the furthest taking complex path into account
// only dies if it is in the wrong place at the wrong time
// if find a way to go the end, stop looking and return its value
int find_path_rec(int field[][FIELD_WIDTH], int ship_row, int ship_col, int tab_already_seen[][FIELD_WIDTH]){
    //is used for optimization purpose, 
    //if the path algorithm already went to this position, 
    //then it doesn't need to calculate again
    if(tab_already_seen[ship_row][ship_col] == 1){
        return ship_col;
    }

    tab_already_seen[ship_row][ship_col] = 1;

    //is used for debugging purpose
    //mvwaddch(stdscr, ship_row, ship_col, 'X');
    //getch();
    

    if(ship_col >= FIELD_WIDTH-SHIP_WIDTH-1){
        return ship_col;
    }
   
    //calculate the front path
    int front_length = search(field, ship_row, ship_col+1, tab_already_seen);

    if(front_length >= FIELD_WIDTH-SHIP_WIDTH-1){
        return front_length;
    }

    //calculate the up path
    int up_length = search(field, ship_row-1, ship_col+1, tab_already_seen);

    if(up_length >= FIELD_WIDTH-SHIP_WIDTH-1){
        return up_length;
    }


    //calculate the down path
    int down_length = search(field, ship_row+1, ship_col+1, tab_already_seen);

    if(down_length >= FIELD_WIDTH-SHIP_WIDTH-1){
        return down_length;
    }

    return max(up_length,front_length,down_length);
}

//will call the recurcive path finding algorithm
//goes either up, down or in front, depending on which returns the highest value
//try by default to stay between 10 and 11 
// is used in the final version of the program
// can go up to 1 000 000 000 (1b) points
int return_true_direction(int field[][FIELD_WIDTH], int ship_row, int tab_already_seen[][FIELD_WIDTH]){
    int up_length;
    int down_length;
    int front_length;

    tab_already_seen[ship_row][0] = 1;
    //if the ship is too high, try finding below first
    if(ship_row<10){
        //calculate the down path
        down_length = search(field, ship_row+1, 1, tab_already_seen);

        if(down_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_DOWN;
        }

        //calculate the front path
        front_length = search(field, ship_row, 1, tab_already_seen);

        if(front_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_NO;
        }

        //calculate the up path
        up_length = search(field, ship_row-1, 1, tab_already_seen);

        if(up_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_UP;
        }
    }
    //if the ship is too low, try finding above first
    else{
        //calculate the up path
        up_length = search(field, ship_row-1, 1, tab_already_seen);

        if(up_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_UP;
        }
        
        //calculate the front path
        front_length = search(field, ship_row, 1, tab_already_seen);

        if(front_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_NO;
        }

        //calculate the down path
        down_length = search(field, ship_row+1, 1, tab_already_seen);

        if(down_length >= FIELD_WIDTH-SHIP_WIDTH-1){
            return MOVE_DOWN;
        }
    }

    if(up_length > down_length){
        if(up_length > front_length){
            return MOVE_UP;
        }
        return MOVE_NO;
    }
    if(down_length > front_length){
        return MOVE_DOWN;
    }
    return MOVE_NO;
}

//function called by the game to get the next action of the ship
struct ship_action move_ship(int field[][FIELD_WIDTH], void *ship_state){
    struct ship_action action;

    //first time, get the Y of the ship
    if(ship_state == NULL || ship_state == 0){
        ship_state = find_ship(field);
    }

    state *ship = (state *)ship_state;

    //calculate the next action

    //is used for optimizing the path finding algorithm
    int  tab_already_seen[FIELD_HEIGHT][FIELD_WIDTH];

    for(int i=0; i<FIELD_HEIGHT; i++){
        for(int j=0; j<FIELD_WIDTH; j++){
            tab_already_seen[i][j] = 0;
        }
    }

    action.move =return_true_direction(field, ship->ship_row, tab_already_seen);

    //update the Y position of the ship depending on the action
    ship->ship_row += action.move;
    action.state = ship_state;

    //is used for debugging purpose
    // only works when asteroids.c is modified to not clear all the terminal
    //print_an_int(ship->ship_row,27,0);
    //print_an_int(find_first_asteroid(field, ship->ship_row, 0),28,0);
    //print_a_float(get_coef_from_row(ship->ship_row),29,0);

    return action;
}

