#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 常量定义 */
#define INF 32767              /* 表示无穷大（不可达） */
#define MAX_NAME_LEN 50        /* 最大景点名称长度 */
#define MAX_VERTEX_NUM 50      /* 最大景点数量 */
#define MAX_EDGE_NUM 200       /* 最大路径数量 */
#define MAX_DESC_LEN 200       /* 最大描述长度 */
#define MAX_ANIMAL_NUM 10      /* 每个展馆最大动物数量 */

/* 颜色定义（ANSI转义码） */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_UNDERLINE "\033[4m"

/* 背景色定义 */
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

/**
 * @brief 状态枚举
 * 用于表示操作是否成功
 */
typedef enum {
    OK = 0,
    ERROR = -1,
    NOT_FOUND = -2
} Status;

/**
 * @brief 景点类型枚举
 */
typedef enum {
    TYPE_ENTRANCE = 0,    /* 入口 */
    TYPE_ANIMAL_HOUSE,    /* 动物展馆 */
    TYPE_SCENIC_SPOT,     /* 景点 */
    TYPE_FACILITY,        /* 设施 */
    TYPE_EDUCATION        /* 科普教育 */
} SpotType;

/**
 * @brief 动物信息结构体
 */
typedef struct {
    char name[MAX_NAME_LEN];         /* 动物名称 */
    char species[MAX_NAME_LEN];      /* 物种 */
    char description[MAX_DESC_LEN];  /* 描述 */
    int popularity;                  /* 受欢迎程度 (1-10) */
} Animal;

/**
 * @brief 景点信息结构体
 */
typedef struct {
    char name[MAX_NAME_LEN];         /* 景点名称 */
    char description[MAX_DESC_LEN];  /* 景点描述 */
    SpotType type;                   /* 景点类型 */
    int visitTime;                   /* 建议游览时间（分钟） */
    int popularity;                  /* 受欢迎程度 (1-10) */
    Animal animals[MAX_ANIMAL_NUM];  /* 动物列表 */
    int animalCount;                 /* 动物数量 */
} SpotInfo;

/**
 * @brief 邻接表节点（边节点）
 */
typedef struct CNode {
    int index;           /* 邻接点的数组下标 */
    int length;          /* 边的权值（距离，米） */
    int time;            /* 步行时间（分钟） */
    struct CNode* next;  /* 下一个邻接点 */
} CNode, *CList;

/**
 * @brief 顶点表节点
 */
typedef struct Vnode {
    char name[MAX_NAME_LEN];  /* 景点名称 */
    SpotInfo info;            /* 景点详细信息 */
    CList first;              /* 第一个邻接点的指针 */
} Vnode;

/**
 * @brief 邻接表表示的图
 */
typedef struct {
    Vnode roadList[MAX_VERTEX_NUM];  /* 顶点数组 */
    int numVertex;                   /* 顶点数量 */
    int numEdge;                     /* 边数量 */
} ALGraph;

/**
 * @brief 路径结果结构体
 */
typedef struct {
    int path[MAX_VERTEX_NUM];     /* 路径数组 */
    int pathLength;               /* 路径长度 */
    int totalDistance;             /* 总距离（米） */
    int totalTime;                 /* 总时间（分钟） */
    int spotCount;                 /* 经过景点数量 */
} PathResult;

/**
 * @brief 颜色主题结构体
 */
typedef struct {
    const char* title;        /* 标题颜色 */
    const char* menu;         /* 菜单颜色 */
    const char* highlight;    /* 高亮颜色 */
    const char* success;      /* 成功颜色 */
    const char* error;        /* 错误颜色 */
    const char* info;         /* 信息颜色 */
    const char* border;       /* 边框颜色 */
    const char* text;         /* 文本颜色 */
} ColorTheme;

#endif /* GLOBAL_H */