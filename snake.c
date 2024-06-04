#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

// Dimensions
#define HEIGHT      25
#define WIDTH       35

// Symbols
#define EMPTY       " "
#define SNAKE       "\e[1;32m#\e[0m"
#define FOOD        "\e[1;31mO\e[0m"

// Directions
const int dx[] = { 0, -1,  1,  0};
const int dy[] = {-1,  0,  0,  1};

typedef enum {UP, LEFT, RIGHT, DOWN} Direction;

// Linked list node
struct Node {
    int x;
    int y;
    struct Node* next;
};

int x;
int y;

Direction direction;

int food_x;
int food_y;

char grid[HEIGHT][WIDTH];

bool alive;


struct Node* push_node(struct Node* head, int x, int y) {
    
    // Add new head
    struct Node* new_head = (struct Node*) malloc(sizeof(struct Node));

    new_head->x = x;
    new_head->y = y;
    new_head->next = head;

    return new_head;
}


struct Node* pop_node(struct Node* head) {
    
    struct Node* current = head;
    while (current->next->next != NULL)
        current = current->next;

    // Delete last node
    free(current->next);
    current->next = NULL;

    return head;
}


void update_and_check_collisions(struct Node* head) {

    // Update food
    grid[food_y][food_x] = 2;
    
    // Update sknake
    while (head != NULL) {
        
        if (grid[head->y][head->x] == 1)
            alive = false;

        grid[head->y][head->x] = 1;
        head = head->next;
    }
}


void gen_food() {
    food_x = rand() % WIDTH;
    food_y = rand() % HEIGHT;
}


void set_nonblocking_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
}


void reset_blocking_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);
}


char get_nonblocking_input() {
    char c;
    if (read(STDIN_FILENO, &c, 1) == -1) {
        return 0;
    } else {

        // Flush the stdin buffer
        tcflush(STDIN_FILENO, TCIFLUSH);

        return c;
    }
}


void handle_user_input() {
    switch (get_nonblocking_input()) {
        case 'w': direction = (direction ==  DOWN) ?  DOWN :    UP; break;
        case 's': direction = (direction ==    UP) ?    UP :  DOWN; break;
        case 'a': direction = (direction == RIGHT) ? RIGHT :  LEFT; break;
        case 'd': direction = (direction ==  LEFT) ?  LEFT : RIGHT; break;
    }
}


void print_grid() {

    system("clear");

    for (int i = 0; i < HEIGHT + 2; i++) {
        
        // Print side frame edge (left)
        printf("%c", (i == 0 || i == HEIGHT + 1) ? '+' : '|');

        for (int j = 0; j < WIDTH; j++) {

            if (i == 0 || i == HEIGHT + 1) {
                // Printing bottom/top frame edge
                printf("--");
            } else {
                switch(grid[i - 1][j]) {
                    case 0:
                        printf("%s ", EMPTY);
                        break;
                    case 1:
                        printf("%s ", SNAKE);
                        break;
                    case 2:
                        printf("%s ", FOOD);
                        break;
                }
            }
        }

        // Printing side frame edge (right)
        printf("%c\n", (i == 0 || i == HEIGHT + 1) ? '+' : '|');
    }
}


void print_game_over() {
    printf(
    "\n\e[31m"
    "  ██████╗  █████╗ ███╗   ███╗███████╗ ██████╗ ██╗   ██╗███████╗██████╗ \n"
    " ██╔════╝ ██╔══██╗████╗ ████║██╔════╝██╔═══██╗██║   ██║██╔════╝██╔══██╗\n"
    " ██║  ███╗███████║██╔████╔██║█████╗  ██║   ██║██║   ██║█████╗  ██████╔╝\n"
    " ██║   ██║██╔══██║██║╚██╔╝██║██╔══╝  ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗\n"
    " ╚██████╔╝██║  ██║██║ ╚═╝ ██║███████╗╚██████╔╝ ╚████╔╝ ███████╗██║  ██║\n"
    "  ╚═════╝ ╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝ ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝\n" 
    "\e[0m\n"
);
}


int main() {
    
    srand(time(NULL));

    // Change terminal mode from raw into non blocking mode
    set_nonblocking_mode();
    atexit(reset_blocking_mode);

    // Set snake initial position and direction
    x = 12;
    y = 12;
    direction = UP;
    alive = true;

    struct Node* head = (struct Node*) malloc(sizeof(struct Node));
    head->x = x;
    head->y = y;
    head->next = NULL;

    gen_food();

    while (alive) {
    
        // Handle user input
        handle_user_input();
                
        // Update position
        x += dx[direction];
        y += dy[direction];
        
        // Fold edges
        x = (x +  WIDTH) % WIDTH;
        y = (y + HEIGHT) % HEIGHT;

        // Update snake linked list
        head = push_node(head, x, y);

        // Generate new food if it was eaten and remove tail otherwise
        if (x == food_x && y == food_y)
            gen_food();
        else
            head = pop_node(head);

        // Zero out grid
        memset(grid, 0, HEIGHT * WIDTH);

        // Update snake and food
        update_and_check_collisions(head);
        
        // Print grid
        print_grid();

        usleep(100000);
    }

    print_game_over();

    return 0;
}

