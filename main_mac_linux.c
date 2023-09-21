/*linux, mac�� ���� �ڵ�� ��ȯ�� ����.*/

#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<math.h>
#include "main.h"

void title(void); //���ӽ���ȭ�� 
void reset(void); //������ �ʱ�ȭ 
void reset_main(void); //���� ������(main_org[][]�� �ʱ�ȭ)
void reset_main_cpy(void); //copy ������(main_cpy[][]�� �ʱ�ȭ)
void draw_map(void); //���� ��ü �������̽��� ǥ�� 
void draw_main(void); //�������� �׸� 
void new_block(void); //���ο� ����� �ϳ� ���� 
void check_key(void); //Ű����� Ű�� �Է¹��� 
void drop_block(void); //����� �Ʒ��� ����Ʈ�� 
int check_crush(int bx, int by, int rotation); //bx, by��ġ�� rotationȸ������ ���� ��� �浹 �Ǵ� 
void move_block(int dir); //dir�������� ����� ������ 
void check_line(void); //���� ����á������ �Ǵ��ϰ� ���� 
void check_level_up(void); //������ǥ�� �޼��Ǿ������� �Ǵ��ϰ� levelup��Ŵ 
void check_game_over(void); //���ӿ������� �Ǵ��ϰ� ���ӿ����� ���� 
void pause(void);//������ �Ͻ�������Ŵ 
int make_rand(void);
void active_color(int type_of_block);
void trace_block(void); 
void remove_trace_block(void);

int autoModeDifficulty = 10;

//���ĺ��ʹ� �ڵ� �˰��� ���Ǵ� �μ���

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
    srand((unsigned)time(NULL)); //����ǥ���� 
    setcursortype(NOCURSOR); //Ŀ�� ���� 
    title(); //����Ÿ��Ʋ ȣ�� 
    reset(); //������ ���� 
    setColor(BLACK, WHITE);
    
    while (1) {
        for (i = 0; i < 5; i++) { //����� ��ĭ�������µ��� 5�� Ű�Է¹��� �� ���� 
            if (auto_mode_flag == 1) i = 5;
            check_key(); //Ű�Է�Ȯ�� 
            
            if (auto_mode_flag == 0) {
                trace_block();
                Sleep(speed); //���Ӽӵ�����
                if (crush_on && check_crush(bx, by + 1, b_rotation) == false) Sleep(100);  //����� �浹���ΰ�� �߰��� �̵��� ȸ���� �ð��� ���� 
            }

            if (space_key_on == 1) { //�����̽��ٸ� �������(hard drop) �߰��� �̵��� ȸ���Ҽ� ���� break; 
                space_key_on = 0;
                break;
            }
        }
        drop_block(); // ����� ��ĭ 
        check_level_up(); // �������� üũ 
        check_game_over(); //���ӿ����� üũ 
        if (new_block_on == 1) 
            new_block(); // �� �� flag�� �ִ� ��� ���ο� �� 

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
    while (crush_on == 0) {  //�ٴڿ� ���������� �̵���Ŵ
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
            gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score); //���� ǥ��

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
    for (i = 0; i < MAIN_Y; i++)  // �������� 0���� �ʱ�ȭ  
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

/*Ÿ��Ʋ ǥ�úκ�*/
void title(void) {
    int x = 5; //Ÿ��Ʋȭ���� ǥ�õǴ� x��ǥ 
    int y = 4; //Ÿ��Ʋȭ���� ǥ�õǴ� y��ǥ 
    int cnt; //Ÿ��Ʋ �������� ���� ����  

    gotoxy(x, y + 0); printf("����������������"); Sleep(100);
    gotoxy(x, y + 1); printf("�����  ����    ������"); Sleep(100);
    gotoxy(x, y + 2); printf("�����              ���  ��"); Sleep(100);
    gotoxy(x, y + 3); printf("������  ��  ��  ������"); Sleep(100);
    gotoxy(x, y + 4); printf("���  �������������"); Sleep(100);
    gotoxy(x, y + 5); printf("                              "); Sleep(100);
    gotoxy(x + 5, y + 2); printf("T E T R I S"); Sleep(100);
    gotoxy(x, y + 7); printf("Please Enter Any Key to Start..");
    gotoxy(x, y + 9); printf("  ��   : Shift");
    gotoxy(x, y + 10); printf("��  �� : Left / Right");
    gotoxy(x, y + 11); printf("  ��   : Soft Drop");
    gotoxy(x, y + 12); printf(" SPACE : Hard Drop");
    gotoxy(x, y + 13); printf("   P   : Pause");
    gotoxy(x, y + 14); printf("  ESC  : Quit");
    gotoxy(x, y + 16); printf("BONUS FOR HARD DROPS / COMBOS");

    for (cnt = 0;; cnt++) { //cnt�� 1�� ������Ű�鼭 ��� �ݺ�    //�ϳ��� ���߿��� �� ��¦�̴� �ִϸ��̼�ȿ�� 
        if (_kbhit()) break; //Ű�Է��� ������ ���ѷ��� ���� 
        if (cnt % 200 == 0) { gotoxy(x + 4, y + 1); printf("��"); }       //cnt�� 200���� ������ �������� ���� ǥ�� 
        if ((cnt % 200 - 100) == 0) { gotoxy(x + 4, y + 1); printf("  "); } //�� ī��Ʈ���� 100ī��Ʈ �������� ���� ���� 
        if ((cnt % 350) == 0) { gotoxy(x + 13, y + 2); printf("��"); } //������ ������ �ð����� ���� 
        if ((cnt % 350 - 100) == 0) { gotoxy(x + 13, y + 2); printf("  "); }
        Sleep(10); // 00.1�� ������  
    }

    while (_kbhit()) _getch(); //���ۿ� ��ϵ� Ű���� ���� 

}

/*���� �ʱ�ȭ*/
void reset(void) {
    /*
    FILE* file = fopen("score.dat", "rt"); // score.dat������ ����
    if (file == 0) { best_score = 0; } //������ ������ �� �ְ������� 0�� ����
    else {
        fscanf(file, "%d", &best_score); // ������ ������ �ְ������� �ҷ���
        fclose(file); //���� ����
    }*/

    level = 1; //�������� �ʱ�ȭ 
    score = 0;
    level_goal = 1000;
    key = 0;
    crush_on = 0;
    cntl = 0;
    speed = 100;

    system("cls"); //ȭ������ 
    reset_main(); // main_org�� �ʱ�ȭ 
    draw_map(); // ����ȭ���� �׸�
    draw_main(); // �������� �׸� 

    b_type_next = make_rand(); //�������� ���� ��� ������ �����ϰ� ���� 
    new_block(); //���ο� ����� �ϳ� ����  
}

/*������ �ʱ�ȭ*/
void reset_main(void) { //�������� �ʱ�ȭ  
    int i, j;

    for (i = 0; i < MAIN_Y; i++) { // �������� 0���� �ʱ�ȭ  
        for (j = 0; j < MAIN_X; j++) {
            main_org[i][j] = 0;
            main_cpy[i][j] = 100;
        }
    }

    for (j = 1; j < MAIN_X; j++) { //y���� 3�� ��ġ�� õ���� ���� 
        main_org[3][j] = CEILLING;
    }
    for (i = 1; i < MAIN_Y - 1; i++) { //�¿� ���� ����  
        main_org[i][0] = WALL;
        main_org[i][MAIN_X - 1] = WALL;
    }
    for (j = 0; j < MAIN_X; j++) { //�ٴں��� ���� 
        main_org[MAIN_Y - 1][j] = WALL;
    }
}

void reset_main_cpy(void) { //main_cpy�� �ʱ�ȭ 
    for (int i = 0; i < MAIN_Y; i++) {         //�����ǿ� ���ӿ� ������ �ʴ� ���ڸ� ���� 
        for (int j = 0; j < MAIN_X; j++) {  //�̴� main_org�� ���� ���ڰ� ���� �ϱ� ���� 
            main_cpy[i][j] = 100;
        }
    }
}

void draw_map(void) { //���� ���� ǥ�ø� ��Ÿ���� �Լ�  
    int y = 3;             // level, goal, score�� �����߿� ���� �ٲ�� �� ���� �� y���� ���� �����ص� 
                         // �׷��� Ȥ�� ���� ���� ǥ�� ��ġ�� �ٲ� �� �Լ����� �ȹٲ㵵 �ǰ�.. 
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
    gotoxy(STATUS_X_ADJ, y + 15); printf("  ��   : Shift        SPACE : Hard Drop");
    gotoxy(STATUS_X_ADJ, y + 16); printf("��  �� : Left / Right   P   : Pause");
    gotoxy(STATUS_X_ADJ, y + 17); printf("  ��   : Soft Drop     ESC  : Quit");
    gotoxy(STATUS_X_ADJ, y + 20); printf("Ai playing mode");
}

void draw_main(void) { //������ �׸��� �Լ� 
    int i, j;

    for (j = 1; j < MAIN_X - 1; j++) { //õ���� ��� ���ο���� �������� �������� ���� �׷��� 
        if (main_org[3][j] == EMPTY) main_org[3][j] = CEILLING;
    }

    for (i = 0; i < MAIN_Y; i++) {
        for (j = 0; j < MAIN_X; j++) {
            if (main_cpy[i][j] != main_org[i][j]) { //cpy�� ���ؼ� ���� �޶��� �κи� ���� �׷���.
                                                //�̰� ������ ��������ü�� ��� �׷��� �������� ��¦�Ÿ�
                gotoxy(MAIN_X_ADJ + j, MAIN_Y_ADJ + i);
                switch (main_org[i][j]) {
                case EMPTY: //��ĭ���
                    printf("  ");
                    break;
                case CEILLING: //õ����
                    setColor(RED, WHITE);
                    printf(". ");
                    setColor(BLACK, WHITE);
                    break;
                case WALL: //����� ��
                    setColor(BLACK, WHITE);
                    printf("��");
                    setColor(BLACK, WHITE);
                    break;
                case INACTIVE_BLOCK: //���� �� ���  ��
                    setColor(GRAY, WHITE);
                    printf("��");
                    setColor(BLACK, WHITE);
                    break;
                case ACTIVE_BLOCK: //�����̰��ִ� �� ���  ��
                    active_color(b_type);
                    printf("��");
                    setColor(BLACK, WHITE);
                    break;
                case TRACE_BLOCK:
                    printf("��");
                }
            }
        }
    }

    for (i = 0; i < MAIN_Y; i++) { //�������� �׸� �� main_cpy�� ����  
        for (j = 0; j < MAIN_X; j++) { 
            main_cpy[i][j] = main_org[i][j];
        }
    }
}

void new_block(void) { //���ο� ��� ����  
    int i, j;

    bx = (MAIN_X / 2) - 1; //��� ���� ��ġx��ǥ(�������� ���) 
    by = 0;  //��� ������ġ y��ǥ(���� ��) 
    b_type = b_type_next; //���������� ������
    b_type_next = make_rand(); //���� ���� ���� 
    b_rotation = 0;  //ȸ���� 0������ ������ 

    new_block_on = 0; //new_block flag�� ��

    for (i = 0; i < 4; i++) { //������ bx, by��ġ�� ������  
        for (j = 0; j < 4; j++) {
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
        }
    }
    for (i = 1; i < 3; i++) { //���ӻ���ǥ�ÿ� ������ ���ú��� �׸� 
        for (j = 0; j < 4; j++) {
            active_color(b_type_next);
            if (blocks[b_type_next][0][i][j] == 1) {
                gotoxy(STATUS_X_ADJ + 2 + j, i + 6);
                printf("��");
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
    key = 0; //Ű�� �ʱ�ȭ  

    if (_kbhit()) { //Ű�Է��� �ִ� ���  
        key = _getch(); //Ű���� ����
        if (key == 224) { //����Ű�ΰ�� 
            do { key = _getch(); } while (key == 224);//����Ű���ð��� ���� 
            switch (key) {
            case LEFT: //����Ű ��������  
                if (check_crush(bx - 1, by, b_rotation) == true) move_block(LEFT);
                break;                            //�������� �� �� �ִ��� üũ �� �����ϸ� �̵�
            case RIGHT: //������ ����Ű ��������- ���� �����ϰ� ó���� 
                if (check_crush(bx + 1, by, b_rotation) == true) move_block(RIGHT);
                break;
            case DOWN: //�Ʒ��� ����Ű ��������-���� �����ϰ� ó���� 
                if (check_crush(bx, by + 1, b_rotation) == true) move_block(DOWN);
                break;
            case UP: //���� ����Ű �������� 
                if (check_crush(bx, by, (b_rotation + 1) % 4) == true) move_block(UP);
                //ȸ���� �� �ִ��� üũ �� �����ϸ� ȸ��
                else if (crush_on == 1 && check_crush(bx, by - 1, (b_rotation + 1) % 4) == true) move_block(100);
            }                    //�ٴڿ� ���� ��� �������� ��ĭ����� ȸ���� �����ϸ� �׷��� ��(Ư������)
        }
        else { //����Ű�� �ƴѰ�� 
            switch (key) {
            case SPACE: //�����̽�Ű �������� 
                space_key_on = 1; //�����̽�Ű flag�� ��� 
                while (crush_on == 0) { //�ٴڿ� ���������� �̵���Ŵ 
                    drop_block();
                    score += level; // hard drop ���ʽ�
                    gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score); //���� ǥ��
                }
                break;
            case P: //P(�빮��) �������� 
            case p: //p(�ҹ���) �������� 
                pause(); //�Ͻ����� 
                break;
            case ESC: //ESC�������� 
                system("cls"); //ȭ���� ����� 
                exit(0); //�������� 
            }
        }
    }
    while (_kbhit()) _getch(); //Ű���۸� ��� 
}

void drop_block(void) {
    int i, j;

    if (crush_on && check_crush(bx, by + 1, b_rotation) == true) crush_on = 0; //���� ��������� crush flag �� 
    if (crush_on && check_crush(bx, by + 1, b_rotation) == false) { //���� ��������ʰ� crush flag�� ���������� 
        for (i = 0; i < MAIN_Y; i++) { //���� �������� ���� ���� 
            for (j = 0; j < MAIN_X; j++) {
                if (main_org[i][j] == ACTIVE_BLOCK) main_org[i][j] = INACTIVE_BLOCK;
            }
        }
        crush_on = 0; //flag�� ��
        check_line(); //����üũ�� ��
        new_block_on = 1; //���ο� ������ flag�� �� 
        return; //�Լ� ����
    }
    if (check_crush(bx, by + 1, b_rotation) == true) move_block(DOWN); //���� ��������� ������ ��ĭ �̵� 
    if (check_crush(bx, by + 1, b_rotation) == false) crush_on++; //������ �̵��� �ȵǸ�  crush flag�� ��
}

int check_crush(int bx, int by, int b_rotation) { //������ ��ǥ�� ȸ�������� �浹�� �ִ��� �˻� 
    int i, j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) { //������ ��ġ�� �����ǰ� ������� ���ؼ� ��ġ�� false�� ���� 
            if (blocks[b_type][b_rotation][i][j] == 1 && main_org[by + i][bx + j] > 0) return false;
        }
    }
    return true; //�ϳ��� �Ȱ�ġ�� true���� 
};

void move_block(int dir) { //����� �̵���Ŵ 
    int i, j;

    switch (dir) {
    case LEFT: //���ʹ��� 
        for (i = 0; i < 4; i++) { //������ǥ�� ���� ���� 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        for (i = 0; i < 4; i++) { //�������� ��ĭ���� active block�� ���� 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j - 1] = ACTIVE_BLOCK;
            }
        }
        bx--; //��ǥ�� �̵� 
        break;

    case RIGHT:    //������ ����. ���ʹ����̶� ���� ������ ���� 
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

    case DOWN:    //�Ʒ��� ����. ���ʹ����̶� ���� ������ ����
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

    case UP: //Ű���� ���� �������� ȸ����Ŵ. 
        for (i = 0; i < 4; i++) { //������ǥ�� ���� ����  
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            }
        }
        b_rotation = (b_rotation + 1) % 4; //ȸ������ 1������Ŵ(3���� 4�� �Ǵ� ���� 0���� �ǵ���) 
        for (i = 0; i < 4; i++) { //ȸ���� ����� ���� 
            for (j = 0; j < 4; j++) {
                if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
            }
        }
        break;

    case 100: //����� �ٴ�, Ȥ�� �ٸ� ��ϰ� ���� ���¿��� ��ĭ���� �÷� ȸ���� ������ ��� 
              //�̸� ���۽�Ű�� Ư������ 
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

    int  block_amount; //������ ��ϰ����� �����ϴ� ���� 
    int combo = 0; //�޺����� �����ϴ� ���� ������ �ʱ�ȭ 

    for (i = MAIN_Y - 2; i > 3;) { //i=MAIN_Y-2 : ���ʺ��� ��ĭ����,  i>3 : õ��(3)�Ʒ����� �˻� 
        block_amount = 0; //��ϰ��� ���� ���� �ʱ�ȭ 
        for (j = 1; j < MAIN_X - 1; j++) { //���� �������� ��ϰ��縦 �� 
            if (main_org[i][j] > 0) block_amount++;
        }
        if (block_amount == MAIN_X - 2) { //����� ���� �� ��� 
            if (level_up_on == 0) { //���������°� �ƴ� ��쿡(�������� �Ǹ� �ڵ� �ٻ����� ����) 
                score += 100 * level; //�����߰� 
                cntl++; //���� �� ���� ī��Ʈ ���� 
                combo++; //�޺��� ����  
            }
            for (k = i; k > 1; k--) { //������ ��ĭ�� ��� ����(������ õ���� �ƴ� ��쿡��) 
                for (l = 1; l < MAIN_X - 1; l++) {
                    if (main_org[k - 1][l] != CEILLING) main_org[k][l] = main_org[k - 1][l];
                    if (main_org[k - 1][l] == CEILLING) main_org[k][l] = EMPTY;
                    //������ õ���� ��쿡�� õ���� ��ĭ ������ �ȵǴϱ� ��ĭ�� ���� 
                }
            }
        }
        else i--;
    }
    if (combo) { //�� ������ �ִ� ��� ������ ���� ��ǥ�� ���� ǥ����
        if (combo > 1) { //2�޺��̻��� ��� ��� ���ʽ��� �޼����� �����ǿ� ����ٰ� ����
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 1, MAIN_Y_ADJ + by - 2); printf("%d COMBO!", combo);
            Sleep(300);
            score += (combo * level * 100);
            reset_main_cpy(); //�ؽ�Ʈ�� ����� ���� main_cpy�� �ʱ�ȭ.
//(main_cpy�� main_org�� ���� �ٸ��Ƿ� ������ draw()ȣ��� ������ ��ü�� ���� �׸��� ��) 
        }
        gotoxy(STATUS_X_ADJ, STATUS_Y_GOAL); printf(" GOAL  : %5d", (cntl <= 10) ? 10 - cntl : 0);
        gotoxy(STATUS_X_ADJ, STATUS_Y_SCORE); printf("        %6d", score);
    }
}

void check_level_up(void) {
    int i, j;


    if (level <10 && cntl >= 10) { //�������� 10�پ� ���־���. 10���̻� ���� ���
        draw_main();
        level_up_on = 1; //������ flag�� ���
        level += 1; //������ 1 �ø�
        cntl = 0; //���� �ټ� �ʱ�ȭ

        for (i = 0; i < 2; i++) {
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 3, MAIN_Y_ADJ + 4);
            printf("             ");
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 2, MAIN_Y_ADJ + 6);
            printf("             ");
            Sleep(200);

            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 3, MAIN_Y_ADJ + 4);
            printf("��LEVEL UP!��");
            gotoxy(MAIN_X_ADJ + (MAIN_X / 2) - 2, MAIN_Y_ADJ + 6);
            printf("��SPEED UP!��");
            Sleep(200);
        }
        reset_main_cpy(); //�ؽ�Ʈ�� ����� ���� main_cpy�� �ʱ�ȭ.
//(main_cpy�� main_org�� ���� �ٸ��Ƿ� ������ draw()ȣ��� ������ ��ü�� ���� �׸��� ��)

        setColor(DARK_YELLOW, WHITE);
            for (i = MAIN_Y - 2; i > MAIN_Y - 2 - (level - 1); i--) { //�������������� �� ����-1�� ����ŭ �Ʒ��� ���� ������ 
                for (j = 1; j < MAIN_X - 1; j++) {
                    main_org[i][j] = INACTIVE_BLOCK; // ���� ������� ��� ä��� 
                    gotoxy(MAIN_X_ADJ + j, MAIN_Y_ADJ + i); // ���� �����.. �̻����̰� 
                    printf("��");
                    Sleep(5);
                }
            }
            setColor(BLACK, WHITE);
        Sleep(100); //�������� �����ֱ� ���� delay 
        check_line(); //������� ��� ä��� �����
//.check_line()�Լ� ���ο��� level up flag�� �����ִ� ��� ������ ����.         
        switch (level) { //�������� �ӵ��� ��������. 
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
        level_up_on = 0; //������ flag����

        gotoxy(STATUS_X_ADJ, STATUS_Y_LEVEL); printf(" LEVEL : %5d", level); //����ǥ�� 
        if (level < 10) {
            gotoxy(STATUS_X_ADJ, STATUS_Y_GOAL); 
            printf(" GOAL  : %5d", 10 - cntl); // ������ǥ ǥ�� 
        }

    }
}

void check_game_over(void) {
    int i;

    int x = 5;
    int y = 5;

    for (i = 1; i < MAIN_X - 2; i++) {
        if (main_org[3][i] > 0) { //õ��(������ ����° ��)�� inactive�� �����Ǹ� ���� ���� 
            gotoxy(x, y + 0); printf("�ǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢ�"); //���ӿ��� �޼��� 
            gotoxy(x, y + 1); printf("��                              ��");
            gotoxy(x, y + 2); printf("��  +-----------------------+   ��");
            gotoxy(x, y + 3); printf("��  |  G A M E  O V E R..   |   ��");
            gotoxy(x, y + 4); printf("��  +-----------------------+   ��");
            gotoxy(x, y + 5); printf("��   YOUR SCORE: %6d         ��", score);
            gotoxy(x, y + 6); printf("��                              ��");
            gotoxy(x, y + 7); printf("��  Press any key to restart..  ��");
            gotoxy(x, y + 8); printf("��                              ��");
            gotoxy(x, y + 9); printf("�ǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢ�");
            last_score = score; //���������� �ű� 

            if (score > best_score) { //�ְ��� ���Ž� 
               // FILE* file = fopen("score.dat", "wt"); //score.dat�� ���� ����                

                gotoxy(x, y + 6); printf("��  �ڡڡ� BEST SCORE! �ڡڡ�   ��  ");
                int el = 1;
                if (el == 0) { //���� �����޼���  
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

void pause(void) { //���� �Ͻ����� �Լ� 
    int i, j;

    int x = 5;
    int y = 5;

    for (i = 1; i < MAIN_X - 2; i++) { //���� �Ͻ����� �޼��� 
        gotoxy(x, y + 0); printf("�ǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢ�");
        gotoxy(x, y + 1); printf("��                              ��");
        gotoxy(x, y + 2); printf("��  +-----------------------+   ��");
        gotoxy(x, y + 3); printf("��  |       P A U S E       |   ��");
        gotoxy(x, y + 4); printf("��  +-----------------------+   ��");
        gotoxy(x, y + 5); printf("��  Press any key to resume..   ��");
        gotoxy(x, y + 6); printf("��  press A key to toggle mode  ��");
        gotoxy(x, y + 7); printf("�ǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢǢ�");
    }


    key = _getch(); //Ű���� ����
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


    system("cls"); //ȭ�� ����� ���� �׸� 
    reset_main_cpy();
    trace_block();
    draw_map();

    active_color(b_type_next);
    for (i = 1; i < 3; i++) { // ������� �׸� 
        for (j = 0; j < 4; j++) {
            if (blocks[b_type_next][0][i][j] == 1) {
                gotoxy(MAIN_X + MAIN_X_ADJ + 3 + j, i + 6);
                printf("��");
            }
            else {
                gotoxy(MAIN_X + MAIN_X_ADJ + 3 + j, i + 6);
                printf("  ");
            }
        }
    }
    setColor(BLACK, WHITE);
}

//���⼭���ʹ� ����ġ �˰����� ����ϴ� �κ���.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void find_best_pos(void)
{
    int stack_top[11] = { 0, };
    int cnt = 0; //rotation�� �����ϱ� ���� ī��Ʈ�Դϴ�.

    int best_rotation, best_bx;
    double score, best_score;
    b_rotation = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
    while (cnt < max_rotation()) {
        simulate_init_bx(); //bx�� ���� ���� ���ʿ� ��ġ��ŵ�ϴ�.
        int cnt2 = 0; //�������� �ʱ�ȭ ������ �ľ��ϱ� ���� ī��Ʈ �Դϴ�.
        while (check_crush(bx, by, b_rotation)) { //���������� ��ĭ�� �浹�� �Ͼ�� ���������� ���������� �̵��մϴ�
            by = 0; //ó�� y���� ������ 0�Դϴ�
            simulate_drop_block(); //����� ������ ������ �Լ��� �����մϴ�
            trace_top(stack_top); //�� ���̸� ���մϴ�

            score = score_height(stack_top) + score_floor_wall() + score_hole_blockade(stack_top) + score_line(); //���� ���� ��ġ���� ������ ���մϴ�
            //score = testScore(stack_top);
            if (cnt == 0 && cnt2 == 0) { //���� 1�� ������ �����, �������� �ʱ�ȭ ���ݴϴ�
                best_rotation = b_rotation;
                best_bx = bx;
                best_score = score;
            }
            if (score > best_score) { //���� ���ھ ����Ʈ ���ھ�� ���� ��, ���� ���ھ ����Ʈ ���ھ�� �����ѵ�, x��ǥ�� ȸ������ ����մϴ�
                best_rotation = b_rotation;
                best_bx = bx;
                best_score = score;
            }

            for (int i = 0; i < 4; i++) //����� �����, ����� ���� �׷ȴ� ����� �����ݴϴ�.
                for (int j = 0; j < 4; j++)
                    if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = EMPTY;
            bx++; //��ĭ ���������� �̵��մϴ�
            cnt2++; //�ʱⰪ �Ǻ����� ��ĭ �÷� ���Ŀ��� �ʱ� ���� ������ �������� �ʽ��ϴ�
            by = 0;
        }
        b_rotation++;
        cnt++; //�����̼� ī��Ʈ�� �ϳ� �ø��ϴ�
    }
    bx = best_bx; //���� �ùķ��̼� ������� x�� �����̼� ���� �����մϴ�.
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
    //�� x�� ��ġ�� ��� ���̸� ���ϴ� �Լ��Դϴ�.
    for (int i = 1; i < MAIN_X - 1; i++) {
        for (int j = 0; j < 25; j++)
            if (main_org[j][i] == WALL || main_org[j][i] == INACTIVE_BLOCK || main_org[j][i] == ACTIVE_BLOCK) { //���� Ȥ�� ��Ȱ��ȭ Ȥ�� Ȱ��ȭ ����� ������ ����ϴ�.
                stack_top[i] = j; //�̶�, �ش� ���� ������ ���̸� �ǹ��մϴ�.
                break;
            }
        stack_top[i] = MAIN_Y - 1 - stack_top[i]; //y���� ���̰� 22�̹Ƿ�, 22���� �츮�� ���� ���̸� ����, �ٴ����κ����� ���̸� ���� �� �ֽ��ϴ�.
    }
}

void simulate_init_bx(void)
{

    //�ʱⰪ���� ����� �� ���������� �ű�� �����Դϴ�.
    bx = 5;
    by = 0;
    while (check_crush(bx - 1, by, b_rotation)) //x���� ũ���� ���� ���� �� ����
        bx--; //��ǥ�� �̵� 
}

void simulate_drop_block(void)
{
    //����� ũ���ð� �� �� ���� ������ �����Դϴ�.
    while (check_crush(bx, by + 1, b_rotation))
        by++;
    //ũ���ð� �߻� �߱� ������, ������ x,y ��ǥ�� ����� ���� ������ ��ġ �Դϴ�.
    //����, �ش� ��ǥ�� Ȱ��ȭ�� ����� ���� ��ǥ�� �׷��ݴϴ�
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) main_org[by + i][bx + j] = ACTIVE_BLOCK;
}

double score_height(int* stack_top)
{
    //���̿� ���� ������ ����ϴ� �Լ��Դϴ�.
    int i, sumOfHeight = 0, sumOfSquareOfHeight = 0;
    int maxHeight = stack_top[1], minEdgeHeight = stack_top[1];
    double scoreOfSumOfHeight = 0.0, scoreOfSdOfHeight = 0.0, scoreOfDifferenceMaxMin = 0.0;
    double averageOfSumOfHeight = 0.0, averageOfSumOfSQHeight = 0.0;

    for (i = 1; i < MAIN_X - 1; i++) {//stack top�� �̿��ؼ� ��� ���̸� ���ع����ϴ�.
        sumOfHeight += stack_top[i];
        sumOfSquareOfHeight += stack_top[i] * stack_top[i]; //StandardDeviation�� ���ϱ� ���� sqr�� ���� ���մϴ�.
        maxHeight = (maxHeight > stack_top[i]) ? maxHeight : stack_top[i];
        if (i == 10 && minEdgeHeight > stack_top[i])
            minEdgeHeight = stack_top[i];
    }
    averageOfSumOfHeight = sumOfHeight / (double)10;
    averageOfSumOfSQHeight = sumOfSquareOfHeight / (double)10;

    scoreOfSumOfHeight = sumOfHeight * weight[_HEIGHT]; //��� ������ �հ� ����ġ�� ���� ����ȭ�մϴ�.
    scoreOfSdOfHeight = sqrt(averageOfSumOfSQHeight - averageOfSumOfHeight * averageOfSumOfHeight) * weight[_SD_HEIGHT];
    //���̿� ���� ǥ�����Ͽ� ����ġ�� ���� ����ȭ �մϴ�.
    scoreOfDifferenceMaxMin = ((double)maxHeight - (double)minEdgeHeight) * weight[_MAX_MIN_HEIGHT]; //�ִ���̿� ���������� ���̿� ����ġ�� ���Ͽ� ����ȭ�մϴ�.
    return scoreOfSumOfHeight + scoreOfSdOfHeight + scoreOfDifferenceMaxMin;
}

double score_hole_blockade(int* stack_top)
{
    //���۰� ������ ������ ���� ������ ���ϴ� �Լ��Դϴ�.
    //����� ������ �߻���, �ش� ���� ���� ���� ���� �����ϴ� Ȱ��ȭ Ȥ�� ��Ȱ��ȭ ��ϵ��� �� �Դϴ�.
    int i, j, k, tmp;
    double scoreOfHole = 0.0, scoreOfBlockade = 0.0;
    int countOfHoleOfColumm, countOfBlocksOfColumm;
    int countOfHole = 0, countOfBlockade = 0;
    for (i = 1; i < MAIN_X - 1; i++) {
        countOfHoleOfColumm = 0, countOfBlocksOfColumm = 0;
        for (j = MAIN_Y - 1; j >= MAIN_Y - 1 - stack_top[i]; j--) { //hole�� ������ ���ϱ� ���ؼ� stack_top ���� �����Ͽ� �� �� �ٴڱ��� empty�� �ֳ� �˻��մϴ�.
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
    //����� �ٴڸ鿡 ���� ������ ���ϴ� �Լ��Դϴ�.
    int i, j;
    double scoreOfFloor = 0.0, scoreOfWall = 0.0;
    int floorTouched = 0, wallTouched = 0;


    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (blocks[b_type][b_rotation][i][j] == 1) {
                int k = 0, l = -1; //k�� ������ ���� ����� ����, l�� ���� ���� ����� ���� ���˴ϴ�. �̶�, ���ʺ��� ����ϱ� ���� l�� -1�� ����˴ϴ�.
                for (int f = 0; f < 3; f++) {
                    switch (main_org[by + k + i][bx + l + j]) {
                    case WALL:
                        if (k == 1) floorTouched++; //k�� 1�̶�� ���� ������ �Ʒ������̶�� ���Դϴ�. ���� �ٴڿ� ���� ������ �������� �մϴ�.
                        else wallTouched++;//k�� 1�� �ƴϸ� ������ �����̶�� ���Դϴ�. ���� ���鿡 ���� ������ �������� �մϴ�.
                        break;
                    case INACTIVE_BLOCK:
                        if (k == 1) floorTouched++; //�ٴ��� ���� �ƴ϶� ��Ȱ��ȭ �� ��쿡�� �ٴڿ� ���� ������ �������� �մϴ�.
                        else wallTouched++;
                            //scoreOfFloor += 3.7;
                        break;
                    default:
                        break;
                    }
                    l++; //l�� -1���� �����ؼ� �ϳ��� ���������Ƿ� ���� ��꿡�� �����࿡ ���� ��������� �����ϴ�.
                    if (l == 0) k = 1; //�����࿡ ���� ��������� ���� ���, �׶��� ������ ���� �˻縦 �����ϱ� ���� ���̹Ƿ�, k�� 1�� �����Ͽ� �� ĭ �Ʒ����� �˻��մϴ�.
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
    int block_amount; //������ ��ϰ����� �����ϴ� ���� 
    double scoreOfLine = 0.0;
    int cnt = 0;

    for (i = MAIN_Y - 2; i > 3; i--) { //i=MAIN_Y-2 : ���ʺ��� ��ĭ����,  i>3 : õ��(3)�Ʒ����� �˻� 
        block_amount = 0; //��ϰ��� ���� ���� �ʱ�ȭ 
        for (j = 1; j < MAIN_X - 1; j++)  //���� �������� ��ϰ��縦 �� 
            if (main_org[i][j] == INACTIVE_BLOCK || main_org[i][j] == ACTIVE_BLOCK) block_amount++;
        if (block_amount == MAIN_X - 2)
            cnt++;
    }
    scoreOfLine = (double)cnt * (double)cnt * 100 * weight[_LINES];
    return scoreOfLine;
}