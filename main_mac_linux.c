/*linux, mac을 위한 코드로 변환할 예정.*/

#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<math.h>
#include "main.h"

void title(void); //게임시작화면 
void reset(void); //게임판 초기화 
void reset_main(void); //메인 게임판(main_org[][]를 초기화)
void reset_main_cpy(void); //copy 게임판(main_cpy[][]를 초기화)
void draw_map(void); //게임 전체 인터페이스를 표시 
void draw_main(void); //게임판을 그림 
void new_block(void); //새로운 블록을 하나 만듦 
void check_key(void); //키보드로 키를 입력받음 
void drop_block(void); //블록을 아래로 떨어트림 
int check_crush(int bx, int by, int rotation); //bx, by위치에 rotation회전값을 같는 경우 충돌 판단 
void move_block(int dir); //dir방향으로 블록을 움직임 
void check_line(void); //줄이 가득찼는지를 판단하고 지움 
void check_level_up(void); //레벨목표가 달성되었는지를 판단하고 levelup시킴 
void check_game_over(void); //게임오버인지 판단하고 게임오버를 진행 
void pause(void);//게임을 일시정지시킴 
int make_rand(void);
void active_color(int type_of_block);
void trace_block(void); 
void remove_trace_block(void);

int autoModeDifficulty = 10;

//이후부터는 자동 알고리즘에 사용되는 인수들

void find_best_pos(void);
int max_rotation(void);
void trace_top(int* stack_top);
void simulate_init_bx(void);
void simulate_drop_block(void);
double score_floor_wall(void);
double score_height(int* stack_top);
double score_hole_blockade(int* stack_top);
double score_line(void);
double testScore(int* stackTop);

void autoModePlay(void);
int autoModeLevel(void);

int main() {
    int i,j;


    system("color F0");
    system("mode con cols=75 lines=30");
    srand((unsigned)time(NULL)); //난수표생성 
    setcursortype(NOCURSOR); //커서 없앰 
    title(); //메인타이틀 호출 
    reset(); //게임판 리셋 
    setColor(BLACK, WHITE);
    
    while (1) {
        for (i = 0; i < 5; i++) { //블록이 한칸떨어지는동안 5번 키입력받을 수 있음 
            if (auto_mode_flag == 1) i = 5;
            check_key(); //키입력확인 
            
            if (auto_mode_flag == 0) {
                trace_block();
                Sleep(speed); //게임속도조절
                if (crush_on && check_crush(bx, by + 1, b_rotation) == false) Sleep(100);  //블록이 충돌중인경우 추가로 이동및 회전할 시간을 갖음 
            }

            if (space_key_on == 1) { //스페이스바를 누른경우(hard drop) 추가로 이동및 회전할수 없음 break; 
                space_key_on = 0;
                break;
            }
        }
        drop_block(); // 블록을 한칸 
        check_level_up(); // 레벨업을 체크 
        check_game_over(); //게임오버를 체크 
        if (new_block_on == 1) 
            new_block(); // 뉴 블럭 flag가 있는 경우 새로운 블럭 

        if (auto_mode_flag == 1)
            autoModePlay();
    }
}

int autoModeLevel(void)
{
    int tmp = 0;
    switch (autoModeDifficulty) {
    case 1:
    case 2:
    case 3:
        tmp = 3;
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        tmp = 2;
        break;
    case 8:
    case 9:
    case 10:
        tmp = 1;
        break;
    default:
        return;
    }
    return tmp;
}


void autoModePlay(void)
{
    find_best_pos();
    int sim_bx = bx, sim_b_rotation = b_rotation, cnt = 0;
    bx = (MAIN_X / 2) - 1;
    if(autoModeLevel() != 1 || (autoModeLevel() == 1 && level < 9)) b_rotation = 0;
    while (crush_on == 0) {  //바닥에 닿을때까지 이동시킴
        if (++cnt % autoModeLevel() == 0) {
            do {
                if (b_rotation < sim_b_rotation && check_crush(bx, by, (b_rotation + 1) % 4) == true) move_block(UP);
                else if (check_crush(bx + 1, by, b_rotation) == true && sim_bx > bx) move_block(RIGHT);
                else if (check_crush(bx - 1, by, b_rotation) == true && sim_bx < bx) move_block(LEFT);
                else break;
            } while (autoModeLevel() == 1 && level > 8);
        }
        Sleep(speed + 10 * autoModeLevel());
        trace_block();
        if ((autoModeDifficulty == 10 && level > 8) || (by > 18 -autoModeDifficulty)) {
            while (crush_on == 0) {
                drop_block();
                score += level;
            }
            gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score); //점수 표시

        }
        else drop_block();
    }
    trace_block();
}

void trace_block(void)
{
    int i, j;
    int trace_x = bx, trace_y = by;
    while (check_crush(trace_x, trace_y + 1, b_rotation))
        trace_y++;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1 && main_org[trace_y + i][trace_x + j] == EMPTY) main_org[trace_y + i][trace_x + j] = TRACE_BLOCK;

    draw_main();

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1 && main_org[trace_y + i][trace_x + j] == TRACE_BLOCK) main_org[trace_y + i][trace_x + j] = EMPTY;

}

void remove_trace_block(void)
{
    int i, j;
    for (i = 0; i < MAIN_Y; i++)  // 게임판을 0으로 초기화  
        for (j = 0; j < MAIN_X; j++) 
            if (main_org[i][j] == TRACE_BLOCK) main_org[i][j] = EMPTY;
}

void active_color(int type_of_block)
{
    switch (type_of_block) {
    case 0:
        setColor(BLUE, WHITE);
        break;
    case 1:
    case 2:
    case 3:
        setColor(GREEN, WHITE);
        break;
    case 4:
    case 5:
        setColor(RED, WHITE);
        break;
    case 6:
        setColor(VIOLET, WHITE);
        break;
    }
}

int make_rand(void)
{
    int tmp = rand() % 10;
    switch (tmp) {
    case 0:
    case 1:
        return 0;
        break;
    case 2:
    case 3:
        return 1;
        break;
    case 4:
    case 5:
        return 2 + rand() % 2;
        break;
    case 6:
    case 7:
        return 4 + rand() % 2;
        break;
    case 8:
    case 9:
        return 6;
        break;
    }
}

/*타이틀 표시부분*/
void title(void) {
    int x = 5; //타이틀화면이 표시되는 x좌표 
    int y = 4; //타이틀화면이 표시되는 y좌표 
    int cnt; //타이틀 프레임을 세는 변수  

    gotoxy(x, y + 0); printf("■□□□■■■□□■■□□■■"); Sleep(100);
    gotoxy(x, y + 1); printf("■■■□  ■□□    ■■□□■"); Sleep(100);
    gotoxy(x, y + 2); printf("□□□■              □■  ■"); Sleep(100);
    gotoxy(x, y + 3); printf("■■□■■  □  ■  □□■□□"); Sleep(100);
    gotoxy(x, y + 4); printf("■■  ■□□□■■■□■■□□"); Sleep(100);
    gotoxy(x, y + 5); printf("                              "); Sleep(100);
    gotoxy(x + 5, y + 2); printf("T E T R I S"); Sleep(100);
    gotoxy(x, y + 7); printf("Please Enter Any Key to Start..");
    gotoxy(x, y + 9); printf("  △   : Shift");
    gotoxy(x, y + 10); printf("◁  ▷ : Left / Right");
    gotoxy(x, y + 11); printf("  ▽   : Soft Drop");
    gotoxy(x, y + 12); printf(" SPACE : Hard Drop");
    gotoxy(x, y + 13); printf("   P   : Pause");
    gotoxy(x, y + 14); printf("  ESC  : Quit");
    gotoxy(x, y + 16); printf("BONUS FOR HARD DROPS / COMBOS");

    for (cnt = 0;; cnt++) { //cnt를 1씩 증가시키면서 계속 반복    //하나도 안중요한 별 반짝이는 애니메이션효과 
        if (_kbhit()) break; //키입력이 있으면 무한루프 종료 
        if (cnt % 200 == 0) { gotoxy(x + 4, y + 1); printf("★"); }       //cnt가 200으로 나누어 떨어질때 별을 표시 
        if ((cnt % 200 - 100) == 0) { gotoxy(x + 4, y + 1); printf("  "); } //위 카운트에서 100카운트 간격으로 별을 지움 
        if ((cnt % 350) == 0) { gotoxy(x + 13, y + 2); printf("☆"); } //윗별과 같지만 시간차를 뒀음 
        if ((cnt % 350 - 100) == 0) { gotoxy(x + 13, y + 2); printf("  "); }
        Sleep(10); // 00.1초 딜레이  
    }

    while (_kbhit()) _getch(); //버퍼에 기록된 키값을 버림 

}

/*변수 초기화*/
void reset(void) {
    /*
    FILE* file = fopen("score.dat", "rt"); // score.dat파일을 연결
    if (file == 0) { best_score = 0; } //파일이 없으면 걍 최고점수에 0을 넣음
    else {
        fscanf(file, "%d", &best_score); // 파일이 열리면 최고점수를 불러옴
        fclose(file); //파일 닫음
    }*/

    level = 1; //각종변수 초기화 
    score = 0;
    level_goal = 1000;
    key = 0;
    crush_on = 0;
    cntl = 0;
    speed = 100;

    system("cls"); //화면지움 
    reset_main(); // main_org를 초기화 
    draw_map(); // 게임화면을 그림
    draw_main(); // 게임판을 그림 

    b_type_next = make_rand(); //다음번에 나올 블록 종류를 랜덤하게 생성 
    new_block(); //새로운 블록을 하나 만듦  
}

/*게임판 초기화*/
void reset_main(void) { //게임판을 초기화  
    int i, j;

    for (i = 0; i < MAIN_Y; i++) { // 게임판을 0으로 초기화  
        for (j = 0; j < MAIN_X; j++) {
            main_org[i][j] = 0;
            main_cpy[i][j] = 100;
        }
    }

    for (j = 1; j < MAIN_X; j++) { //y값이 3인 위치에 천장을 만듦 
        main_org[3][j] = CEILLING;
    }
    for (i = 1; i < MAIN_Y - 1; i++) { //좌우 벽을 만듦  
        main_org[i][0] = WALL;
        main_org[i][MAIN_X - 1] = WALL;
    }
    for (j = 0; j < MAIN_X; j++) { //바닥벽을 만듦 
        main_org[MAIN_Y - 1][j] = WALL;
    }
}

void reset_main_cpy(void) { //main_cpy를 초기화 
    for (int i = 0; i < MAIN_Y; i++) {         //게임판에 게임에 사용되지 않는 숫자를 넣음 
        for (int j = 0; j < MAIN_X; j++) {  //이는 main_org와 같은 숫자가 없게 하기 위함 
            main_cpy[i][j] = 100;
        }
    }
}

void draw_map(void) { //게임 상태 표시를 나타내는 함수  
    int y = 3;             // level, goal, score만 게임중에 값이 바뀔수 도 있음 그 y값을 따로 저장해둠 
                         // 그래서 혹시 게임 상태 표시 위치가 바뀌어도 그 함수에서 안바꿔도 되게.. 
    gotoxy(STATUS_X_ADJ, STATUS_Y_LEVEL = y); printf(" LEVEL : %5d", level);
    gotoxy(STATUS_X_ADJ, STATUS_Y_GOAL = y + 1); printf(" GOAL  : %5d", 10 - cntl);
    gotoxy(STATUS_X_ADJ, y + 2); printf("+-  N E X T  -+ ");
    gotoxy(STATUS_X_ADJ, y + 3); printf("|             | ");
    gotoxy(STATUS_X_ADJ, y + 4); printf("|             | ");
    gotoxy(STATUS_X_ADJ, y + 5); printf("|             | ");
    gotoxy(STATUS_X_ADJ, y + 6); printf("|             | ");
    gotoxy(STATUS_X_ADJ, y + 7); printf("+-- -  -  - --+ ");
    gotoxy(STATUS_X_ADJ, y + 8); printf(" YOUR SCORE :");
    gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE = y + 9); printf("        %6d", score);
    gotoxy(STATUS_X_ADJ, y + 10); printf(" LAST SCORE :");
    gotoxy(STATUS_X_ADJ, y + 11); printf("        %6d", last_score);
    gotoxy(STATUS_X_ADJ, y + 12); printf(" BEST SCORE :");
    gotoxy(STATUS_X_ADJ, y + 13); printf("        %6d", best_score);
    gotoxy(STATUS_X_ADJ, y + 15); printf("  △   : Shift        SPACE : Hard Drop");
    gotoxy(STATUS_X_ADJ, y + 16); printf("◁  ▷ : Left / Right   P   : Pause");
    gotoxy(STATUS_X_ADJ, y + 17); printf("  ▽   : Soft Drop     ESC  : Quit");
    gotoxy(STATUS_X_ADJ, y + 20); printf("Ai playing mode");
}

void draw_main(void) { //게임판 그리는 함수 
    int i, j;

    for (j = 1; j < MAIN_X - 1; j++) { //천장은 계속 새로운블럭이 지나가서 지워지면 새로 그려줌 
        if (main_org[3][j] == EMPTY) main_org[3][j] = CEILLING;
    }

    for (i = 0; i < MAIN_Y; i++) {
        for (j = 0; j < MAIN_X; j++) {
            if (main_cpy[i][j] != main_org[i][j]) { //cpy랑 비교해서 값이 달라진 부분만 새로 그려줌.
                                                //이게 없으면 게임판전체를 계속 그려서 느려지고 반짝거림
                gotoxy(MAIN_X_ADJ + j, MAIN_Y_ADJ + i);
                switch (main_org[i][j]) {
                case EMPTY: //빈칸모양
                    printf("  ");
                    break;
                case CEILLING: //천장모양
                    setColor(RED, WHITE);
                    printf(". ");
                    setColor(BLACK, WHITE);
                    break;
                case WALL: //벽모양 ▩
                    setColor(BLACK, WHITE);
                    printf("▩");
                    setColor(BLACK, WHITE);
                    break;
                case INACTIVE_BLOCK: //굳은 블럭 모양  □
                    setColor(GRAY, WHITE);
                    printf("■");
                    setColor(BLACK, WHITE);
                    break;
                case ACTIVE_BLOCK: //움직이고있는 블럭 모양  ■
                    active_color(b_type);
                    printf("■");
                    setColor(BLACK, WHITE);
                    break;
                case TRACE_BLOCK:
                    printf("□");
                }
            }
        }
    }

    for (i = 0; i < MAIN_Y; i++) { //게임판을 그린 후 main_cpy에 복사  
        for (j = 0; j < MAIN_X; j++) { 
            main_cpy[i][j] = main_org[i][j];
        }
    }
}

void new_block(void) { //새로운 블록 생성  
    int i, j;

    bx = (MAIN_X / 2) - 1; //블록 생성 위치x좌표(게임판의 가운데) 
    by = 0;  //블록 생성위치 y좌표(제일 위) 
    b_type = b_type_next; //다음블럭값을 가져옴
    b_type_next = make_rand(); //다음 블럭을 만듦 
    b_rotation = 0;  //회전은 0번으로 가져옴 

    new_block_on = 0; //new_block flag를 끔

    for (i = 0; i < 4; i++) { //게임판 bx, by위치에 블럭생성  
        for (j = 0; j < 4; j++) {
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
        }
    }
    for (i = 1; i < 3; i++) { //게임상태표시에 다음에 나올블럭을 그림 
        for (j = 0; j < 4; j++) {
            active_color(b_type_next);
            if (blocks[b_type_next][0][i][j] == 1) {
                gotoxy(STATUS_X_ADJ + 2 + j, i + 6);
                printf("■");
            }
            else {
                gotoxy(STATUS_X_ADJ + 2 + j, i + 6);
                printf("  ");
            }
            setColor(BLACK, WHITE);
        }
    }
}

void check_key(void) {
    key = 0; //키값 초기화  

    if (_kbhit()) { //키입력이 있는 경우  
        key = _getch(); //키값을 받음
        if (key == 224) { //방향키인경우 
            do { key = _getch(); } while (key == 224);//방향키지시값을 버림 
            switch (key) {
            case LEFT: //왼쪽키 눌렀을때  
                if (check_crush(bx - 1, by, b_rotation) == true) move_block(LEFT);
                break;                            //왼쪽으로 갈 수 있는지 체크 후 가능하면 이동
            case RIGHT: //오른쪽 방향키 눌렀을때- 위와 동일하게 처리됨 
                if (check_crush(bx + 1, by, b_rotation) == true) move_block(RIGHT);
                break;
            case DOWN: //아래쪽 방향키 눌렀을때-위와 동일하게 처리됨 
                if (check_crush(bx, by + 1, b_rotation) == true) move_block(DOWN);
                break;
            case UP: //위쪽 방향키 눌렀을때 
                if (check_crush(bx, by, (b_rotation + 1) % 4) == true) move_block(UP);
                //회전할 수 있는지 체크 후 가능하면 회전
                else if (crush_on == 1 && check_crush(bx, by - 1, (b_rotation + 1) % 4) == true) move_block(100);
            }                    //바닥에 닿은 경우 위쪽으로 한칸띄워서 회전이 가능하면 그렇게 함(특수동작)
        }
        else { //방향키가 아닌경우 
            switch (key) {
            case SPACE: //스페이스키 눌렀을때 
                space_key_on = 1; //스페이스키 flag를 띄움 
                while (crush_on == 0) { //바닥에 닿을때까지 이동시킴 
                    drop_block();
                    score += level; // hard drop 보너스
                    gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score); //점수 표시
                }
                break;
            case P: //P(대문자) 눌렀을때 
            case p: //p(소문자) 눌렀을때 
                pause(); //일시정지 
                break;
            case ESC: //ESC눌렀을때 
                system("cls"); //화면을 지우고 
                exit(0); //게임종료 
            }
        }
    }
    while (_kbhit()) _getch(); //키버퍼를 비움 
}

void drop_block(void) {
    int i, j;

    if (crush_on && check_crush(bx, by + 1, b_rotation) == true) crush_on = 0; //밑이 비어있으면 crush flag 끔 
    if (crush_on && check_crush(bx, by + 1, b_rotation) == false) { //밑이 비어있지않고 crush flag가 켜저있으면 
        for (i = 0; i < MAIN_Y; i++) { //현재 조작중인 블럭을 굳힘 
            for (j = 0; j < MAIN_X; j++) {
                if (main_org[i][j] == ACTIVE_BLOCK) main_org[i][j] = INACTIVE_BLOCK;
            }
        }
        crush_on = 0; //flag를 끔
        check_line(); //라인체크를 함
        new_block_on = 1; //새로운 블럭생성 flag를 켬 
        return; //함수 종료
    }
    if (check_crush(bx, by + 1, b_rotation) == true) move_block(DOWN); //밑이 비어있으면 밑으로 한칸 이동 
    if (check_crush(bx, by + 1, b_rotation) == false) crush_on++; //밑으로 이동이 안되면  crush flag를 켬
}

int check_crush(int bx, int by, int b_rotation) { //지정된 좌표와 회전값으로 충돌이 있는지 검사 
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) { //지정된 위치의 게임판과 블럭모양을 비교해서 겹치면 false를 리턴 
            if (blocks[b_type][b_rotation][i][j] == 1 && main_org[by + i][bx + j] > 0) return false;
        }
    }
    return true; //하나도 안겹치면 true리턴 
};

void move_block(int dir) { //블록을 이동시킴 
    int i, j;

    switch (dir) {
    case LEFT: //왼쪽방향 
        for (i = 0; i < 4; i++) { //현재좌표의 블럭을 지움 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        for (i = 0; i < 4; i++) { //왼쪽으로 한칸가서 active block을 찍음 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j - 1] = ACTIVE_BLOCK;
            }
        }
        bx--; //좌표값 이동 
        break;

    case RIGHT:    //오른쪽 방향. 왼쪽방향이랑 같은 원리로 동작 
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j + 1] = ACTIVE_BLOCK;
            }
        }
        bx++;
        break;

    case DOWN:    //아래쪽 방향. 왼쪽방향이랑 같은 원리로 동작
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i + 1][bx + j] = ACTIVE_BLOCK;
            }
        }
        by++;
        break;

    case UP: //키보드 위쪽 눌렀을때 회전시킴. 
        for (i = 0; i < 4; i++) { //현재좌표의 블럭을 지움  
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        b_rotation = (b_rotation + 1) % 4; //회전값을 1증가시킴(3에서 4가 되는 경우는 0으로 되돌림) 
        for (i = 0; i < 4; i++) { //회전된 블록을 찍음 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
            }
        }
        break;

    case 100: //블록이 바닥, 혹은 다른 블록과 닿은 상태에서 한칸위로 올려 회전이 가능한 경우 
              //이를 동작시키는 특수동작 
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        b_rotation = (b_rotation + 1) % 4;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i - 1][bx + j] = ACTIVE_BLOCK;
            }
        }
        by--;
        break;
    }
}

void check_line(void) {
    int i, j, k, l;

    int  block_amount; //한줄의 블록갯수를 저장하는 변수 
    int combo = 0; //콤보갯수 저장하는 변수 지정및 초기화 

    for (i = MAIN_Y - 2; i > 3;) { //i=MAIN_Y-2 : 밑쪽벽의 윗칸부터,  i>3 : 천장(3)아래까지 검사 
        block_amount = 0; //블록갯수 저장 변수 초기화 
        for (j = 1; j < MAIN_X - 1; j++) { //벽과 벽사이의 블록갯루를 셈 
            if (main_org[i][j] > 0) block_amount++;
        }
        if (block_amount == MAIN_X - 2) { //블록이 가득 찬 경우 
            if (level_up_on == 0) { //레벨업상태가 아닌 경우에(레벨업이 되면 자동 줄삭제가 있음) 
                score += 100 * level; //점수추가 
                cntl++; //지운 줄 갯수 카운트 증가 
                combo++; //콤보수 증가  
            }
            for (k = i; k > 1; k--) { //윗줄을 한칸씩 모두 내림(윗줄이 천장이 아닌 경우에만) 
                for (l = 1; l < MAIN_X - 1; l++) {
                    if (main_org[k - 1][l] != CEILLING) main_org[k][l] = main_org[k - 1][l];
                    if (main_org[k - 1][l] == CEILLING) main_org[k][l] = EMPTY;
                    //윗줄이 천장인 경우에는 천장을 한칸 내리면 안되니까 빈칸을 넣음 
                }
            }
        }
        else i--;
    }
    if (combo) { //줄 삭제가 있는 경우 점수와 레벨 목표를 새로 표시함
        if (combo > 1) { //2콤보이상인 경우 경우 보너스및 메세지를 게임판에 띄웠다가 지움
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 1, MAIN_Y_ADJ + by - 2); printf("%d COMBO!", combo);
            Sleep(300);
            score += (combo * level * 100);
            reset_main_cpy(); //텍스트를 지우기 위해 main_cpy을 초기화.
//(main_cpy와 main_org가 전부 다르므로 다음번 draw()호출시 게임판 전체를 새로 그리게 됨) 
        }
        gotoxy(STATUS_X_ADJ, STATUS_Y_GOAL); printf(" GOAL  : %5d", (cntl <= 10) ? 10 - cntl : 0);
        gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score);
    }
}

void check_level_up(void) {
    int i, j;


    if (level <10 && cntl >= 10) { //레벨별로 10줄씩 없애야함. 10줄이상 없앤 경우
        draw_main();
        level_up_on = 1; //레벨업 flag를 띄움
        level += 1; //레벨을 1 올림
        cntl = 0; //지운 줄수 초기화

        for (i = 0; i < 2; i++) {
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 3, MAIN_Y_ADJ + 4);
            printf("             ");
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 2, MAIN_Y_ADJ + 6);
            printf("             ");
            Sleep(200);

            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 3, MAIN_Y_ADJ + 4);
            printf("☆LEVEL UP!☆");
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 2, MAIN_Y_ADJ + 6);
            printf("☆SPEED UP!☆");
            Sleep(200);
        }
        reset_main_cpy(); //텍스트를 지우기 위해 main_cpy을 초기화.
//(main_cpy와 main_org가 전부 다르므로 다음번 draw()호출시 게임판 전체를 새로 그리게 됨)

        setColor(DARK_YELLOW, WHITE);
            for (i = MAIN_Y - 2; i > MAIN_Y - 2 - (level - 1); i--) { //레벨업보상으로 각 레벨-1의 수만큼 아랫쪽 줄을 지워줌 
                for (j = 1; j < MAIN_X - 1; j++) {
                    main_org[i][j] = INACTIVE_BLOCK; // 줄을 블록으로 모두 채우고 
                    gotoxy(MAIN_X_ADJ + j, MAIN_Y_ADJ + i); // 별을 찍어줌.. 이뻐보이게 
                    printf("★");
                    Sleep(5);
                }
            }
            setColor(BLACK, WHITE);
        Sleep(100); //별찍은거 보여주기 위해 delay 
        check_line(); //블록으로 모두 채운것 지우기
//.check_line()함수 내부에서 level up flag가 켜져있는 경우 점수는 없음.         
        switch (level) { //레벨별로 속도를 조절해줌. 
        case 2:
            speed = 50;
            break;
        case 3:
            speed = 45;
            break;
        case 4:
            speed = 40;
            break;
        case 5:
            speed = 35;
            break;
        case 6:
            speed = 30;
            break;
        case 7:
            speed = 25;
            break;
        case 8:
            speed = 20;
            break;
        case 9:
            speed = 10;
            break;
        case 10:
            speed = 3;
            break;
        }
        level_up_on = 0; //레벨업 flag꺼줌

        gotoxy(STATUS_X_ADJ, STATUS_Y_LEVEL); printf(" LEVEL : %5d", level); //레벨표시 
        if (level < 10) {
            gotoxy(STATUS_X_ADJ, STATUS_Y_GOAL); 
            printf(" GOAL  : %5d", 10 - cntl); // 레벨목표 표시 
        }

    }
}

void check_game_over(void) {
    int i;

    int x = 5;
    int y = 5;

    for (i = 1; i < MAIN_X - 2; i++) {
        if (main_org[3][i] > 0) { //천장(위에서 세번째 줄)에 inactive가 생성되면 게임 오버 
            gotoxy(x, y + 0); printf("▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤"); //게임오버 메세지 
            gotoxy(x, y + 1); printf("▤                              ▤");
            gotoxy(x, y + 2); printf("▤  +-----------------------+   ▤");
            gotoxy(x, y + 3); printf("▤  |  G A M E  O V E R..   |   ▤");
            gotoxy(x, y + 4); printf("▤  +-----------------------+   ▤");
            gotoxy(x, y + 5); printf("▤   YOUR SCORE: %6d         ▤", score);
            gotoxy(x, y + 6); printf("▤                              ▤");
            gotoxy(x, y + 7); printf("▤  Press any key to restart..  ▤");
            gotoxy(x, y + 8); printf("▤                              ▤");
            gotoxy(x, y + 9); printf("▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤");
            last_score = score; //게임점수를 옮김 

            if (score > best_score) { //최고기록 갱신시 
               // FILE* file = fopen("score.dat", "wt"); //score.dat에 점수 저장                

                gotoxy(x, y + 6); printf("▤  ★★★ BEST SCORE! ★★★   ▤  ");
                int el = 1;
                if (el == 0) { //파일 에러메세지  
                    gotoxy(0, 0);
                    printf("FILE ERROR: SYSTEM CANNOT WRITE BEST SCORE ON \"SCORE.DAT\"");
                }
                else {
                    //fprintf(file, "%d", score);
                    //fclose(file);
                }
            }
            Sleep(1000);
            while (_kbhit()) _getch();
            key = _getch();
            reset();
        }
    }
}

void pause(void) { //게임 일시정지 함수 
    int i, j;

    int x = 5;
    int y = 5;

    for (i = 1; i < MAIN_X - 2; i++) { //게임 일시정지 메세지 
        gotoxy(x, y + 0); printf("▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤");
        gotoxy(x, y + 1); printf("▤                              ▤");
        gotoxy(x, y + 2); printf("▤  +-----------------------+   ▤");
        gotoxy(x, y + 3); printf("▤  |       P A U S E       |   ▤");
        gotoxy(x, y + 4); printf("▤  +-----------------------+   ▤");
        gotoxy(x, y + 5); printf("▤  Press any key to resume..   ▤");
        gotoxy(x, y + 6); printf("▤  press A key to toggle mode  ▤");
        gotoxy(x, y + 7); printf("▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤▤");
    }


    key = _getch(); //키값을 받음
    switch (key) {
    case 97:
        switch (auto_mode_flag) {
        case 0:
            auto_mode_flag = 1;
            break;
        case 1:
            auto_mode_flag = 0;
            break;
        }
        break;
    default:
        break;
    }
    key = 0;


    system("cls"); //화면 지우고 새로 그림 
    reset_main_cpy();
    trace_block();
    draw_map();

    active_color(b_type_next);
    for (i = 1; i < 3; i++) { // 다음블록 그림 
        for (j = 0; j < 4; j++) {
            if (blocks[b_type_next][0][i][j] == 1) {
                gotoxy(MAIN_X + MAIN_X_ADJ + 3 + j, i + 6);
                printf("■");
            }
            else {
                gotoxy(MAIN_X + MAIN_X_ADJ + 3 + j, i + 6);
                printf("  ");
            }
        }
    }
    setColor(BLACK, WHITE);
}

//여기서부터는 가중치 알고리즘을 기술하는 부분임.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void find_best_pos(void)
{
    int stack_top[11] = { 0, };
    int cnt = 0; //rotation을 제한하기 위한 카운트입니다.

    int best_rotation, best_bx;
    double score, best_score;
    b_rotation = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
    while (cnt < max_rotation()) {
        simulate_init_bx(); //bx를 먼제 제일 왼쪽에 위치시킵니다.
        int cnt2 = 0; //변수들의 초기화 시작을 파악하기 위한 카운트 입니다.
        while (check_crush(bx, by, b_rotation)) { //수평축으로 한칸씩 충돌이 일어나지 않을때까지 오른쪽으로 이동합니다
            by = 0; //처음 y축의 시작은 0입니다
            simulate_drop_block(); //블록을 끝까지 내리는 함수를 실행합니다
            trace_top(stack_top); //각 높이를 구합니다

            score = score_height(stack_top) + score_floor_wall() + score_hole_blockade(stack_top) + score_line(); //이후 현재 위치에서 점수를 구합니다
            //score = testScore(stack_top);
            if (cnt == 0 && cnt2 == 0) { //최초 1번 루프가 실행시, 변수들을 초기화 해줍니다
                best_rotation = b_rotation;
                best_bx = bx;
                best_score = score;
            }
            if (score > best_score) { //현재 스코어가 베스트 스코어보다 높을 시, 현재 스코어를 베스트 스코어로 지정한뒤, x좌표와 회전값을 기억합니다
                best_rotation = b_rotation;
                best_bx = bx;
                best_score = score;
            }

            for (int i = 0; i < 4; i++) //계산이 종료시, 계산을 위해 그렸던 블록을 지워줍니다.
                for (int j = 0; j < 4; j++)
                    if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            bx++; //한칸 오른쪽으로 이동합니다
            cnt2++; //초기값 판별값을 한칸 올려 이후에는 초기 변수 세팅을 진행하지 않습니다
            by = 0;
        }
        b_rotation++;
        cnt++; //로테이션 카운트를 하나 올립니다
    }
    bx = best_bx; //위의 시뮬레이션 결과값을 x와 로테이션 값에 저장합니다.
    b_rotation = best_rotation;
    by = 0;
}

int max_rotation(void)
{
    switch (b_type) {
    case 0:
        return 1;
        break;
    case 1:
    case 2:
    case 3:
        return 2;
        break;
    case 4:
    case 5:
    case 6:
        return 4;
        break;
    }
}

void trace_top(int* stack_top)
{
    //각 x의 위치의 모든 높이를 구하는 함수입니다.
    for (int i = 1; i < MAIN_X - 1; i++) {
        for (int j = 0; j < 25; j++)
            if (main_org[j][i] == WALL || main_org[j][i] == INACTIVE_BLOCK || main_org[j][i] == ACTIVE_BLOCK) { //벽면 혹은 비활성화 혹은 활성화 블록을 만나면 멈춥니다.
                stack_top[i] = j; //이때, 해당 멈춘 지점이 높이를 의미합니다.
                break;
            }
        stack_top[i] = MAIN_Y - 1 - stack_top[i]; //y축의 높이가 22이므로, 22에서 우리가 구한 높이를 빼야, 바닥으로부터의 높이를 구할 수 있습니다.
    }
}

void simulate_init_bx(void)
{

    //초기값으로 블록을 맨 오른쪽으로 옮기는 과정입니다.
    bx = 5;
    by = 0;
    while (check_crush(bx - 1, by, b_rotation)) //x값이 크러시 나지 않을 때 까지
        bx--; //좌표값 이동 
}

void simulate_drop_block(void)
{
    //블록을 크러시가 날 때 까지 내리는 과정입니다.
    while (check_crush(bx, by + 1, b_rotation))
        by++;
    //크러시가 발생 했기 때문에, 현재의 x,y 좌표가 블록이 굳어 버리는 위치 입니다.
    //따라서, 해당 좌표에 활성화된 블록을 메인 좌표에 그려줍니다
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
}

double score_height(int* stack_top)
{
    //높이에 관한 점수를 계산하는 함수입니다.
    int i, sumOfHeight = 0, sumOfSquareOfHeight = 0;
    int maxHeight = stack_top[1], minEdgeHeight = stack_top[1];
    double scoreOfSumOfHeight = 0.0, scoreOfSdOfHeight = 0.0, scoreOfDifferenceMaxMin = 0.0;
    double averageOfSumOfHeight = 0.0, averageOfSumOfSQHeight = 0.0;

    for (i = 1; i < MAIN_X - 1; i++) {//stack top을 이용해서 모든 높이를 더해버립니다.
        sumOfHeight += stack_top[i];
        sumOfSquareOfHeight += stack_top[i] * stack_top[i]; //StandardDeviation를 구하기 위해 sqr의 합을 구합니다.
        maxHeight = (maxHeight > stack_top[i]) ? maxHeight : stack_top[i];
        if (i == 10 && minEdgeHeight > stack_top[i])
            minEdgeHeight = stack_top[i];
    }
    averageOfSumOfHeight = sumOfHeight / (double)10;
    averageOfSumOfSQHeight = sumOfSquareOfHeight / (double)10;

    scoreOfSumOfHeight = sumOfHeight * weight[_HEIGHT]; //모든 높이의 합과 가중치를 곱해 점수화합니다.
    scoreOfSdOfHeight = sqrt(averageOfSumOfSQHeight - averageOfSumOfHeight * averageOfSumOfHeight) * weight[_SD_HEIGHT];
    //높이에 대한 표준편하와 가중치를 곱해 점수화 합니다.
    scoreOfDifferenceMaxMin = ((double)maxHeight - (double)minEdgeHeight) * weight[_MAX_MIN_HEIGHT]; //최대높이와 최저높이의 차이와 가중치를 곱하여 점수화합니다.
    return scoreOfSumOfHeight + scoreOfSdOfHeight + scoreOfDifferenceMaxMin;
}

double score_hole_blockade(int* stack_top)
{
    //구멍과 봉쇄의 정도에 대한 점수를 구하는 함수입니다.
    //봉쇄는 구멍이 발생시, 해당 구멍 보다 높은 곳에 존재하는 활성화 혹은 비활성화 블록들의 합 입니다.
    int i, j, k, tmp;
    double scoreOfHole = 0.0, scoreOfBlockade = 0.0;
    int countOfHoleOfColumm, countOfBlocksOfColumm;
    int countOfHole = 0, countOfBlockade = 0;
    for (i = 1; i < MAIN_X - 1; i++) {
        countOfHoleOfColumm = 0, countOfBlocksOfColumm = 0;
        for (j = MAIN_Y - 1; j >= MAIN_Y - 1 - stack_top[i]; j--) { //hole의 점수를 구하기 위해서 stack_top 에서 시작하여 맨 밑 바닥까지 empty가 있나 검사합니다.
            if (main_org[j][i] == EMPTY)
                countOfHoleOfColumm++;
            else if (countOfHoleOfColumm > 0)
                countOfBlockade++;
        }
        countOfHole += countOfHoleOfColumm;
    }
    scoreOfHole = (double)countOfHole * weight[_HOLE];
    scoreOfBlockade = (double)countOfBlockade * weight[_BLOCKADE];
    return (scoreOfHole + scoreOfBlockade);
}

double score_floor_wall(void)
{
    //벽면과 바닥면에 대한 점수를 구하는 함수입니다.
    int i, j;
    double scoreOfFloor = 0.0, scoreOfWall = 0.0;
    int floorTouched = 0, wallTouched = 0;


    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) {
                int k = 0, l = -1; //k는 수직에 대한 계산을 위해, l은 수평에 대한 계산을 위해 사용됩니다. 이때, 왼쪽부터 계산하기 위해 l은 -1로 선언됩니다.
                for (int f = 0; f < 3; f++) {
                    switch (main_org[by + k + i][bx + l + j]) {
                    case WALL:
                        if (k == 1) floorTouched++; //k가 1이라는 것은 방향이 아래방향이라는 뜻입니다. 따라서 바닥에 대한 점수를 누적합을 합니다.
                        else wallTouched++;//k가 1이 아니면 방향이 수평이라는 뜻입니다. 따라서 벽면에 대한 점수를 누적합을 합니다.
                        break;
                    case INACTIVE_BLOCK:
                        if (k == 1) floorTouched++; //바닥이 벽이 아니라 비활성화 인 경우에도 바닥에 대한 점수를 누적합을 합니다.
                        else wallTouched++;
                            //scoreOfFloor += 3.7;
                        break;
                    default:
                        break;
                    }
                    l++; //l이 -1부터 시작해서 하나가 더해졌으므로 다음 계산에는 수평축에 대한 변경사항이 없습니다.
                    if (l == 0) k = 1; //수평축에 대한 변경사항이 없는 경우, 그때는 수직에 대한 검사를 진행하기 위한 것이므로, k를 1로 설정하여 한 칸 아래쪽을 검사합니다.
                    else k = 0;
                }
            }
    scoreOfWall = (double)wallTouched * weight[_WALL];
    scoreOfFloor = (double)floorTouched * 10 * weight[_FLOOR];
    return (scoreOfFloor + scoreOfWall);
}

double score_line(void)
{
    int i, j;
    int block_amount; //한줄의 블록갯수를 저장하는 변수 
    double scoreOfLine = 0.0;
    int cnt = 0;

    for (i = MAIN_Y - 2; i > 3; i--) { //i=MAIN_Y-2 : 밑쪽벽의 윗칸부터,  i>3 : 천장(3)아래까지 검사 
        block_amount = 0; //블록갯수 저장 변수 초기화 
        for (j = 1; j < MAIN_X - 1; j++)  //벽과 벽사이의 블록갯루를 셈 
            if (main_org[i][j] == INACTIVE_BLOCK || main_org[i][j] == ACTIVE_BLOCK) block_amount++;
        if (block_amount == MAIN_X - 2)
            cnt++;
    }
    scoreOfLine = (double)cnt * (double)cnt * 100 * weight[_LINES];
    return scoreOfLine;
}