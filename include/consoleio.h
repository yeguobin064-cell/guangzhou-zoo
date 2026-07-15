#ifndef CONSOLEIO_H
#define CONSOLEIO_H

#include "global.h"

/* 初始化控制台 */
void InitConsole(void);

/* 清屏 */
void ClearScreen(void);

/* 设置文本颜色 */
void SetColor(const char *color);

/* 重置文本颜色 */
void ResetColor(void);

/* 打印带颜色的文本 */
void PrintColor(const char *color, const char *format, ...);

/* 打印成功信息 */
void PrintSuccess(const char *format, ...);

/* 打印错误信息 */
void PrintError(const char *format, ...);

/* 打印信息 */
void PrintInfo(const char *format, ...);

/* 打印标题 */
void PrintTitle(const char *title, int width);

/* 打印菜单项 */
void PrintMenuItem(const char *key, const char *desc, const char *color);

/* 等待用户按键 */
void PressAnyKey(void);

/* 读取用户输入 */
void ReadInput(const char *prompt, char *buffer, int size);

/* 读取整数输入 */
int ReadInt(const char *prompt, int min, int max);

/* 跨平台读取一行输入 */
int ReadLineUTF8(char *buf, int size);

/* 打印景点图标 */
void PrintSpotIcon(SpotType type, const char *color);

#endif /* CONSOLEIO_H */