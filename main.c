#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        } else {
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
// ===== 플레이어 상태 =====
char player_name[50] = "학생";
char player_id[20] = "00000000";
int player_hp = 100;
int player_score = 0;
int player_lv = 1;
int has_jokbo = 0;        // 족보 보유 여부 (0=없음, 1=있음)
int jokbo_used = 0;       // 족보 사용 여부 (0=미사용, 1=사용함)

// ===== 터미널 정보 =====
int get_terminal_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

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
        } else {
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
    int bar_total = 20;
    int bar_fill = (player_hp * bar_total) / 100;
    if (bar_fill < 0) bar_fill = 0;

    // 첫 번째 줄: 이름 (학번) - 가운데 정렬
    char name_line[80];
    sprintf(name_line, "%s ( %s )", player_name, player_id);
    
    // 표시 폭 계산 (한글 2, 영문/숫자 1)
    int name_w = 0;
    for (int i = 0; name_line[i]; i++) {
        unsigned char c = (unsigned char)name_line[i];
        if (c >= 0xE0) { name_w += 2; i += 2; }
        else name_w += 1;
    }
    
    int pad1 = (term_w - name_w) / 2;
    if (pad1 < 0) pad1 = 0;
    for (int i = 0; i < pad1; i++) printf(" ");
    printf(CYAN "%s" RESET "\n", name_line);

    // 두 번째 줄: LV / HP / 학점
    int display_w = 50;
    int pad = (term_w - display_w) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");

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
    
    // 족보 보유 표시
    if (has_jokbo && !jokbo_used) {
        printf(CYAN "   [족보]" RESET);
    } else if (has_jokbo && jokbo_used) {
        printf(GRAY "   [족보:사용됨]" RESET);
    }
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
            } else {
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
        } else {
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
// 선택지 박스 (정답 표시 옵션 추가)
// correct_idx: 정답 인덱스 (-1이면 정답 표시 안 함, 0~3이면 초록색 강조)
void show_choice_box_full(const char* choices[], int widths[], int selected, int term_w, int correct_idx) {
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
            int used = 0;
            // 좌측 패딩 3칸
            printf("   ");
            used += 3;
            // 화살표 또는 공백 (2칸 고정)
            if (idx == selected) {
                printf(YELLOW "> " RESET);
                used += 2;
            } else {
                printf("  ");
                used += 2;
            }
            // 텍스트 색상 결정 (우선순위: 정답표시 > 선택중 > 일반)
            if (correct_idx >= 0 && idx == correct_idx) {
                // 정답 - 초록색 강조 (족보 사용)
                printf(GREEN "%s" RESET, choices[idx]);
            } else if (idx == selected) {
                printf(YELLOW "%s" RESET, choices[idx]);
            } else {
                printf(WHITE "%s" RESET, choices[idx]);
            }
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

// 기존 함수 (이전 호출 호환용 - 정답 표시 없음)
void show_choice_box(const char* choices[], int widths[], int selected, int term_w) {
    show_choice_box_full(choices, widths, selected, term_w, -1);
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

// ===== 플레이어 정보 입력 화면 =====
void input_player_info() {
    system("cls");
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 14) / 2;
    if (top < 0) top = 0;
    for (int i = 0; i < top; i++) printf("\n");

    printf(CYAN);
    print_centered("========================================", W);
    print_centered_kr("       [ 학생 정보를 입력하세요 ]       ", W, 38);
    print_centered("========================================", W);
    printf(RESET);
    printf("\n");

    // 이름과 학번 한 줄에 입력 받기 (이름 학번 형식, 공백으로 구분)
    int input_pad = (W - 50) / 2;
    if (input_pad < 0) input_pad = 0;

    int student_id = 0;

    for (int i = 0; i < input_pad; i++) printf(" ");
    printf(YELLOW "   이름과 학번을 입력하세요 (예: 김바다 20241947)" RESET "\n");

    for (int i = 0; i < input_pad; i++) printf(" ");
    printf(YELLOW "   >> " RESET);

    // scanf로 이름(문자열)과 학번(정수)을 한 번에 받음
    scanf("%49s %d", player_name, &student_id);
    while (getchar() != '\n');  // 입력 버퍼 비우기

    // 학번을 문자열로 변환 (8자리 형식으로)
    sprintf(player_id, "%08d", student_id);

    printf("\n");
    printf(CYAN);
    print_centered("========================================", W);
    printf(RESET);
    printf("\n");

    // 환영 메시지
    printf(GREEN);
    char welcome[100];
    sprintf(welcome, "환영합니다, %s 학생! (%s)", player_name, player_id);
    // 표시 폭 계산 (대략)
    int wlen = 0;
    for (int i = 0; welcome[i]; i++) {
        unsigned char c = (unsigned char)welcome[i];
        if (c >= 0xE0) { wlen += 2; i += 2; }
        else wlen++;
    }
    print_centered_kr(welcome, W, wlen);
    printf(RESET);
    printf("\n");

    print_centered_kr("[ 엔터를 눌러 게임을 시작합니다 ]", W, 33);
    while (_getch() != KEY_ENTER);
}

// ===== 플레이어 정보 입력 =====
void player_setup() {
    system("cls");
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 15) / 2;
    if (top < 0) top = 0;
    for (int i = 0; i < top; i++) printf("\n");

    printf(YELLOW);
    print_centered_kr("=== 학생 정보 등록 ===", W, 22);
    printf(RESET);
    printf("\n");

    print_centered_kr("학교에서 당신을 기록합니다.", W, 28);
    print_centered_kr("이름과 학번 8자리를 공백으로 구분해 입력하세요.", W, 48);
    printf("\n");
    printf(GRAY);
    print_centered_kr("(예: 김바다 20241947  또는  Kim 20241947)", W, 42);
    printf(RESET);
    printf("\n");

    // === 이름과 학번 한 번에 입력 ===
    int pad = (W - 50) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) printf(" ");
    printf(CYAN "이름 학번 >> " RESET);
    
    // 학번이 정확히 8자리가 될 때까지 재입력 받기
    while (1) {
        if (scanf("%31s %8s", player_name, player_id) == 2) {
            // 학번이 8자리이고 모두 숫자인지 확인
            int id_len = 0;
            int is_valid = 1;
            while (player_id[id_len]) {
                if (player_id[id_len] < '0' || player_id[id_len] > '9') is_valid = 0;
                id_len++;
            }
            while (getchar() != '\n');  // 버퍼 비우기
            
            if (id_len == 8 && is_valid) break;  // 유효한 입력
        } else {
            while (getchar() != '\n');
        }
        
        // 잘못된 입력
        for (int i = 0; i < pad; i++) printf(" ");
        printf(RED "다시 입력 >> " RESET);
    }

    printf("\n");

    // 등록 완료 메시지
    char welcome[100];
    sprintf(welcome, "%s (%s) 학생, 등록 완료!", player_name, player_id);
    int wlen = 0;
    for (int i = 0; welcome[i]; i++) {
        unsigned char c = (unsigned char)welcome[i];
        if (c >= 0xE0) { wlen += 2; i += 2; }
        else wlen += 1;
    }
    int wpad = (W - wlen) / 2;
    if (wpad < 0) wpad = 0;
    for (int i = 0; i < wpad; i++) printf(" ");
    printf(GREEN "%s" RESET "\n", welcome);

    printf("\n");
    print_centered_kr("[ 엔터를 눌러 수업에 들어가기 ]", W, 32);

    while (_getch() != KEY_ENTER);
}

// ===== 스테이지 1: 개강총회 =====
void stage1() {
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 22) / 2;
    if (top < 0) top = 0;

    // ===== 0단계: 긴장감 조성 =====
    clear_screen_with_padding(top + 8);
    Sleep(500);
    blink_print(YELLOW "* 처음 보는 동기, 선배들이 잔을 들고 있다." RESET, W, 38);
    Sleep(700);

    // ===== 1단계: 상황 묘사 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ STAGE 1 - 개강총회 ]", W, 22);
    printf(RESET);
    printf("\n");

    const char* intro_lines[] = {
        "* 신입생 환영 개강총회 날.",
        "* 오늘 처음 보는 동기들과 선배들이 가득하다.",
        "",
        YELLOW "* 선배: \"신입생! 한 잔 받아!\"" RESET,
        "",
        GRAY "* (술을 제량껏 마셔야겠다...)" RESET
    };
    int intro_widths[] = {18, 42, 0, 26, 0, 28};

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
        "1차만 가기",
        "2차까지 가기",
        "선배들과 끝까지",
        "얼굴도장만"
    };
    int choice_widths[] = {10, 12, 15, 10};

    while (1) {
        clear_screen_with_padding(top);

        printf(YELLOW);
        print_centered_kr("[ STAGE 1 - 개강총회 ]", W, 22);
        printf(RESET);
        printf("\n");

        const char* prompt_lines[] = {
            "* 어디까지 갈 것인가?"
        };
        int prompt_widths[] = {21};
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
    int got_jokbo = 0;

    switch (selected) {
        case 0:
            result_msg = YELLOW "* 1차만 깔끔하게 마치고 귀가했다." RESET;
            result_msg_w = 33;
            hp_change = 0; score_change = 0;
            break;
        case 1:
            result_msg = YELLOW "* 2차까지 갔다. 머리가 좀 아프지만 살만하다." RESET;
            result_msg_w = 44;
            hp_change = -15; score_change = 0;
            break;
        case 2:
            result_msg = YELLOW "* 선배들과 끝까지 마셨다! 족보를 얻었다!" RESET;
            result_msg_w = 40;
            hp_change = -40; score_change = 0;
            got_jokbo = 1;
            break;
        case 3:
            result_msg = YELLOW "* 도망가려다 잡혀버렸다. 술자리가 끝장났다..." RESET;
            result_msg_w = 45;
            hp_change = -50; score_change = 0;
            break;
    }

    char hp_line[64];
    if (hp_change < 0)
        sprintf(hp_line, RED "* HP %d" RESET, hp_change);
    else
        sprintf(hp_line, GREEN "* HP 변동 없음" RESET);

    const char* result_lines[5];
    int result_widths[5];
    int line_count = 3;

    result_lines[0] = result_msg;
    result_widths[0] = result_msg_w;
    result_lines[1] = "";
    result_widths[1] = 0;
    result_lines[2] = hp_line;
    result_widths[2] = (hp_change < 0) ? 10 : 14;

    if (got_jokbo) {
        result_lines[3] = CYAN "* 족보 획득! (시험에서 1회 사용 가능)" RESET;
        result_widths[3] = 36;
        line_count = 4;
    }

    show_text_box(result_lines, result_widths, line_count, W);

    apply_result(hp_change, score_change);
    if (got_jokbo) has_jokbo = 1;

    printf("\n");
    draw_hp_bar(W);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 계속 ]", W, 22);
    printf(RESET);

    while (_getch() != KEY_ENTER);

    // ===== 4단계: 스테이지 종료 =====
    clear_screen_with_padding(H / 2 - 3);
    blink_print(CYAN "STAGE 1 클리어!" RESET, W, 17);
    printf("\n");
    print_centered_kr("내일은 첫 수업이다...", W, 22);
    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 다음 스테이지 ]", W, 30);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}

// ===== 스테이지 2: 발표 지목 =====
void stage2() {
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 22) / 2;
    if (top < 0) top = 0;

    // ===== 0단계: 긴장감 조성 (깜빡임만) =====
    clear_screen_with_padding(top + 8);
    Sleep(500);
    blink_print(RED "* 교수님이 당신을 노려본다." RESET, W, 24);
    Sleep(700);

    // ===== 1단계: 상황 묘사 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ STAGE 2 - 갑작스러운 발표 지목 ]", W, 36);
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
    int intro_widths[] = {43, 32, 0, 44, 0, 34};

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
    int choice_widths[] = {11, 11, 10, 8};

    // 선택지 화면 - 상황 부여 끝났으니 즉시 출력
    while (1) {
        clear_screen_with_padding(top);

        printf(YELLOW);
        print_centered_kr("[ STAGE 2 - 갑작스러운 발표 지목 ]", W, 36);
        printf(RESET);
        printf("\n");

        const char* prompt_lines[] = {
            "* 어떻게 할 것인가?"
        };
        int prompt_widths[] = {19};

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
    print_centered_kr("[ STAGE 2 - 결과 ]", W, 18);
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
    int result_widths[] = {result_msg_w, 0, 10, 14};
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
    blink_print(CYAN "STAGE 2 클리어!" RESET, W, 17);
    printf(RESET);
    printf("\n");
    print_centered_kr("중간고사가 다가온다...", W, 22);
    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 돌아가기 ]", W, 26);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}

// ===== 시험 문제 하나 출제 =====
// 정답 인덱스를 받아서, 맞으면 1 반환
int ask_question(const char* title, const char* question_lines[], int question_widths[], int q_line_count,
                 const char* choices[], int choice_widths[], int correct_idx) {
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 24) / 2;
    if (top < 0) top = 0;

    int selected = 0;
    int key;
    int jokbo_used_here = 0;   // 이 문제에서 족보를 사용했는지

    while (1) {
        clear_screen_with_padding(top);

        printf(YELLOW);
        print_centered_kr(title, W, 30);
        printf(RESET);
        printf("\n");

        // 박스 깨짐 방지를 위해 항상 즉시 출력
        show_text_box_instant(question_lines, question_widths, q_line_count, W);
        printf("\n");

        // 이 문제에서 족보 썼으면 정답 인덱스 넘김
        int show_answer = jokbo_used_here ? correct_idx : -1;
        show_choice_box_full(choices, choice_widths, selected, W, show_answer);
        printf("\n");

        draw_hp_bar(W);
        printf("\n");

        // 족보 사용 가능 안내
        if (has_jokbo && !jokbo_used) {
            printf(CYAN);
            print_centered_kr("[ J 키: 족보 사용하기 (1회 한정) ]", W, 36);
            printf(RESET);
        } else if (jokbo_used_here) {
            printf(GREEN);
            print_centered_kr("[ 족보가 정답을 알려준다! ]", W, 28);
            printf(RESET);
        }
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
        else if (key == 'j' || key == 'J') {
            // 족보 사용 - 정답 선택지를 초록색으로 강조
            if (has_jokbo && !jokbo_used) {
                jokbo_used = 1;
                jokbo_used_here = 1;
                clear_screen_with_padding(top + 5);
                blink_print(CYAN "[ 족보를 펼쳤다! ]" RESET, W, 20);
                Sleep(500);
            }
        }
        else if (key == KEY_ENTER) break;
    }

    // 결과 표시
    clear_screen_with_padding(top + 5);
    int correct = (selected == correct_idx);

    if (correct) {
        printf(GREEN);
        blink_print(GREEN "* 정답이다!" RESET, W, 12);
        printf(RESET);
    } else {
        printf(RED);
        blink_print(RED "* 오답이다..." RESET, W, 14);
        printf(RESET);
        char correct_msg[80];
        sprintf(correct_msg, "정답은 %d번이었다.", correct_idx + 1);
        int clen = 0;
        for (int i = 0; correct_msg[i]; i++) {
            unsigned char c = (unsigned char)correct_msg[i];
            if (c >= 0xE0) { clen += 2; i += 2; }
            else clen += 1;
        }
        printf("\n");
        print_centered_kr(correct_msg, W, clen);
    }
    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 계속 ]", W, 22);
    printf(RESET);
    while (_getch() != KEY_ENTER);

    return correct;
}

// ===== 스테이지 3: 중간고사 =====
void stage3() {
    int W = get_terminal_width();
    int H = get_terminal_height();
    int top = (H - 22) / 2;
    if (top < 0) top = 0;

    // ===== 0단계: 시험 시작 =====
    clear_screen_with_padding(top + 8);
    Sleep(500);
    blink_print(RED "* 중간고사 시험지가 배포된다." RESET, W, 28);
    Sleep(700);

    // ===== 1단계: 상황 묘사 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ STAGE 3 - 중간고사 ]", W, 22);
    printf(RESET);
    printf("\n");

    const char* intro_lines[] = {
        "* 드디어 중간고사 날이다.",
        "* 시험지를 받아들고 한숨이 나온다.",
        "",
        YELLOW "* 교수: \"시작!\"" RESET,
        "",
        GRAY "* (총 3문제. 맞춰야 학점이 안 깎인다.)" RESET
    };
    int intro_widths[] = {16, 28, 0, 14, 0, 38};

    show_text_box(intro_lines, intro_widths, 6, W);
    printf("\n");
    draw_hp_bar(W);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 시험 시작 ]", W, 26);
    printf(RESET);

    while (_getch() != KEY_ENTER);

    int correct_count = 0;

    // ===== 문제 1: stdio.h 철자 =====
    const char* q1_lines[] = {
        "* 다음 중 올바른 헤더 파일은?"
    };
    int q1_widths[] = {28};
    const char* q1_choices[] = {
        "studio.h", "stdio.h", "stido.h", "stdoi.h"
    };
    int q1_choice_widths[] = {8, 7, 7, 7};
    if (ask_question("[ 문제 1 / 3 - 헤더 파일 ]", q1_lines, q1_widths, 1,
                     q1_choices, q1_choice_widths, 1)) {
        correct_count++;
    } else {
        player_score -= 15;
    }

    // ===== 문제 2: GitHub 업로드 순서 =====
    const char* q2_lines[] = {
        "* GitHub에 업로드하는 올바른 순서는?"
    };
    int q2_widths[] = {36};
    const char* q2_choices[] = {
        "add -> push -> commit",
        "commit -> add -> push",
        "add -> commit -> push",
        "push -> commit -> add"
    };
    int q2_choice_widths[] = {21, 21, 21, 21};
    if (ask_question("[ 문제 2 / 3 - GitHub 업로드 ]", q2_lines, q2_widths, 1,
                     q2_choices, q2_choice_widths, 2)) {
        correct_count++;
    } else {
        player_score -= 15;
    }

    // ===== 문제 3: scanf vs _getch =====
    const char* q3_lines[] = {
        "* scanf와 _getch의 차이로 옳은 것은?"
    };
    int q3_widths[] = {36};
    const char* q3_choices[] = {
        "scanf는 즉시 입력",
        "_getch는 엔터 필요",
        "_getch는 즉시 입력",
        "둘 다 동일하다"
    };
    int q3_choice_widths[] = {15, 17, 17, 13};
    if (ask_question("[ 문제 3 / 3 - 입력 함수 ]", q3_lines, q3_widths, 1,
                     q3_choices, q3_choice_widths, 2)) {
        correct_count++;
    } else {
        player_score -= 15;
    }

    // ===== 결과 발표 =====
    clear_screen_with_padding(top);

    printf(YELLOW);
    print_centered_kr("[ 중간고사 결과 ]", W, 18);
    printf(RESET);
    printf("\n");

    char result_msg[80];
    sprintf(result_msg, "* %d / 3 문제 정답!", correct_count);
    int rlen = 0;
    for (int i = 0; result_msg[i]; i++) {
        unsigned char c = (unsigned char)result_msg[i];
        if (c >= 0xE0) { rlen += 2; i += 2; }
        else rlen += 1;
    }

    const char* final_lines[3];
    int final_widths[3];

    final_lines[0] = result_msg;
    final_widths[0] = rlen;
    final_lines[1] = "";
    final_widths[1] = 0;

    if (correct_count == 3) {
        final_lines[2] = GREEN "* 완벽하다! 학점 손실 없음!" RESET;
        final_widths[2] = 26;
    } else if (correct_count == 2) {
        final_lines[2] = YELLOW "* 한 문제 틀렸다. 학점 -15" RESET;
        final_widths[2] = 25;
    } else if (correct_count == 1) {
        final_lines[2] = RED "* 두 문제 틀렸다. 학점 -30" RESET;
        final_widths[2] = 26;
    } else {
        final_lines[2] = RED "* 다 틀렸다. 학점 -45" RESET;
        final_widths[2] = 21;
    }

    show_text_box(final_lines, final_widths, 3, W);

    printf("\n");
    draw_hp_bar(W);
    printf("\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 계속 ]", W, 22);
    printf(RESET);
    while (_getch() != KEY_ENTER);

    // ===== 스테이지 종료 =====
    clear_screen_with_padding(H / 2 - 3);
    blink_print(CYAN "STAGE 3 클리어!" RESET, W, 17);
    printf("\n");
    print_centered_kr("한 학기가 끝났다...", W, 20);
    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 결과 확인 ]", W, 26);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}

// ===== 엔딩 화면 =====
void show_ending() {
    int W = get_terminal_width();
    int H = get_terminal_height();
    system("cls");
    for (int i = 0; i < H / 4; i++) printf("\n");

    printf(YELLOW);
    print_centered_kr("=== 한 학기를 마치며 ===", W, 24);
    printf(RESET);
    printf("\n\n");

    // 최종 결과
    char hp_msg[60], score_msg[60];
    sprintf(hp_msg, "최종 체력: %d / 100", player_hp);
    sprintf(score_msg, "최종 학점: %d", player_score);
    print_centered_kr(hp_msg, W, 20);
    print_centered_kr(score_msg, W, 16);
    printf("\n");

    // 엔딩 분기
    if (player_hp >= 50 && player_score >= 50) {
        printf(GREEN);
        print_centered_kr("[ TRUE END: 졸업 ]", W, 20);
        printf(RESET);
        printf("\n");
        print_centered_kr("체력도 학점도 챙긴 당신!", W, 26);
        print_centered_kr("무사히 졸업합니다.", W, 18);
    } else if (player_hp < 50 && player_score >= 50) {
        printf(YELLOW);
        print_centered_kr("[ BAD END: 대원가 ]", W, 21);
        printf(RESET);
        printf("\n");
        print_centered_kr("학점은 좋았지만 몸이 망가졌다.", W, 30);
        print_centered_kr("입대 통지서가 도착했다...", W, 26);
    } else if (player_hp >= 50 && player_score < 50) {
        printf(CYAN);
        print_centered_kr("[ NORMAL END: 휴학 ]", W, 22);
        printf(RESET);
        printf("\n");
        print_centered_kr("몸은 멀쩡하지만 학점이 부족하다.", W, 32);
        print_centered_kr("일단 휴학하기로 했다.", W, 22);
    } else {
        printf(RED);
        print_centered_kr("[ WORST END: 제적 ]", W, 21);
        printf(RESET);
        printf("\n");
        print_centered_kr("체력도, 학점도 모두 잃었다...", W, 30);
        print_centered_kr("학교를 떠나야 할 시간이다.", W, 26);
    }

    printf("\n\n");
    printf(GREEN);
    print_centered_kr("[ 엔터를 눌러 타이틀로 ]", W, 26);
    printf(RESET);
    while (_getch() != KEY_ENTER);
}


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
    printf("\n\n");

    printf(GRAY);
    print_centered("================================================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("체력이 높으면 졸업  /  낮으면 대원가        ver 1.0", W, 50);
    printf("\n\n");
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
    print_centered_kr("프론트 / UI     :  강민",   W, 25);
    print_centered_kr("백엔드 / 시스템 :  김언국", W, 26);
    printf("\n");
    printf(CYAN);
    print_centered("================================", W);
    printf(RESET);
    printf("\n");
    print_centered_kr("제작  :  2026",   W, 14);
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
                // 게임 시작 - 상태 초기화 후 플레이어 정보 입력 → 전체 스테이지
                player_hp = 100;
                player_score = 50;   // 기본 학점 50점 시작 (감점/획득용)
                player_lv = 1;
                has_jokbo = 0;
                jokbo_used = 0;
                player_setup();      // 이름/학번 입력
                stage1();            // 개강총회
                stage2();            // 발표 지목
                stage3();            // 중간고사
                show_ending();       // 엔딩
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
