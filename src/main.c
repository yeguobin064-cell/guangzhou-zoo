#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "global.h"
#include "graph.h"
#include "consoleio.h"

/* 打印主菜单 */
static void PrintMainMenu(void) {
    printf("\n");
    printf("  %s=== 广州动物园路线规划系统 ===%s\n\n", COLOR_BOLD COLOR_CYAN, COLOR_RESET);

    PrintMenuItem("1", "查看园区地图", COLOR_GREEN);
    PrintMenuItem("2", "查看景点列表", COLOR_GREEN);
    PrintMenuItem("3", "查看景点详情", COLOR_GREEN);
    PrintMenuItem("4", "查询最短路径", COLOR_CYAN);
    PrintMenuItem("5", "规划游览路线", COLOR_CYAN);
    PrintMenuItem("6", "DFS导游线路", COLOR_MAGENTA);
    PrintMenuItem("7", "检测回路", COLOR_MAGENTA);
    PrintMenuItem("8", "查看所有路径", COLOR_YELLOW);
    printf("\n  %s--- 景点管理 ---%s\n", COLOR_CYAN, COLOR_RESET);
    PrintMenuItem("9", "添加景点", COLOR_GREEN);
    PrintMenuItem("10", "删除景点", COLOR_RED);
    PrintMenuItem("11", "查询景点", COLOR_CYAN);
    PrintMenuItem("12", "修改景点", COLOR_YELLOW);
    printf("\n  %s--- 智能推荐 ---%s\n", COLOR_CYAN, COLOR_RESET);
    PrintMenuItem("13", "随机路线(含厕所)", COLOR_MAGENTA);
    PrintMenuItem("0", "退出系统", COLOR_RED);

    printf("\n");
}

/* 主函数 */
int main(void) {
    ALGraph graph;
    int choice;

    /* 初始化控制台 */
    InitConsole();

    /* 显示欢迎信息 */
    ClearScreen();
    printf("\n  %s广州动物园智能路线规划系统%s\n", COLOR_BOLD COLOR_GREEN, COLOR_RESET);
    printf("  %s支持园区地图、路径查询、路线规划等功能%s\n\n", COLOR_CYAN, COLOR_RESET);

    /* 初始化广州动物园数据 */
    PrintInfo("正在加载数据...");
    Status status = InitGuangzhouZoo(&graph);
    if (status != OK) {
        PrintError("数据加载失败!");
        return 1;
    }
    PrintSuccess("加载完成! %d个景点, %d条路径", graph.numVertex, graph.numEdge);
    PressAnyKey();

    /* 主循环 */
    do {
        ClearScreen();
        PrintMainMenu();

        choice = ReadInt("请输入功能编号", 0, 13);

        switch (choice) {
            case 1:
                ClearScreen();
                PrintTitle("园区地图", 60);
                ShowGraph(&graph);
                PressAnyKey();
                break;

            case 2:
                ClearScreen();
                PrintTitle("景点列表", 60);
                ShowVertex(&graph);
                PressAnyKey();
                break;

            case 3:
                ClearScreen();
                PrintTitle("景点详情", 60);
                ShowVertex(&graph);
                int spotIdx = ReadInt("请输入景点编号", 1, graph.numVertex) - 1;
                ShowSpotDetail(&graph, spotIdx);
                PressAnyKey();
                break;

            case 4:
                ClearScreen();
                PrintTitle("查询最短路径", 60);
                ShowVertex(&graph);
                int s1 = ReadInt("请输入起点编号", 1, graph.numVertex) - 1;
                int s2 = ReadInt("请输入终点编号", 1, graph.numVertex) - 1;
                QueryShortestPath(&graph, graph.roadList[s1].name, graph.roadList[s2].name);
                PressAnyKey();
                break;

            case 5:
                ClearScreen();
                PrintTitle("规划游览路线", 60);
                ShowVertex(&graph);
                int startIdx = ReadInt("请输入起始景点编号", 1, graph.numVertex) - 1;
                int timeLimit = ReadInt("请输入游览时间限制（分钟）", 30, 480);
                PlanBestRoute(&graph, startIdx, timeLimit);
                PressAnyKey();
                break;

            case 6:
                ClearScreen();
                PrintTitle("DFS导游线路", 60);
                ShowVertex(&graph);
                int tourStart = ReadInt("请输入起始景点编号", 1, graph.numVertex) - 1;
                TourRoute(&graph, tourStart);
                PressAnyKey();
                break;

            case 7:
                ClearScreen();
                PrintTitle("检测回路", 60);
                FindCycle(&graph);
                PressAnyKey();
                break;

            case 8:
                ClearScreen();
                PrintTitle("所有景点间最短路径", 60);

                int idx1, idx2;
                PathResult pr;

                printf("\n");
                for (idx1 = 0; idx1 < graph.numVertex; idx1++) {
                    for (idx2 = idx1 + 1; idx2 < graph.numVertex; idx2++) {
                        if (Dijkstra(&graph, idx1, idx2, &pr) == OK) {
                            printf("  %s%s", COLOR_CYAN, graph.roadList[idx1].name);
                            for (int k = 1; k < pr.pathLength; k++) {
                                printf(" %s->%s %s", COLOR_YELLOW, COLOR_RESET, graph.roadList[pr.path[k]].name);
                            }
                            printf("  %s(%d米, %d分钟)%s\n", COLOR_GREEN, pr.totalDistance, pr.totalTime, COLOR_RESET);
                        }
                    }
                }
                printf("\n");
                PressAnyKey();
                break;

            case 9: /* 添加景点 */
                ClearScreen();
                PrintTitle("添加景点", 60);
                {
                    char name[MAX_NAME_LEN];
                    char desc[MAX_DESC_LEN];
                    int type, visitTime, popularity;

                    printf("  请输入景点名称: ");
                    ReadLineUTF8(name, MAX_NAME_LEN);
                    printf("  [调试] name=[%s] len=%d hex=", name, (int)strlen(name));
                    for (int di = 0; di < (int)strlen(name) && di < 20; di++) {
                        printf("%02X ", (unsigned char)name[di]);
                    }
                    printf("\n");
                    PressAnyKey();

                    /* 检查名称是否已存在 */
                    if (SearchSpotByName(&graph, name) >= 0) {
                        PrintError("景点名称已存在!");
                        PressAnyKey();
                        break;
                    }

                    printf("  请输入景点描述: ");
                    ReadLineUTF8(desc, MAX_DESC_LEN);

                    printf("  请选择景点类型:\n");
                    printf("    0 - 入口\n");
                    printf("    1 - 动物展馆\n");
                    printf("    2 - 景点\n");
                    printf("    3 - 设施\n");
                    printf("    4 - 科普教育\n");
                    type = ReadInt("请输入类型编号", 0, 4);

                    visitTime = ReadInt("请输入游览时间(分钟)", 1, 120);
                    popularity = ReadInt("请输入受欢迎程度(1-10)", 1, 10);

                    AddSpot(&graph, name, desc, (SpotType)type, visitTime, popularity);
                }
                PressAnyKey();
                break;

            case 10: /* 删除景点 */
                ClearScreen();
                PrintTitle("删除景点", 60);
                ShowVertex(&graph);
                {
                    int delIdx = ReadInt("请输入要删除的景点编号", 1, graph.numVertex) - 1;
                    printf("\n  确定要删除景点 [%s] 吗? (1=确认, 0=取消): ", graph.roadList[delIdx].name);
                    int confirm;
                    scanf("%d", &confirm);
                    getchar();
                    if (confirm == 1) {
                        DeleteSpot(&graph, delIdx);
                    } else {
                        PrintInfo("已取消删除");
                    }
                }
                PressAnyKey();
                break;

            case 11: /* 查询景点 */
                ClearScreen();
                PrintTitle("查询景点", 60);
                printf("\n  查询方式:\n");
                printf("    1 - 按名称查询\n");
                printf("    2 - 按类型查询\n");
                printf("    3 - 显示全部\n");
                {
                    int searchType = ReadInt("请选择查询方式", 1, 3);
                    if (searchType == 1) {
                        char name[MAX_NAME_LEN];
                        printf("  请输入景点名称: ");
                        ReadLineUTF8(name, MAX_NAME_LEN);
                        int idx = SearchSpotByName(&graph, name);
                        if (idx >= 0) {
                            ShowSpotDetail(&graph, idx);
                        } else {
                            PrintError("未找到该景点!");
                        }
                    } else if (searchType == 2) {
                        printf("  请选择景点类型:\n");
                        printf("    0 - 入口\n");
                        printf("    1 - 动物展馆\n");
                        printf("    2 - 景点\n");
                        printf("    3 - 设施\n");
                        printf("    4 - 科普教育\n");
                        int type = ReadInt("请输入类型编号", 0, 4);
                        int result[MAX_VERTEX_NUM];
                        int count = SearchSpotByType(&graph, (SpotType)type, result);
                        if (count > 0) {
                            printf("\n  找到 %d 个景点:\n", count);
                            for (int i = 0; i < count; i++) {
                                printf("    %d. %s\n", i + 1, graph.roadList[result[i]].name);
                            }
                        } else {
                            PrintInfo("未找到该类型的景点");
                        }
                    } else {
                        ShowVertex(&graph);
                    }
                }
                PressAnyKey();
                break;

            case 12: /* 修改景点 */
                ClearScreen();
                PrintTitle("修改景点", 60);
                ShowVertex(&graph);
                {
                    int modIdx = ReadInt("请输入要修改的景点编号", 1, graph.numVertex) - 1;
                    printf("\n  当前信息:\n");
                    printf("    名称: %s\n", graph.roadList[modIdx].name);
                    printf("    描述: %s\n", graph.roadList[modIdx].info.description);
                    printf("    类型: %d\n", graph.roadList[modIdx].info.type);
                    printf("    游览时间: %d分钟\n", graph.roadList[modIdx].info.visitTime);
                    printf("    受欢迎程度: %d\n", graph.roadList[modIdx].info.popularity);

                    printf("\n  请输入新信息 (直接回车表示不修改):\n");

                    char newName[MAX_NAME_LEN];
                    printf("  新名称: ");
                    ReadLineUTF8(newName, MAX_NAME_LEN);
                    char *namePtr = (strlen(newName) > 0) ? newName : NULL;

                    char newDesc[MAX_DESC_LEN];
                    printf("  新描述: ");
                    ReadLineUTF8(newDesc, MAX_DESC_LEN);
                    char *descPtr = (strlen(newDesc) > 0) ? newDesc : NULL;

                    printf("  新类型 (0-4, -1不修改): ");
                    int newType;
                    scanf("%d", &newType);
                    getchar();

                    printf("  新游览时间 (分钟, -1不修改): ");
                    int newTime;
                    scanf("%d", &newTime);
                    getchar();

                    printf("  新受欢迎程度 (1-10, -1不修改): ");
                    int newPop;
                    scanf("%d", &newPop);
                    getchar();

                    UpdateSpot(&graph, modIdx, namePtr, descPtr, newType, newTime, newPop);
                }
                PressAnyKey();
                break;

            case 13: /* 随机路线(含厕所) */
                ClearScreen();
                PrintTitle("随机游览路线", 60);
                {
                    int maxTime = ReadInt("请输入最大游览时间(分钟, 0=不限制)", 0, 480);
                    GenerateRandomRoute(&graph, maxTime);
                }
                PressAnyKey();
                break;

            case 0:
                ClearScreen();
                printf("\n  %s感谢使用广州动物园路线规划系统!%s\n\n", COLOR_BOLD COLOR_GREEN, COLOR_RESET);
                break;

            default:
                PrintError("输入错误，请重新输入!");
                PressAnyKey();
                break;
        }
    } while (choice != 0);

    /* 释放内存 */
    FreeGraph(&graph);

    return 0;
}