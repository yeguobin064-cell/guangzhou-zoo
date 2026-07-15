#include "global.h"
#include "graph.h"
#include "consoleio.h"
#include <time.h>

/**
 * @brief Dijkstra算法求解最短路径
 * @param graph 图的指针
 * @param startIdx 起点下标
 * @param endIdx 终点下标
 * @param result 路径结果
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status Dijkstra(const ALGraph *graph, int startIdx, int endIdx, PathResult *result) {
    if (graph == NULL || result == NULL) {
        return ERROR;
    }

    if (startIdx < 0 || startIdx >= graph->numVertex ||
        endIdx < 0 || endIdx >= graph->numVertex) {
        return ERROR;
    }

    int dist[MAX_VERTEX_NUM];      /* 距离数组 */
    int prev[MAX_VERTEX_NUM];      /* 前驱数组 */
    int visited[MAX_VERTEX_NUM];   /* 访问标记数组 */
    int timeDist[MAX_VERTEX_NUM];  /* 时间距离数组 */

    /* 初始化 */
    for (int i = 0; i < graph->numVertex; i++) {
        dist[i] = INF;
        prev[i] = -1;
        visited[i] = 0;
        timeDist[i] = INF;
    }

    dist[startIdx] = 0;
    timeDist[startIdx] = 0;

    /* Dijkstra主循环 */
    for (int i = 0; i < graph->numVertex; i++) {
        /* 找到未访问的距离最小的顶点 */
        int minDist = INF;
        int u = -1;

        for (int j = 0; j < graph->numVertex; j++) {
            if (!visited[j] && dist[j] < minDist) {
                minDist = dist[j];
                u = j;
            }
        }

        if (u == -1) {
            break;  /* 所有可达顶点都已访问 */
        }

        visited[u] = 1;

        /* 如果找到终点，可以提前结束 */
        if (u == endIdx) {
            break;
        }

        /* 更新邻接顶点的距离 */
        CNode *node = graph->roadList[u].first;
        while (node != NULL) {
            int v = node->index;
            if (!visited[v]) {
                int newDist = dist[u] + node->length;
                int newTime = timeDist[u] + node->time;

                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                    timeDist[v] = newTime;
                }
            }
            node = node->next;
        }
    }

    /* 检查是否找到路径 */
    if (dist[endIdx] == INF) {
        return NOT_FOUND;
    }

    /* 构建路径 */
    result->pathLength = 0;
    result->totalDistance = dist[endIdx];
    result->totalTime = timeDist[endIdx];

    /* 反向追踪路径 */
    int path[MAX_VERTEX_NUM];
    int pathLen = 0;
    int current = endIdx;

    while (current != -1) {
        path[pathLen++] = current;
        current = prev[current];
    }

    /* 反转路径 */
    for (int i = 0; i < pathLen; i++) {
        result->path[i] = path[pathLen - 1 - i];
    }
    result->pathLength = pathLen;
    result->spotCount = pathLen;

    return OK;
}

/**
 * @brief Floyd算法求解所有顶点之间的最短路径
 * @param graph 图的指针
 * @param dist 距离矩阵
 * @param path 路径矩阵
 */
void Floyd(const ALGraph *graph, int dist[][MAX_VERTEX_NUM], int path[][MAX_VERTEX_NUM]) {
    if (graph == NULL) {
        return;
    }

    int n = graph->numVertex;

    /* 初始化距离矩阵和路径矩阵 */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i == j) {
                dist[i][j] = 0;
                path[i][j] = -1;
            } else {
                dist[i][j] = INF;
                path[i][j] = -1;
            }
        }
    }

    /* 根据邻接表填充初始距离 */
    for (int i = 0; i < n; i++) {
        CNode *node = graph->roadList[i].first;
        while (node != NULL) {
            dist[i][node->index] = node->length;
            path[i][node->index] = i;
            node = node->next;
        }
    }

    /* Floyd主循环 */
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    path[i][j] = path[k][j];
                }
            }
        }
    }
}

/**
 * @brief 查询两个景点之间的最短路径
 * @param graph 图的指针
 * @param startName 起点名称
 * @param endName 终点名称
 */
void QueryShortestPath(const ALGraph *graph, const char *startName, const char *endName) {
    if (graph == NULL || startName == NULL || endName == NULL) {
        PrintError("参数错误");
        return;
    }

    int startIdx = LocateVertex(graph, startName);
    int endIdx = LocateVertex(graph, endName);

    if (startIdx == -1) {
        PrintError("未找到起点: %s", startName);
        return;
    }

    if (endIdx == -1) {
        PrintError("未找到终点: %s", endName);
        return;
    }

    PathResult result;
    Status status = Dijkstra(graph, startIdx, endIdx, &result);

    if (status == OK) {
        PrintSuccess("找到最短路径!");
        printf("\n");

        PrintColor(COLOR_CYAN, "  路线: ");
        for (int i = 0; i < result.pathLength; i++) {
            if (i > 0) {
                PrintColor(COLOR_YELLOW, " -> ");
            }
            PrintColor(COLOR_WHITE, "%s", graph->roadList[result.path[i]].name);
        }
        printf("\n\n");

        PrintColor(COLOR_GREEN, "  总距离: %d 米\n", result.totalDistance);
        PrintColor(COLOR_GREEN, "  预计步行时间: %d 分钟\n", result.totalTime);
        PrintColor(COLOR_GREEN, "  途经景点数: %d 个\n", result.spotCount);
    } else {
        PrintError("无法找到从 %s 到 %s 的路径", startName, endName);
    }
}

/**
 * @brief 规划最佳游览路线（贪心算法）
 * @param graph 图的指针
 * @param startIdx 起始景点下标
 * @param timeLimit 时间限制（分钟）
 */
void PlanBestRoute(const ALGraph *graph, int startIdx, int timeLimit) {
    if (graph == NULL || startIdx < 0 || startIdx >= graph->numVertex) {
        PrintError("参数错误");
        return;
    }

    int visited[MAX_VERTEX_NUM];
    int route[MAX_VERTEX_NUM];
    int routeLen = 0;
    int totalTime = 0;
    int totalDistance = 0;
    int current = startIdx;

    /* 初始化访问标记 */
    for (int i = 0; i < graph->numVertex; i++) {
        visited[i] = 0;
    }

    PrintColor(COLOR_CYAN, "\n  正在规划最佳游览路线...\n\n");

    /* 贪心算法：每次选择最近的未访问景点 */
    while (totalTime < timeLimit && routeLen < graph->numVertex) {
        visited[current] = 1;
        route[routeLen++] = current;
        totalTime += graph->roadList[current].info.visitTime;

        /* 找到最近的未访问景点 */
        int nearest = -1;
        int minDist = INF;

        CNode *node = graph->roadList[current].first;
        while (node != NULL) {
            if (!visited[node->index]) {
                int estTime = totalTime + node->time +
                             graph->roadList[node->index].info.visitTime;
                if (estTime <= timeLimit && node->length < minDist) {
                    minDist = node->length;
                    nearest = node->index;
                }
            }
            node = node->next;
        }

        if (nearest == -1) {
            break;  /* 没有可到达的未访问景点 */
        }

        totalDistance += minDist;
        totalTime += minDist / 50;  /* 假设步行速度50米/分钟 */
        current = nearest;
    }

    /* 输出路线 */
    PrintSuccess("最佳游览路线规划完成!");
    printf("\n");

    PrintColor(COLOR_CYAN, "  推荐路线:\n");
    for (int i = 0; i < routeLen; i++) {
        PrintColor(COLOR_YELLOW, "  %d. ", i + 1);
        PrintColor(COLOR_WHITE, "%s", graph->roadList[route[i]].name);

        if (i < routeLen - 1) {
            /* 计算到下一个景点的距离 */
            int nextIdx = route[i + 1];
            CNode *node = graph->roadList[route[i]].first;
            while (node != NULL) {
                if (node->index == nextIdx) {
                    PrintColor(COLOR_CYAN, " --(%dm, %dmin)--> ", node->length, node->time);
                    break;
                }
                node = node->next;
            }
        }
        printf("\n");
    }

    printf("\n");
    PrintColor(COLOR_GREEN, "  游览景点数: %d 个\n", routeLen);
    PrintColor(COLOR_GREEN, "  总步行距离: %d 米\n", totalDistance);
    PrintColor(COLOR_GREEN, "  预计总时间: %d 分钟\n", totalTime);
    PrintColor(COLOR_GREEN, "  建议游览时间: %d 分钟\n", timeLimit);
}

/**
 * @brief 生成随机游览路线（确保经过公共厕所）
 *
 * 算法设计:
 *   1. 从入口出发
 *   2. 随机选取3-5个景点
 *   3. 在路线中间位置插入至少一个公共厕所
 *   4. 使用Dijkstra算法连接相邻站点
 *   5. 输出完整路线及距离/时间信息
 *
 * @param graph 图的指针
 * @param maxTime 最大游览时间限制（分钟），0表示不限制
 */
void GenerateRandomRoute(const ALGraph *graph, int maxTime) {
    if (graph == NULL || graph->numVertex == 0) {
        PrintError("图数据为空");
        return;
    }

    int n = graph->numVertex;

    /* 1. 找到入口 */
    int entranceIdx = -1;
    for (int i = 0; i < n; i++) {
        if (graph->roadList[i].info.type == TYPE_ENTRANCE) {
            entranceIdx = i;
            break;
        }
    }
    if (entranceIdx == -1) {
        PrintError("未找到入口景点");
        return;
    }

    /* 2. 找到所有公共厕所 */
    int restrooms[MAX_VERTEX_NUM];
    int restroomCount = 0;
    for (int i = 0; i < n; i++) {
        if (graph->roadList[i].info.type == TYPE_FACILITY &&
            strstr(graph->roadList[i].name, "厕所") != NULL) {
            restrooms[restroomCount++] = i;
        }
    }
    if (restroomCount == 0) {
        PrintError("未找到公共厕所设施");
        return;
    }

    /* 3. 收集所有可游览景点（排除入口和厕所） */
    int candidates[MAX_VERTEX_NUM];
    int candCount = 0;
    for (int i = 0; i < n; i++) {
        if (i != entranceIdx) {
            int isRestroom = 0;
            for (int j = 0; j < restroomCount; j++) {
                if (restrooms[j] == i) { isRestroom = 1; break; }
            }
            if (!isRestroom) {
                candidates[candCount++] = i;
            }
        }
    }

    if (candCount == 0) {
        PrintError("没有可游览的景点");
        return;
    }

    /* 4. Fisher-Yates 洗牌算法随机打乱候选景点 */
    srand((unsigned int)time(NULL));
    for (int i = candCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    /* 5. 选取3-5个景点（不超过候选数） */
    int selectCount = 3 + rand() % 3;  /* 3~5 */
    if (selectCount > candCount) selectCount = candCount;

    /* 6. 构建途经点序列: 入口 -> 选中景点(含厕所插入) */
    /*    厕所插入位置: 大约在路线1/3处 */
    int waypoints[MAX_VERTEX_NUM + 2];
    int wpCount = 0;
    waypoints[wpCount++] = entranceIdx;

    int restroomInsertPos = (selectCount >= 3) ? (selectCount / 3 + 1) : 1;

    for (int i = 0; i < selectCount; i++) {
        waypoints[wpCount++] = candidates[i];
        /* 在指定位置插入公共厕所 */
        if (i + 1 == restroomInsertPos) {
            /* 选择离当前景点最近的厕所 */
            int bestRestroom = restrooms[0];
            int bestDist = INF;
            for (int r = 0; r < restroomCount; r++) {
                PathResult tmp;
                if (Dijkstra(graph, candidates[i], restrooms[r], &tmp) == OK) {
                    if (tmp.totalDistance < bestDist) {
                        bestDist = tmp.totalDistance;
                        bestRestroom = restrooms[r];
                    }
                }
            }
            waypoints[wpCount++] = bestRestroom;
        }
    }

    /* 如果没插入过厕所，追加一个 */
    int hasRestroom = 0;
    for (int i = 0; i < wpCount; i++) {
        for (int r = 0; r < restroomCount; r++) {
            if (waypoints[i] == restrooms[r]) { hasRestroom = 1; break; }
        }
    }
    if (!hasRestroom) {
        int bestRestroom = restrooms[0];
        int bestDist = INF;
        for (int r = 0; r < restroomCount; r++) {
            PathResult tmp;
            if (Dijkstra(graph, waypoints[wpCount - 1], restrooms[r], &tmp) == OK) {
                if (tmp.totalDistance < bestDist) {
                    bestDist = tmp.totalDistance;
                    bestRestroom = restrooms[r];
                }
            }
        }
        waypoints[wpCount++] = bestRestroom;
    }

    /* 7. 用Dijkstra连接相邻途经点，拼接完整路径 */
    int fullPath[MAX_VERTEX_NUM * 4];
    int fullPathLen = 0;
    int totalDist = 0;
    int totalTime = 0;
    int routeValid = 1;

    for (int i = 0; i < wpCount - 1; i++) {
        PathResult seg;
        if (Dijkstra(graph, waypoints[i], waypoints[i + 1], &seg) != OK) {
            PrintError("无法从 %s 到达 %s",
                       graph->roadList[waypoints[i]].name,
                       graph->roadList[waypoints[i + 1]].name);
            routeValid = 0;
            break;
        }
        /* 拼接子路径（跳过首点避免重复） */
        int start = (i == 0) ? 0 : 1;
        for (int j = start; j < seg.pathLength; j++) {
            fullPath[fullPathLen++] = seg.path[j];
        }
        totalDist += seg.totalDistance;
        totalTime += seg.totalTime;
    }

    if (!routeValid) return;

    /* 加上游览时间 */
    int visitTotal = 0;
    for (int i = 0; i < fullPathLen; i++) {
        visitTotal += graph->roadList[fullPath[i]].info.visitTime;
    }
    totalTime += visitTotal;

    /* 时间限制检查 */
    if (maxTime > 0 && totalTime > maxTime) {
        PrintInfo("注意: 路线总时间 %d 分钟超过限制 %d 分钟", totalTime, maxTime);
    }

    /* 8. 输出路线 */
    printf("\n");
    PrintSuccess("随机游览路线生成完成!");
    printf("\n");

    /* 输出路线概览 */
    PrintColor(COLOR_CYAN, "  路线概览:\n\n");
    for (int i = 0; i < fullPathLen; i++) {
        int idx = fullPath[i];
        const char *name = graph->roadList[idx].name;
        int isRestroom = 0;
        for (int r = 0; r < restroomCount; r++) {
            if (restrooms[r] == idx) { isRestroom = 1; break; }
        }

        PrintColor(COLOR_YELLOW, "  %2d. ", i + 1);

        if (isRestroom) {
            PrintColor(COLOR_MAGENTA, "[%s]", name);
        } else {
            PrintSpotIcon(graph->roadList[idx].info.type, COLOR_GREEN);
            printf(" %s", name);
        }

        /* 显示到下一站的距离 */
        if (i < fullPathLen - 1) {
            int nextIdx = fullPath[i + 1];
            CNode *node = graph->roadList[idx].first;
            while (node != NULL) {
                if (node->index == nextIdx) {
                    PrintColor(COLOR_CYAN, " --(%dm, %dmin)--> ", node->length, node->time);
                    break;
                }
                node = node->next;
            }
        }
        printf("\n");
    }

    /* 输出统计信息 */
    printf("\n");
    PrintColor(COLOR_GREEN, "  总距离: %d 米\n", totalDist);
    PrintColor(COLOR_GREEN, "  步行时间: %d 分钟\n", totalTime - visitTotal);
    PrintColor(COLOR_GREEN, "  游览时间: %d 分钟\n", visitTotal);
    PrintColor(COLOR_GREEN, "  总时间: %d 分钟\n", totalTime);
    PrintColor(COLOR_GREEN, "  途经景点数: %d 个\n", fullPathLen);
    PrintColor(COLOR_MAGENTA, "  公共厕所位置: ");

    /* 标记厕所在线路中的位置 */
    int first = 1;
    for (int i = 0; i < fullPathLen; i++) {
        int isRestroom = 0;
        for (int r = 0; r < restroomCount; r++) {
            if (restrooms[r] == fullPath[i]) { isRestroom = 1; break; }
        }
        if (isRestroom) {
            if (!first) printf(", ");
            printf("第%d站", i + 1);
            first = 0;
        }
    }
    printf("\n\n");
}