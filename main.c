#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 40

typedef struct {
    int x, y;
    char TankView[16][4];
    int direction;
    bool active;
} Tank;

typedef struct {
    int x, y;
    char BlockView[4][4];
    int HP;
    int IsBreakable;
    int active;
} Block;

typedef struct {
    int x, y;
    char bulletTop[2];
    char bulletLeft[2];
    int direction;
    HANDLE thread;
    int active;
} Bullet;

typedef struct {
    int x, y;
    int direction;
    int active;
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
bool CheckBlockCollision(Map *map) {
    Tank *tank = map->Tank;
    for (int i = 0; i < map->BlockCount; i++) {
        Block block = map->Blocks[i];

        int tankLeft = tank->x;
        int tankRight = tank->x + 4; // ширина танка
        int tankTop = tank->y;
        int tankBottom = tank->y + 4; // высота танка

        int blockLeft = block.x;
        int blockRight = block.x + 4; // ширина врага
        int blockTop = block.y;
        int blockBottom = block.y + 4; // высота врага
        printf("\033[%d;%dH%i\t%i", 1, WIDTH, tankTop, blockTop);
        printf("\033[%d;%dH%i\t%i", 2, WIDTH, tankRight, blockRight);
        printf("\033[%d;%dH%i\t%i", 3, WIDTH, tankBottom, blockBottom);
        printf("\033[%d;%dH%i\t%i", 4, WIDTH, tankLeft, blockLeft);

        if (!(tankRight <= blockLeft ||
              tankLeft >= blockRight ||
              tankBottom <= blockTop ||
              tankTop >= blockBottom))
            return true;
        //
        // return !(tankRight <= blockLeft ||
        //      tankLeft >= blockRight ||
        //      tankBottom <= blockTop ||
        //      tankTop >= blockBottom);
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
    //for (int i = 0; i < map->EnemyCount; i++) {
    Enemy enemy = map->Enemies[i];
    if (enemy.active) {
        int k = 0;
        for (int i = 4 * (enemy.direction - 1); i < 4 * (enemy.direction); i++) {
            printf("\033[%d;%dH%.4s", enemy.y + k++, enemy.x, enemy.enemyView[i]);
        }
    }
    //}
}

//Функция отрисовки блока
void draw_block(Map *map) {
    for (int i = 0; i < map->BlockCount; i++) {
        Block block = map->Blocks[i];
        int k = 0;
        for (int i = 0; i < 4; i++) {
            printf("\033[%d;%dH%.4s", block.y + k++, block.x, block.BlockView[i]);
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

//Функция движения врага
// void EnemyRules(*map) {
//     Map* map = &map;
//     int sh;
//     for (int i = 0; i < map->EnemyCount; i++) {
//         struct tm *ptr;
//         ptr = time(NULL);
//         if (map->Enemies[i].active) {
//                 if (map->Enemies[i].active) {
//                     int k = 0;
//                     for (int j = 4 * (map->Enemies[i].direction - 1); j < 4 * (map->Enemies[i].direction); j++) {
//                         printf("\033[%d;%dH%.4s", map->Enemies[i].y + k++, map->Enemies[i].x, map->Enemies[i].enemyView[i]);
//                     }
//                 }
//
//             if (map->Tank->y!=map->Enemies[i].y&&map->Tank->x!=map->Enemies[i].x) {
//                 if(abs(map->Tank->y-map->Enemies[i].y)>abs(map->Tank->x-map->Enemies[i].x)) {
//                     while (map->Tank->x!=map->Enemies[i].x) {
//                         if (map->Tank->x>map->Enemies[i].x)
//                             sh=-1;
//                         else
//                             sh=1;
//                         map->Enemies[i].x+=sh;
//                     }
//                 }
//                 else {
//                     while (map->Tank->y!=map->Enemies[i].y) {
//                         if (map->Tank->y>map->Enemies[i].y)
//                             sh=-1;
//                         else
//                             sh=1;
//                         map->Enemies[i].y+=sh;
//                     }
//                 }
//             }
//             if (map->Tank->y==map->Enemies[i].y) {
//                 if (map->Tank->x>map->Enemies[i].x)
//                     map->Enemies[i].direction==2;
//                 else
//                     map->Enemies[i].direction==3;
//                 if (ptr->tm_sec==5) {
//                     Bullet bullet = InitBullet(map->Enemies->x + 1, map->Enemies->y + 1, map->Enemies->direction, &map);
//                     bullet.thread = CreateThread(NULL, 0, shoot, &map, 0, NULL);
//                     time(NULL);
//                 }
//             }
//             if (map->Tank->x==map->Enemies[i].x) {
//                 if (map->Tank->y>map->Enemies[i].y)
//                     map->Enemies[i].direction==4;
//                 else
//                     map->Enemies[i].direction==1;
//                 if (ptr->tm_sec==5) {
//                     Bullet bullet = InitBullet(map->Enemies->x + 1, map->Enemies->y + 1, map->Enemies->direction, &map);
//                     bullet.thread = CreateThread(NULL, 0, shoot, &map, 0, NULL);
//                     time(NULL);
//                 }
//             }
//         }
//     }
// }


// Очистка экрана
void clear_screen() {
    system("cls");
}


void MoveEnemyTowardsTank(Map *map, int i) {
    Tank *tank = map->Tank;
    Enemy *enemy = &map->Enemies[i];
    int sh;
    if (abs(tank->y - enemy->y) >= abs(tank->x - enemy->x)) {
        enemy->y += (tank->y > enemy->y) ? 1 : -1;
    } else {
        enemy->x += (tank->x > enemy->x) ? 1 : -1;
    }
}

void SetEnemyDirection(Map *map, int i) {
    Tank *tank = map->Tank;
    Enemy *enemy = &map->Enemies[i];
    if (tank->y == enemy->y) {
        enemy->direction = (tank->x > enemy->x) ? 2 : 3; // Вправо или влево
    } else if (tank->x == enemy->x) {
        enemy->direction = (tank->y > enemy->y) ? 4 : 1; // Вниз или вверх
    }
}

void EnemyShoot(Map *map, int i) {
    Enemy *enemy = &map->Enemies[i];
    Bullet bullet = InitBullet(enemy->x + 1, enemy->y + 1, enemy->direction, map);
    bullet.thread = CreateThread(NULL, 0, shoot, map, 0, NULL);
}

void EnemyRules(Map *map) {
    for (int i = 0; i < map->EnemyCount; i++) {
        if (map->Enemies[i].active) {
            draw_enemy(map, i);
            MoveEnemyTowardsTank(map, i);
            SetEnemyDirection(map, i);

            time_t ptr = time(NULL);

            if ((map->Tank->y == map->Enemies[i].y || map->Tank->x == map->Enemies[i].x) && ptr % 5 == 0) {
                EnemyShoot(map, i);
            }
        }
    }
}

// Основная функция
int main() {
    Tank tank = InitTank(10, 10);
    Map map = InitMap(WIDTH, HEIGHT);
    Enemy enemy = InitEnemy(20, 20, &map);
    map.Tank = &tank;
    int enemyShootCooldown = 0;
    Block block = InitBlock(1, 1, &map);
    map.Blocks[0] = block;
    map.Enemies[0] = enemy;

    printf("\033[?25l");
    while (map.Tank->active) {
        clear_screen();
        EnemyRules(&map);
        draw_tank(&map);
        draw_block(&map);
        for (int i = 0; i < map.EnemyCount; i++) {
            if (map.Enemies[i].active) {
            }
        }
        printf("\033[%d;%dH%i", WIDTH, 0, CheckBlockCollision(&map));

        // Обработка клавиш движения
        if (GetAsyncKeyState('W') & 0x8000 && map.Tank->y > 0) {
            map.Tank->y--;
            if (CheckBlockCollision(&map)) {
                map.Tank->y++;
            }
            CheckCollision(&map);
            map.Tank->direction = 1;
        }
        if (GetAsyncKeyState('S') & 0x8000 && map.Tank->y < HEIGHT - 4) {
            map.Tank->y++;
            if (CheckBlockCollision(&map)) {
                map.Tank->y--;
            }
            CheckCollision(&map);
            map.Tank->direction = 4;
        }
        if (GetAsyncKeyState('A') & 0x8000 && map.Tank->x > 0) {
            map.Tank->x--;
            if (CheckBlockCollision(&map)) {
                map.Tank->x++;
            }
            CheckCollision(&map);
            map.Tank->direction = 2;
        }
        if (GetAsyncKeyState('D') & 0x8000 && map.Tank->x < WIDTH - 4) {
            map.Tank->x++;
            if (CheckBlockCollision(&map)) {
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
    free(map.Bullets);
    free(map.Enemies);
    printf("Log : Game Over");
    Sleep(1000);
    return 0;
}
