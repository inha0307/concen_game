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
    BodyPart head; // �Ӹ�
    BodyPart body; // ����
    BodyPart leftLeg; // ���� �ٸ�
    BodyPart rightLeg; // ������ �ٸ�
    float velocity; // �ӵ�
    float jumpVelocity; // ���� �ӵ�
    int LwalkFrame; // �ȱ� ������
    int RwalkFrame; // �ȱ� ������
    int health; // ĳ������ ü��
    float attackPower; // ĳ������ ���ݷ�
    Hitbox hitbox; // ��Ʈ�ڽ�
} Character;

// ���� ����ü ����
typedef struct
{
    bool jumpState; // ���� ����
    bool isAlive; // ������ ���� ����
    BodyPart head; // �Ӹ�
    BodyPart body; // ����
    BodyPart leftLeg; // ���� �ٸ�
    BodyPart rightLeg; // ������ �ٸ�
    float velocity; // �ӵ�
    float jumpVelocity; // ���� �ӵ�
    int LwalkFrame; // �ȱ� ������
    int RwalkFrame; // �ȱ� ������
    int health; // ������ ü��
    float attackPower; // ������ ���ݷ�
    Hitbox hitbox; // ��Ʈ�ڽ�
} Monster;

// �÷��̾� �迭 �ʱ�ȭ
Character player = { false, true, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0, 10, 5 }; // �ʱ�ȭ

// ���� �迭 ����
Monster monsters[MONSTER_COUNT];

void gotoxy(int x, int y); //Ŀ�� �̵� �Լ�
void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH]);// �� ��� �Լ�
void initializeHitbox(Character* player); // ĳ���� ��Ʈ�ڽ�
void initialize_monsterHitbox(Monster* mosnters); // ���� ��Ʈ�ڽ�
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Character player, int cameraOffset);// �÷��̾� �ʱ�ȭ �Լ�(��ġ)
void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Monster mosnters[], int cameraOffset);// ���� �ʱ�ȭ �Լ�(��ġ)
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Monster mosnters[], int cameraOffset);// �� �ʱ�ȭ �Լ�
void handleJumpAndGravity(Character* player);// ���� �� �߷� ó�� �Լ�
void moveCharacter(Character* player, char input); //ĳ���� �̵� �Լ�
bool checkCollision(Hitbox a, Hitbox b); //�浹 ���� �Լ�

// ĳ���� ��Ʈ�ڽ� ����� �Լ�
void printHitbox(Character player)
{
    printf("�÷��̾� ��Ʈ�ڽ�: x=%d, y=%d, width=%d, height=%d\n",
        player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height);
}

// ���� ��Ʈ�ڽ� ����� �Լ�
void printMonsterHitboxes(Monster monsters[])
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        printf("���� %d ��Ʈ�ڽ�: x=%d, y=%d, width=%d, height=%d\n",
            i, monsters[i].hitbox.x, monsters[i].hitbox.y,
            monsters[i].hitbox.width, monsters[i].hitbox.height);
    }
}

int main() {

    char map[MAP_HEIGHT][MAP_WIDTH]; // �� ����
    char input;
    int i;
    int cameraOffset = 0; // ī�޶� ������ �ʱ�ȭ
    // ���� �õ� ����
    srand(time(NULL));
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        // ���� �ʱ�ȭ
        monsters[i] = (Monster)
        {
            .jumpState = false,          // ���� ���� �ʱ�ȭ
            .isAlive = true,             // ���� ���� �ʱ�ȭ
            .head = (BodyPart){5, 0},    // �Ӹ� �ʱ�ȭ
            .body = (BodyPart){rand() % MAP_WIDTH + cameraOffset, MAP_HEIGHT - 1},    // ���� �ʱ�ȭ
            .leftLeg = (BodyPart){4, 2},  // ���� �ٸ� �ʱ�ȭ
            .rightLeg = (BodyPart){6, 2}, // ������ �ٸ� �ʱ�ȭ
            .velocity = 0.0f,            // �ӵ� �ʱ�ȭ
            .jumpVelocity = 0.0f,        // ���� �ӵ� �ʱ�ȭ
            .LwalkFrame = 0,             // ���� �ȱ� ������ �ʱ�ȭ
            .RwalkFrame = 0,             // ������ �ȱ� ������ �ʱ�ȭ
            .health = 10,                // ü�� �ʱ�ȭ
            .attackPower = 5.0f,         // ���ݷ� �ʱ�ȭ
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
        initializeHitbox(&player);
        initialize_monsterHitbox(&monsters);
        // ī�޶� ������ ����
        cameraOffset = player.head.x - P_MAP_WIDTH / 2; // �÷��̾ ȭ�� �߾ӿ� ������ ����

        // ī�޶� �������� ��� ����
        if (cameraOffset < 0)
            cameraOffset = 0; // ���� ���
        if (cameraOffset > MAP_WIDTH - P_MAP_WIDTH) // ������ ��� ����
            cameraOffset = MAP_WIDTH - P_MAP_WIDTH; // ������ ���

        initializeMap(map, player, monsters, cameraOffset); // �� �ʱ�ȭ
        printMap(player, map); // �� ���
        handleJumpAndGravity(&player);

        // Ű �Է��� �ִ��� Ȯ��
        if (_kbhit())
        {
            input = _getch(); // Ű �Է� �ޱ�
            moveCharacter(&player, input);
        }

        Sleep(5); // CPU ��뷮�� ���̱� ���� ��� ���
    }
    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH])
{
    // ȭ�� �����
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    int x, y, i, j;
    // ��ü �� ���
    for (i = 0; i < P_MAP_HEIGHT; i++)
    {
        for (j = 0; j < P_MAP_WIDTH; j++)
        {
            // �⺻ �� ���� ���
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
    //��Ʈ�ڽ� �����
    printHitbox(player);
    printMonsterHitboxes(monsters);
}

void initializeHitbox(Character* player) {
    // ��Ʈ�ڽ��� ��ġ�� ũ�⸦ ����
    player->hitbox.x = player->body.x - 1; // ���� ���� x��ǥ
    player->hitbox.y = player->body.y - 1; // ���� ���� y��ǥ
    player->hitbox.width = 3; // ��Ʈ�ڽ��� �ʺ� 
    player->hitbox.height = 3; // ��Ʈ�ڽ��� ����
}

void initialize_monsterHitbox(Monster* monsters)
{
    int i;
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        // ��Ʈ�ڽ��� ��ġ�� ũ�⸦ ����
        monsters[i].hitbox.x = monsters[i].body.x; // ���� ���� x��ǥ
        monsters[i].hitbox.y = monsters[i].body.y; // ���� ���� y��ǥ
        monsters[i].hitbox.width = 1; // ��Ʈ�ڽ��� �ʺ�
        monsters[i].hitbox.height = 1; // ��Ʈ�ڽ��� ����
    }
}

void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Character player, int cameraOffset)
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

void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Monster monsters[], int cameraOffset)
{

    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        // ������ ��ġ�� ��ȿ���� Ȯ��
        int monsterX = monsters[i].body.x - cameraOffset; // ī�޶� �������� ����� ������ x��ǥ
        int monsterY = monsters[i].body.y;

        // ������ y��ǥ�� ���� �ٴڿ� �ְ�, x��ǥ�� ��ȿ�� ���� ���� �ִ��� Ȯ��
        if (monsterY < MAP_HEIGHT && monsterY >= 0 && monsterX >= 0 && monsterX < P_MAP_WIDTH)
        {
            map[monsterY][monsterX] = 'M'; // ���� ��ġ
        }
    }

}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Monster monsters[], int cameraOffset)
{
    // �� �ʱ�ȭ
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
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
// �浹 üũ
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (checkCollision(player.hitbox, monsters[i].hitbox))
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

void handleJumpAndGravity(Character* player)
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

void moveCharacter(Character* player, char input)
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
    switch (input)
    {
    case 'a': // �������� �̵�
        if (player->head.x > 0)
        {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->RwalkFrame = 0;
        }
        break;
    case 'd': // ���������� �̵�
        if (player->head.x < MAP_WIDTH - 1)
        {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->LwalkFrame = 0;
        }
        break;
    case ' ': // �����̽��ٷ� ����
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1)
        { // �ٴڿ� ���� ���� ���� ����
            player->jumpVelocity = -3; // ���� �ӵ��� �ٿ��� �ִ� ���̸� ����
            player->jumpState = true; // ���� ���� ����
        }
        break;
    default:
        break;
    }
}

bool checkCollision(Hitbox a, Hitbox b)
{
    // �浹�ϸ� true, �ƴϸ� false
    return (a.x < b.x + b.width &&
        a.x + a.width > b.x &&
        a.y < b.y + b.height &&
        a.y + a.height > b.y);
}

