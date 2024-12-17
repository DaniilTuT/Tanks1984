#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#define WIDTH 160
#define HEIGHT 40
typedef struct {
    int x, y;
    char TankView[16][4];
    short int direction;
    bool active;
} Tank;
typedef struct {
    int x, y;
    char BlockView[4][4];
    short int HP;
    bool IsBreakable;
    bool active;
} Block;
typedef struct {
    int x, y;
    char bulletTop[2];
    char bulletLeft[2];
    short int direction;
    HANDLE thread;
    bool active;
} Bullet;
typedef struct {
    int x, y;
    short int direction;
    bool active;
    HANDLE thread;
    char enemyView[16][4];
} Enemy;
typedef struct {
    int Height;
    int Width;
    Tank *Tank;
    int BulletCount;
    Bullet *Bullets;
    int EnemyCount;
    Enemy *Enemies;
    int BlockCount;
    Block *Blocks;
} Map;
// Функция проверки столкновения с врагом
void CheckCollision(Map *map) {
    Tank *tank = map->Tank;
    for (int i = 0; i < map->EnemyCount; i++) {
        Enemy enemy = map->Enemies[i];
        if (enemy.active) {
            int tankLeft = tank->x;
            int tankRight = tank->x + 4; // ширина танка
            int tankTop = tank->y;
            int tankBottom = tank->y + 4; // высота танка
            int enemyLeft = enemy.x;
            int enemyRight = enemy.x + 4; // ширина врага
            int enemyTop = enemy.y;
            int enemyBottom = enemy.y + 4; // высота врага
            tank->active = (tankRight <= enemyLeft ||
                            tankLeft >= enemyRight ||
                            tankBottom <= enemyTop ||
                            tankTop >= enemyBottom);
        }
    }
}
// Функция проверки колизии блока
bool CheckBlockCollision(Map *map, int x, int y) {
    for (int i = 0; i < map->BlockCount; i++) {
        Block block = map->Blocks[i];
        int tankLeft = x;
        int tankRight = x + 4; // ширина танка
        int tankTop = y;
        int tankBottom = y + 4; // высота танка
        int blockLeft = block.x;
        int blockRight = block.x + 4; // ширина врага
        int blockTop = block.y;
        int blockBottom = block.y + 4; // высота врага
        if (!(tankRight <= blockLeft ||
              tankLeft >= blockRight ||
              tankBottom <= blockTop ||
              tankTop >= blockBottom))
            return true;
    }
    return false;
}
// Функция инициализация врага
Enemy InitEnemy(int x, int y, Map *map) {
    Enemy enemy = {
            x, y,
            4,
            1,
            NULL,
            {
                    {' ', '|', '|', ' '}, //1
                    {'{', '|', '|', '}'},
                    {'|', '[', ']', '|'},
                    {'{', '_', '_', '}'},
                    {' ', '/', '=', '\\'}, //2
                    {'-', '-', '=', '|'},
                    {'-', '-', '=', '|'},
                    {' ', '\\', '=', '/'},
                    {'/', '=', '\\', ' '}, //3
                    {'|', '=', '-', '-'},
                    {'|', '=', '-', '-'},
                    {'\\', '=', '/', ' '},
                    {'{', '_', '_', '}'}, //4
                    {'|', '[', ']', '|'},
                    {'{', '|', '|', '}'},
                    {' ', '|', '|', ' '}
            }
    };
    map->Enemies = realloc(map->Enemies, sizeof(Enemy) * (++map->EnemyCount));
    map->Enemies[map->EnemyCount - 1] = enemy;
    return enemy;
}
// Функция инициализации карты
Map InitMap(int width, int height) {
    Map map = {
            .Width = width,
            .Height = height,
            .Tank = NULL,
            .Bullets = malloc(sizeof(Bullet)),
            .BulletCount = 0,
    };
    return map;
}
//Фунция инициализации блока
Block InitBlock(int x, int y, Map *map) {
    Block block = {
            x,
            y,
            {"####", "####", "####", "####"},
            -1,
            0,
            1
    };
    map->Blocks = realloc(map->Blocks, sizeof(Block) * (++map->BlockCount));
    map->Blocks[map->BlockCount - 1] = block;
    return block;
}
// Функция инициализации танка
Tank InitTank(int x, int y) {
    Tank tank = {
            x, y,
            {
                    {' ', '|', '|', ' '},
                    {'{', '|', '|', '}'},
                    {'|', '[', ']', '|'},
                    {'{', '_', '_', '}'},
                    {' ', '/', '=', '\\'},
                    {'-', '-', '=', '|'},
                    {'-', '-', '=', '|'},
                    {' ', '\\', '=', '/'},
                    {'/', '=', '\\', ' '},
                    {'|', '=', '-', '-'},
                    {'|', '=', '-', '-'},
                    {'\\', '=', '/', ' '},
                    {'{', '_', '_', '}'},
                    {'|', '[', ']', '|'},
                    {'{', '|', '|', '}'},
                    {' ', '|', '|', ' '}
            },
            1,
            1
    };
    return tank;
}
// Функция инициализации пули танка
Bullet InitBullet(int x, int y, int direction, Map *map) {
    if (direction == 1) y -= 2;
    if (direction == 2) x -= 2;
    if (direction == 3) x += 3;
    if (direction == 4) y += 3;
    Bullet bullet = {
            x, y,
            {'|', '|'},
            {'-', '-'},
            direction,
            NULL,
            1
    };
    map->Bullets = realloc(map->Bullets, sizeof(Bullet) * (++map->BulletCount));
    map->Bullets[map->BulletCount - 1] = bullet;
    return bullet;
}
// Функция отрисовки танка
void draw_tank(Map *map) {
    Tank *tank = map->Tank;
    if (tank->active) {
        int k = 0;
        for (int i = 4 * (tank->direction - 1); i < 4 * (tank->direction); i++) {
            printf("\033[%d;%dH%.4s", tank->y + k++, tank->x, tank->TankView[i]);
        }
    }
}
// Функция отрисовки врага
void draw_enemy(Map *map, int i) {
    Enemy enemy = map->Enemies[i];
    if (enemy.active) {
        int k = 0;
        for (int i = 4 * (enemy.direction - 1); i < 4 * (enemy.direction); i++) {
            printf("\033[%d;%dH%.4s", enemy.y + k++, enemy.x, enemy.enemyView[i]);
        }
    }
}
//Функция отрисовки блока
void draw_block(Map *map) {
    for (int i = 0; i < map->BlockCount; i++) {
        Block block = map->Blocks[i];
        if (block.active) {
            int k = 0;
            for (int i = 0; i < 4; i++) {
                printf("\033[%d;%dH%.4s", block.y + k++, block.x, block.BlockView[i]);
            }
        }
    }
}
// Поток для обработки движения пули
DWORD WINAPI shoot(LPVOID param) {
    Map *map = (Map *) param;
    Bullet bullet = map->Bullets[map->BulletCount - 1];
    while (bullet.active) {
        if (map->Tank->active &&
            bullet.x >= map->Tank->x && bullet.x < map->Tank->x + 4 &&
            bullet.y >= map->Tank->y && bullet.y < map->Tank->y + 4) {
            map->Tank->active = 0;
            bullet.active = false;
        }
        for (int i = 0; i < map->EnemyCount; i++) {
            if (map->Enemies[i].active &&
                bullet.x >= map->Enemies[i].x && bullet.x < map->Enemies[i].x + 4 &&
                bullet.y >= map->Enemies[i].y && bullet.y < map->Enemies[i].y + 4) {
                map->Enemies[i].active = 0;
                bullet.active = false;
            }
        }
        for (int i = 0; i < map->BlockCount; i++) {
            if (
                    bullet.x >= map->Blocks[i].x && bullet.x < map->Blocks[i].x + 4 &&
                    bullet.y >= map->Blocks[i].y && bullet.y < map->Blocks[i].y + 4) {
                if (!map->Blocks[i].IsBreakable)
                    map->Blocks[i].HP -= 1;
                if (map->Blocks[i].HP == 0)
                    map->Blocks[i].active = 0;
                bullet.active = false;
            }
        }
        if (bullet.direction == 1) bullet.y--;
        if (bullet.direction == 2) bullet.x--;
        if (bullet.direction == 3) bullet.x++;
        if (bullet.direction == 4) bullet.y++;
        if (bullet.x < 0 || bullet.x >= WIDTH || bullet.y < 0 || bullet.y >= HEIGHT) {
            bullet.active = 0;
            map->Bullets = realloc(map->Bullets, sizeof(Bullet) * (--map->BulletCount));
        }
        if (bullet.direction == 1 || bullet.direction == 4 && bullet.active) {
            printf("\033[%d;%dH%.2s", bullet.y, bullet.x, bullet.bulletTop);
            Sleep(50);
            printf("\033[%d;%dH  ", bullet.y, bullet.x);
        }
        if (bullet.direction == 2 || bullet.direction == 3 && bullet.active) {
            printf("\033[%d;%dH%c", bullet.y, bullet.x, bullet.bulletLeft[0]);
            printf("\033[%d;%dH%c", bullet.y + 1, bullet.x, bullet.bulletLeft[1]);
            Sleep(50);
            printf("\033[%d;%dH ", bullet.y, bullet.x);
            printf("\033[%d;%dH ", bullet.y + 1, bullet.x);
        }
    }
    return 0;
}
// Очистка экрана
void clear_screen() {
    system("cls");
}
// Функция для движение врага
void MoveEnemyTowardsTank(Map *map, int i) {
    Enemy *enemy = &map->Enemies[i];
    int dx = map->Tank->x - enemy->x;
    int dy = map->Tank->y - enemy->y;
    if (dx == 0 && dy == 0)
        return;
    bool moveAlongX = abs(dx) > abs(dy);
    if (moveAlongX)
        if (CheckBlockCollision(map, enemy->x + 1, enemy->y) || CheckBlockCollision(map, enemy->x - 1, enemy->y)) {
            enemy->y++;
            enemy->direction = 4;
        } else if (dx > 0 && !CheckBlockCollision(map, enemy->x + 1, enemy->y)) {
            enemy->x++;
            enemy->direction = 3;
        } else if (dx < 0 && !CheckBlockCollision(map, enemy->x - 1, enemy->y)) {
            enemy->x--;
            enemy->direction = 2;
        } else
            moveAlongX = false;
    if (!moveAlongX)
        if (CheckBlockCollision(map, enemy->x, enemy->y + 1) || CheckBlockCollision(map, enemy->x, enemy->y - 1)) {
            enemy->x++;
            enemy->direction = 3;
        } else if (dy > 0 && !CheckBlockCollision(map, enemy->x, enemy->y + 1)) {
            enemy->y++;
            enemy->direction = 4;
        } else if (dy < 0 && !CheckBlockCollision(map, enemy->x, enemy->y - 1)) {
            enemy->y--;
            enemy->direction = 1;
        } else
            moveAlongX = true;
}
//Функция стрельбы врага
void EnemyShoot(Map *map, int i) {
    Enemy *enemy = &map->Enemies[i];
    Bullet bullet = InitBullet(enemy->x + 1, enemy->y + 1, enemy->direction, map);
    bullet.thread = CreateThread(NULL, 0, shoot, map, 0, NULL);
}
//Функция для обработки врага
DWORD WINAPI EnemyRules(LPVOID param) {
    Map *map = (Map *) param;
    while (map->Tank->active) {
        for (int i = 0; i < map->EnemyCount; i++) {
            if (map->Enemies[i].active) {
                Sleep(400);
                CheckCollision(map);
                MoveEnemyTowardsTank(map, i);
                time_t ptr = time(NULL);
                if ((map->Tank->y == map->Enemies[i].y || map->Tank->x == map->Enemies[i].x) && ptr % 5 == 0) {
                    EnemyShoot(map, i);
                }
            }
        }
    }
    return 0;
}
//Процедура генерации карты
Map MapGenerator(Map *Maps, short int BlockCount, short int EnemyCount, int seed) {
    Map map = *Maps;
    srand(seed);
    map.Tank = malloc(sizeof(Tank));
    *map.Tank = InitTank(1, 1);
    int maxAttempts = BlockCount * 10;
    for (int i = 0; i < BlockCount && maxAttempts > 0; maxAttempts--) {
        int blockLeft = rand() % (WIDTH - 4);
        int blockTop = rand() % (HEIGHT - 4);
        bool canPlace = true;
        if (!(blockLeft + 4 <= map.Tank->x ||
              blockLeft >= map.Tank->x + 4 ||
              blockTop + 4 <= map.Tank->y ||
              blockTop >= map.Tank->y + 4)) {
            canPlace = false;
        }
        for (int j = 0; j < map.BlockCount && canPlace; j++) {
            if (!(blockLeft + 4 <= map.Blocks[j].x ||
                  blockLeft >= map.Blocks[j].x + 4 ||
                  blockTop + 4 <= map.Blocks[j].y ||
                  blockTop >= map.Blocks[j].y + 4)) {
                canPlace = false;
                break;
            }
        }
        if (canPlace) {
            InitBlock(blockLeft, blockTop, &map);
            i++;
        }
    }
    maxAttempts = EnemyCount * 10;
    for (int i = 0; i < EnemyCount && maxAttempts > 0; maxAttempts--) {
        int enemyLeft = rand() % (WIDTH - 4);
        int enemyTop = rand() % (HEIGHT - 4);
        bool canPlace = true;
        if (!(enemyLeft + 4 <= map.Tank->x ||
              enemyLeft >= map.Tank->x + 4 ||
              enemyTop + 4 <= map.Tank->y ||
              enemyTop >= map.Tank->y + 4)) {
            canPlace = false;
        }
        for (int j = 0; j < map.BlockCount && canPlace; j++) {
            if (!(enemyLeft + 4 <= map.Blocks[j].x ||
                  enemyLeft >= map.Blocks[j].x + 4 ||
                  enemyTop + 4 <= map.Blocks[j].y ||
                  enemyTop >= map.Blocks[j].y + 4)) {
                canPlace = false;
                break;
            }
        }
        for (int j = 0; j < map.EnemyCount && canPlace; j++) {
            if (!(enemyLeft + 4 <= map.Enemies[j].x ||
                  enemyLeft >= map.Enemies[j].x + 4 ||
                  enemyTop + 4 <= map.Enemies[j].y ||
                  enemyTop >= map.Enemies[j].y + 4)) {
                canPlace = false;
                break;
            }
        }
        if (canPlace) {
            InitEnemy(enemyLeft, enemyTop, &map);
            i++;
        }
    }
    return map;
}
// Основная функция
int main() {
    printf("Введите кол-во блоков на карте:\n");
    short int BlockCount;
    scanf_s("%hd",&BlockCount);
    clear_screen();
    printf("Кол-во блоков = %hd",BlockCount);
    printf("\nВведите кол-во врагов на карте:\n");
    short int EnemyCount;
    scanf_s("%hd",&EnemyCount);
    clear_screen();
    printf("Кол-во блоков = %hd",BlockCount);
    printf("\nКол-во блоков = %hd",EnemyCount);
    printf("\nВведите сид генерации:\n");
    int seed;
    scanf_s("%d",&seed);
    clear_screen();
    printf("Кол-во блоков = %hd",BlockCount);
    printf("\nКол-во врагов = %hd",EnemyCount);
    printf("\nСид генерации = %d",seed);
    if (seed==0) {
        srand(time(NULL));
        seed=rand();
    }
    //Генерация карты,Создание ядра для врагов
    Map map = InitMap(WIDTH, HEIGHT);
    map = (MapGenerator(&map, BlockCount, EnemyCount, seed));
    HANDLE enemyThread = CreateThread(NULL, 0, EnemyRules, &map, 0, NULL);
    printf("\033[?25l");
    while (map.Tank->active) {
        clear_screen();
        draw_tank(&map);
        draw_block(&map);
        for (int i = 0; i < map.EnemyCount; i++) {
            if (map.Enemies[i].active) {
                draw_enemy(&map, i);
            }
        }
        // Обработка клавиш движения
        if (GetAsyncKeyState('W') & 0x8000 && map.Tank->y > 0) {
            map.Tank->y--;
            if (CheckBlockCollision(&map, map.Tank->x, map.Tank->y)) {
                map.Tank->y++;
            }
            CheckCollision(&map);
            map.Tank->direction = 1;
        }
        if (GetAsyncKeyState('S') & 0x8000 && map.Tank->y < HEIGHT - 4) {
            map.Tank->y++;
            if (CheckBlockCollision(&map, map.Tank->x, map.Tank->y)) {
                map.Tank->y--;
            }
            CheckCollision(&map);
            map.Tank->direction = 4;
        }
        if (GetAsyncKeyState('A') & 0x8000 && map.Tank->x > 0) {
            map.Tank->x--;
            if (CheckBlockCollision(&map, map.Tank->x, map.Tank->y)) {
                map.Tank->x++;
            }
            CheckCollision(&map);
            map.Tank->direction = 2;
        }
        if (GetAsyncKeyState('D') & 0x8000 && map.Tank->x < WIDTH - 4) {
            map.Tank->x++;
            if (CheckBlockCollision(&map, map.Tank->x, map.Tank->y)) {
                map.Tank->x--;
            }
            CheckCollision(&map);
            map.Tank->direction = 3;
        }
        // Обработка выстрела
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            Bullet bullet = InitBullet(map.Tank->x + 1, map.Tank->y + 1, map.Tank->direction, &map);
            bullet.thread = CreateThread(NULL, 0, shoot, &map, 0, NULL);
        }
        Sleep(100);
    }
    free(map.Tank);
    free(map.Bullets);
    free(map.Enemies);
    free(map.Blocks);
    clear_screen();
    printf("Log : Game Over");
    Sleep(10000);
    return 0;
}