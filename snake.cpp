#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

#define FULLSCREEN 0
#define BLACK   0,   0,   0,   255
#define WHITE   255, 255, 255, 255
#define RED     255, 0,   0,   255
#define GREEN   0,   0,   255, 255

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 700;

const int SIZE = 50;

const int CELL_WIDTH =  SIZE;
const int CELL_HEIGHT = SIZE;

const int COLS = SCREEN_WIDTH / SIZE;
const int ROWS = SCREEN_HEIGHT / SIZE;


struct Position
{
    int x;
    int y;
};

inline bool operator==(const Position pos1, const Position pos2)
{
    return pos1.x == pos2.x && pos1.y == pos2.y;
}

inline bool operator!=(const Position pos1, const Position pos2)
{
    return !(pos1 == pos2);
}

class Food
{
    public:
        Food(SDL_Renderer* renderer)
        {
            this->renderer = renderer;
        }
        Food(SDL_Renderer* renderer, int x, int y)
        {
            this->renderer = renderer;
            pos.x = x;
            pos.y = y;
        }
        ~Food()
        {
            SDL_DestroyRenderer(renderer);
        }
        void newPos (Position validPos)
        {
           pos = validPos;
        }
        void render()
        {
            SDL_SetRenderDrawColor(renderer, RED);
            SDL_Rect rect = {pos.x, pos.y, CELL_WIDTH, CELL_HEIGHT};
            SDL_RenderDrawRect(renderer, &rect);
        }
        Position getPos() {return {pos.x, pos.y};}
    private:
        SDL_Renderer* renderer;
        Position pos;
};

class Snake
{
    public:
        Snake(SDL_Renderer *renderer)
        {
            this->renderer = renderer;
            init_body();
            xdir = 1, ydir = 0;
        }
        ~Snake()
        {
            SDL_DestroyRenderer(renderer);
        }
        void init_body()
        {
            head.x = 5 * CELL_WIDTH;
            head.y = 5 * CELL_HEIGHT;
            for (int i = 1; i <= 3; i++)
            {
                tail.push_back({head.x - i * CELL_WIDTH, head.y});
            }
        }
        void update()
        {
            if (!tail.empty())
            {
                for (int i = tail.size() - 1; i > 0; i--)
                {
                    tail.at(i) = tail.at(i - 1);
                }
                tail.front() = head;
            }
            head.x += xdir * CELL_WIDTH;
            head.y += ydir * CELL_HEIGHT;
            if (head.x < 0) head.x = COLS * SIZE - SIZE;
            if (head.x > COLS * SIZE) head.x = 0;
            if (head.y < 0) head.y = ROWS * SIZE - SIZE;
            if (head.y > ROWS * SIZE) head.y = 0;
        }
        void turn()
        {
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            if (keystate[82] || keystate[26] && !(keystate[81] || keystate[21])) // up
                xdir = 0, ydir = (ydir ? ydir : -1);
            else if (keystate[81] || keystate[22]&& !(keystate[82] || keystate[26])) // Down
                xdir = 0, ydir = (ydir ? ydir : 1);
            else if (keystate[80] || keystate[4 ]&& !(keystate[79] || keystate[7 ])) // Left
                xdir = (xdir ? xdir : -1), ydir = 0;
            else if (keystate[79] || keystate[7 ]&& !(keystate[80] || keystate[4 ])) // Right
                xdir = (xdir ? xdir : 1), ydir = 0;
            if (keystate[SDL_SCANCODE_SPACE])
                eat();
        }
        Position getRandomValidPos()
        {
            if (!validPositions.empty())
                return validPositions.at(rand() % (validPositions.size() - 1));
            else
                return {rand() % COLS * SIZE, rand() % ROWS * SIZE};
        }
        void render()
        {
            SDL_SetRenderDrawColor(renderer, WHITE);
            SDL_Rect rect = {head.x, head.y, SIZE, SIZE};
            SDL_RenderFillRect(renderer, &rect);    
            for (int i = 0; i < tail.size(); i++)
            {
                rect = {tail.at(i).x, tail.at(i).y, SIZE, SIZE};
                SDL_RenderDrawRect(renderer, &rect);    
            }
        }
        void eat()
        {
            validPositions.clear();
            if (tail.empty())
                tail.push_back(head);
            else
                tail.push_back(tail.back());
            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    Position pos = {j * SIZE, i * SIZE};
                    bool isSafe = true;
                    for (Position tailPos : tail)
                        if (tailPos == pos) isSafe = false;
                    if (isSafe) validPositions.push_back(pos);
                }
            }
            
        }
        void reset()
        {
            tail.clear();
            init_body();
        }
        bool die()
        {
            for (int i = 0; i < tail.size(); i++)
            {
                if (head == tail.at(i))
                    return true;
            }
            return false;
        }
        bool win()
        {
            return tail.size() + 1 == COLS * ROWS;
        }
        Position getPos() {return head;}
    private:
        Position head;
        int xdir, ydir;
        SDL_Renderer *renderer;
        std::vector<Position> tail;
        std::vector<Position> validPositions;
};

int main()
{
    bool isRunning = true;
    bool gameOver = false, win = false;
    const char* gameOverMessage = "Game Over";
    const char* winMessage = "You win!";
    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    TTF_Font* Sans = TTF_OpenFont("FreeSans.ttf", 30);
    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN ? 1 : 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    Snake snake(renderer);
    Food food(renderer);
    srand(time(NULL));

    int messageW, messageH;

    SDL_Surface* surfaceWinMessage  = TTF_RenderText_Solid(Sans, winMessage, {WHITE});
    messageW = surfaceWinMessage->w, messageH = surfaceWinMessage->h;
    SDL_Texture* textureWinMessage = SDL_CreateTextureFromSurface(renderer, surfaceWinMessage);
    SDL_Rect winMessageRect = {SCREEN_WIDTH / 2 - messageW / 2, SCREEN_HEIGHT / 2 - messageH / 2, messageW, messageH};

    SDL_Surface* surfaceGameOverMessage  = TTF_RenderText_Solid(Sans, gameOverMessage, {WHITE});
    messageW = surfaceGameOverMessage->w, messageH = surfaceGameOverMessage->h;
    SDL_Texture* textureGameOverMessage = SDL_CreateTextureFromSurface(renderer, surfaceGameOverMessage);
    SDL_Rect gameOverMessageRect = {SCREEN_WIDTH / 2 - messageW / 2, SCREEN_HEIGHT / 2 - messageH / 2, messageW, messageH};

    SDL_Surface* surfaceReplayMessage  = TTF_RenderText_Solid(Sans, "Press [Enter] to restart", {WHITE});
    messageW = surfaceReplayMessage->w, messageH = surfaceReplayMessage->h;
    SDL_Texture* textureReplayMessage = SDL_CreateTextureFromSurface(renderer, surfaceReplayMessage);
    SDL_Rect replayMessageRect = {SCREEN_WIDTH / 2 - messageW / 2, SCREEN_HEIGHT / 2 - messageH / 2 + messageH + 10, messageW, messageH};

    food.newPos(snake.getRandomValidPos());
    while (isRunning)
    {
        SDL_SetRenderDrawColor(renderer, BLACK);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, WHITE);
        if (!gameOver && !win)
        {
            if (snake.die()) gameOver = true;
            if (snake.win()) win = true;
            if (snake.getPos() == food.getPos())
            {
                snake.eat();
                food.newPos(snake.getRandomValidPos());
            }
            snake.turn();
            snake.update();
            food.render();
            snake.render();
            SDL_Delay(100);
            
        }
        else
        {
            const Uint8* keystate = SDL_GetKeyboardState(NULL);
            if (win)
                SDL_RenderCopy(renderer, textureWinMessage, NULL, &winMessageRect);
            else
                SDL_RenderCopy(renderer, textureGameOverMessage, NULL, &gameOverMessageRect);
            SDL_RenderCopy(renderer, textureReplayMessage, NULL, &replayMessageRect);
            
            if (keystate[SDL_SCANCODE_RETURN])
            {
                snake.reset();
                gameOver = false;
                win = false;
            }
        }
        SDL_RenderPresent(renderer);
        
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_DestroyRenderer(renderer);
                    SDL_FreeSurface(surfaceWinMessage);
                    SDL_DestroyTexture(textureWinMessage);
                    SDL_FreeSurface(surfaceGameOverMessage);
                    SDL_DestroyTexture(textureGameOverMessage);
                    SDL_FreeSurface(surfaceReplayMessage);
                    SDL_DestroyTexture(textureReplayMessage);
                    TTF_CloseFont(Sans);
                    TTF_Quit();
                    SDL_Quit();
                    return 0;
                default:
                    break;
            }
        }
    }

    return 0;
}