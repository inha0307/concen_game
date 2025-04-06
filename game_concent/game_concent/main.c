#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h> // rand()�� srand()�� ����ϱ� ���� �ʿ�
#include <time.h>   // time()�� ����Ͽ� �õ� ����

#define MAP_WIDTH 120
#define MAP_HEIGHT 20
#define TREE_COUNT 5 // ���� ����

// ĳ������ �� ������ ��Ÿ���� ����ü
typedef struct {
    int x; // x��ǥ
    int y; // y��ǥ
} BodyPart;

// ĳ���� ����ü ����
typedef struct {
    bool jumpState; // ���� ����
    BodyPart head; // �Ӹ�
    BodyPart body; // ����
    BodyPart leftLeg; // ���� �ٸ�
    BodyPart rightLeg; // ������ �ٸ�
    float velocity; // �ӵ�
    float jumpVelocity; // ���� �ӵ�
    int LwalkFrame; // �ȱ� ������
    int RwalkFrame; // �ȱ� ������
} Character;

// ���� ����ü ����
typedef struct {
    int x; // ������ x��ǥ
    int y; // ������ y��ǥ
} Tree;

Character player = { false, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0 }; // �ʱ�ȭ
// ���� �迭 �ʱ�ȭ
Tree trees[TREE_COUNT];

void gotoxy(int x, int y);
void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH]);// �� ��� �Լ�
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Character player, int cameraOffset);// �÷��̾� �ʱ�ȭ �Լ�
void initializeTrees(char map[MAP_HEIGHT][MAP_WIDTH], Tree trees[], int treeCount, int cameraOffset);// ���� �ʱ�ȭ �Լ�
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset);// �� �ʱ�ȭ �Լ�
void handleJumpAndGravity(Character* player);// ���� �� �߷� ó�� �Լ�
void moveCharacter(Character* player, char input);

int main() {
   
    char map[MAP_HEIGHT][MAP_WIDTH]; // �� ����
    char input;
    int i;
    int cameraOffset = 0; // ī�޶� ������ �ʱ�ȭ
    // ���� �õ� ����
    srand(time(NULL));

    for (i = 0; i < TREE_COUNT; i++)
    {
        // ���� x��ǥ ���� (ī�޶� �������� ����Ͽ�)
        trees[i].x = rand() % MAP_WIDTH + cameraOffset; // 0���� MAP_WIDTH-1������ ���� ���� cameraOffset �߰�
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
        cameraOffset = player.head.x - MAP_WIDTH / 2; // �÷��̾ ȭ�� �߾ӿ� ������ ����
        if (cameraOffset < 0) cameraOffset = 0; // ���� ���
        if (cameraOffset > MAP_WIDTH - 1) cameraOffset = MAP_WIDTH - 1; // ������ ���

        initializeMap(map, player, trees, TREE_COUNT, cameraOffset); // �� �ʱ�ȭ
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

    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            printf("%c", map[i][j]);
        }
        printf("\n");
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
    map[headY][headX] = 'o'; // �Ӹ�
    map[bodyY][bodyX] = 'T'; // ����

    // ������ �ٸ� ���
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

    // ���� �ٸ� ���
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

void initializeTrees(char map[MAP_HEIGHT][MAP_WIDTH], Tree trees[], int treeCount, int cameraOffset)
{

    for (int i = 0; i < treeCount; i++)
    {
       

        // ������ �ٴڿ��� ��ġ�ϹǷ� y��ǥ�� MAP_HEIGHT - 1�� ����
        int treeY = MAP_HEIGHT - 1;

        // ���� ��ġ�� ��ȿ���� Ȯ��
        if (treeY < MAP_HEIGHT && trees[i].x >= cameraOffset && trees[i].x < cameraOffset + MAP_WIDTH)
        {
            map[treeY][trees[i].x - cameraOffset] = 'T'; // ���� ��ġ
        }
    }
}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset)
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
    initializeTrees(map, trees, treeCount, cameraOffset);
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

void moveCharacter(Character* player, char input) {
    // ���� ���� ������Ʈ
    if (player->jumpState) {
        player->head.y += player->jumpVelocity; // y��ǥ ������Ʈ
        player->body.y += player->jumpVelocity;
        player->leftLeg.y += player->jumpVelocity;
        player->rightLeg.y += player->jumpVelocity;

        // �߷� ����
        player->jumpVelocity += 0.1; // �߷� ȿ��
        if (player->head.y >= MAP_HEIGHT - 1) { // �ٴڿ� ������
            player->head.y = MAP_HEIGHT - 1; // �ٴڿ� ����
            player->body.y = MAP_HEIGHT - 1;
            player->leftLeg.y = MAP_HEIGHT - 1;
            player->rightLeg.y = MAP_HEIGHT - 1;
            player->jumpState = false; // ���� ���� ����
            player->jumpVelocity = 0; // ���� �ӵ� �ʱ�ȭ
        }
    }

    // �̵� ���⿡ ���� x��ǥ ����
    switch (input) {
    case 'a': // �������� �̵�
        if (player->head.x > 0) {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->RwalkFrame = 0;
        }
        break;
    case 'd': // ���������� �̵�
        if (player->head.x < MAP_WIDTH - 1) {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
            player->LwalkFrame = 0;
        }
        break;
    case ' ': // �����̽��ٷ� ����
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1) { // �ٴڿ� ���� ���� ���� ����
            player->jumpVelocity = -3; // ���� �ӵ��� �ٿ��� �ִ� ���̸� ����
            player->jumpState = true; // ���� ���� ����
        }
        break;
    default:
        break;
    }
}
