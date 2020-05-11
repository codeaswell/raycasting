#define SDL_MAIN_HANDLED

#include "display.h"

bool is_running = false;
int ticksLastFrame;

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

struct Player
{
    float x;
    float y;
    float width;
    float height;
    int turn_direction; // -1 for left, 1 for right
    int walk_direction; // -1 for back, 1 for front
    float rotation_angle;
    float walk_speed;
    float turn_speed;
} player;

void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT:
    {
        is_running = false;
        break;
    }
    case SDL_KEYDOWN:
    {
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        if (event.key.keysym.sym == SDLK_UP)
            player.walk_direction = +1;
        if (event.key.keysym.sym == SDLK_DOWN)
            player.walk_direction = -1;
        if (event.key.keysym.sym == SDLK_RIGHT)
            player.turn_direction = +1;
        if (event.key.keysym.sym == SDLK_LEFT)
            player.turn_direction = -1;
        break;
    }
    case SDL_KEYUP:
    {
        if (event.key.keysym.sym == SDLK_UP)
            player.walk_direction = 0;
        if (event.key.keysym.sym == SDLK_DOWN)
            player.walk_direction = 0;
        if (event.key.keysym.sym == SDLK_RIGHT)
            player.turn_direction = 0;
        if (event.key.keysym.sym == SDLK_LEFT)
            player.turn_direction = 0;
        break;
    }
    }
}

void setup()
{
    player.x = WINDOW_WIDTH / 2;
    player.y = WINDOW_HEIGHT / 2;
    player.width = 1;
    player.height = 1;
    player.turn_direction = 0;
    player.walk_direction = 0;
    player.rotation_angle = PI / 2;
    player.walk_speed = 150;
    player.turn_speed = 150 * (PI / 180);
}

void renderMap()
{
    for (int i = 0; i < MAP_NUM_ROWS; i++)
    {
        for (int j = 0; j < MAP_NUM_COLS; j++)
        {
            float x = j * TILE_SIZE;
            float y = i * TILE_SIZE;
            int color = map[i][j] != 0 ? 255 : 000;
            SDL_Rect rect = {
                x * MINIMAP_SCALE_FACTOR,
                y * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR,
                TILE_SIZE * MINIMAP_SCALE_FACTOR};
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void renderPlayer()
{
    SDL_Rect rect = {
        player.x * MINIMAP_SCALE_FACTOR,
        player.y * MINIMAP_SCALE_FACTOR,
        player.width * MINIMAP_SCALE_FACTOR,
        player.height * MINIMAP_SCALE_FACTOR};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderDrawLine(
        renderer,
        player.x * MINIMAP_SCALE_FACTOR,
        player.y * MINIMAP_SCALE_FACTOR,
        (player.x + cos(player.rotation_angle) * 40) * MINIMAP_SCALE_FACTOR,
        (player.y + sin(player.rotation_angle) * 40) * MINIMAP_SCALE_FACTOR);
}

bool hasWallAt(float x, float y) 
{
    if( x < 0 || x > window_width || y < 0 || y > window_height)
        return true;
        
    int mapIndexX = floor(x / TILE_SIZE);
    int mapIndexY = floor(y / TILE_SIZE);

    return map[mapIndexY][mapIndexX] != 0;
}

void movePlayer(float dt)
{
    player.rotation_angle += player.turn_direction * player.turn_speed * dt;
    float move_step = player.walk_direction * player.walk_speed * dt;
    float new_player_x = player.x + cos(player.rotation_angle) * move_step;
    float new_player_y = player.y + sin(player.rotation_angle) * move_step;

    if (!hasWallAt(new_player_x, new_player_y))
    {
        player.x = new_player_x;
        player.y = new_player_y;
    }
}

void update()
{
    // Compute how long we have until the reach the target frame time in milliseconds
    int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

    // Only delay execution if we are running too fast
    if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH)
        SDL_Delay(timeToWait);

    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    ticksLastFrame = SDL_GetTicks();

    movePlayer(deltaTime);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    renderMap();
    renderPlayer();

    SDL_RenderPresent(renderer);
}

void destroy_window()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char **argv)
{
    SDL_SetMainReady();
    is_running = initialize_window();

    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
