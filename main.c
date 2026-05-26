#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

// ===== 색상 코드 =====
#define RESET   "\033[0m"
#define YELLOW  "\033[1;33m"
#define GREEN   "\033[1;32m"
#define RED     "\033[1;31m"
#define CYAN    "\033[1;36m"
#define GRAY    "\033[90m"
#define WHITE   "\033[1;37m"

// ===== 키 코드 =====
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77
#define KEY_ENTER 13

// ===== 타이핑 효과 설정 =====
#define TYPE_DELAY      25    // 일반 글자 (밀리초)
#define TYPE_DELAY_SLOW 250   // 문장부호 (마침표, 쉼표, 물음표 등)
#define TYPE_DELAY_NONE 0     // ESC 코드 (색상) - 지연 없음

// 한 글자가 문장부호인지 판단
int is_punct(char c) {
    return (c == '.' || c == ',' || c == '!' || c == '?' || c == ':');
}

// 타이핑 효과로 출력 (한 글자씩)
// ESC 색상 코드는 한 번에 통과시킴 (\033 으로 시작하는 시퀀스)
void type_print(const char* text) {
    int i = 0;
    while (text[i]) {
        // ANSI 이스케이프 시퀀스는 통째로 즉시 출력
        if (text[i] == '\033') {
            while (text[i] && text[i] != 'm') {
                putchar(text[i]);
                i++;
            }
            if (text[i] == 'm') {
                putchar(text[i]);
                i++;
            }
            continue;
        }

        putchar(text[i]);
        fflush(stdout);  // 즉시 화면에 표시

        // 한글 1글자 = 3바이트 (UTF-8) 이므로 한 번에 처리
        unsigned char c = (unsigned char)text[i];
        if (c >= 0xE0) {
            // 한글 1글자 출력 (나머지 2바이트도 같이)
            i++;
            putchar(text[i]); i++;
            putchar(text[i]); i++;
            fflush(stdout);
            Sleep(TYPE_DELAY);
        }
        else {
            // 영문/기호: 문장부호면 느리게
            if (is_punct(text[i])) Sleep(TYPE_DELAY_SLOW);
            else Sleep(TYPE_DELAY);
            i++;
        }

        // 사용자가 키 누르면 즉시 스킵
        if (_kbhit()) {
            int k = _getch();
            if (k == KEY_ENTER || k == ' ') {
                // 남은 텍스트 한 번에 출력
                while (text[i]) {
                    putchar(text[i]);
                    i++;
                }
                fflush(stdout);
                break;
            }
        }
    }
    printf("\n");
}

// 깜빡임 효과 (text를 2번 깜빡임)
void blink_print(const char* text, int term_w, int display_width) {
    int pad = (term_w - display_width) / 2;
    if (pad < 0) pad = 0;

    for (int i = 0; i < 2; i++) {
        // 보이게
        printf("\r");
        for (int j = 0; j < pad; j++) printf(" ");
        printf("%s", text);
        fflush(stdout);
        Sleep(300);

        // 안 보이게 (공백으로 덮기)
        printf("\r");
        for (int j = 0; j < pad + display_width + 20; j++) printf(" ");
        fflush(stdout);
        Sleep(200);
    }
    // 마지막에 한 번 더 표시
    printf("\r");
    for (int j = 0; j < pad; j++) printf(" ");
    printf("%s\n", text);
    fflush(stdout);
}

// ===== 플레이어 상태 =====
int player_hp = 100;
int player_score = 0;
int player_lv = 1;

// ===== 터미널 정보 =====
// csbi.srWindow.Left + 1; 터미널 에서 인덱스값이 0부터 시작하기에 값을 뺄려면 +1 필요
//  CONSOLE_SCREEN_BUFFER_INFO csbi; 빈 화면( 터미널 ) 만들기
// GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi); 화면 (터미널)을 가리키는 손잡이 가져오기
// return csbi.srWindow.Right - csbi.srWindow.Left + 1; 화면의 가로 칸 수 계산
// return은 가로줄 칸 수

int get_terminal_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

// 위와 같은 방법으로 세로 칸 수 구하는 함수

int get_terminal_height() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

// ===== 정렬 함수 =====
void print_centered(const char* line, int term_w) {
    int line_len = 0;
    while (line[line_len]) line_len++;
    int pad = (term_w - line_len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s\n", line);
}

// 위에 터미널 정보를 바탕으로 한국어를 중앙 정렬 하기위한 함수
void print_centered_kr(const char* line, int term_w, int display_width) {
    int pad = (term_w - display_width) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s\n", line);
}

// ===== 박스 그리기 =====
// 박스 윗줄
void draw_box_top(int term_w, int box_w) {
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf(WHITE "+");
    for (int i = 0; i < box_w - 2; i++) printf("-");
    printf("+" RESET "\n");
}

// 박스 아랫줄
void draw_box_bottom(int term_w, int box_w) {
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf(WHITE "+");
    for (int i = 0; i < box_w - 2; i++) printf("-");
    printf("+" RESET "\n");
}

// 박스 안 텍스트 한 줄 (한글 표시 폭 직접 지정)
void draw_box_line_kr(const char* text, int term_w, int box_w, int display_width) {
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf(WHITE "|" RESET " %s", text);
    int inner = box_w - 4;
    int remain = inner - display_width;
    for (int i = 0; i < remain; i++) printf(" ");
    printf(WHITE " |" RESET "\n");
}

// 박스 안 텍스트를 타이핑 효과로 출력 (오른쪽 세로선 없이 한 줄 안에서 진행)
void type_box_line_kr(const char* text, int term_w, int box_w, int display_width) {
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;
    int inner = box_w - 4;
    int remain = inner - display_width;

    // 왼쪽 박스선 + 공백 먼저 출력
    for (int i = 0; i < pad; i++) printf(" ");
    printf(WHITE "|" RESET " ");
    fflush(stdout);

    // 타이핑 효과로 텍스트 출력
    int i = 0;
    int skipped = 0;
    while (text[i]) {
        // ANSI 이스케이프는 통째로
        if (text[i] == '\033') {
            while (text[i] && text[i] != 'm') {
                putchar(text[i]);
                i++;
            }
            if (text[i] == 'm') {
                putchar(text[i]);
                i++;
            }
            continue;
        }

        putchar(text[i]);
        fflush(stdout);

        unsigned char c = (unsigned char)text[i];
        if (c >= 0xE0) {
            // 한글 (3바이트)
            i++;
            putchar(text[i]); i++;
            putchar(text[i]); i++;
            fflush(stdout);
            if (!skipped) Sleep(TYPE_DELAY);
        }
        else {
            if (!skipped) {
                if (is_punct(text[i])) Sleep(TYPE_DELAY_SLOW);
                else Sleep(TYPE_DELAY);
            }
            i++;
        }

        // 스킵 처리
        if (!skipped && _kbhit()) {
            int k = _getch();
            if (k == KEY_ENTER || k == ' ') skipped = 1;
        }
    }

    // 오른쪽 패딩 + 박스선
    for (int j = 0; j < remain; j++) printf(" ");
    printf(WHITE " |" RESET "\n");
}

// 빈 박스 줄
void draw_box_empty(int term_w, int box_w) {
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf(WHITE "|" RESET);
    for (int i = 0; i < box_w - 2; i++) printf(" ");
    printf(WHITE "|" RESET "\n");
}

// ===== HP 바 그리기 =====
void draw_hp_bar(int term_w) {
    int bar_total = 20;  // 막대 칸 수
    int bar_fill = (player_hp * bar_total) / 100;
    if (bar_fill < 0) bar_fill = 0;

    // 전체 상태 줄 표시폭: "학생   LV 1   HP [====================] 100/100"
    int display_w = 50;
    int pad = (term_w - display_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");

    printf(WHITE "학생 " RESET);
    printf(YELLOW " LV %d " RESET, player_lv);
    printf(WHITE " HP " RESET);
    printf(RED "[");
    for (int i = 0; i < bar_total; i++) {
        if (i < bar_fill) printf(YELLOW "=");
        else printf(GRAY "-");
    }
    printf(RED "]" RESET);
    printf(YELLOW " %d/100" RESET, player_hp);
    printf(WHITE "   학점: " RESET);
    printf(GREEN "%d" RESET, player_score);
    printf("\n");
}

// ===== 텍스트 박스 (대사/상황 묘사) - 타이핑 효과 적용 =====
// 박스를 먼저 완전히 그린 후, 안쪽에만 글자를 채워 넣음 (박스 깨짐 방지)
void show_text_box(const char* lines[], int widths[], int line_count, int term_w) {
    int box_w = 70;
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;

    // 1) 박스 먼저 완전히 그리기 (빈 박스)
    draw_box_top(term_w, box_w);
    draw_box_empty(term_w, box_w);  // 위쪽 여백 줄
    for (int i = 0; i < line_count; i++) {
        draw_box_empty(term_w, box_w);  // 일단 비워둠
    }
    draw_box_empty(term_w, box_w);  // 아래쪽 여백 줄
    draw_box_bottom(term_w, box_w);

    // 2) 현재 커서가 박스 아래에 있음.
    //    위로 거슬러 올라가서 각 줄에 글자를 타이핑으로 채워넣기
    //    박스 전체 줄 수 = 1(상단) + 1(위공백) + line_count + 1(아래공백) + 1(하단) = line_count+4
    //    그 중 텍스트가 들어갈 줄은 (line_count+4)줄 중 위에서 3번째 줄부터

    // 박스 시작 줄로 이동: 위로 (line_count + 3) 줄 올라감
    //   현재 커서 = 박스 바로 아래
    //   line_count+3 = 하단선 + 아래공백 + 텍스트 줄들 + 위공백
    //   즉 위로 거슬러 가면 첫 텍스트 줄에 도달

    for (int i = 0; i < line_count; i++) {
        if (widths[i] == 0) continue;  // 빈 줄은 스킵

        // 위로 이동할 줄 수: 현재 위치(박스 아래)에서 i번째 텍스트 줄까지
        // = 하단선(1) + 아래공백(1) + (line_count - 1 - i) + 1 = line_count + 2 - i
        int up = line_count + 2 - i;

        // 커서를 정확히 (pad+3)번째 칸으로 이동
        // pad+1 = 박스 왼쪽 선 위치
        // pad+2 = 박스 안쪽 공백
        // pad+3 = 텍스트 시작 위치
        // ANSI: \033[N A = 위로 N줄, \033[N G = N번째 칸으로 이동 (1-based)
        printf("\033[%dA", up);                 // 위로 N줄
        printf("\033[%dG", pad + 3);            // 가로 위치 직접 지정

        // 타이핑 효과로 텍스트 출력
        int idx = 0;
        int skipped = 0;
        const char* text = lines[i];

        while (text[idx]) {
            // ANSI 이스케이프는 통째로
            if (text[idx] == '\033') {
                while (text[idx] && text[idx] != 'm') {
                    putchar(text[idx]);
                    idx++;
                }
                if (text[idx] == 'm') {
                    putchar(text[idx]);
                    idx++;
                }
                continue;
            }

            putchar(text[idx]);
            fflush(stdout);

            unsigned char c = (unsigned char)text[idx];
            if (c >= 0xE0) {
                // 한글 (3바이트)
                idx++;
                putchar(text[idx]); idx++;
                putchar(text[idx]); idx++;
                fflush(stdout);
                if (!skipped) Sleep(TYPE_DELAY);
            }
            else {
                if (!skipped) {
                    if (is_punct(text[idx])) Sleep(TYPE_DELAY_SLOW);
                    else Sleep(TYPE_DELAY);
                }
                idx++;
            }

            // 스킵 처리
            if (!skipped && _kbhit()) {
                int k = _getch();
                if (k == KEY_ENTER || k == ' ') skipped = 1;
            }
        }

        // 다음 줄 처리를 위해 다시 박스 아래로 커서 이동
        printf("\033[%dB", line_count + 2 - i);  // 아래로 N줄
        printf("\033[1G");                        // 가로 첫 칸으로
    }
}

// ===== 텍스트 박스 (즉시 출력 버전 - 선택지 화면 같은 곳용) =====
void show_text_box_instant(const char* lines[], int widths[], int line_count, int term_w) {
    int box_w = 70;
    draw_box_top(term_w, box_w);
    draw_box_empty(term_w, box_w);
    for (int i = 0; i < line_count; i++) {
        if (widths[i] == 0) {
            draw_box_empty(term_w, box_w);
        }
        else {
            draw_box_line_kr(lines[i], term_w, box_w, widths[i]);
        }
    }
    draw_box_empty(term_w, box_w);
    draw_box_bottom(term_w, box_w);
}

// ===== 선택지 박스 =====
// 4개 선택지, 2x2 배치
// 박스 너비 70, 안쪽 너비 66 (양쪽 | 와 공백 1칸씩 = 4 빼기)
// 한 칸 너비 = 33씩 두 칸
void show_choice_box(const char* choices[], int widths[], int selected, int term_w) {
    int box_w = 70;
    int inner_w = box_w - 2;     // | | 빼고 안쪽 = 68
    int slot_w = inner_w / 2;    // 한 칸 = 34
    int pad = (term_w - box_w) / 2;
    if (pad < 0) pad = 0;

    draw_box_top(term_w, box_w);
    draw_box_empty(term_w, box_w);

    // 2x2 선택지 출력
    for (int row = 0; row < 2; row++) {
        for (int i = 0; i < pad; i++) printf(" ");
        printf(WHITE "|" RESET);

        for (int col = 0; col < 2; col++) {
            int idx = row * 2 + col;
            // 슬롯 안 내용: "   > 텍스트" 또는 "     텍스트"
            // 좌측 패딩 3칸 + 화살표/공백 3칸 + 텍스트 + 우측 패딩
            int used = 0;
            // 좌측 패딩 3칸
            printf("   ");
            used += 3;
            // 화살표 또는 공백 (3칸 고정)
            if (idx == selected) {
                printf(YELLOW "> " RESET);
                used += 2;
            }
            else {
                printf("  ");
                used += 2;
            }
            // 텍스트
            if (idx == selected) printf(YELLOW "%s" RESET, choices[idx]);
            else printf(WHITE "%s" RESET, choices[idx]);
            used += widths[idx];

            // 나머지 공백으로 채우기
            int remain = slot_w - used;
            for (int j = 0; j < remain; j++) printf(" ");
        }
        printf(WHITE "|" RESET "\n");
    }

    draw_box_empty(term_w, box_w);
    draw_box_bottom(term_w, box_w);
}

// ===== 화면 초기화 + 위 여백 =====
void clear_screen_with_padding(int top_padding) {
    system("cls");
    for (int i = 0; i < top_padding; i++) printf("\n");
}

// ===== 결과 적용 =====
void apply_result(int hp_change, int score_change) {
    player_hp += hp_change;
    player_score += score_change;
    if (player_hp < 0) player_hp = 0;
    if (player_hp > 100) player_hp = 100;
}

// ===== 스테이지 1: 발표 지목 =====
void stage1() {
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 22) / 2;
    if (top < 0) top = 0;

    // ===== 0단계: 긴장감 조성 (깜빡임) =====
    clear_screen_with_padding(top + 8);
    Sleep(500);
    blink_print(RED "* 교수님이 당신을 노려본다." RESET, W, 24);
    Sleep(700);

    // ===== 1단계: 상황 묘사 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ STAGE 1 - 갑작스러운 발표 지목 ]", W, 36);
    printf(RESET);
    printf("\n");

    const char* intro_lines[] = {
        "* 수업 시작 5분... 졸음이 쏟아지는 그 순간,",
        "* 교수님의 눈빛이 당신을 향한다.",
        "",
        YELLOW "* 교수: \"거기 학생, 앞에 나와서 발표해봐요.\"" RESET,
        "",
        GRAY "* (준비한 것은 없다. 인생도 없다.)" RESET
    };
    int intro_widths[] = { 43, 32, 0, 44, 0, 34 };

    show_text_box(intro_lines, intro_widths, 6, W);
    printf("\n");
    draw_hp_bar(W);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 계속 ]", W, 22);
    printf(RESET);

    while (_getch() != KEY_ENTER);

    // ===== 2단계: 선택지 =====
    int selected = 0;
    int key;

    const char* choices[] = {
        "당당히 발표",
        "화장실 도망",
        "못 들은 척",
        "솔직하게"
    };
    int choice_widths[] = { 11, 11, 10, 8 };

    // 선택지 화면 - 상황 부여 끝났으니 즉시 출력
    while (1) {
        clear_screen_with_padding(top);

        printf(YELLOW);
        print_centered_kr("[ STAGE 1 - 갑작스러운 발표 지목 ]", W, 36);
        printf(RESET);
        printf("\n");

        const char* prompt_lines[] = {
            "* 어떻게 할 것인가?"
        };
        int prompt_widths[] = { 19 };

        // 선택지 화면은 항상 즉시 출력
        show_text_box_instant(prompt_lines, prompt_widths, 1, W);
        printf("\n");

        show_choice_box(choices, choice_widths, selected, W);
        printf("\n");

        draw_hp_bar(W);
        printf("\n");
        printf(GRAY);
        print_centered_kr("[ 방향키로 이동    엔터로 선택 ]", W, 34);
        printf(RESET);

        key = _getch();
        if (key == 224 || key == 0) {
            key = _getch();
            if (key == KEY_LEFT && selected % 2 == 1) selected--;
            else if (key == KEY_RIGHT && selected % 2 == 0) selected++;
            else if (key == KEY_UP && selected >= 2) selected -= 2;
            else if (key == KEY_DOWN && selected < 2) selected += 2;
        }
        else if (key == KEY_ENTER) break;
    }

    // ===== 3단계: 결과 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ STAGE 1 - 결과 ]", W, 18);
    printf(RESET);
    printf("\n");

    const char* result_msg = "";
    int result_msg_w = 0;
    int hp_change = 0;
    int score_change = 0;

    switch (selected) {
    case 0:
        result_msg = YELLOW "* 식은땀을 흘리며 나갔지만... 의외로 잘 넘겼다!" RESET;
        result_msg_w = 47;
        hp_change = -20; score_change = 30;
        break;
    case 1:
        result_msg = YELLOW "* 화장실에서 10분을 버텼다. 잠깐 살았다..." RESET;
        result_msg_w = 42;
        hp_change = 10; score_change = -10;
        break;
    case 2:
        result_msg = YELLOW "* 교수: \"거기, 핸드폰 그만보고 나와봐요.\"" RESET;
        result_msg_w = 41;
        hp_change = -10; score_change = -20;
        break;
    case 3:
        result_msg = YELLOW "* 솔직함을 높이 산 교수님이 가산점을 줬다!" RESET;
        result_msg_w = 42;
        hp_change = -30; score_change = 40;
        break;
    }

    char hp_line[64];
    char score_line[64];
    if (hp_change < 0)
        sprintf(hp_line, RED "* HP %d" RESET, hp_change);
    else
        sprintf(hp_line, GREEN "* HP +%d 회복!" RESET, hp_change);

    if (score_change > 0)
        sprintf(score_line, GREEN "* 학점 +%d 획득!" RESET, score_change);
    else
        sprintf(score_line, RED "* 학점 %d" RESET, score_change);

    const char* result_lines[] = {
        result_msg,
        "",
        hp_line,
        score_line
    };
    int result_widths[] = { result_msg_w, 0, 10, 14 };
    show_text_box(result_lines, result_widths, 4, W);

    // 실제 적용
    apply_result(hp_change, score_change);

    printf("\n");
    draw_hp_bar(W);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 계속 ]", W, 22);
    printf(RESET);

    while (_getch() != KEY_ENTER);

    // ===== 4단계: 스테이지 종료 =====
    clear_screen_with_padding(H / 2 - 3);
    printf(CYAN);
    blink_print(CYAN "STAGE 1 클리어!" RESET, W, 17);
    printf(RESET);
    printf("\n");
    print_centered_kr("다음 수업이 기다린다...", W, 24);
    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 돌아가기 ]", W, 26);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}

// ===== 타이틀 메뉴 관련 =====
void print_menu_item(const char* label, int term_w, int is_selected) {
    int display_width = 26;
    int pad = (term_w - display_width) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    if (is_selected) printf(YELLOW " > %s " RESET "\n", label);
    else printf(GRAY "   %s " RESET "\n", label);
}

void show_title(int selected) {
    system("cls");
    system("chcp 65001 > nul");
    int W = get_terminal_width();
    int H = get_terminal_height();
    int content_h = 34;
    int top_pad = (H - content_h) / 2;
    if (top_pad < 0) top_pad = 0;
    for (int i = 0; i < top_pad; i++) printf("\n");

    printf(YELLOW);
    print_centered("ooooo      ooo                oooooooooooo", W);
    print_centered("`888b.     `8'                `888'     `8", W);
    print_centered(" 8 `88b.    8   .ooooo.        888        ", W);
    print_centered(" 8   `88b.  8  d88' `88b       888oooo8   ", W);
    print_centered(" 8     `88b.8  888   888        888    \" ", W);
    print_centered(" 8       `888  888   888        888       ", W);
    print_centered("o8o        `8  `Y8bod8P'       o888o      ", W);
    printf(RESET);
    printf("\n");

    printf(RED);
    print_centered("ooooo      ooo                ooooo   ooooo                     oooo      .   oooo         ", W);
    print_centered("`888b.     `8'                `888'   `888'                     `888    .o8   `888         ", W);
    print_centered(" 8 `88b.    8   .ooooo.        888     888   .ooooo.   .oooo.    888  .o888oo  888 .oo.    ", W);
    print_centered(" 8   `88b.  8  d88' `88b       888ooooo888  d88' `88b `P  )88b   888    888    888P\"Y88b   ", W);
    print_centered(" 8     `88b.8  888   888       888     888  888ooo888  .oP\"888   888    888    888   888   ", W);
    print_centered(" 8       `888  888   888       888     888  888    .o d8(  888   888    888 .  888   888   ", W);
    print_centered("o8o        `8  `Y8bod8P'      o888o   o888o `Y8bod8P' `Y888\"\"8o o888o   \"888\" o888o o888o  ", W);
    printf(RESET);
    printf("\n");

    printf(CYAN);
    print_centered("~ No Health, No Life ~", W);
    printf(RESET);
    printf("\n");
    printf(GRAY);
    print_centered("================================================================", W);
    printf(RESET);
    printf("\n");

    print_menu_item("[ 1 ]   게임 시작     ", W, selected == 0);
    printf("\n");
    print_menu_item("[ 2 ]   플레이 소개   ", W, selected == 1);
    printf("\n");
    print_menu_item("[ 3 ]   팀 소개       ", W, selected == 2);
    printf("\n");
    print_menu_item("[ 4 ]   게임 종료     ", W, selected == 3);
    printf("\n");

    printf(GRAY);
    print_centered("================================================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("체력이 높으면 졸업  /  낮으면 대원가        ver 1.0", W, 50);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 화살표 위/아래 = 이동    엔터 = 선택 ]", W, 42);
    printf(RESET);
}

void pause_game_simple() {
    int W = get_terminal_width();
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 누르면 돌아가기 ]", W, 28);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}

void show_play_info() {
    system("cls");
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 20) / 2;
    if (top < 0) top = 0;
    for (int i = 0; i < top; i++) printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    print_centered_kr("       [ 플레이 소개 ]         ", W, 32);
    print_centered("================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("장르   : 텍스트 기반 턴제 게임", W, 31);
    print_centered_kr("목표   : 교수님과의 싸움에서 살아남기", W, 38);
    printf("\n");
    printf(YELLOW);
    print_centered_kr("[ 핵심 규칙 ]", W, 14);
    printf(RESET);
    printf("\n");
    print_centered_kr("체력이 높을수록  ->  학점 낮음", W, 32);
    print_centered_kr("체력이 낮을수록  ->  학점 높음", W, 32);
    printf("\n");
    printf(YELLOW);
    print_centered_kr("[ 조작법 ]", W, 11);
    printf(RESET);
    printf("\n");
    print_centered_kr("방향키   :  선택지 이동", W, 25);
    print_centered_kr("엔터     :  확인 / 진행", W, 25);
    printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    printf(RESET);
    pause_game_simple();
}

void show_team_info() {
    system("cls");
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 18) / 2;
    if (top < 0) top = 0;
    for (int i = 0; i < top; i++) printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    print_centered_kr("         [ 팀 소개 ]           ", W, 32);
    print_centered("================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("팀명   :  미정", W, 15);
    printf("\n");
    printf(YELLOW);
    print_centered_kr("[ 팀원 ]", W, 9);
    printf(RESET);
    printf("\n");
    print_centered_kr("기획 / 스토리   :  김바다", W, 26);
    print_centered_kr("프론트 / UI     :  강민", W, 25);
    print_centered_kr("백엔드 / 시스템 :  김언국", W, 26);
    printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("제작  :  2026", W, 14);
    print_centered_kr("버전  :  ver 1.0", W, 17);
    printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    printf(RESET);
    pause_game_simple();
}

// ===== 메인 =====
int main() {
    int selected = 0;
    int key;

    while (1) {
        show_title(selected);
        key = _getch();

        if (key == 224 || key == 0) {
            key = _getch();
            if (key == KEY_UP) selected = (selected - 1 + 4) % 4;
            else if (key == KEY_DOWN) selected = (selected + 1) % 4;
        }
        else if (key == KEY_ENTER) {
            if (selected == 0) {
                // 게임 시작 - 상태 초기화 후 스테이지 1
                player_hp = 100;
                player_score = 0;
                player_lv = 1;
                stage1();
            }
            else if (selected == 1) show_play_info();
            else if (selected == 2) show_team_info();
            else if (selected == 3) {
                system("cls");
                int W = get_terminal_width();
                int H = get_terminal_height();
                for (int i = 0; i < H / 2; i++) printf("\n");
                printf(RED);
                print_centered_kr("게임을 종료합니다...", W, 22);
                printf(RESET);
                printf("\n");
                break;
            }
        }
        else if (key >= '1' && key <= '4') {
            selected = key - '1';
        }
    }
    return 0;
}

