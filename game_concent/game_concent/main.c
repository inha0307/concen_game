#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h> // rand()와 srand()를 사용하기 위해 필요
#include <time.h>   // time()을 사용하여 시드 설정

#define MAP_WIDTH 120
#define MAP_HEIGHT 20
#define TREE_COUNT 5 // 나무 개수

// 캐릭터의 각 부위를 나타내는 구조체
typedef struct {
    int x; // x좌표
    int y; // y좌표
} BodyPart;

// 캐릭터 구조체 정의
typedef struct {
    bool jumpState; // 점프 상태
    BodyPart head; // 머리
    BodyPart body; // 몸통
    BodyPart leftLeg; // 왼쪽 다리
    BodyPart rightLeg; // 오른쪽 다리
    float velocity; // 속도
    float jumpVelocity; // 점프 속도
    int LwalkFrame; // 걷기 프레임
    int RwalkFrame; // 걷기 프레임
} Character;

// 나무 구조체 정의
typedef struct {
    int x; // 나무의 x좌표
    int y; // 나무의 y좌표
} Tree;

Character player = { false, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0 }; // 초기화
// 나무 배열 초기화
Tree trees[TREE_COUNT];

void gotoxy(int x, int y);
void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH]);// 맵 출력 함수
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Character player, int cameraOffset);// 플레이어 초기화 함수
void initializeTrees(char map[MAP_HEIGHT][MAP_WIDTH], Tree trees[], int treeCount, int cameraOffset);// 나무 초기화 함수
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset);// 맵 초기화 함수
void handleJumpAndGravity(Character* player);// 점프 및 중력 처리 함수
void moveCharacter(Character* player, char input);

int main() {
   
    char map[MAP_HEIGHT][MAP_WIDTH]; // 맵 버퍼
    char input;
    int i;
    int cameraOffset = 0; // 카메라 오프셋 초기화
    // 랜덤 시드 설정
    srand(time(NULL));

    for (i = 0; i < TREE_COUNT; i++)
    {
        // 랜덤 x좌표 생성 (카메라 오프셋을 고려하여)
        trees[i].x = rand() % MAP_WIDTH + cameraOffset; // 0부터 MAP_WIDTH-1까지의 랜덤 값에 cameraOffset 추가
    }
    

    // 콘솔 커서 숨기기
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // 커서 숨기기
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    

    while (true)
    {
        // 카메라 오프셋 조정
        cameraOffset = player.head.x - MAP_WIDTH / 2; // 플레이어가 화면 중앙에 오도록 조정
        if (cameraOffset < 0) cameraOffset = 0; // 왼쪽 경계
        if (cameraOffset > MAP_WIDTH - 1) cameraOffset = MAP_WIDTH - 1; // 오른쪽 경계

        initializeMap(map, player, trees, TREE_COUNT, cameraOffset); // 맵 초기화
        printMap(player, map); // 맵 출력
        handleJumpAndGravity(&player);


        // 키 입력이 있는지 확인
        if (_kbhit())
        {
            input = _getch(); // 키 입력 받기
            moveCharacter(&player, input);
        }

        Sleep(5); // CPU 사용량을 줄이기 위해 잠시 대기
    }

    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH])
{
    // 화면 지우기
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
    // 플레이어의 머리, 몸통, 다리 위치를 계산
    int headX = player.head.x - cameraOffset;
    int headY = player.head.y;
    int bodyX = player.body.x - cameraOffset;
    int bodyY = player.body.y;
    int rightLegX = player.rightLeg.x - cameraOffset;
    int rightLegY = player.rightLeg.y;
    int leftLegX = player.leftLeg.x - cameraOffset;
    int leftLegY = player.leftLeg.y;

    // 플레이어의 위치를 map 배열에 설정
    map[headY][headX] = 'o'; // 머리
    map[bodyY][bodyX] = 'T'; // 몸통

    // 오른쪽 다리 출력
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

    // 왼쪽 다리 출력
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
       

        // 나무는 바닥에만 위치하므로 y좌표는 MAP_HEIGHT - 1로 설정
        int treeY = MAP_HEIGHT - 1;

        // 나무 위치가 유효한지 확인
        if (treeY < MAP_HEIGHT && trees[i].x >= cameraOffset && trees[i].x < cameraOffset + MAP_WIDTH)
        {
            map[treeY][trees[i].x - cameraOffset] = 'T'; // 나무 위치
        }
    }
}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset)
{
    // 맵 초기화
    for (int i = 0; i < MAP_HEIGHT; i++)
    {
        for (int j = 0; j < MAP_WIDTH; j++)
        {
            if (i == MAP_HEIGHT - 1)
            {
                map[i][j] = '_'; // 바닥
            }
            else
            {
                map[i][j] = '.'; // 빈 공간
            }
        }
    }

    // 플레이어와 나무 초기화
    initializePlayer(map, player, cameraOffset);
    initializeTrees(map, trees, treeCount, cameraOffset);
}

void handleJumpAndGravity(Character* player)
{
    // 중력 적용
    if (player->leftLeg.y < MAP_HEIGHT - 1) // 왼쪽 다리가 바닥에 닿지 않았을 때만 중력 적용
    {
        player->velocity += 0.8; // 중력으로 인해 속도 증가
        player->head.y += player->velocity; // 속도에 따라 y좌표 변경
        player->body.y += player->velocity; // 몸통 y좌표 변경
        player->leftLeg.y += player->velocity; // 왼쪽 다리 y좌표 변경
        player->rightLeg.y += player->velocity; // 오른쪽 다리 y좌표 변경

        // 바닥에 닿으면
        if (player->leftLeg.y >= MAP_HEIGHT - 1)
        {
            // 현재 위치에 고정
            player->head.y = MAP_HEIGHT - 3; // 바닥 위치로 고정
            player->body.y = MAP_HEIGHT - 2; // 바닥 위치로 고정
            player->leftLeg.y = MAP_HEIGHT - 1; // 바닥 위치로 고정
            player->rightLeg.y = MAP_HEIGHT - 1; // 바닥 위치로 고정

            // 속도 초기화
            player->velocity = 0;
            player->jumpState = false; // 점프 상태 초기화
        }
    }

    // 점프 처리
    if (player->jumpState)
    {
        player->head.y += player->jumpVelocity; // 점프 속도에 따라 y좌표 변경
        player->body.y += player->jumpVelocity; // 몸통 y좌표 변경
        player->leftLeg.y += player->jumpVelocity; // 왼쪽 다리 y좌표 변경
        player->rightLeg.y += player->jumpVelocity; // 오른쪽 다리 y좌표 변경
        player->jumpVelocity += 1; // 점프 속도 감소 (중력 효과)

        // 바닥에 닿으면
        if (player->leftLeg.y >= MAP_HEIGHT - 1)
        {
            player->head.y = MAP_HEIGHT - 3; // 바닥 위치로 고정
            player->body.y = MAP_HEIGHT - 2; // 바닥 위치로 고정
            player->leftLeg.y = MAP_HEIGHT - 1; // 바닥 위치로 고정
            player->rightLeg.y = MAP_HEIGHT - 1; // 바닥 위치로 고정
            player->jumpVelocity = 0; // 점프 속도 초기화
            player->jumpState = false; // 점프 상태 초기화
        }
    }
}

void moveCharacter(Character* player, char input) {
    // 점프 상태 업데이트
    if (player->jumpState) {
        player->head.y += player->jumpVelocity; // y좌표 업데이트
        player->body.y += player->jumpVelocity;
        player->leftLeg.y += player->jumpVelocity;
        player->rightLeg.y += player->jumpVelocity;

        // 중력 적용
        player->jumpVelocity += 0.1; // 중력 효과
        if (player->head.y >= MAP_HEIGHT - 1) { // 바닥에 닿으면
            player->head.y = MAP_HEIGHT - 1; // 바닥에 고정
            player->body.y = MAP_HEIGHT - 1;
            player->leftLeg.y = MAP_HEIGHT - 1;
            player->rightLeg.y = MAP_HEIGHT - 1;
            player->jumpState = false; // 점프 상태 해제
            player->jumpVelocity = 0; // 점프 속도 초기화
        }
    }

    // 이동 방향에 따라 x좌표 변경
    switch (input) {
    case 'a': // 왼쪽으로 이동
        if (player->head.x > 0) {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->RwalkFrame = 0;
        }
        break;
    case 'd': // 오른쪽으로 이동
        if (player->head.x < MAP_WIDTH - 1) {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->LwalkFrame = 0;
        }
        break;
    case ' ': // 스페이스바로 점프
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1) { // 바닥에 있을 때만 점프 가능
            player->jumpVelocity = -3; // 점프 속도를 줄여서 최대 높이를 낮춤
            player->jumpState = true; // 점프 상태 설정
        }
        break;
    default:
        break;
    }
}
