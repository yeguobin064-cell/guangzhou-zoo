#ifndef GRAPH_H
#define GRAPH_H

#include "global.h"

/**
 * @brief 创建景区景点图
 * @param graph 图的指针
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status CreateSceneGraph(ALGraph *graph);

/**
 * @brief 从文件加载景区景点图
 * @param graph 图的指针
 * @param filename 文件名
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status CreateSceneGraphByFile(ALGraph *graph, const char *filename);

/**
 * @brief 初始化广州动物园数据
 * @param graph 图的指针
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status InitGuangzhouZoo(ALGraph *graph);

/**
 * @brief 按名称定位顶点下标
 * @param graph 图的指针
 * @param name 景点名称
 * @return int 景点下标，未找到返回 -1
 */
int LocateVertex(const ALGraph *graph, const char *name);

/**
 * @brief 以邻接矩阵形式展示景区分布图
 * @param graph 图的指针
 */
void ShowGraph(const ALGraph *graph);

/**
 * @brief 显示所有景点信息
 * @param graph 图的指针
 */
void ShowVertex(const ALGraph *graph);

/**
 * @brief 显示景点详细信息
 * @param graph 图的指针
 * @param index 景点下标
 */
void ShowSpotDetail(const ALGraph *graph, int index);

/**
 * @brief 基于 DFS 输出导游线路
 * @param graph 图的指针
 * @param startIdx 起始景点下标
 */
void TourRoute(const ALGraph *graph, int startIdx);

/**
 * @brief 检测导游线路中的回路并输出
 * @param graph 图的指针
 */
void FindCycle(const ALGraph *graph);

/**
 * @brief Dijkstra算法求解最短路径
 * @param graph 图的指针
 * @param startIdx 起点下标
 * @param endIdx 终点下标
 * @param result 路径结果
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status Dijkstra(const ALGraph *graph, int startIdx, int endIdx, PathResult *result);

/**
 * @brief Floyd算法求解所有顶点之间的最短路径
 * @param graph 图的指针
 * @param dist 距离矩阵
 * @param path 路径矩阵
 */
void Floyd(const ALGraph *graph, int dist[][MAX_VERTEX_NUM], int path[][MAX_VERTEX_NUM]);

/**
 * @brief 查询两个景点之间的最短路径
 * @param graph 图的指针
 * @param startName 起点名称
 * @param endName 终点名称
 */
void QueryShortestPath(const ALGraph *graph, const char *startName, const char *endName);

/**
 * @brief 规划最佳游览路线
 * @param graph 图的指针
 * @param startIdx 起始景点下标
 * @param timeLimit 时间限制（分钟）
 */
void PlanBestRoute(const ALGraph *graph, int startIdx, int timeLimit);

/**
 * @brief 释放图的内存
 * @param graph 图的指针
 */
void FreeGraph(ALGraph *graph);

/**
 * @brief 添加新景点
 * @param graph 图的指针
 * @param name 景点名称
 * @param desc 景点描述
 * @param type 景点类型
 * @param visitTime 游览时间
 * @param popularity 受欢迎程度
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status AddSpot(ALGraph *graph, const char *name, const char *desc, SpotType type, int visitTime, int popularity);

/**
 * @brief 删除景点
 * @param graph 图的指针
 * @param index 景点下标
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status DeleteSpot(ALGraph *graph, int index);

/**
 * @brief 按名称查询景点
 * @param graph 图的指针
 * @param name 景点名称
 * @return int 景点下标，未找到返回 -1
 */
int SearchSpotByName(const ALGraph *graph, const char *name);

/**
 * @brief 按类型查询景点
 * @param graph 图的指针
 * @param type 景点类型
 * @param result 结果数组
 * @return int 结果数量
 */
int SearchSpotByType(const ALGraph *graph, SpotType type, int *result);

/**
 * @brief 修改景点信息
 * @param graph 图的指针
 * @param index 景点下标
 * @param name 新名称（NULL表示不修改）
 * @param desc 新描述（NULL表示不修改）
 * @param type 新类型（-1表示不修改）
 * @param visitTime 新游览时间（-1表示不修改）
 * @param popularity 新受欢迎程度（-1表示不修改）
 * @return Status 成功返回 OK，失败返回 ERROR
 */
Status UpdateSpot(ALGraph *graph, int index, const char *name, const char *desc, int type, int visitTime, int popularity);

/**
 * @brief 生成随机游览路线（确保经过公共厕所）
 * @param graph 图的指针
 * @param maxTime 最大游览时间限制（分钟），0表示不限制
 */
void GenerateRandomRoute(const ALGraph *graph, int maxTime);

#endif /* GRAPH_H */