#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h> 
#include <time.h>  

#define MAP_WIDTH 200 //�� ����
#define MAP_HEIGHT 20 //�� ����
#define P_MAP_WIDTH 120 //��� ȭ�� ����
#define P_MAP_HEIGHT 20 // ��� ȭ�� ����
#define MONSTER_COUNT 5 // ���� ����
#define RED_BACKGROUND "\033[41m" // ���� ���
#define RESET_COLOR "\033[0m" // ���� �ʱ�ȭ
#define ATTACK_RANGE 10 // ���� �ʱ�ȭ


// ĳ������ �� ������ ��Ÿ���� ����ü
typedef struct
{
    int x; // x��ǥ
    int y; // y��ǥ
} BodyPart;

typedef struct
{
    int x; // ��Ʈ�ڽ��� x��ǥ
    int y; // ��Ʈ�ڽ��� y��ǥ
    int width; // ��Ʈ�ڽ��� �ʺ�
    int height; // ��Ʈ�ڽ��� ����
} Hitbox;

// ĳ���� ����ü ����
typedef struct
{
    bool jumpState; // ���� ����
    bool isAlive; // ĳ������ ���� ����
    bool attackState; // ���� ����
    BodyPart head; // �Ӹ�
    BodyPart body; // ����
    BodyPart leftLeg; // ���� �ٸ�
    BodyPart rightLeg; // ������ �ٸ�
    float velocity; // �ӵ�
    float jumpVelocity; // ���� �ӵ�
    int LwalkFrame; // �ȱ� ������
    int RwalkFrame; // �ȱ� ������
    int health; // ĳ������ ü��
    int RIGHT; //RlGHT = 1, LEFT = 0
    int attackPower; // ĳ������ ���ݷ�
    Hitbox hitbox[2]; // ��Ʈ�ڽ�
} Object;


// �÷��̾� �迭 �ʱ�ȭ
Object player = { false, true, false, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0, 10, 1, 5 }; // �ʱ�ȭ

// ���� �迭 ����
Object monsters[MONSTER_COUNT];

char map[MAP_HEIGHT][MAP_WIDTH]; // �� ����

int cameraOffset = 0; // ī�޶� ������ �ʱ�ȭ

void gotoxy(int x, int y); //Ŀ�� �̵� �Լ�
void printMap(Object player, char map[MAP_HEIGHT][MAP_WIDTH]);// �� ��� �Լ�
void initializeHitbox(Object* player); // ĳ���� ��Ʈ�ڽ�
void initialize_monsterHitbox(Object* mosnters); // ���� ��Ʈ�ڽ�
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Object player, int cameraOffset);// �÷��̾� �ʱ�ȭ �Լ�(��ġ)
void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Object mosnters[], int cameraOffset);// ���� �ʱ�ȭ �Լ�(��ġ)
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Object player, Object mosnters[], int cameraOffset);// �� �ʱ�ȭ �Լ�
void handleJumpAndGravity(Object* player);// ���� �� �߷� ó�� �Լ�
void moveCharacter(Object* player); //ĳ���� �̵� �Լ�
bool checkCollision(Object* player, Object* mosnters[], int i); //�浹 ���� �Լ�

// ĳ���� ��Ʈ�ڽ� ����� �Լ�
void printHitbox(Object player)
{
    printf("�÷��̾� ��Ʈ�ڽ�: x=%d, y=%d, width=%d, height=%d, HP=%d, attack=%d, attackPower=%d\n",
        player.hitbox[0].x, player.hitbox[0].y, player.hitbox[0].width, player.hitbox[0].height, player.health, player.attackState, player.attackPower);
}

// ���� ��Ʈ�ڽ� ����� �Լ�
void printMonsterHitboxes(Object monsters[])
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        printf("���� %d ��Ʈ�ڽ�: x=%d, y=%d, width=%d, height=%d,HP=%d, attack=%d\n",
            i, monsters[i].hitbox[0].x, monsters[i].hitbox[0].y,
            monsters[i].hitbox[0].width, monsters[i].hitbox[0].height, monsters[i].health, monsters[i].attackState);
    }
}

int main() {

    
    char input;
    int i;

    // ���� �õ� ����
    srand(time(NULL));
    
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        int monsterX = rand() % MAP_WIDTH + cameraOffset;
        int monsterY = MAP_HEIGHT;
        // ���� �ʱ�ȭ
        monsters[i] = (Object)
        {
            .jumpState = false,          // ���� ���� �ʱ�ȭ
            .isAlive = true,             // ���� ���� �ʱ�ȭ
            .attackState = false,
            .head = (BodyPart){monsterX - cameraOffset, monsterY -2},    // �Ӹ� �ʱ�ȭ
            .body = (BodyPart){monsterX - cameraOffset, monsterY - 1},    // ���� �ʱ�ȭ
            .leftLeg = (BodyPart){monsterX - cameraOffset - 1, monsterY - 1},  // ���� �ٸ� �ʱ�ȭ
            .rightLeg = (BodyPart){monsterX - cameraOffset + 1, monsterY - 1}, // ������ �ٸ� �ʱ�ȭ
            .velocity = 0.0f,            // �ӵ� �ʱ�ȭ
            .jumpVelocity = 0.0f,        // ���� �ӵ� �ʱ�ȭ
            .LwalkFrame = 0,             // ���� �ȱ� ������ �ʱ�ȭ
            .RwalkFrame = 0,             // ������ �ȱ� ������ �ʱ�ȭ
            .health = 10,                // ü�� �ʱ�ȭ
            .RIGHT = 1,
            .attackPower = 5.0,         // ���ݷ� �ʱ�ȭ
        };
    }

    // �ܼ� Ŀ�� �����
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Ŀ�� �����
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    while (true)
    {
        // ī�޶� ������ ����
        cameraOffset = player.head.x - P_MAP_WIDTH / 2; // �÷��̾ ȭ�� �߾ӿ� ������ ����

        // ī�޶� �������� ��� ����
        if (cameraOffset < 0)
            cameraOffset = 0; // ���� ���
        if (cameraOffset > MAP_WIDTH - P_MAP_WIDTH) // ������ ��� ����
            cameraOffset = MAP_WIDTH - P_MAP_WIDTH; // ������ ���

        initializeHitbox(&player);
        initialize_monsterHitbox(&monsters);
        initializeMap(map, player, monsters, cameraOffset); // �� �ʱ�ȭ
        printMap(player, map); // �� ���
        handleJumpAndGravity(&player);

       
       moveCharacter(&player);
            
        
        Sleep(10); // CPU ��뷮�� ���̱� ���� ��� ���
    }
       


        

    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printMap(Object player, char map[MAP_HEIGHT][MAP_WIDTH])
{
    // ȭ�� �����
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    int x, y, i, j, k;

    // ��ü �� ���
    for (i = 0; i < P_MAP_HEIGHT; i++)
    {
        for (j = 0; j < P_MAP_WIDTH; j++)
        {
            if (i == player.hitbox[1].y && j == player.hitbox[1].x)
            {
                printf(RED_BACKGROUND"%c"RESET_COLOR, map[player.hitbox[1].y][player.hitbox[1].x]);
                
            }
            else
            // �⺻ �� ���� ���
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
    //��Ʈ�ڽ� �����
    printHitbox(player);
    printMonsterHitboxes(monsters);
}

void initializeHitbox(Object* player) {
    // ��Ʈ�ڽ��� ��ġ�� ũ�⸦ ����
    player->hitbox[0].x = player->body.x - 1; // ���� ���� x��ǥ
    player->hitbox[0].y = player->body.y - 1; // ���� ���� y��ǥ
    player->hitbox[0].width = 3; // ��Ʈ�ڽ��� �ʺ� 
    player->hitbox[0].height = 3; // ��Ʈ�ڽ��� ����

    player->hitbox[1].x = player->body.x; // ���� ���� x��ǥ
    player->hitbox[1].y = player->body.y; // ���� ���� y��ǥ
    player->hitbox[1].width = 3; // ��Ʈ�ڽ��� �ʺ� 
    player->hitbox[1].height = 3; // ��Ʈ�ڽ��� ����
}

void initialize_monsterHitbox(Object* monsters)
{
    int i;
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        // ��Ʈ�ڽ��� ��ġ�� ũ�⸦ ����
        monsters[i].hitbox[0].x = monsters[i].body.x - 1; // ���� ���� x��ǥ
        monsters[i].hitbox[0].y = monsters[i].body.y - 1; // ���� ���� y��ǥ
        monsters[i].hitbox[0].width = 3; // ��Ʈ�ڽ��� �ʺ�
        monsters[i].hitbox[0].height = 2; // ��Ʈ�ڽ��� ����
    }
}

void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Object player, int cameraOffset)
{
    // �÷��̾��� �Ӹ�, ����, �ٸ� ��ġ�� ���
    int headX = player.head.x - cameraOffset;
    int headY = player.head.y;
    int bodyX = player.body.x - cameraOffset;
    int bodyY = player.body.y;
    int rightLegX = player.rightLeg.x - cameraOffset;
    int rightLegY = player.rightLeg.y;
    int leftLegX = player.leftLeg.x - cameraOffset;
    int leftLegY = player.leftLeg.y;
    

    // �÷��̾��� ��ġ�� map �迭�� ����
    map[headY][headX] = 'O'; // �Ӹ�
    map[bodyY][bodyX] = 'T'; // ����

    // DŰ ���� �� �ִϸ��̼�
    if (player.RwalkFrame == 0 && player.LwalkFrame == 0)
    {
        map[rightLegY][rightLegX] = '|';
        map[leftLegY][leftLegX] = '|';
    }
    else if (player.RwalkFrame == 1 && player.LwalkFrame == 0)
    {
        map[rightLegY][rightLegX] = '/';
        map[leftLegY][leftLegX] = '|';
    }
    else if (player.RwalkFrame == 2 && player.LwalkFrame == 0)
    {
        map[rightLegY][rightLegX] = '|';
        map[leftLegY][leftLegX] = '|';
    }
    else if (player.RwalkFrame == 3 && player.LwalkFrame == 0)
    {
        map[rightLegY][rightLegX] = '|';
        map[leftLegY][leftLegX] = '/';
    }

    // AŰ ���� �� �ִϸ��̼�
    if (player.LwalkFrame == 0 && player.RwalkFrame == 0)
    {
        map[leftLegY][leftLegX] = '|';
        map[rightLegY][rightLegX] = '|';
    }
    else if (player.LwalkFrame == 1 && player.RwalkFrame == 0)
    {
        map[leftLegY][leftLegX] = 'L';
        map[rightLegY][rightLegX] = '|';
    }
    else if (player.LwalkFrame == 2 && player.RwalkFrame == 0)
    {
        map[leftLegY][leftLegX] = '|';
        map[rightLegY][rightLegX] = '|';
    }
    else if (player.LwalkFrame == 3 && player.RwalkFrame == 0)
    {
        map[leftLegY][leftLegX] = '|';
        map[rightLegY][rightLegX] = 'L';
    }

    
    
}

void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Object monsters[], int cameraOffset)
{

    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        // ������ ��ġ�� ��ȿ���� Ȯ��

        // ������ y��ǥ�� ���� �ٴڿ� �ְ�, x��ǥ�� ��ȿ�� ���� ���� �ִ��� Ȯ��
        if (monsters[i].body.y < MAP_HEIGHT && monsters[i].body.y >= 0 && monsters[i].body.x >= 0 && monsters[i].body.x < P_MAP_WIDTH)
        {
            map[monsters[i].body.y][monsters[i].body.x - cameraOffset] = 'A'; // ���� ��ġ
            map[monsters[i].head.y][monsters[i].head.x - cameraOffset] = 'M'; // ���� ��ġ
            map[monsters[i].leftLeg.y][monsters[i].leftLeg.x - cameraOffset] = '&'; // ���� ��ġ
            map[monsters[i].rightLeg.y][monsters[i].rightLeg.x - cameraOffset] = '&'; // ���� ��ġ

        }
    }

}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Object player, Object monsters[], int cameraOffset)
{
    // �� �ʱ�ȭ
    int i, j;
    for (i = 0; i < MAP_HEIGHT; i++)
    {
        for (j = 0; j < MAP_WIDTH; j++)
        {
            if (i == MAP_HEIGHT - 1)
            {
                map[i][j] = '_'; // �ٴ�
            }
            else
            {
                map[i][j] = '.'; // �� ����
            }
        }
    }

    // �÷��̾�� ���� �ʱ�ȭ
    initializePlayer(map, player, cameraOffset);
    initializeMonster(map, monsters, cameraOffset);


// �浹 üũ
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (checkCollision(&player, &monsters[i], i))
        {
            char str[10]; // ���� �� �Է�
            printf("�浹 �߻�! ���� ����.\n");

            // ����� �Է��� �ޱ� ���� �ڵ� �߰�
            printf("���� ���� �ƹ� Ű�� ��������: ");
            fgets(str, sizeof(str), stdin); // ����� �Է� �ޱ�

            exit(0); // ���� ����
        }
    }
}

void handleJumpAndGravity(Object* player)
{
    // �߷� ����
    if (player->leftLeg.y < MAP_HEIGHT - 1) // ���� �ٸ��� �ٴڿ� ���� �ʾ��� ���� �߷� ����
    {
        player->velocity += 0.8; // �߷����� ���� �ӵ� ����
        player->head.y += player->velocity; // �ӵ��� ���� y��ǥ ����
        player->body.y += player->velocity; // ���� y��ǥ ����
        player->leftLeg.y += player->velocity; // ���� �ٸ� y��ǥ ����
        player->rightLeg.y += player->velocity; // ������ �ٸ� y��ǥ ����

        // �ٴڿ� ������
        if (player->leftLeg.y >= MAP_HEIGHT - 1)
        {
            // ���� ��ġ�� ����
            player->head.y = MAP_HEIGHT - 3; // �ٴ� ��ġ�� ����
            player->body.y = MAP_HEIGHT - 2; // �ٴ� ��ġ�� ����
            player->leftLeg.y = MAP_HEIGHT - 1; // �ٴ� ��ġ�� ����
            player->rightLeg.y = MAP_HEIGHT - 1; // �ٴ� ��ġ�� ����

            // �ӵ� �ʱ�ȭ
            player->velocity = 0;
            player->jumpState = false; // ���� ���� �ʱ�ȭ
        }
    }

    // ���� ó��
    if (player->jumpState)
    {
        player->head.y += player->jumpVelocity; // ���� �ӵ��� ���� y��ǥ ����
        player->body.y += player->jumpVelocity; // ���� y��ǥ ����
        player->leftLeg.y += player->jumpVelocity; // ���� �ٸ� y��ǥ ����
        player->rightLeg.y += player->jumpVelocity; // ������ �ٸ� y��ǥ ����
        player->jumpVelocity += 1; // ���� �ӵ� ���� (�߷� ȿ��)

        // �ٴڿ� ������
        if (player->leftLeg.y >= MAP_HEIGHT - 1)
        {
            player->head.y = MAP_HEIGHT - 3; // �ٴ� ��ġ�� ����
            player->body.y = MAP_HEIGHT - 2; // �ٴ� ��ġ�� ����
            player->leftLeg.y = MAP_HEIGHT - 1; // �ٴ� ��ġ�� ����
            player->rightLeg.y = MAP_HEIGHT - 1; // �ٴ� ��ġ�� ����
            player->jumpVelocity = 0; // ���� �ӵ� �ʱ�ȭ
            player->jumpState = false; // ���� ���� �ʱ�ȭ
        }
    }
}

void moveCharacter(Object* player)
{
    // ���� ���� ������Ʈ
    if (player->jumpState)
    {
        player->head.y += player->jumpVelocity; // y��ǥ ������Ʈ
        player->body.y += player->jumpVelocity;
        player->leftLeg.y += player->jumpVelocity;
        player->rightLeg.y += player->jumpVelocity;

        // �߷� ����
        player->jumpVelocity += 0.1; // �߷� ȿ��
        if (player->head.y >= MAP_HEIGHT - 1)
        { // �ٴڿ� ������
            player->head.y = MAP_HEIGHT - 1; // �ٴڿ� ����
            player->body.y = MAP_HEIGHT - 1;
            player->leftLeg.y = MAP_HEIGHT - 1;
            player->rightLeg.y = MAP_HEIGHT - 1;
            player->jumpState = false; // ���� ���� ����
            player->jumpVelocity = 0; // ���� �ӵ� �ʱ�ȭ
        }
    }

    // �̵� ���⿡ ���� x��ǥ ����
    if (GetAsyncKeyState('A') & 0x8000) // �������� �̵�
    {
        if (player->head.x > 0)
        {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->RwalkFrame = 0;
            player->RIGHT = 0;
        }
    }
    if (GetAsyncKeyState('D') & 0x8000) // ���������� �̵�
    {
        if (player->head.x < MAP_WIDTH - 1)
        {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->LwalkFrame = 0;
            player->RIGHT = 1;
        }
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) // �����̽��ٷ� ����
    {
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1)
        { // �ٴڿ� ���� ���� ���� ����
            player->jumpVelocity = -3; // ���� �ӵ��� �ٿ��� �ִ� ���̸� ����
            player->jumpState = true; // ���� ���� ����
        }
    }
    if (GetAsyncKeyState('K') & 0x8000) // ����
    {
        int i;
        player->attackState = true;
        if (player->attackState == true) // ���� ���°� true�� ���� ����
        {
            for (i = 0; i < ATTACK_RANGE; i++)
            {
                if (player->RIGHT == 1)
                {
                    player->hitbox[1].x = player->body.x + i + 2 - cameraOffset; // hitbox[1]�� ����
                    player->hitbox[1].y = player->body.y;

                    map[player->body.y][player->body.x + i + 1 - cameraOffset] = map[player->body.y][player->body.x + i + 2 - cameraOffset];
                    if (i % 2 == 0)
                        map[player->body.y][player->body.x + i + 2 - cameraOffset] = 'o'; // �ٴ�
                    else
                        map[player->body.y][player->body.x + i + 2 - cameraOffset] = 'O'; // �ٴ�
                    printMap(*player, map);
                    Sleep(10);
                }
                else if (player->RIGHT == 0)
                {
                    player->hitbox[1].x = player->body.x - i - 2 - cameraOffset; // hitbox[1]�� ����
                    player->hitbox[1].y = player->body.y;
                    map[player->body.y][player->body.x - i - 1 - cameraOffset] = map[player->body.y][player->body.x - i - 2 - cameraOffset];
                    if (i % 2 == 0)
                        map[player->body.y][player->body.x - i - 2 - cameraOffset] = 'o'; // �ٴ�
                    else
                        map[player->body.y][player->body.x - i - 2 - cameraOffset] = 'O'; // �ٴ�
                    printMap(*player, map);
                    Sleep(10);
                }
            }
        }
        player->attackState = false;
    }
}

bool checkCollision(Object* player, Object* mosnters[], int i)
{
    Hitbox hitboxA0 = player->hitbox[0]; // �迭�� ù ��° ��Ʈ�ڽ�
    Hitbox hitboxA1 = player->hitbox[1]; // �迭�� �� ��° ��Ʈ�ڽ�
    Hitbox b = monsters[i].hitbox[0]; // �迭�� ù ��° ��Ʈ�ڽ�

    // a[0]�� ��ȿ�� ���
    if (hitboxA0.width > 0 && hitboxA0.height > 0) // hitboxA0�� ��ȿ���� Ȯ��
    {
        if (hitboxA0.x < b.x + b.width &&
            hitboxA0.x + hitboxA0.width > b.x &&
            hitboxA0.y < b.y + b.height &&
            hitboxA0.y + hitboxA0.height > b.y)
        {
            monsters[i].health = monsters[i].health - player->attackPower;
            return true; // �浹 �߻�
        }
    }

    // a[1]�� ��ȿ�� ���
    if (hitboxA1.width > 0 && hitboxA1.height > 0) // hitboxA1�� ��ȿ���� Ȯ��
    {
        if (hitboxA1.x < b.x + b.width &&
            hitboxA1.x + hitboxA1.width > b.x &&
            hitboxA1.y < b.y + b.height &&
            hitboxA1.y + hitboxA1.height > b.y)
        {
            monsters[i].health = monsters[i].health - player->attackPower;
            return true; // �浹 �߻�
        }
    }

    // �浹�� ������ false ��ȯ
    return false;
}


