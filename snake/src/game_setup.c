#include "game_setup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Some handy dandy macros for decompression
#define E_CAP_HEX 0x45
#define E_LOW_HEX 0x65
#define S_CAP_HEX 0x53
#define S_LOW_HEX 0x73
#define W_CAP_HEX 0x57
#define W_LOW_HEX 0x77
#define DIGIT_START 0x30
#define DIGIT_END 0x39

/** Initializes the board with walls around the edge of the board.
 *
 * Modifies values pointed to by cells_p, width_p, and height_p and initializes
 * cells array to reflect this default board.
 *
 * Returns INIT_SUCCESS to indicate that it was successful.
 *
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 */
enum board_init_status initialize_default_board(int** cells_p, size_t* width_p,
                                                size_t* height_p) {
    *width_p = 20;
    *height_p = 10;
    int* cells = malloc(20 * 10 * sizeof(int));
    *cells_p = cells;
    for (int i = 0; i < 20 * 10; i++) {
        cells[i] = FLAG_PLAIN_CELL;
    }

    // Set edge cells!
    // Top and bottom edges:
    for (int i = 0; i < 20; ++i) {
        cells[i] = FLAG_WALL;
        cells[i + (20 * (10 - 1))] = FLAG_WALL;
    }
    // Left and right edges:
    for (int i = 0; i < 10; ++i) {
        cells[i * 20] = FLAG_WALL;
        cells[i * 20 + 20 - 1] = FLAG_WALL;
    }

    // Add snake
    cells[20 * 2 + 2] = FLAG_SNAKE;
    
    // Add Food
    place_food(cells, *width_p, *height_p);

    return INIT_SUCCESS;
}

/** Initialize variables relevant to the game board.
 * Arguments:
 *  - cells_p: a pointer to a memory location where a pointer to the first
 *             element in a newly initialized array of cells should be stored.
 *  - width_p: a pointer to a memory location where the newly initialized
 *             width should be stored.
 *  - height_p: a pointer to a memory location where the newly initialized
 *              height should be stored.
 *  - snake_p: a pointer to your snake struct (not used until part 2!)
 *  - board_rep: a string representing the initial board. May be NULL for
 * default board.
 */
enum board_init_status initialize_game(int** cells_p, size_t* width_p,
                                       size_t* height_p, snake_t* snake_p,
                                       char* board_rep) {
    // TODO: implement! 
    g_game_over = 0;
    g_score = 0;
    snake_p->snake_direction = INPUT_NONE;
    snake_p->snake_position = (node_t*)malloc(sizeof(node_t));
    snake_p->snake_position->prev = NULL;
    snake_p->snake_position->next = NULL;
    snake_p->snake_position->data = (int*)malloc(sizeof(int));
    *((int*)snake_p->snake_position->data) = 42;

    if (board_rep == NULL){
        return initialize_default_board(cells_p, width_p, height_p);    
    } else {
        return decompress_board_str(cells_p, width_p, height_p, snake_p, board_rep);
    }
}

/** Takes in a string `compressed` and initializes values pointed to by
 * cells_p, width_p, and height_p accordingly. Arguments:
 *      - cells_p: a pointer to the pointer representing the cells array
 *                 that we would like to initialize.
 *      - width_p: a pointer to the width variable we'd like to initialize.
 *      - height_p: a pointer to the height variable we'd like to initialize.
 *      - snake_p: a pointer to your snake struct (not used until part 2!)
 *      - compressed: a string that contains the representation of the board.
 * Note: We assume that the string will be of the following form:
 * B24x80|E5W2E73|E5W2S1E72... To read it, we scan the string row-by-row
 * (delineated by the `|` character), and read out a letter (E, S or W) a number
 * of times dictated by the number that follows the letter.
 */
int* get_number(char* token, int index){
    char number[5] = {'0'};
    int x = 0;
    while(47 < token[index] && token[index] < 58){
                number[x] = token[index];
                index = index + 1;
                x = x + 1;
    }
    int* return2 = (int*)malloc(8);
    return2[0] = atoi(number);
    return2[1] = index;
    return return2;
}
int get_character(char* token1, int index){
    int flag;
    if(token1[index] == 'W'){
        flag = 0b0100;
    } else if (token1[index] == 'E'){
        flag = 0b0001;
    } else if (token1[index] == 'S'){
        flag = 0b0010;
    } else {
        flag = 0;
    }
    return flag;
}

enum board_init_status decompress_board_str(int** cells_p, size_t* width_p,
                                            size_t* height_p, snake_t* snake_p,
                                            char* compressed) {
    // TODO: implement!
    if(strlen(compressed) < 3) {
        return INIT_ERR_BAD_CHAR;
    }
    char* pointer1;
    int* numbermalloc;
    char* token1 = strtok_r(compressed, "|", &pointer1);
    int index = 0;
    int snake_count = 0;
    int cell_pos = 0;
    while (token1 != NULL){
        if (index == 0){
            if(token1[0] != 'B'){
                return INIT_ERR_BAD_CHAR;
            }
            int i = 1;
            char number[100] = {'0'};
             while(47 < token1[i] && token1[i] < 58){
                number[i-1] = token1[i];
                i = i+1;
            }
            *height_p = atoi(number);
            if(token1[i] != 'x'){
                return INIT_ERR_BAD_CHAR;
            } else {
                i = i+1;
            }
            char number2[100] = {'0'};
            for(int j = 0; 47 < token1[i] && token1[i] < 58; j++){
                number2[j] = token1[i];
                i = i+1;
            }
            *width_p = atoi(number2);
            int* cells = malloc((*height_p) * (*width_p) * sizeof(int));
            *cells_p = cells;
        } else {
            int total_num = 0;
            int i = 0;
            int flag;
            int repeat = 0;
            while(token1[i] != '\0'){
                flag = get_character(token1, i);
                i = i+1;
                if (flag == 0) {
                    return INIT_ERR_BAD_CHAR;
                }
                numbermalloc = get_number(token1, i);
                repeat = numbermalloc[0];
                i = numbermalloc[1];
                free(numbermalloc);
                total_num = total_num + repeat;
                if (repeat == 0){
                    return INIT_ERR_BAD_CHAR;
                } else if (repeat > (int)*width_p || total_num > (int)*width_p || index > (int)*height_p){
                    return INIT_ERR_INCORRECT_DIMENSIONS;
                }
                if (flag == 2){
                    snake_count = snake_count + repeat;
                    *((int*)snake_p->snake_position->data) = cell_pos;
                }
                for(int j = 0; j < repeat; j++){
                    (*cells_p)[cell_pos + j] = flag;
                    // printf("Cell Position: %d, flag: %d\n", (cell_pos+j), flag);
                }
                cell_pos = cell_pos + repeat;
            }
            if (total_num != (int)(*width_p)){
                return INIT_ERR_INCORRECT_DIMENSIONS;
            }
        }
        token1 = strtok_r(NULL, "|", &pointer1);
        index = index +1;
    }
    if((index-1) != (int)*height_p){
        return INIT_ERR_INCORRECT_DIMENSIONS;
    }
    if(snake_count != 1){
        return INIT_ERR_WRONG_SNAKE_NUM;
    }
    // printf("The height is: %d vs. index: %d, the width is: %d, the snake_count is %d", (int)*height_p, index, (int)*width_p, snake_count);
    // Add Food
    place_food(*cells_p, *width_p, *height_p);
    return INIT_SUCCESS;
}
