# Team_work

콘솔 텍스트 어드벤처 게임 (C, Windows 전용). 개강총회, 발표 지목, 중간고사 세 스테이지를 지나 엔딩을 보는 구성입니다.

- `main.c` — 전체 소스 (타이틀 메뉴, 플레이어 정보 입력, 스테이지 1~3, 엔딩)
- 조작: 방향키(위/아래)로 선택, Enter로 확정

## 빌드 및 실행 (Windows, Visual Studio)

`windows.h`, `conio.h`를 사용하므로 Windows에서만 빌드됩니다. 개발자 명령 프롬프트에서:

```
cl /utf-8 main.c
main.exe
```

ANSI 색상 출력을 쓰므로 Windows Terminal 같은 ANSI 지원 콘솔을 권장합니다.
