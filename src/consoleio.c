#include "global.h"
#include "consoleio.h"
#include <stdarg.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
/* Windows: 根据stdin类型选择读取方式 */
static int WinReadLine(char *buf, int size) {
    int fd = _fileno(stdin);

    if (_isatty(fd)) {
        /* 真实控制台: 用ReadConsoleW读取宽字符再转UTF-8 */
        wchar_t wbuf[512];
        DWORD nRead = 0;
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        if (ReadConsoleW(hIn, wbuf, 510, &nRead, NULL) && nRead > 0) {
            while (nRead > 0 && (wbuf[nRead - 1] == L'\n' || wbuf[nRead - 1] == L'\r'))
                nRead--;
            wbuf[nRead] = L'\0';
            int n = WideCharToMultiByte(CP_UTF8, 0, wbuf, (int)nRead, buf, size - 1, NULL, NULL);
            buf[n] = '\0';
            return (n > 0) ? 1 : 0;
        }
    }

    /* 管道模式(Trae终端): 用fgets读取 */
    if (fgets(buf, size, stdin) != NULL) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';
        return (len > 0) ? 1 : 0;
    }

    buf[0] = '\0';
    return 0;
}
#endif

/* 初始化控制台 */
void InitConsole(void) {
#ifdef _WIN32
    system("chcp 65001 > nul");
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    setlocale(LC_ALL, ".UTF8");
#else
    setlocale(LC_ALL, "");
#endif
}

/* 清屏 */
void ClearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

/* 设置文本颜色 */
void SetColor(const char *color) {
    printf("%s", color);
}

/* 重置文本颜色 */
void ResetColor(void) {
    printf("%s", COLOR_RESET);
}

/* 打印带颜色的文本 */
void PrintColor(const char *color, const char *format, ...) {
    printf("%s", color);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("%s", COLOR_RESET);
}

/* 打印成功信息 */
void PrintSuccess(const char *format, ...) {
    printf("  %s[成功]%s ", COLOR_GREEN, COLOR_RESET);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/* 打印错误信息 */
void PrintError(const char *format, ...) {
    printf("  %s[错误]%s ", COLOR_RED, COLOR_RESET);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/* 打印信息 */
void PrintInfo(const char *format, ...) {
    printf("  %s[信息]%s ", COLOR_CYAN, COLOR_RESET);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

/* 打印标题 */
void PrintTitle(const char *title, int width) {
    printf("\n  %s=== %s ===%s\n\n", COLOR_BOLD COLOR_CYAN, title, COLOR_RESET);
}

/* 打印菜单项 */
void PrintMenuItem(const char *key, const char *desc, const char *color) {
    printf("  %s[%s]%s %s\n", color, key, COLOR_RESET, desc);
}

/* 等待用户按键 */
void PressAnyKey(void) {
    printf("\n  %s按任意键继续...%s", COLOR_YELLOW, COLOR_RESET);
#ifdef _WIN32
    system("pause > nul");
#else
    getchar();
#endif
}

/* 读取整数输入 */
int ReadInt(const char *prompt, int min, int max) {
    int value;
    char buffer[100];

    while (1) {
        printf("  %s%s (%d-%d): %s", COLOR_CYAN, prompt, min, max, COLOR_RESET);
        fflush(stdout);

#ifdef _WIN32
        WinReadLine(buffer, sizeof(buffer));
#else
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            buffer[0] = '\0';
        } else {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
        }
#endif

        if (sscanf(buffer, "%d", &value) == 1) {
            if (value >= min && value <= max) {
                return value;
            }
        }
        PrintError("输入无效，请输入 %d 到 %d 之间的数字", min, max);
    }
}

/* 跨平台读取一行输入 */
int ReadLineUTF8(char *buf, int size) {
    fflush(stdout);

#ifdef _WIN32
    return WinReadLine(buf, size);
#else
    if (fgets(buf, size, stdin) == NULL) {
        buf[0] = '\0';
        return 0;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    return 1;
#endif
}

/* 打印景点图标 */
void PrintSpotIcon(SpotType type, const char *color) {
    switch (type) {
        case TYPE_ENTRANCE:
            printf("%s[入]%s", color, COLOR_RESET);
            break;
        case TYPE_ANIMAL_HOUSE:
            printf("%s[馆]%s", color, COLOR_RESET);
            break;
        case TYPE_SCENIC_SPOT:
            printf("%s[景]%s", color, COLOR_RESET);
            break;
        case TYPE_FACILITY:
            printf("%s[设]%s", color, COLOR_RESET);
            break;
        case TYPE_EDUCATION:
            printf("%s[教]%s", color, COLOR_RESET);
            break;
        default:
            printf("%s[?]%s", color, COLOR_RESET);
            break;
    }
}
