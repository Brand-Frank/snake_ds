#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// ===================== 常量定义 =====================
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GRID_SIZE 20
#define GRID_WIDTH (WINDOW_WIDTH / GRID_SIZE)
#define GRID_HEIGHT ((WINDOW_HEIGHT - 100) / GRID_SIZE)  // 留出分数显示区域

// 颜色定义 (RGBA)
#define COLOR_BACKGROUND 0x1E, 0x1E, 0x1E, 0xFF
#define COLOR_GRID 0x2D, 0x2D, 0x30, 0xFF
#define COLOR_SNAKE_HEAD 0x4C, 0xAF, 0x50, 0xFF
#define COLOR_SNAKE_BODY 0x81, 0xC7, 0x84, 0xFF
#define COLOR_FOOD 0xF4, 0x43, 0x36, 0xFF
#define COLOR_TEXT 0xFF, 0xFF, 0xFF, 0xFF
#define COLOR_GAME_OVER 0xD3, 0x2F, 0x2F, 0xFF

// 方向枚举
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

// ===================== 数据结构定义 =====================
// 蛇身节点
typedef struct SnakeNode {
    int x, y;
    struct SnakeNode* next;
} SnakeNode;

// 蛇结构体
typedef struct {
    SnakeNode* head;
    SnakeNode* tail;
    Direction direction;
    int length;
    int pending_growth;  // 待增长的长度
} Snake;

// 食物结构体
typedef struct {
    int x, y;
} Food;

// 游戏状态
typedef enum {
    GAME_START,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

// 游戏主结构
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;

    Snake snake;
    Food food;
    GameState state;

    int score;
    int high_score;
    int speed;          // 移动速度（毫秒/帧）
    Uint32 last_move_time;
    bool running;
} Game;

// ===================== 函数声明 =====================
// 初始化函数
bool init_game(Game* game);
bool init_graphics(Game* game);
void init_snake(Game* game);
void init_food(Game* game);

// 游戏逻辑函数
void handle_input(Game* game);
void update_game(Game* game);
void move_snake(Game* game);
void check_collisions(Game* game);
bool check_food_collision(Game* game);
bool check_self_collision(Game* game);
bool check_wall_collision(Game* game);

// 渲染函数
void render_game(Game* game);
void render_snake(Game* game);
void render_food(Game* game);
void render_grid(Game* game);
void render_ui(Game* game);
void render_text(Game* game, const char* text, int x, int y, SDL_Color color);

// 工具函数
void spawn_food(Game* game);
void grow_snake(Game* game);
void reset_game(Game* game);
void cleanup(Game* game);

// ===================== 函数实现 =====================

// 初始化SDL和游戏
bool init_game(Game* game) {
    game->window = NULL;
    game->renderer = NULL;
    game->font = NULL;
    game->snake.head = NULL;
    game->snake.tail = NULL;
    game->state = GAME_START;
    game->score = 0;
    game->high_score = 0;
    game->speed = 150;  // 初始速度：150ms/帧
    game->last_move_time = 0;
    game->running = true;

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return false;
    }

    // 初始化SDL_ttf
    if (TTF_Init() < 0) {
        printf("SDL_ttf初始化失败: %s\n", TTF_GetError());
        return false;
    }

    // 初始化图形
    if (!init_graphics(game)) {
        return false;
    }

    // 设置随机种子
    srand(time(NULL));

    // 初始化蛇和食物
    init_snake(game);
    init_food(game);

    return true;
}

// 初始化图形系统
bool init_graphics(Game* game) {
    // 创建窗口
    game->window = SDL_CreateWindow(
        "贪吃蛇游戏 - Snake Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!game->window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        return false;
    }

    // 创建渲染器
    game->renderer = SDL_CreateRenderer(
        game->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!game->renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        return false;
    }

    // 加载字体
    // game->font = TTF_OpenFont("/mingw64/share/fonts/TTF/arial.ttf", 24);
    // if (!game->font) {
    //     // 尝试备用字体路径
    //     game->font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    //     if (!game->font) {
    //         printf("字体加载失败: %s\n", TTF_GetError());
    //         printf("请确保系统中有可用的字体文件\n");
    //         return false;
    //     }
    // }

    // 加载字体 - 使用支持中文的字体
    game->font = TTF_OpenFont("/mingw64/share/fonts/wqy-microhei/wqy-microhei.ttc", 24);

    if (!game->font) {
        // 尝试其他可能的路径
        game->font = TTF_OpenFont("/ucrt64/share/fonts/wqy-microhei/wqy-microhei.ttc", 24);
        if (!game->font) {
            game->font = TTF_OpenFont("C:/Windows/Fonts/msyh.ttc", 24);  // 微软雅黑
            if (!game->font) {
                game->font = TTF_OpenFont("C:/Windows/Fonts/simhei.ttf", 24);  // 黑体
                if (!game->font) {
                    printf("中文字体加载失败，使用英文字体: %s\n", TTF_GetError());
                    game->font = TTF_OpenFont("/mingw64/share/fonts/TTF/arial.ttf", 24);
                    if (!game->font) {
                        printf("字体加载失败: %s\n", TTF_GetError());
                        printf("将使用纯图形模式\n");
                    }
                }
            }
        }
    }

    return true;
}

// 初始化蛇
void init_snake(Game* game) {
    // 清理现有的蛇
    SnakeNode* current = game->snake.head;
    while (current) {
        SnakeNode* next = current->next;
        free(current);
        current = next;
    }

    game->snake.head = NULL;
    game->snake.tail = NULL;

    // 初始化蛇的起点（屏幕中央）
    int start_x = GRID_WIDTH / 2;
    int start_y = GRID_HEIGHT / 2;

    // 创建初始蛇身（4节）
    for (int i = 0; i < 4; i++) {
        SnakeNode* new_node = (SnakeNode*)malloc(sizeof(SnakeNode));
        if (!new_node) {
            printf("内存分配失败！\n");
            exit(1);
        }

        new_node->x = start_x - i;
        new_node->y = start_y;
        new_node->next = NULL;

        if (!game->snake.head) {
            game->snake.head = new_node;
            game->snake.tail = new_node;
        } else {
            game->snake.tail->next = new_node;
            game->snake.tail = new_node;
        }
    }

    game->snake.direction = DIR_RIGHT;
    game->snake.length = 4;
    game->snake.pending_growth = 0;
}

// 初始化食物
void init_food(Game* game) {
    spawn_food(game);
}

// 生成食物
void spawn_food(Game* game) {
    bool valid_position = false;

    while (!valid_position) {
        game->food.x = rand() % GRID_WIDTH;
        game->food.y = rand() % GRID_HEIGHT;

        // 检查食物是否与蛇身重叠
        valid_position = true;
        SnakeNode* current = game->snake.head;
        while (current) {
            if (current->x == game->food.x && current->y == game->food.y) {
                valid_position = false;
                break;
            }
            current = current->next;
        }
    }
}

// 处理输入
void handle_input(Game* game) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game->running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    // 方向控制
                    case SDLK_UP:
                    case SDLK_w:
                        if (game->snake.direction != DIR_DOWN && game->state == GAME_PLAYING)
                            game->snake.direction = DIR_UP;
                        break;

                    case SDLK_DOWN:
                    case SDLK_s:
                        if (game->snake.direction != DIR_UP && game->state == GAME_PLAYING)
                            game->snake.direction = DIR_DOWN;
                        break;

                    case SDLK_LEFT:
                    case SDLK_a:
                        if (game->snake.direction != DIR_RIGHT && game->state == GAME_PLAYING)
                            game->snake.direction = DIR_LEFT;
                        break;

                    case SDLK_RIGHT:
                    case SDLK_d:
                        if (game->snake.direction != DIR_LEFT && game->state == GAME_PLAYING)
                            game->snake.direction = DIR_RIGHT;
                        break;

                    // 游戏控制
                    case SDLK_SPACE:
                        if (game->state == GAME_START) {
                            game->state = GAME_PLAYING;
                        } else if (game->state == GAME_PLAYING) {
                            game->state = GAME_PAUSED;
                        } else if (game->state == GAME_PAUSED) {
                            game->state = GAME_PLAYING;
                        } else if (game->state == GAME_OVER) {
                            reset_game(game);
                        }
                        break;

                    case SDLK_RETURN:
                        if (game->state == GAME_START) {
                            game->state = GAME_PLAYING;
                        } else if (game->state == GAME_OVER) {
                            reset_game(game);
                        }
                        break;

                    case SDLK_ESCAPE:
                        game->running = false;
                        break;

                    case SDLK_r:
                        if (game->state == GAME_OVER) {
                            reset_game(game);
                        }
                        break;
                }
                break;
        }
    }
}

// 更新游戏逻辑
void update_game(Game* game) {
    if (game->state != GAME_PLAYING) {
        return;
    }

    Uint32 current_time = SDL_GetTicks();

    // 控制蛇的移动速度
    if (current_time - game->last_move_time >= game->speed) {
        move_snake(game);
        check_collisions(game);
        game->last_move_time = current_time;
    }

    // 每得100分增加一次速度（最多到50ms）
    if (game->score >= 100 && game->speed > 50) {
        game->speed = 150 - (game->score / 10);
        if (game->speed < 50) game->speed = 50;
    }
}

// 移动蛇
void move_snake(Game* game) {
    if (!game->snake.head) return;

    // 计算新头部位置
    SnakeNode* new_head = (SnakeNode*)malloc(sizeof(SnakeNode));
    if (!new_head) {
        printf("内存分配失败！\n");
        exit(1);
    }

    new_head->x = game->snake.head->x;
    new_head->y = game->snake.head->y;

    // 根据方向移动头部
    switch (game->snake.direction) {
        case DIR_UP:    new_head->y--; break;
        case DIR_DOWN:  new_head->y++; break;
        case DIR_LEFT:  new_head->x--; break;
        case DIR_RIGHT: new_head->x++; break;
    }

    // 处理边界穿越（可选：如果想实现穿墙，可以启用下面的代码）
    // new_head->x = (new_head->x + GRID_WIDTH) % GRID_WIDTH;
    // new_head->y = (new_head->y + GRID_HEIGHT) % GRID_HEIGHT;

    // 将新头部插入
    new_head->next = game->snake.head;
    game->snake.head = new_head;

    // 如果有待增长的长度，不删除尾部
    if (game->snake.pending_growth > 0) {
        game->snake.pending_growth--;
        game->snake.length++;
    } else {
        // 删除尾部节点
        SnakeNode* current = game->snake.head;
        while (current->next != game->snake.tail) {
            current = current->next;
        }
        free(game->snake.tail);
        current->next = NULL;
        game->snake.tail = current;
    }
}

// 检查碰撞
void check_collisions(Game* game) {
    // 检查墙壁碰撞
    if (check_wall_collision(game)) {
        game->state = GAME_OVER;
        return;
    }

    // 检查自身碰撞
    if (check_self_collision(game)) {
        game->state = GAME_OVER;
        return;
    }

    // 检查食物碰撞
    if (check_food_collision(game)) {
        game->score += 10;
        game->snake.pending_growth += 2;  // 吃一个食物增长2节
        spawn_food(game);

        // 更新最高分
        if (game->score > game->high_score) {
            game->high_score = game->score;
        }
    }
}

// 检查食物碰撞
bool check_food_collision(Game* game) {
    return (game->snake.head->x == game->food.x &&
            game->snake.head->y == game->food.y);
}

// 检查自身碰撞
bool check_self_collision(Game* game) {
    SnakeNode* current = game->snake.head->next;  // 从第二节开始检查

    while (current) {
        if (game->snake.head->x == current->x &&
            game->snake.head->y == current->y) {
            return true;
        }
        current = current->next;
    }

    return false;
}

// 检查墙壁碰撞
bool check_wall_collision(Game* game) {
    return (game->snake.head->x < 0 ||
            game->snake.head->x >= GRID_WIDTH ||
            game->snake.head->y < 0 ||
            game->snake.head->y >= GRID_HEIGHT);
}

// 增长蛇身
void grow_snake(Game* game) {
    game->snake.pending_growth++;
}

// 重置游戏
void reset_game(Game* game) {
    game->score = 0;
    game->speed = 150;
    game->state = GAME_PLAYING;
    init_snake(game);
    spawn_food(game);
}

// 渲染游戏
void render_game(Game* game) {
    // 清屏
    SDL_SetRenderDrawColor(game->renderer, COLOR_BACKGROUND);
    SDL_RenderClear(game->renderer);

    // 绘制网格
    render_grid(game);

    // 绘制蛇
    render_snake(game);

    // 绘制食物
    render_food(game);

    // 绘制UI
    render_ui(game);

    // 显示渲染结果
    SDL_RenderPresent(game->renderer);
}

// 绘制网格
void render_grid(Game* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_GRID);

    // 绘制垂直线
    for (int x = 0; x <= WINDOW_WIDTH; x += GRID_SIZE) {
        SDL_RenderDrawLine(game->renderer, x, 0, x, WINDOW_HEIGHT - 100);
    }

    // 绘制水平线
    for (int y = 0; y <= WINDOW_HEIGHT - 100; y += GRID_SIZE) {
        SDL_RenderDrawLine(game->renderer, 0, y, WINDOW_WIDTH, y);
    }
}

// 绘制蛇
void render_snake(Game* game) {
    SnakeNode* current = game->snake.head;
    int index = 0;

    while (current) {
        SDL_Rect rect = {
            current->x * GRID_SIZE,
            current->y * GRID_SIZE,
            GRID_SIZE,
            GRID_SIZE
        };

        // 蛇头用不同颜色
        if (index == 0) {
            SDL_SetRenderDrawColor(game->renderer, COLOR_SNAKE_HEAD);
        } else {
            SDL_SetRenderDrawColor(game->renderer, COLOR_SNAKE_BODY);
        }

        SDL_RenderFillRect(game->renderer, &rect);

        current = current->next;
        index++;
    }
}

// 绘制食物
void render_food(Game* game) {
    SDL_Rect rect = {
        game->food.x * GRID_SIZE,
        game->food.y * GRID_SIZE,
        GRID_SIZE,
        GRID_SIZE
    };

    SDL_SetRenderDrawColor(game->renderer, COLOR_FOOD);
    SDL_RenderFillRect(game->renderer, &rect);

    // 绘制食物内部的小矩形，使其看起来更像苹果
    SDL_SetRenderDrawColor(game->renderer, 0xFF, 0xCC, 0xCC, 0xFF);
    SDL_Rect inner_rect = {
        game->food.x * GRID_SIZE + 4,
        game->food.y * GRID_SIZE + 4,
        GRID_SIZE - 8,
        GRID_SIZE - 8
    };
    SDL_RenderFillRect(game->renderer, &inner_rect);
}

// 绘制UI
void render_ui(Game* game) {
    SDL_Color text_color = {COLOR_TEXT};

    // 绘制分数区域背景
    SDL_Rect score_rect = {0, WINDOW_HEIGHT - 100, WINDOW_WIDTH, 100};
    SDL_SetRenderDrawColor(game->renderer, 0x25, 0x25, 0x25, 0xFF);
    SDL_RenderFillRect(game->renderer, &score_rect);

    // 绘制分数
    char score_text[100];
    snprintf(score_text, sizeof(score_text), "分数: %d", game->score);
    render_text(game, score_text, 20, WINDOW_HEIGHT - 80, text_color);

    // 绘制最高分
    snprintf(score_text, sizeof(score_text), "最高分: %d", game->high_score);
    render_text(game, score_text, 20, WINDOW_HEIGHT - 50, text_color);

    // 绘制速度/等级
    snprintf(score_text, sizeof(score_text), "速度: %d", (160 - game->speed) / 10);
    render_text(game, score_text, WINDOW_WIDTH - 200, WINDOW_HEIGHT - 80, text_color);

    // 绘制操作提示
    snprintf(score_text, sizeof(score_text), "控制: 方向键/WASD 移动 | 空格 暂停 | R 重新开始 | ESC 退出");
    render_text(game, score_text, WINDOW_WIDTH/2 - 250, WINDOW_HEIGHT - 30, text_color);

    // 根据游戏状态显示不同信息
    switch (game->state) {
        case GAME_START:
            render_text(game, "贪吃蛇游戏", WINDOW_WIDTH/2 - 100, 100, text_color);
            render_text(game, "按 SPACE 或 ENTER 开始游戏", WINDOW_WIDTH/2 - 150, 150, text_color);
            break;

        case GAME_PAUSED:
            SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 128);
            SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderFillRect(game->renderer, &overlay);
            SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_NONE);

            render_text(game, "游戏暂停", WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 - 50, text_color);
            render_text(game, "按 SPACE 继续", WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, text_color);
            break;

        case GAME_OVER:
            SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 192);
            SDL_Rect overlay2 = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
            SDL_RenderFillRect(game->renderer, &overlay2);
            SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_NONE);

            SDL_Color game_over_color = {COLOR_GAME_OVER};
            render_text(game, "游戏结束!", WINDOW_WIDTH/2 - 80, WINDOW_HEIGHT/2 - 100, game_over_color);

            snprintf(score_text, sizeof(score_text), "最终分数: %d", game->score);
            render_text(game, score_text, WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 50, text_color);

            render_text(game, "按 R 或 ENTER 重新开始", WINDOW_WIDTH/2 - 150, WINDOW_HEIGHT/2, text_color);
            render_text(game, "按 ESC 退出游戏", WINDOW_WIDTH/2 - 120, WINDOW_HEIGHT/2 + 40, text_color);
            break;

        case GAME_PLAYING:
            // 游戏中不显示额外信息
            break;
    }
}

// 渲染文本
// void render_text(Game* game, const char* text, int x, int y, SDL_Color color) {
//     SDL_Surface* surface = TTF_RenderText_Solid(game->font, text, color);
//     if (!surface) return;

//     SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
//     if (!texture) {
//         SDL_FreeSurface(surface);
//         return;
//     }

//     SDL_Rect rect = {x, y, surface->w, surface->h};
//     SDL_RenderCopy(game->renderer, texture, NULL, &rect);

//     SDL_DestroyTexture(texture);
//     SDL_FreeSurface(surface);
// }
void render_text(Game* game, const char* text, int x, int y, SDL_Color color) {
    if (!game->font) {
        // 如果没有字体，绘制一个简单的矩形作为占位符
        SDL_Rect rect = {x, y, strlen(text) * 10, 20};
        SDL_SetRenderDrawColor(game->renderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawRect(game->renderer, &rect);
        return;
    }

    // 使用UTF-8编码渲染文本
    SDL_Surface* surface = TTF_RenderUTF8_Solid(game->font, text, color);
    if (!surface) {
        // 如果UTF-8失败，尝试使用默认编码
        surface = TTF_RenderText_Solid(game->font, text, color);
        if (!surface) {
            printf("文本渲染失败: %s\n", TTF_GetError());
            return;
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(game->renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(game->renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}


// 清理资源
void cleanup(Game* game) {
    // 清理蛇身链表
    SnakeNode* current = game->snake.head;
    while (current) {
        SnakeNode* next = current->next;
        free(current);
        current = next;
    }

    // 清理字体
    if (game->font) {
        TTF_CloseFont(game->font);
    }

    // 清理渲染器和窗口
    if (game->renderer) {
        SDL_DestroyRenderer(game->renderer);
    }

    if (game->window) {
        SDL_DestroyWindow(game->window);
    }

    // 退出SDL
    TTF_Quit();
    SDL_Quit();
}

// ===================== 主函数 =====================
int main(int argc, char* argv[]) {
    Game game;

    printf("=== 贪吃蛇游戏 ===\n");
    printf("正在初始化游戏...\n");

    // 初始化游戏
    if (!init_game(&game)) {
        printf("游戏初始化失败！\n");
        return 1;
    }

    printf("游戏初始化成功！\n");
    printf("游戏控制说明：\n");
    printf("  方向键/WASD - 控制蛇移动\n");
    printf("  空格键 - 暂停/继续\n");
    printf("  R键 - 重新开始（游戏结束后）\n");
    printf("  ESC键 - 退出游戏\n");

    // 主游戏循环
    while (game.running) {
        // 处理输入
        handle_input(&game);

        // 更新游戏逻辑
        update_game(&game);

        // 渲染游戏
        render_game(&game);

        // 控制帧率
        SDL_Delay(16);  // 约60FPS
    }

    // 清理资源
    cleanup(&game);

    printf("游戏结束。感谢游玩！\n");
    return 0;
}
