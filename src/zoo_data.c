#include "global.h"
#include "graph.h"

/**
 * @brief 初始化广州动物园数据
 * @param graph 图的指针
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status InitGuangzhouZoo(ALGraph *graph) {
    if (graph == NULL) {
        return ERROR;
    }

    /* 初始化图 */
    graph->numVertex = 0;
    graph->numEdge = 0;

    /* 定义8个精选景点 */
    struct {
        const char *name;
        const char *desc;
        SpotType type;
        int visitTime;
        int popularity;
    } spots[] = {
        {"正门", "广州动物园正门入口，设有售票处和游客服务中心", TYPE_ENTRANCE, 10, 8},
        {"熊猫馆", "国宝大熊猫的家园，可近距离观赏可爱的大熊猫", TYPE_ANIMAL_HOUSE, 30, 10},
        {"大象馆", "亚洲象和非洲象的栖息地，定时有大象表演", TYPE_ANIMAL_HOUSE, 25, 9},
        {"海洋馆", "海豚、海狮等海洋动物的表演和展览", TYPE_ANIMAL_HOUSE, 40, 10},
        {"猴山", "各种灵长类动物的乐园，活泼可爱的猴子们", TYPE_ANIMAL_HOUSE, 25, 8},
        {"虎山", "东北虎和华南虎的领地，威风凛凛的百兽之王", TYPE_ANIMAL_HOUSE, 20, 9},
        {"水禽湖", "天鹅、鸳鸯等水鸟的栖息地，湖光山色", TYPE_SCENIC_SPOT, 20, 8},
        {"科普馆", "动物知识科普教育基地，设有互动展览", TYPE_EDUCATION, 30, 7},
        {"公共厕所A", "正门附近公共厕所，靠近熊猫馆和大象馆", TYPE_FACILITY, 5, 5},
        {"公共厕所B", "园区中部公共厕所，靠近猴山和虎山", TYPE_FACILITY, 5, 5}
    };

    int spotCount = sizeof(spots) / sizeof(spots[0]);

    /* 添加顶点 */
    for (int i = 0; i < spotCount; i++) {
        strcpy(graph->roadList[i].name, spots[i].name);
        strcpy(graph->roadList[i].info.description, spots[i].desc);
        graph->roadList[i].info.type = spots[i].type;
        graph->roadList[i].info.visitTime = spots[i].visitTime;
        graph->roadList[i].info.popularity = spots[i].popularity;
        graph->roadList[i].info.animalCount = 0;
        graph->roadList[i].first = NULL;
        graph->numVertex++;
    }

    /* 添加边（景点之间的连接） */
    int edges[][3] = {
        /* 起点, 终点, 距离(米) */
        {0, 1, 150},   /* 正门 -> 熊猫馆 */
        {0, 2, 200},   /* 正门 -> 大象馆 */
        {1, 2, 120},   /* 熊猫馆 -> 大象馆 */
        {1, 4, 420},   /* 熊猫馆 -> 猴山 */
        {2, 3, 350},   /* 大象馆 -> 海洋馆 */
        {2, 4, 340},   /* 大象馆 -> 猴山 */
        {3, 6, 500},   /* 海洋馆 -> 水禽湖 */
        {3, 7, 320},   /* 海洋馆 -> 科普馆 */
        {4, 5, 190},   /* 猴山 -> 虎山 */
        {5, 6, 520},   /* 虎山 -> 水禽湖 */
        {6, 7, 180},   /* 水禽湖 -> 科普馆 */
        /* 公共厕所连接 */
        {0, 8, 80},    /* 正门 -> 公共厕所A */
        {1, 8, 100},   /* 熊猫馆 -> 公共厕所A */
        {2, 8, 120},   /* 大象馆 -> 公共厕所A */
        {4, 9, 90},    /* 猴山 -> 公共厕所B */
        {5, 9, 110},   /* 虎山 -> 公共厕所B */
        {6, 9, 150},   /* 水禽湖 -> 公共厕所B */
        {3, 9, 200},   /* 海洋馆 -> 公共厕所B */
        {7, 9, 160},   /* 科普馆 -> 公共厕所B */
    };

    int edgeCount = sizeof(edges) / sizeof(edges[0]);

    for (int i = 0; i < edgeCount; i++) {
        int from = edges[i][0];
        int to = edges[i][1];
        int dist = edges[i][2];
        int time = dist / 50;  /* 假设步行速度50米/分钟 */

        /* 添加正向边 */
        CNode *node1 = (CNode *)malloc(sizeof(CNode));
        node1->index = to;
        node1->length = dist;
        node1->time = time;
        node1->next = graph->roadList[from].first;
        graph->roadList[from].first = node1;

        /* 添加反向边（无向图） */
        CNode *node2 = (CNode *)malloc(sizeof(CNode));
        node2->index = from;
        node2->length = dist;
        node2->time = time;
        node2->next = graph->roadList[to].first;
        graph->roadList[to].first = node2;

        graph->numEdge++;
    }

    return OK;
}
