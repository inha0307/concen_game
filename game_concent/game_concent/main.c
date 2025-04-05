#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>

#define MAP_WIDTH 120
#define MAP_HEIGHT 10
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

Character player = { false, {2, 0}, {2, 1}, {1, 2}, {3, 2}, 0, 0, 0, 0 }; // 초기화

// 맵 출력 함수
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

// 맵 초기화 함수
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Tree trees[], int treeCount, int cameraOffset)
{
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            int adjustedX = j + cameraOffset; // 카메라 오프셋 적용
            if (i == player.head.y && adjustedX == player.head.x) {
                // 머리 출력
                map[i][j] = 'o';
            }
            else if (i == player.body.y && adjustedX == player.body.x) {
                // 몸통 출력
                map[i][j] = '\\';
            }
            else if (i == player.leftLeg.y && adjustedX == player.leftLeg.x) {
                // 왼쪽 다리 출력
                if (player.LwalkFrame == 0)
                {
                    map[i][j] = '|'; //왼쪽다리
                    map[i+1][j] = '|'; //오른쪽다리
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
                    map[i][j] = '|'; //왼쪽다리
                    map[i + 1][j] = '|'; //오른쪽다리
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
                map[i][j] = '_'; // 바닥
            }
            else {
                map[i][j] = '.'; // 빈 공간
            }
        }
    }

    // 나무 추가
    for (int i = 0; i < treeCount; i++) {
        if (trees[i].y < MAP_HEIGHT && trees[i].x >= cameraOffset && trees[i].x < cameraOffset + MAP_WIDTH) {
            map[trees[i].y][trees[i].x - cameraOffset] = 'T'; // 나무 위치
        }
    }
}

// 점프 및 중력 처리 함수
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
        player->jumpVelocity += 0.5; // 점프 속도 감소 (중력 효과)

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
            player->head.x--;
            player->body.x--;
            player->leftLeg.x--;
            player->rightLeg.x--;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // 걷기 프레임 업데이트
        }
        break;
    case 'd': // 오른쪽으로 이동
        if (player->head.x < MAP_WIDTH - 1) {
            player->head.x++;
            player->body.x++;
            player->leftLeg.x++;
            player->rightLeg.x++;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // 걷기 프레임 업데이트
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

int main() {
   
    char map[MAP_HEIGHT][MAP_WIDTH]; // 맵 버퍼
    char input;

    // 나무 배열 초기화
    Tree trees[TREE_COUNT] = {
        {10, 6}, {20, 5}, {30, 4}, {40, 3}, {50, 2}
    };

    // 콘솔 커서 숨기기
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // 커서 숨기기
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    int cameraOffset = 0; // 카메라 오프셋 초기화

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

        Sleep(100); // CPU 사용량을 줄이기 위해 잠시 대기
    }

    return 0;
}
