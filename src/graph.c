#include "global.h"
#include "graph.h"
#include "consoleio.h"

/**
 * @brief 创建景区景点图
 * @param graph 图的指针
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status CreateSceneGraph(ALGraph *graph) {
    if (graph == NULL) {
        return ERROR;
    }

    int numVertex, numEdge;

    printf("  请输入景点数量: ");
    scanf("%d", &numVertex);
    getchar();

    if (numVertex <= 0 || numVertex > MAX_VERTEX_NUM) {
        PrintError("景点数量必须在 1 到 %d 之间", MAX_VERTEX_NUM);
        return ERROR;
    }

    graph->numVertex = numVertex;
    graph->numEdge = 0;

    /* 输入景点名称 */
    for (int i = 0; i < numVertex; i++) {
        printf("  请输入第 %d 个景点名称: ", i + 1);
        ReadLineUTF8(graph->roadList[i].name, MAX_NAME_LEN);
        graph->roadList[i].first = NULL;
    }

    printf("  请输入路径数量: ");
    scanf("%d", &numEdge);
    getchar();

    if (numEdge < 0 || numEdge > MAX_EDGE_NUM) {
        PrintError("路径数量必须在 0 到 %d 之间", MAX_EDGE_NUM);
        return ERROR;
    }

    /* 输入路径信息 */
    for (int i = 0; i < numEdge; i++) {
        char fromName[MAX_NAME_LEN], toName[MAX_NAME_LEN];
        int distance, time;

        printf("  请输入第 %d 条路径 (起点 终点 距离 时间): ", i + 1);
        scanf("%s %s %d %d", fromName, toName, &distance, &time);
        getchar();

        int fromIdx = LocateVertex(graph, fromName);
        int toIdx = LocateVertex(graph, toName);

        if (fromIdx == -1) {
            PrintError("未找到景点: %s", fromName);
            return ERROR;
        }

        if (toIdx == -1) {
            PrintError("未找到景点: %s", toName);
            return ERROR;
        }

        /* 添加边（使用头插法） */
        CNode *node1 = (CNode *)malloc(sizeof(CNode));
        if (node1 == NULL) {
            return ERROR;
        }
        node1->index = toIdx;
        node1->length = distance;
        node1->time = time;
        node1->next = graph->roadList[fromIdx].first;
        graph->roadList[fromIdx].first = node1;

        /* 添加反向边（无向图） */
        CNode *node2 = (CNode *)malloc(sizeof(CNode));
        if (node2 == NULL) {
            return ERROR;
        }
        node2->index = fromIdx;
        node2->length = distance;
        node2->time = time;
        node2->next = graph->roadList[toIdx].first;
        graph->roadList[toIdx].first = node2;

        graph->numEdge++;
    }

    return OK;
}

/**
 * @brief 从文件加载景区景点图
 * @param graph 图的指针
 * @param filename 文件名
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status CreateSceneGraphByFile(ALGraph *graph, const char *filename) {
    if (graph == NULL || filename == NULL) {
        return ERROR;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        PrintError("无法打开文件: %s", filename);
        return ERROR;
    }

    int numVertex, numEdge;
    if (fscanf(file, "%d %d", &numVertex, &numEdge) != 2) {
        fclose(file);
        return ERROR;
    }

    if (numVertex <= 0 || numVertex > MAX_VERTEX_NUM) {
        fclose(file);
        return ERROR;
    }

    graph->numVertex = numVertex;
    graph->numEdge = 0;

    /* 读取景点名称 */
    for (int i = 0; i < numVertex; i++) {
        if (fscanf(file, "%s", graph->roadList[i].name) != 1) {
            fclose(file);
            return ERROR;
        }
        graph->roadList[i].first = NULL;
    }

    /* 读取路径信息 */
    for (int i = 0; i < numEdge; i++) {
        char fromName[MAX_NAME_LEN], toName[MAX_NAME_LEN];
        int distance;

        if (fscanf(file, "%s %s %d", fromName, toName, &distance) != 3) {
            fclose(file);
            return ERROR;
        }

        int fromIdx = LocateVertex(graph, fromName);
        int toIdx = LocateVertex(graph, toName);

        if (fromIdx == -1 || toIdx == -1) {
            fclose(file);
            return ERROR;
        }

        /* 添加边 */
        CNode *node1 = (CNode *)malloc(sizeof(CNode));
        if (node1 == NULL) {
            fclose(file);
            return ERROR;
        }
        node1->index = toIdx;
        node1->length = distance;
        node1->time = distance / 50;  /* 估算时间 */
        node1->next = graph->roadList[fromIdx].first;
        graph->roadList[fromIdx].first = node1;

        CNode *node2 = (CNode *)malloc(sizeof(CNode));
        if (node2 == NULL) {
            fclose(file);
            return ERROR;
        }
        node2->index = fromIdx;
        node2->length = distance;
        node2->time = distance / 50;
        node2->next = graph->roadList[toIdx].first;
        graph->roadList[toIdx].first = node2;

        graph->numEdge++;
    }

    fclose(file);
    return OK;
}

/**
 * @brief 按名称定位顶点下标
 */
int LocateVertex(const ALGraph *graph, const char *name) {
    if (graph == NULL || name == NULL) {
        return -1;
    }

    for (int i = 0; i < graph->numVertex; i++) {
        if (strcmp(graph->roadList[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief 以邻接表形式展示景区分布图
 */
void ShowGraph(const ALGraph *graph) {
    if (graph == NULL) {
        return;
    }

    printf("\n  %s景点连接关系:%s\n\n", COLOR_CYAN, COLOR_RESET);

    for (int i = 0; i < graph->numVertex; i++) {
        printf("  %s%-10s%s ->", COLOR_BOLD, graph->roadList[i].name, COLOR_RESET);

        CNode *node = graph->roadList[i].first;
        int count = 0;
        while (node != NULL) {
            if (count > 0) {
                printf(",");
            }
            printf(" %s(%s,%dm)%s", COLOR_GREEN, graph->roadList[node->index].name, node->length, COLOR_RESET);
            count++;
            node = node->next;
        }
        printf("\n");
    }
    printf("\n");
}

/**
 * @brief 显示所有景点信息
 */
void ShowVertex(const ALGraph *graph) {
    if (graph == NULL) {
        return;
    }

    printf("\n  %s景点列表:%s\n\n", COLOR_CYAN, COLOR_RESET);

    for (int i = 0; i < graph->numVertex; i++) {
        const char *color;
        const char *typeChar;

        switch (graph->roadList[i].info.type) {
            case TYPE_ENTRANCE:
                color = COLOR_GREEN;
                typeChar = "入口";
                break;
            case TYPE_ANIMAL_HOUSE:
                color = COLOR_YELLOW;
                typeChar = "展馆";
                break;
            case TYPE_SCENIC_SPOT:
                color = COLOR_BLUE;
                typeChar = "景点";
                break;
            case TYPE_FACILITY:
                color = COLOR_MAGENTA;
                typeChar = "设施";
                break;
            case TYPE_EDUCATION:
                color = COLOR_CYAN;
                typeChar = "科普";
                break;
            default:
                color = COLOR_WHITE;
                typeChar = "其他";
                break;
        }

        printf("  %s%2d.%s %s%-10s%s %s[%s]%s %s%d分钟%s\n",
               COLOR_WHITE, i + 1, COLOR_RESET,
               COLOR_BOLD, graph->roadList[i].name, COLOR_RESET,
               color, typeChar, COLOR_RESET,
               COLOR_GREEN, graph->roadList[i].info.visitTime, COLOR_RESET);
    }
    printf("\n");
}

/**
 * @brief 显示景点详细信息
 */
void ShowSpotDetail(const ALGraph *graph, int index) {
    if (graph == NULL || index < 0 || index >= graph->numVertex) {
        return;
    }

    const Vnode *spot = &graph->roadList[index];
    const char *typeNames[] = {"入口", "动物展馆", "景点", "设施", "科普教育"};

    printf("\n  %s=== %s ===%s\n\n", COLOR_BOLD COLOR_CYAN, spot->name, COLOR_RESET);
    printf("  %s类型:%s %s\n", COLOR_CYAN, COLOR_RESET, typeNames[spot->info.type]);
    printf("  %s描述:%s %s\n", COLOR_CYAN, COLOR_RESET, spot->info.description);
    printf("  %s游览时间:%s %d分钟\n", COLOR_CYAN, COLOR_RESET, spot->info.visitTime);

    printf("\n  %s相邻景点:%s\n", COLOR_CYAN, COLOR_RESET);
    CNode *node = spot->first;
    while (node != NULL) {
        printf("  %s-> %s%s (%d米, %d分钟)\n",
               COLOR_GREEN, graph->roadList[node->index].name, COLOR_RESET,
               node->length, node->time);
        node = node->next;
    }
    printf("\n");
}

/**
 * @brief 基于 DFS 输出导游线路
 */
void TourRoute(const ALGraph *graph, int startIdx) {
    if (graph == NULL || startIdx < 0 || startIdx >= graph->numVertex) {
        return;
    }

    int visited[MAX_VERTEX_NUM];
    int route[MAX_VERTEX_NUM];
    int routeLen = 0;

    /* 初始化访问标记 */
    for (int i = 0; i < graph->numVertex; i++) {
        visited[i] = 0;
    }

    /* DFS遍历 */
    int stack[MAX_VERTEX_NUM];
    int top = -1;
    stack[++top] = startIdx;

    printf("\n");
    PrintColor(COLOR_CYAN, "  导游线路 (从 %s 出发):\n\n", graph->roadList[startIdx].name);

    while (top >= 0) {
        int current = stack[top--];

        if (visited[current]) {
            continue;
        }

        visited[current] = 1;
        route[routeLen++] = current;

        /* 将邻接点压入栈（逆序，保证DFS顺序） */
        CNode *node = graph->roadList[current].first;
        int adjCount = 0;
        int adj[MAX_VERTEX_NUM];

        while (node != NULL) {
            if (!visited[node->index]) {
                adj[adjCount++] = node->index;
            }
            node = node->next;
        }

        /* 逆序压栈 */
        for (int i = adjCount - 1; i >= 0; i--) {
            stack[++top] = adj[i];
        }
    }

    /* 输出路线 */
    printf("  ");
    for (int i = 0; i < routeLen; i++) {
        if (i > 0) {
            printf(" %s->%s ", COLOR_YELLOW, COLOR_RESET);
        }
        PrintSpotIcon(graph->roadList[route[i]].info.type, COLOR_GREEN);
        printf("%s", graph->roadList[route[i]].name);
    }
    printf("\n\n");

    PrintColor(COLOR_GREEN, "  共游览 %d 个景点\n", routeLen);
}

/**
 * @brief 检测导游线路中的回路
 */
void FindCycle(const ALGraph *graph) {
    if (graph == NULL) {
        return;
    }

    int visited[MAX_VERTEX_NUM];
    int recStack[MAX_VERTEX_NUM];
    int parent[MAX_VERTEX_NUM];

    /* 初始化 */
    for (int i = 0; i < graph->numVertex; i++) {
        visited[i] = 0;
        recStack[i] = 0;
        parent[i] = -1;
    }

    int cycleFound = 0;

    /* DFS检测回路 */
    for (int i = 0; i < graph->numVertex; i++) {
        if (!visited[i]) {
            int stack[MAX_VERTEX_NUM];
            int top = -1;
            stack[++top] = i;

            while (top >= 0) {
                int current = stack[top];

                if (!visited[current]) {
                    visited[current] = 1;
                    recStack[current] = 1;

                    CNode *node = graph->roadList[current].first;
                    while (node != NULL) {
                        if (!visited[node->index]) {
                            parent[node->index] = current;
                            stack[++top] = node->index;
                        } else if (recStack[node->index]) {
                            /* 找到回路 */
                            cycleFound = 1;
                            printf("\n");
                            PrintColor(COLOR_RED, "  发现回路!\n\n");
                            printf("  回路路径: ");

                            int path[MAX_VERTEX_NUM];
                            int pathLen = 0;
                            int v = current;

                            while (v != node->index) {
                                path[pathLen++] = v;
                                v = parent[v];
                            }
                            path[pathLen++] = node->index;

                            for (int j = pathLen - 1; j >= 0; j--) {
                                if (j < pathLen - 1) {
                                    printf(" -> ");
                                }
                                printf("%s", graph->roadList[path[j]].name);
                            }
                            printf(" -> %s\n", graph->roadList[path[pathLen - 1]].name);
                        }
                        node = node->next;
                    }
                } else {
                    recStack[current] = 0;
                    top--;
                }
            }
        }
    }

    if (!cycleFound) {
        printf("\n");
        PrintColor(COLOR_GREEN, "  未发现回路\n");
    }
}

/**
 * @brief 添加新景点
 */
Status AddSpot(ALGraph *graph, const char *name, const char *desc, SpotType type, int visitTime, int popularity) {
    if (graph == NULL || name == NULL || graph->numVertex >= MAX_VERTEX_NUM) {
        return ERROR;
    }

    /* 检查名称是否已存在 */
    for (int i = 0; i < graph->numVertex; i++) {
        if (strcmp(graph->roadList[i].name, name) == 0) {
            PrintError("景点名称已存在");
            return ERROR;
        }
    }

    int idx = graph->numVertex;
    strcpy(graph->roadList[idx].name, name);
    strcpy(graph->roadList[idx].info.description, desc);
    graph->roadList[idx].info.type = type;
    graph->roadList[idx].info.visitTime = visitTime;
    graph->roadList[idx].info.popularity = popularity;
    graph->roadList[idx].info.animalCount = 0;
    graph->roadList[idx].first = NULL;
    graph->numVertex++;

    PrintSuccess("添加景点成功: %s", name);
    return OK;
}

/**
 * @brief 删除景点
 */
Status DeleteSpot(ALGraph *graph, int index) {
    if (graph == NULL || index < 0 || index >= graph->numVertex) {
        return ERROR;
    }

    /* 释放该景点的所有边 */
    CNode *node = graph->roadList[index].first;
    while (node != NULL) {
        CNode *temp = node;
        node = node->next;
        free(temp);
    }

    /* 从其他景点中删除指向该景点的边 */
    for (int i = 0; i < graph->numVertex; i++) {
        if (i == index) continue;

        CNode *prev = NULL;
        CNode *curr = graph->roadList[i].first;

        while (curr != NULL) {
            if (curr->index == index) {
                if (prev == NULL) {
                    graph->roadList[i].first = curr->next;
                } else {
                    prev->next = curr->next;
                }
                free(curr);
                graph->numEdge--;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    /* 更新指向被删除景点之后的边的索引 */
    for (int i = 0; i < graph->numVertex; i++) {
        CNode *node = graph->roadList[i].first;
        while (node != NULL) {
            if (node->index > index) {
                node->index--;
            }
            node = node->next;
        }
    }

    /* 移动景点数据 */
    for (int i = index; i < graph->numVertex - 1; i++) {
        graph->roadList[i] = graph->roadList[i + 1];
    }
    graph->numVertex--;

    PrintSuccess("删除景点成功");
    return OK;
}

/**
 * @brief 按名称查询景点
 */
int SearchSpotByName(const ALGraph *graph, const char *name) {
    if (graph == NULL || name == NULL) {
        return -1;
    }

    for (int i = 0; i < graph->numVertex; i++) {
        if (strcmp(graph->roadList[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 按类型查询景点
 */
int SearchSpotByType(const ALGraph *graph, SpotType type, int *result) {
    if (graph == NULL || result == NULL) {
        return 0;
    }

    int count = 0;
    for (int i = 0; i < graph->numVertex; i++) {
        if (graph->roadList[i].info.type == type) {
            result[count++] = i;
        }
    }
    return count;
}

/**
 * @brief 修改景点信息
 */
Status UpdateSpot(ALGraph *graph, int index, const char *name, const char *desc, int type, int visitTime, int popularity) {
    if (graph == NULL || index < 0 || index >= graph->numVertex) {
        return ERROR;
    }

    /* 检查新名称是否与其他景点冲突 */
    if (name != NULL) {
        for (int i = 0; i < graph->numVertex; i++) {
            if (i != index && strcmp(graph->roadList[i].name, name) == 0) {
                PrintError("景点名称已存在");
                return ERROR;
            }
        }
        strcpy(graph->roadList[index].name, name);
    }

    if (desc != NULL) {
        strcpy(graph->roadList[index].info.description, desc);
    }
    if (type >= 0) {
        graph->roadList[index].info.type = (SpotType)type;
    }
    if (visitTime > 0) {
        graph->roadList[index].info.visitTime = visitTime;
    }
    if (popularity > 0 && popularity <= 10) {
        graph->roadList[index].info.popularity = popularity;
    }

    PrintSuccess("修改景点成功: %s", graph->roadList[index].name);
    return OK;
}

/**
 * @brief 释放图的内存
 */
void FreeGraph(ALGraph *graph) {
    if (graph == NULL) {
        return;
    }

    for (int i = 0; i < graph->numVertex; i++) {
        CNode *node = graph->roadList[i].first;
        while (node != NULL) {
            CNode *temp = node;
            node = node->next;
            free(temp);
        }
        graph->roadList[i].first = NULL;
    }
    graph->numVertex = 0;
    graph->numEdge = 0;
}