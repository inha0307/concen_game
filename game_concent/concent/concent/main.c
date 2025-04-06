#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <Windows.h>
#include <stdbool.h>
#include <conio.h>
#include <stdlib.h> 
#include <time.h>  

#define MAP_WIDTH 200 //맵 길이
#define MAP_HEIGHT 20 //맵 높이
#define P_MAP_WIDTH 120 //출력 화면 길이
#define P_MAP_HEIGHT 20 // 출력 화면 높이
#define MONSTER_COUNT 5 // 몬스터 갯수
#define RED_BACKGROUND "\033[41m" // 붉은 배경
#define RESET_COLOR "\033[0m" // 색상 초기화


// 캐릭터의 각 부위를 나타내는 구조체
typedef struct
{
    int x; // x좌표
    int y; // y좌표
} BodyPart;

typedef struct
{
    int x; // 히트박스의 x좌표
    int y; // 히트박스의 y좌표
    int width; // 히트박스의 너비
    int height; // 히트박스의 높이
} Hitbox;

// 캐릭터 구조체 정의
typedef struct
{
    bool jumpState; // 점프 상태
    bool isAlive; // 캐릭터의 생존 상태
    BodyPart head; // 머리
    BodyPart body; // 몸통
    BodyPart leftLeg; // 왼쪽 다리
    BodyPart rightLeg; // 오른쪽 다리
    float velocity; // 속도
    float jumpVelocity; // 점프 속도
    int LwalkFrame; // 걷기 프레임
    int RwalkFrame; // 걷기 프레임
    int health; // 캐릭터의 체력
    float attackPower; // 캐릭터의 공격력
    Hitbox hitbox; // 히트박스
} Character;

// 몬스터 구조체 정의
typedef struct
{
    bool jumpState; // 점프 상태
    bool isAlive; // 몬스터의 생존 상태
    BodyPart head; // 머리
    BodyPart body; // 몸통
    BodyPart leftLeg; // 왼쪽 다리
    BodyPart rightLeg; // 오른쪽 다리
    float velocity; // 속도
    float jumpVelocity; // 점프 속도
    int LwalkFrame; // 걷기 프레임
    int RwalkFrame; // 걷기 프레임
    int health; // 몬스터의 체력
    float attackPower; // 몬스터의 공격력
    Hitbox hitbox; // 히트박스
} Monster;

// 플레이어 배열 초기화
Character player = { false, true, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0, 10, 5 }; // 초기화

// 몬스터 배열 선언
Monster monsters[MONSTER_COUNT];

void gotoxy(int x, int y); //커서 이동 함수
void printMap(Character player, char map[MAP_HEIGHT][MAP_WIDTH]);// 맵 출력 함수
void initializeHitbox(Character* player); // 캐릭터 히트박스
void initialize_monsterHitbox(Monster* mosnters); // 몬스터 히트박스
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Character player, int cameraOffset);// 플레이어 초기화 함수(위치)
void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Monster mosnters[], int cameraOffset);// 몬스터 초기화 함수(위치)
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Monster mosnters[], int cameraOffset);// 맵 초기화 함수
void handleJumpAndGravity(Character* player);// 점프 및 중력 처리 함수
void moveCharacter(Character* player, char input); //캐릭터 이동 함수
bool checkCollision(Hitbox a, Hitbox b); //충돌 감지 함수

// 캐릭터 히트박스 디버깅 함수
void printHitbox(Character player)
{
    printf("플레이어 히트박스: x=%d, y=%d, width=%d, height=%d\n",
        player.hitbox.x, player.hitbox.y, player.hitbox.width, player.hitbox.height);
}

// 몬스터 히트박스 디버깅 함수
void printMonsterHitboxes(Monster monsters[])
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        printf("몬스터 %d 히트박스: x=%d, y=%d, width=%d, height=%d\n",
            i, monsters[i].hitbox.x, monsters[i].hitbox.y,
            monsters[i].hitbox.width, monsters[i].hitbox.height);
    }
}

int main() {

    char map[MAP_HEIGHT][MAP_WIDTH]; // 맵 버퍼
    char input;
    int i;
    int cameraOffset = 0; // 카메라 오프셋 초기화
    // 랜덤 시드 설정
    srand(time(NULL));
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        // 몬스터 초기화
        monsters[i] = (Monster)
        {
            .jumpState = false,          // 점프 상태 초기화
            .isAlive = true,             // 생존 상태 초기화
            .head = (BodyPart){5, 0},    // 머리 초기화
            .body = (BodyPart){rand() % MAP_WIDTH + cameraOffset, MAP_HEIGHT - 1},    // 몸통 초기화
            .leftLeg = (BodyPart){4, 2},  // 왼쪽 다리 초기화
            .rightLeg = (BodyPart){6, 2}, // 오른쪽 다리 초기화
            .velocity = 0.0f,            // 속도 초기화
            .jumpVelocity = 0.0f,        // 점프 속도 초기화
            .LwalkFrame = 0,             // 왼쪽 걷기 프레임 초기화
            .RwalkFrame = 0,             // 오른쪽 걷기 프레임 초기화
            .health = 10,                // 체력 초기화
            .attackPower = 5.0f,         // 공격력 초기화
        };
    }

    // 콘솔 커서 숨기기
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; // 커서 숨기기
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    while (true)
    {
        initializeHitbox(&player);
        initialize_monsterHitbox(&monsters);
        // 카메라 오프셋 조정
        cameraOffset = player.head.x - P_MAP_WIDTH / 2; // 플레이어가 화면 중앙에 오도록 조정

        // 카메라 오프셋의 경계 설정
        if (cameraOffset < 0)
            cameraOffset = 0; // 왼쪽 경계
        if (cameraOffset > MAP_WIDTH - P_MAP_WIDTH) // 오른쪽 경계 수정
            cameraOffset = MAP_WIDTH - P_MAP_WIDTH; // 오른쪽 경계

        initializeMap(map, player, monsters, cameraOffset); // 맵 초기화
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
    int x, y, i, j;
    // 전체 맵 출력
    for (i = 0; i < P_MAP_HEIGHT; i++)
    {
        for (j = 0; j < P_MAP_WIDTH; j++)
        {
            // 기본 맵 문자 출력
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
    //히트박스 디버깅
    printHitbox(player);
    printMonsterHitboxes(monsters);
}

void initializeHitbox(Character* player) {
    // 히트박스의 위치와 크기를 설정
    player->hitbox.x = player->body.x - 1; // 몸통 기준 x좌표
    player->hitbox.y = player->body.y - 1; // 몸통 기준 y좌표
    player->hitbox.width = 3; // 히트박스의 너비 
    player->hitbox.height = 3; // 히트박스의 높이
}

void initialize_monsterHitbox(Monster* monsters)
{
    int i;
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        // 히트박스의 위치와 크기를 설정
        monsters[i].hitbox.x = monsters[i].body.x; // 몸통 기준 x좌표
        monsters[i].hitbox.y = monsters[i].body.y; // 몸통 기준 y좌표
        monsters[i].hitbox.width = 1; // 히트박스의 너비
        monsters[i].hitbox.height = 1; // 히트박스의 높이
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
    map[headY][headX] = 'O'; // 머리
    map[bodyY][bodyX] = 'T'; // 몸통

    // D키 누른 후 애니메이션
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

    // A키 누른 후 애니메이션
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
        // 몬스터의 위치가 유효한지 확인
        int monsterX = monsters[i].body.x - cameraOffset; // 카메라 오프셋을 고려한 몬스터의 x좌표
        int monsterY = monsters[i].body.y;

        // 몬스터의 y좌표가 맵의 바닥에 있고, x좌표가 유효한 범위 내에 있는지 확인
        if (monsterY < MAP_HEIGHT && monsterY >= 0 && monsterX >= 0 && monsterX < P_MAP_WIDTH)
        {
            map[monsterY][monsterX] = 'M'; // 몬스터 위치
        }
    }

}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Character player, Monster monsters[], int cameraOffset)
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
    initializeMonster(map, monsters, cameraOffset);

    // 충돌 체크
// 충돌 체크
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (checkCollision(player.hitbox, monsters[i].hitbox))
        {
            char str[10]; // 종료 전 입력
            printf("충돌 발생! 게임 종료.\n");

            // 사용자 입력을 받기 위한 코드 추가
            printf("종료 전에 아무 키나 누르세요: ");
            fgets(str, sizeof(str), stdin); // 사용자 입력 받기

            exit(0); // 게임 종료
        }
    }
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

void moveCharacter(Character* player, char input)
{
    // 점프 상태 업데이트
    if (player->jumpState)
    {
        player->head.y += player->jumpVelocity; // y좌표 업데이트
        player->body.y += player->jumpVelocity;
        player->leftLeg.y += player->jumpVelocity;
        player->rightLeg.y += player->jumpVelocity;

        // 중력 적용
        player->jumpVelocity += 0.1; // 중력 효과
        if (player->head.y >= MAP_HEIGHT - 1)
        { // 바닥에 닿으면
            player->head.y = MAP_HEIGHT - 1; // 바닥에 고정
            player->body.y = MAP_HEIGHT - 1;
            player->leftLeg.y = MAP_HEIGHT - 1;
            player->rightLeg.y = MAP_HEIGHT - 1;
            player->jumpState = false; // 점프 상태 해제
            player->jumpVelocity = 0; // 점프 속도 초기화
        }
    }

    // 이동 방향에 따라 x좌표 변경
    switch (input)
    {
    case 'a': // 왼쪽으로 이동
        if (player->head.x > 0)
        {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->RwalkFrame = 0;
        }
        break;
    case 'd': // 오른쪽으로 이동
        if (player->head.x < MAP_WIDTH - 1)
        {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->LwalkFrame = 0;
        }
        break;
    case ' ': // 스페이스바로 점프
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1)
        { // 바닥에 있을 때만 점프 가능
            player->jumpVelocity = -3; // 점프 속도를 줄여서 최대 높이를 낮춤
            player->jumpState = true; // 점프 상태 설정
        }
        break;
    default:
        break;
    }
}

bool checkCollision(Hitbox a, Hitbox b)
{
    // 충돌하면 true, 아니면 false
    return (a.x < b.x + b.width &&
        a.x + a.width > b.x &&
        a.y < b.y + b.height &&
        a.y + a.height > b.y);
}

