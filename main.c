#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>

#define MAP_WIDTH 120
#define MAP_HEIGHT 10
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

Character player = { false, {2, 0}, {2, 1}, {1, 2}, {3, 2}, 0, 0, 0, 0 }; // �ʱ�ȭ

// �� ��� �Լ�
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

// �� �ʱ�ȭ �Լ�
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset)
{
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            int adjustedX = j + cameraOffset; // ī�޶� ������ ����
            if (i == player.head.y && adjustedX == player.head.x) {
                // �Ӹ� ���
                map[i][j] = 'o';
            }
            else if (i == player.body.y && adjustedX == player.body.x) {
                // ���� ���
                map[i][j] = '\\';
            }
            else if (i == player.leftLeg.y && adjustedX == player.leftLeg.x) {
                // ���� �ٸ� ���
                if (player.LwalkFrame == 0)
                {
                    map[i][j] = '|'; //���ʴٸ�
                    map[i+1][j] = '|'; //�����ʴٸ�
                }
                else if (player.LwalkFrame == 1)
                {
                    map[i][j] = 'L';
                    map[i + 1][j] = '|';
                }
                else if (player.LwalkFrame == 2)
                {
                    map[i][j] = '|';
                    map[i + 1][j] = '|';
                }
                else if (player.LwalkFrame == 3)
                {
                    map[i][j] = 'L';
                    map[i + 1][j+1] = 'L';
                }
            }
            else if (i == player.rightLeg.y && adjustedX == player.rightLeg.x) 
            {
                if (player.RwalkFrame == 0)
                {
                    map[i][j] = '|'; //���ʴٸ�
                    map[i + 1][j] = '|'; //�����ʴٸ�
                }
                else if (player.RwalkFrame == 1)
                {
                    map[i][j] = '/';
                    map[i - 1][j] = '|';
                }
                else if (player.RwalkFrame == 2)
                {
                    map[i][j] = '|';
                    map[i + 1][j] = '|';
                }
                else if (player.RwalkFrame == 3)
                {
                    map[i][j] = '|';
                    map[i + 1][j] = '/';
                }
            }
            
            else if (i == MAP_HEIGHT - 1) {
                map[i][j] = '_'; // �ٴ�
            }
            else {
                map[i][j] = '.'; // �� ����
            }
        }
    }

    // ���� �߰�
    for (int i = 0; i < treeCount; i++) {
        if (trees[i].y < MAP_HEIGHT && trees[i].x >= cameraOffset && trees[i].x < cameraOffset + MAP_WIDTH) {
            map[trees[i].y][trees[i].x - cameraOffset] = 'T'; // ���� ��ġ
        }
    }
}

// ���� �� �߷� ó�� �Լ�
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
        player->jumpVelocity += 0.5; // ���� �ӵ� ���� (�߷� ȿ��)

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
            player->head.x--;
            player->body.x--;
            player->leftLeg.x--;
            player->rightLeg.x--;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
        }
        break;
    case 'd': // ���������� �̵�
        if (player->head.x < MAP_WIDTH - 1) {
            player->head.x++;
            player->body.x++;
            player->leftLeg.x++;
            player->rightLeg.x++;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // �ȱ� ������ ������Ʈ
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

int main() {
   
    char map[MAP_HEIGHT][MAP_WIDTH]; // �� ����
    char input;

    // ���� �迭 �ʱ�ȭ
    Tree trees[TREE_COUNT] = {
        {10, 6}, {20, 5}, {30, 4}, {40, 3}, {50, 2}
    };

    // �ܼ� Ŀ�� �����
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // Ŀ�� �����
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    int cameraOffset = 0; // ī�޶� ������ �ʱ�ȭ

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

        Sleep(100); // CPU ��뷮�� ���̱� ���� ��� ���
    }

    return 0;
}
