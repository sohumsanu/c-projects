#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "linked_list.h"
#include "mbstrings.h"

/** Updates the game by a single step, and modifies the game information
 * accordingly. Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: width of the board.
 *  - height: height of the board.
 *  - snake_p: pointer to your snake struct (not used until part 2!)
 *  - input: the next input.
 *  - growing: 0 if the snake does not grow on eating, 1 if it does.
 */


void update(int* cells, size_t width, size_t height, snake_t* snake_p,
            enum input_key input, int growing) {
    // `update` should update the board, your snake's data, and global
    // variables representing game information to reflect new state. If in the
    // updated position, the snake runs into a wall or itself, it will not move
    // and global variable g_game_over will be 1. Otherwise, it will be moved
    // to the new position. If the snake eats food, the game score (`g_score`)
    // increases by 1. This function assumes that the board is surrounded by
    // walls, so it does not handle the case where a snake runs off the board.

    // TODO: implement!
    if (g_game_over == 1){
        return;
    }
    if(growing == 0){
        if (cells[*((int*)snake_p->snake_position->data)] == FLAG_WALL){
            g_game_over = 1;
            return;
        }
        if (input == INPUT_NONE && (snake_p->snake_direction) != input){
            input = snake_p->snake_direction;
        }
        int old_snake_position = *((int*)snake_p->snake_position->data);
        if(input == INPUT_NONE){
            *((int*)snake_p->snake_position->data) += 1;
        } else if (input == INPUT_RIGHT){
            *((int*)snake_p->snake_position->data) += 1;
        } else if (input == INPUT_LEFT){
            *((int*)snake_p->snake_position->data) -= 1;
        } else if (input == INPUT_UP){
            *((int*)snake_p->snake_position->data) -= (int)width;
        } else if (input == INPUT_DOWN){
            *((int*)snake_p->snake_position->data) += (int)width;
        }
        if (cells[*((int*)snake_p->snake_position->data)] == FLAG_WALL){
            g_game_over = 1;
            return;
        }
        if (cells[*((int*)snake_p->snake_position->data)] == FLAG_FOOD){
            g_score = g_score + 1;
            cells[old_snake_position] = FLAG_PLAIN_CELL;
            cells[*((int*)snake_p->snake_position->data)] = FLAG_SNAKE;
            place_food(cells, width, height);
        }
        else{
            cells[old_snake_position] = FLAG_PLAIN_CELL;
            cells[*((int*)snake_p->snake_position->data)] = FLAG_SNAKE;
            // printf("Snake moved from %d to %d\n", old_snake_position, snake_position);
            snake_p->snake_direction = input;
        }
    } else if (growing == 1){
        if (cells[*((int*)snake_p->snake_position->data)] == FLAG_WALL){
            g_game_over = 1;
            return;
        }
        if (input == INPUT_NONE && (snake_p->snake_direction) != input){
                input = snake_p->snake_direction;
        } else if (((input == INPUT_RIGHT && (snake_p->snake_direction) == INPUT_LEFT) || (input == INPUT_LEFT && (snake_p->snake_direction) == INPUT_RIGHT)
                || (input == INPUT_UP && (snake_p->snake_direction) == INPUT_DOWN) || (input == INPUT_DOWN && (snake_p->snake_direction) == INPUT_UP))){
                    if(g_score >=1){
                        input = snake_p->snake_direction;
                    }
        }
        int new_snake_position = *((int*)snake_p->snake_position->data);
        int* snake_pointer = &new_snake_position;
        if(input == INPUT_NONE){
            new_snake_position += 1;
        } else if (input == INPUT_RIGHT){
            new_snake_position += 1;
        } else if (input == INPUT_LEFT){
            new_snake_position -= 1;
        } else if (input == INPUT_UP){
            new_snake_position -= (int)width;
        } else if (input == INPUT_DOWN){
            new_snake_position += (int)width;
        }
        if (cells[new_snake_position] == FLAG_WALL){
            g_game_over = 1;
            return;
        } else if (cells[new_snake_position] == FLAG_FOOD){
            g_score = g_score + 1;
            if(extracredit ==1){
                int last = *((int*)get_last((snake_p->snake_position)));
                if(cells[last-1] == FLAG_WALL){
                    if(cells[last-width] == FLAG_WALL){
                        last += width;
                    } else {
                        last -= width;
                    }
                } else {
                    last -=1;
                }
                insert_last(&(snake_p->snake_position), &last, sizeof(int));
                cells[last] = FLAG_SNAKE;
            }
            insert_first(&(snake_p->snake_position), snake_pointer, sizeof(int));
            cells[new_snake_position] = FLAG_SNAKE;
            usleep(10000);
            place_food(cells, width, height);
            snake_p->snake_direction = input;
        }
        else{
            if (cells[new_snake_position] == FLAG_SNAKE){
                if (!(*((int*)get_last(snake_p->snake_position)) == new_snake_position)) { // Making sure the snake can follow right behind the tail
                    g_game_over = 1;
                    return;
                }
            }
            int* last_pos = remove_last(&(snake_p->snake_position));
            insert_first(&(snake_p->snake_position), snake_pointer, sizeof(int));
            cells[*last_pos] = FLAG_PLAIN_CELL;
            cells[new_snake_position] = FLAG_SNAKE;
            // printf("Snake moved from %d to %d\n", old_snake_position, snake_position);
            free(last_pos);
            snake_p->snake_direction = input;
        }
    }
}

/** Sets a random space on the given board to food.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - width: the width of the board
 *  - height: the height of the board
 */
void place_food(int* cells, size_t width, size_t height) {
    /* DO NOT MODIFY THIS FUNCTION */
    unsigned food_index = generate_index(width * height);
    if (*(cells + food_index) == FLAG_PLAIN_CELL) {
        *(cells + food_index) = FLAG_FOOD;
    } else {
        place_food(cells, width, height);
    }
    /* DO NOT MODIFY THIS FUNCTION */
}

/** Prompts the user for their name and saves it in the given buffer.
 * Arguments:
 *  - `write_into`: a pointer to the buffer to be written into.
 */
void read_name(char* write_into) {
    // TODO: implement! (remove the call to strcpy once you begin your
    // implementation)
    
    int x = 0;
    while(x == 0){
        printf("\nName > ");
        fflush(0);
        x = read(0, write_into, 1000);
        if (x==1) {
            printf("\n Name Invalid: must be longer than 0 characters.");
            x = 0;
        }
    }
    write_into[x-1] = '\0';
}

/** Cleans up on game over — should free any allocated memory so that the
 * LeakSanitizer doesn't complain.
 * Arguments:
 *  - cells: a pointer to the first integer in an array of integers representing
 *    each board cell.
 *  - snake_p: a pointer to your snake struct. (not needed until part 2)
 */
void teardown(int* cells, snake_t* snake_p) {
    // TODO: implement!
    while(snake_p->snake_position != NULL) {
        free(remove_first(&(snake_p->snake_position)));
    }
    free(snake_p->snake_position);
    free(cells);
}
