#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480
#define TILE_SIZE 16
#define BASE_DELAY 100
#define SPEED_INCR 1
#define DELAY_MIN 10
#define BORDER_WEIGHT 4

const SDL_Color SNAKE_TIP_COLOR = {255, 255, 255, 255};
const SDL_Color BORDER_COLOR = {128, 128, 128, 255};
const SDL_Color FOOD_COLOR = {255, 0, 0, 255};
const SDL_Color MAIN_BG_COLOR = {32, 32, 32, 255};
const SDL_Color BG_COLOR = {16, 16, 16, 255};
const char* EASY_KEYWORD = "easy";

typedef enum Direction { UP, DOWN, LEFT, RIGHT } Direction;

typedef struct Point { int x, y; } Point;

typedef struct Snake {
	bool running, speeding, easy;
	int length, lives;
	Direction direction;
	Point *body;
} Snake;

void Game_DrawRect(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color color);
void Game_Close(SDL_Window *window, SDL_Renderer *renderer, Snake *snake);

void Snake_Die(Snake *snake, SDL_Window *window, SDL_Renderer *renderer) {
	snake->lives --;
	if (snake->lives < 1) Game_Close(window, renderer, snake);
}

void Snake_Update(Snake *snake, Point *food) {
	for (int i = snake->length - 1; i > 0; --i)
		snake->body[i] = snake->body[i-1];

	switch (snake->direction) {
		case UP: snake->body[0].y -= TILE_SIZE; break;
		case DOWN: snake->body[0].y += TILE_SIZE; break;
		case LEFT: snake->body[0].x -= TILE_SIZE; break;
		case RIGHT: snake->body[0].x += TILE_SIZE; break;
	}
}

void Snake_Draw(SDL_Renderer *renderer, Snake *snake) {
    Uint8 g = 255;
    for (int i = 0; i < snake->length; ++i) {
        SDL_Color color; 

		if (i == 0) color = SNAKE_TIP_COLOR;
		else {
			color.r = 0;
			color.g = g -= 10 < 32 ? 32 : g;
			color.b = 0;
		}

        Game_DrawRect(renderer, snake->body[i].x, snake->body[i].y, TILE_SIZE, TILE_SIZE, color);
    }
}

bool Snake_CheckFoodCollision(Snake *snake, Point *food) {
	return snake->body[0].x == food->x && snake->body[0].y == food->y;
}

bool Snake_CheckEdgeCollision(Snake *snake) {
	return snake->body[0].x < 0 || snake->body[0].x >= WIDTH || snake->body[0].y < 0 || snake->body[0].y >= HEIGHT;
}

bool Snake_CheckSelfCollision(Snake *snake) {
	for (int i = 1; i < snake->length; ++i)
		if (snake->body[0].x == snake->body[i].x &&
			snake->body[0].y == snake->body[i].y) return true;

	return false;
}

int Snake_GetDelay(int snakeLen) {
	int delay = BASE_DELAY - (snakeLen - 1) * SPEED_INCR;
	return delay > DELAY_MIN ? delay : DELAY_MIN;
}

void Snake_Extend(Snake* snake) {
	snake->length++;
	if (snake->length >= 64) snake->easy = true;
	
	snake->body = (Point *)realloc(snake->body, snake->length * sizeof(Point));
	
	printf("\rScore: %03d", snake->length-1);
	fflush(stdout);
}

bool Game_IsEasy(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++)
		if (strcmp(argv[i], EASY_KEYWORD) == 0) return true;

	return false;
}

void Game_InitSDL(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }

    *window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (*window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        exit(1);
    }
}

void Game_CloseSDL(SDL_Window *window, SDL_Renderer *renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Game_HandleEvent(SDL_Event *event, Snake *snake, SDL_Window *window, SDL_Renderer *renderer) {
	if (event->type != SDL_KEYDOWN) return;
	switch (event->key.keysym.sym) {
		case SDLK_q:
			Game_Close(window, renderer, snake);
			break;
		case SDLK_SPACE:
		case SDLK_RETURN:
			snake->running = !snake->running;
			break;
		case SDLK_w:
		case SDLK_UP:
			if (snake->direction != DOWN) snake->direction = UP;
			snake->running = true;
			break;
		case SDLK_s:
		case SDLK_DOWN:
			if (snake->direction != UP) snake->direction = DOWN;
			snake->running = true;
			break;
		case SDLK_a:
		case SDLK_LEFT:
			if (snake->direction != RIGHT) snake->direction = LEFT;
			snake->running = true;
			break;
		case SDLK_d:
		case SDLK_RIGHT:
			if (snake->direction != LEFT) snake->direction = RIGHT;
			snake->running = true;
			break;
		case SDLK_0:
			Snake_Extend(snake);
			break;
		case SDLK_9:
			snake->speeding = !snake->speeding;
			break;
		case SDLK_e:
			snake->easy = !snake->easy;
			break;
	}
}

void Game_DrawRect(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color color) {
	SDL_Rect rect = {x, y, w, h};
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderFillRect(renderer, &rect);
}

Point Game_GetRandomFoodPosition() {
	Point food;
	food.x = (rand() % (WIDTH / TILE_SIZE)) * TILE_SIZE;
	food.y = (rand() % (HEIGHT / TILE_SIZE)) * TILE_SIZE;
	return food;
}

void Game_Close(SDL_Window *window, SDL_Renderer *renderer, Snake *snake) {
	free(snake->body);
	Game_CloseSDL(window, renderer);
	printf("\nGame Over\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	printf("Hello World\n");
	srand(time(NULL));

	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	bool first = true;
	bool quit = false;

	Game_InitSDL(&window, &renderer);

	Snake snake = {
		.speeding = true, .running = true, .easy = Game_IsEasy(argc, argv),
		.length = 1, .lives = 2,
		.direction = LEFT,
		.body = (Point *)malloc(sizeof(Point))
	};

	snake.body[0].x = WIDTH / 2;
	snake.body[0].y = HEIGHT / 2;

	Point food = Game_GetRandomFoodPosition();

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) quit = true;
			Game_HandleEvent(&event, &snake, window, renderer);
		}

		if (!snake.running) continue;
		if (first) {
			first = false;
			snake.running = false;
		}

		Snake_Update(&snake, &food);

		if (Snake_CheckEdgeCollision(&snake)) {
			if (!snake.easy)
				Snake_Die(&snake, window, renderer);			

			switch (snake.direction) {
				case UP: snake.body[0].y = HEIGHT - TILE_SIZE; break;
				case DOWN: snake.body[0].y = 0; break;
				case LEFT: snake.body[0].x = WIDTH - TILE_SIZE; break;
				case RIGHT: snake.body[0].x = 0; break;
			}
		}

		if (Snake_CheckSelfCollision(&snake)) Snake_Die(&snake, window, renderer);

		if (Snake_CheckFoodCollision(&snake, &food)) {
			food = Game_GetRandomFoodPosition();
			Snake_Extend(&snake);
		}

		SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
		SDL_RenderClear(renderer);

		Game_DrawRect(renderer, 0, 0, WIDTH, HEIGHT, MAIN_BG_COLOR);

		Snake_Draw(renderer, &snake);
		Game_DrawRect(renderer, food.x, food.y, TILE_SIZE, TILE_SIZE, FOOD_COLOR);

		if (!snake.easy) {
			Game_DrawRect(renderer, 0, 0, WIDTH, BORDER_WEIGHT, BORDER_COLOR);
			Game_DrawRect(renderer, 0, 0, BORDER_WEIGHT, HEIGHT, BORDER_COLOR);
			Game_DrawRect(renderer, 0, HEIGHT - BORDER_WEIGHT, WIDTH, BORDER_WEIGHT, BORDER_COLOR);
			Game_DrawRect(renderer, WIDTH - BORDER_WEIGHT, 0, BORDER_WEIGHT, HEIGHT, BORDER_COLOR);
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(snake.speeding ? Snake_GetDelay(snake.length) : BASE_DELAY);
	}

	Game_Close(window, renderer, &snake);
	return 0;
}
