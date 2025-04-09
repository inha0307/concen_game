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
#define ATTACK_RANGE 10 // 색상 초기화


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
    bool attackState; // 공격 상태
    BodyPart head; // 머리
    BodyPart body; // 몸통
    BodyPart leftLeg; // 왼쪽 다리
    BodyPart rightLeg; // 오른쪽 다리
    float velocity; // 속도
    float jumpVelocity; // 점프 속도
    int LwalkFrame; // 걷기 프레임
    int RwalkFrame; // 걷기 프레임
    int health; // 캐릭터의 체력
    int RIGHT; //RlGHT = 1, LEFT = 0
    int attackPower; // 캐릭터의 공격력
    Hitbox hitbox[2]; // 히트박스
} Object;


// 플레이어 배열 초기화
Object player = { false, true, false, {5, 0}, {5, 1}, {4, 2}, {6, 2}, 0, 0, 0, 0, 10, 1, 5 }; // 초기화

// 몬스터 배열 선언
Object monsters[MONSTER_COUNT];

char map[MAP_HEIGHT][MAP_WIDTH]; // 맵 버퍼

int cameraOffset = 0; // 카메라 오프셋 초기화

void gotoxy(int x, int y); //커서 이동 함수
void printMap(Object player, char map[MAP_HEIGHT][MAP_WIDTH]);// 맵 출력 함수
void initializeHitbox(Object* player); // 캐릭터 히트박스
void initialize_monsterHitbox(Object* mosnters); // 몬스터 히트박스
void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Object player, int cameraOffset);// 플레이어 초기화 함수(위치)
void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Object mosnters[], int cameraOffset);// 몬스터 초기화 함수(위치)
void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Object player, Object mosnters[], int cameraOffset);// 맵 초기화 함수
void handleJumpAndGravity(Object* player);// 점프 및 중력 처리 함수
void moveCharacter(Object* player); //캐릭터 이동 함수
bool checkCollision(Object* player, Object* mosnters[], int i); //충돌 감지 함수

// 캐릭터 히트박스 디버깅 함수
void printHitbox(Object player)
{
    printf("플레이어 히트박스: x=%d, y=%d, width=%d, height=%d, HP=%d, attack=%d, attackPower=%d\n",
        player.hitbox[0].x, player.hitbox[0].y, player.hitbox[0].width, player.hitbox[0].height, player.health, player.attackState, player.attackPower);
}

// 몬스터 히트박스 디버깅 함수
void printMonsterHitboxes(Object monsters[])
{
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        printf("몬스터 %d 히트박스: x=%d, y=%d, width=%d, height=%d,HP=%d, attack=%d\n",
            i, monsters[i].hitbox[0].x, monsters[i].hitbox[0].y,
            monsters[i].hitbox[0].width, monsters[i].hitbox[0].height, monsters[i].health, monsters[i].attackState);
    }
}

int main() {

    
    char input;
    int i;

    // 랜덤 시드 설정
    srand(time(NULL));
    
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        int monsterX = rand() % MAP_WIDTH + cameraOffset;
        int monsterY = MAP_HEIGHT;
        // 몬스터 초기화
        monsters[i] = (Object)
        {
            .jumpState = false,          // 점프 상태 초기화
            .isAlive = true,             // 생존 상태 초기화
            .attackState = false,
            .head = (BodyPart){monsterX - cameraOffset, monsterY -2},    // 머리 초기화
            .body = (BodyPart){monsterX - cameraOffset, monsterY - 1},    // 몸통 초기화
            .leftLeg = (BodyPart){monsterX - cameraOffset - 1, monsterY - 1},  // 왼쪽 다리 초기화
            .rightLeg = (BodyPart){monsterX - cameraOffset + 1, monsterY - 1}, // 오른쪽 다리 초기화
            .velocity = 0.0f,            // 속도 초기화
            .jumpVelocity = 0.0f,        // 점프 속도 초기화
            .LwalkFrame = 0,             // 왼쪽 걷기 프레임 초기화
            .RwalkFrame = 0,             // 오른쪽 걷기 프레임 초기화
            .health = 10,                // 체력 초기화
            .RIGHT = 1,
            .attackPower = 5.0,         // 공격력 초기화
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
        // 카메라 오프셋 조정
        cameraOffset = player.head.x - P_MAP_WIDTH / 2; // 플레이어가 화면 중앙에 오도록 조정

        // 카메라 오프셋의 경계 설정
        if (cameraOffset < 0)
            cameraOffset = 0; // 왼쪽 경계
        if (cameraOffset > MAP_WIDTH - P_MAP_WIDTH) // 오른쪽 경계 수정
            cameraOffset = MAP_WIDTH - P_MAP_WIDTH; // 오른쪽 경계

        initializeHitbox(&player);
        initialize_monsterHitbox(&monsters);
        initializeMap(map, player, monsters, cameraOffset); // 맵 초기화
        printMap(player, map); // 맵 출력
        handleJumpAndGravity(&player);

       
       moveCharacter(&player);
            
        
        Sleep(10); // CPU 사용량을 줄이기 위해 잠시 대기
    }
       


        

    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printMap(Object player, char map[MAP_HEIGHT][MAP_WIDTH])
{
    // 화면 지우기
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    int x, y, i, j, k;

    // 전체 맵 출력
    for (i = 0; i < P_MAP_HEIGHT; i++)
    {
        for (j = 0; j < P_MAP_WIDTH; j++)
        {
            if (i == player.hitbox[1].y && j == player.hitbox[1].x)
            {
                printf(RED_BACKGROUND"%c"RESET_COLOR, map[player.hitbox[1].y][player.hitbox[1].x]);
                
            }
            else
            // 기본 맵 문자 출력
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
    //히트박스 디버깅
    printHitbox(player);
    printMonsterHitboxes(monsters);
}

void initializeHitbox(Object* player) {
    // 히트박스의 위치와 크기를 설정
    player->hitbox[0].x = player->body.x - 1; // 몸통 기준 x좌표
    player->hitbox[0].y = player->body.y - 1; // 몸통 기준 y좌표
    player->hitbox[0].width = 3; // 히트박스의 너비 
    player->hitbox[0].height = 3; // 히트박스의 높이

    player->hitbox[1].x = player->body.x; // 몸통 기준 x좌표
    player->hitbox[1].y = player->body.y; // 몸통 기준 y좌표
    player->hitbox[1].width = 3; // 히트박스의 너비 
    player->hitbox[1].height = 3; // 히트박스의 높이
}

void initialize_monsterHitbox(Object* monsters)
{
    int i;
    for (i = 0; i < MONSTER_COUNT; i++)
    {
        // 히트박스의 위치와 크기를 설정
        monsters[i].hitbox[0].x = monsters[i].body.x - 1; // 몸통 기준 x좌표
        monsters[i].hitbox[0].y = monsters[i].body.y - 1; // 몸통 기준 y좌표
        monsters[i].hitbox[0].width = 3; // 히트박스의 너비
        monsters[i].hitbox[0].height = 2; // 히트박스의 높이
    }
}

void initializePlayer(char map[MAP_HEIGHT][MAP_WIDTH], Object player, int cameraOffset)
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

void initializeMonster(char map[MAP_HEIGHT][MAP_WIDTH], Object monsters[], int cameraOffset)
{

    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        // 몬스터의 위치가 유효한지 확인

        // 몬스터의 y좌표가 맵의 바닥에 있고, x좌표가 유효한 범위 내에 있는지 확인
        if (monsters[i].body.y < MAP_HEIGHT && monsters[i].body.y >= 0 && monsters[i].body.x >= 0 && monsters[i].body.x < P_MAP_WIDTH)
        {
            map[monsters[i].body.y][monsters[i].body.x - cameraOffset] = 'A'; // 몬스터 위치
            map[monsters[i].head.y][monsters[i].head.x - cameraOffset] = 'M'; // 몬스터 위치
            map[monsters[i].leftLeg.y][monsters[i].leftLeg.x - cameraOffset] = '&'; // 몬스터 위치
            map[monsters[i].rightLeg.y][monsters[i].rightLeg.x - cameraOffset] = '&'; // 몬스터 위치

        }
    }

}

void initializeMap(char map[MAP_HEIGHT][MAP_WIDTH], Object player, Object monsters[], int cameraOffset)
{
    // 맵 초기화
    int i, j;
    for (i = 0; i < MAP_HEIGHT; i++)
    {
        for (j = 0; j < MAP_WIDTH; j++)
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

    // 플레이어와 몬스터 초기화
    initializePlayer(map, player, cameraOffset);
    initializeMonster(map, monsters, cameraOffset);


// 충돌 체크
    for (int i = 0; i < MONSTER_COUNT; i++)
    {
        if (checkCollision(&player, &monsters[i], i))
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

void handleJumpAndGravity(Object* player)
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

void moveCharacter(Object* player)
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
    if (GetAsyncKeyState('A') & 0x8000) // 왼쪽으로 이동
    {
        if (player->head.x > 0)
        {
            player->head.x -= 2;
            player->body.x -= 2;
            player->leftLeg.x -= 2;
            player->rightLeg.x -= 2;
            player->LwalkFrame = (player->LwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->RwalkFrame = 0;
            player->RIGHT = 0;
        }
    }
    if (GetAsyncKeyState('D') & 0x8000) // 오른쪽으로 이동
    {
        if (player->head.x < MAP_WIDTH - 1)
        {
            player->head.x += 2;
            player->body.x += 2;
            player->leftLeg.x += 2;
            player->rightLeg.x += 2;
            player->RwalkFrame = (player->RwalkFrame + 1) % 4; // 걷기 프레임 업데이트
            player->LwalkFrame = 0;
            player->RIGHT = 1;
        }
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) // 스페이스바로 점프
    {
        if (!player->jumpState && player->leftLeg.y == MAP_HEIGHT - 1)
        { // 바닥에 있을 때만 점프 가능
            player->jumpVelocity = -3; // 점프 속도를 줄여서 최대 높이를 낮춤
            player->jumpState = true; // 점프 상태 설정
        }
    }
    if (GetAsyncKeyState('K') & 0x8000) // 공격
    {
        int i;
        player->attackState = true;
        if (player->attackState == true) // 공격 상태가 true일 때만 실행
        {
            for (i = 0; i < ATTACK_RANGE; i++)
            {
                if (player->RIGHT == 1)
                {
                    player->hitbox[1].x = player->body.x + i + 2 - cameraOffset; // hitbox[1]에 접근
                    player->hitbox[1].y = player->body.y;

                    map[player->body.y][player->body.x + i + 1 - cameraOffset] = map[player->body.y][player->body.x + i + 2 - cameraOffset];
                    if (i % 2 == 0)
                        map[player->body.y][player->body.x + i + 2 - cameraOffset] = 'o'; // 바닥
                    else
                        map[player->body.y][player->body.x + i + 2 - cameraOffset] = 'O'; // 바닥
                    printMap(*player, map);
                    Sleep(10);
                }
                else if (player->RIGHT == 0)
                {
                    player->hitbox[1].x = player->body.x - i - 2 - cameraOffset; // hitbox[1]에 접근
                    player->hitbox[1].y = player->body.y;
                    map[player->body.y][player->body.x - i - 1 - cameraOffset] = map[player->body.y][player->body.x - i - 2 - cameraOffset];
                    if (i % 2 == 0)
                        map[player->body.y][player->body.x - i - 2 - cameraOffset] = 'o'; // 바닥
                    else
                        map[player->body.y][player->body.x - i - 2 - cameraOffset] = 'O'; // 바닥
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
    Hitbox hitboxA0 = player->hitbox[0]; // 배열의 첫 번째 히트박스
    Hitbox hitboxA1 = player->hitbox[1]; // 배열의 두 번째 히트박스
    Hitbox b = monsters[i].hitbox[0]; // 배열의 첫 번째 히트박스

    // a[0]이 유효한 경우
    if (hitboxA0.width > 0 && hitboxA0.height > 0) // hitboxA0가 유효한지 확인
    {
        if (hitboxA0.x < b.x + b.width &&
            hitboxA0.x + hitboxA0.width > b.x &&
            hitboxA0.y < b.y + b.height &&
            hitboxA0.y + hitboxA0.height > b.y)
        {
            monsters[i].health = monsters[i].health - player->attackPower;
            return true; // 충돌 발생
        }
    }

    // a[1]이 유효한 경우
    if (hitboxA1.width > 0 && hitboxA1.height > 0) // hitboxA1가 유효한지 확인
    {
        if (hitboxA1.x < b.x + b.width &&
            hitboxA1.x + hitboxA1.width > b.x &&
            hitboxA1.y < b.y + b.height &&
            hitboxA1.y + hitboxA1.height > b.y)
        {
            monsters[i].health = monsters[i].health - player->attackPower;
            return true; // 충돌 발생
        }
    }

    // 충돌이 없으면 false 반환
    return false;
}


