/**
 * 算法层 - 图论算法实现
 * 与C语言版本保持一致
 */

class ZooAlgorithms {
    constructor(data) {
        this.data = data;
    }

    /**
     * Dijkstra算法 - 最短路径
     * @param {number} startIdx 起点ID
     * @param {number} endIdx 终点ID
     * @returns {Object} 路径结果
     */
    dijkstra(startIdx, endIdx) {
        const spots = this.data.getAllSpots();
        const adj = this.data.getAdjacencyList();
        const n = spots.length;

        if (startIdx < 0 || startIdx >= n || endIdx < 0 || endIdx >= n) {
            return null;
        }

        const dist = new Array(n).fill(Infinity);
        const prev = new Array(n).fill(-1);
        const visited = new Array(n).fill(false);
        const timeDist = new Array(n).fill(Infinity);

        dist[startIdx] = 0;
        timeDist[startIdx] = 0;

        for (let i = 0; i < n; i++) {
            let minDist = Infinity;
            let u = -1;

            for (let j = 0; j < n; j++) {
                if (!visited[j] && dist[j] < minDist) {
                    minDist = dist[j];
                    u = j;
                }
            }

            if (u === -1) break;
            visited[u] = true;

            if (u === endIdx) break;

            if (adj[u]) {
                adj[u].forEach(edge => {
                    const v = edge.to;
                    if (!visited[v]) {
                        const newDist = dist[u] + edge.distance;
                        const newTime = timeDist[u] + edge.time;
                        if (newDist < dist[v]) {
                            dist[v] = newDist;
                            prev[v] = u;
                            timeDist[v] = newTime;
                        }
                    }
                });
            }
        }

        if (dist[endIdx] === Infinity) {
            return null;
        }

        const path = [];
        let current = endIdx;
        while (current !== -1) {
            path.unshift(current);
            current = prev[current];
        }

        return {
            path,
            totalDistance: dist[endIdx],
            totalTime: timeDist[endIdx],
            spotCount: path.length
        };
    }

    /**
     * 生成随机路线（含公共厕所）
     * @param {number} maxTime 最大时间限制（分钟），0表示不限制
     * @returns {Object} 路线结果
     */
    generateRandomRoute(maxTime) {
        const spots = this.data.getAllSpots();
        const n = spots.length;

        // 找到入口
        const entranceIdx = spots.findIndex(s => s.type === SpotType.ENTRANCE);
        if (entranceIdx === -1) return null;

        // 找到所有公共厕所
        const restrooms = spots
            .map((s, i) => ({ ...s, idx: i }))
            .filter(s => s.type === SpotType.FACILITY && s.name.includes('厕所'));

        if (restrooms.length === 0) return null;

        // 收集候选景点（排除入口和厕所）
        const candidates = spots
            .map((s, i) => ({ ...s, idx: i }))
            .filter(s => s.type !== SpotType.ENTRANCE && !restrooms.find(r => r.idx === s.idx));

        if (candidates.length === 0) return null;

        // Fisher-Yates 洗牌
        for (let i = candidates.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [candidates[i], candidates[j]] = [candidates[j], candidates[i]];
        }

        // 选取3-5个景点
        const selectCount = Math.min(3 + Math.floor(Math.random() * 3), candidates.length);
        const selected = candidates.slice(0, selectCount);

        // 构建途经点序列
        const waypoints = [entranceIdx];
        const restroomInsertPos = selectCount >= 3 ? Math.floor(selectCount / 3) + 1 : 1;

        for (let i = 0; i < selectCount; i++) {
            waypoints.push(selected[i].idx);

            if (i + 1 === restroomInsertPos) {
                // 选择最近的厕所
                let bestRestroom = restrooms[0].idx;
                let bestDist = Infinity;
                restrooms.forEach(r => {
                    const result = this.dijkstra(selected[i].idx, r.idx);
                    if (result && result.totalDistance < bestDist) {
                        bestDist = result.totalDistance;
                        bestRestroom = r.idx;
                    }
                });
                waypoints.push(bestRestroom);
            }
        }

        // 确保有厕所
        const hasRestroom = waypoints.some(w => restrooms.find(r => r.idx === w));
        if (!hasRestroom) {
            let bestRestroom = restrooms[0].idx;
            let bestDist = Infinity;
            restrooms.forEach(r => {
                const result = this.dijkstra(waypoints[waypoints.length - 1], r.idx);
                if (result && result.totalDistance < bestDist) {
                    bestDist = result.totalDistance;
                    bestRestroom = r.idx;
                }
            });
            waypoints.push(bestRestroom);
        }

        // 拼接完整路径
        const fullPath = [];
        let totalDist = 0;
        let totalTime = 0;

        for (let i = 0; i < waypoints.length - 1; i++) {
            const seg = this.dijkstra(waypoints[i], waypoints[i + 1]);
            if (!seg) return null;

            const start = i === 0 ? 0 : 1;
            for (let j = start; j < seg.path.length; j++) {
                fullPath.push(seg.path[j]);
            }
            totalDist += seg.totalDistance;
            totalTime += seg.totalTime;
        }

        // 计算游览时间
        let visitTotal = 0;
        fullPath.forEach(idx => {
            visitTotal += spots[idx].visitTime;
        });
        totalTime += visitTotal;

        return {
            path: fullPath,
            totalDistance: totalDist,
            walkTime: totalTime - visitTotal,
            visitTime: visitTotal,
            totalTime,
            spotCount: fullPath.length,
            restrooms: fullPath.filter(idx => restrooms.find(r => r.idx === idx))
        };
    }

    /**
     * 贪心路线规划
     * @param {number} startIdx 起始景点ID
     * @param {number} timeLimit 时间限制（分钟）
     * @returns {Object} 路线结果
     */
    planBestRoute(startIdx, timeLimit) {
        const spots = this.data.getAllSpots();
        const adj = this.data.getAdjacencyList();
        const n = spots.length;

        const visited = new Array(n).fill(false);
        const route = [];
        let totalTime = 0;
        let totalDistance = 0;
        let current = startIdx;

        while (totalTime < timeLimit && route.length < n) {
            visited[current] = true;
            route.push(current);
            totalTime += spots[current].visitTime;

            let nearest = -1;
            let minDist = Infinity;

            if (adj[current]) {
                adj[current].forEach(edge => {
                    if (!visited[edge.to]) {
                        const estTime = totalTime + edge.time + spots[edge.to].visitTime;
                        if (estTime <= timeLimit && edge.distance < minDist) {
                            minDist = edge.distance;
                            nearest = edge.to;
                        }
                    }
                });
            }

            if (nearest === -1) break;

            totalDistance += minDist;
            totalTime += Math.ceil(minDist / 50);
            current = nearest;
        }

        return {
            path: route,
            totalDistance,
            totalTime,
            spotCount: route.length
        };
    }

    /**
     * 获取所有景点间的路径（用于展示）
     * @returns {Array} 路径列表
     */
    getAllPaths() {
        const spots = this.data.getAllSpots();
        const paths = [];

        for (let i = 0; i < spots.length; i++) {
            for (let j = i + 1; j < spots.length; j++) {
                const result = this.dijkstra(i, j);
                if (result) {
                    paths.push({
                        from: spots[i].name,
                        to: spots[j].name,
                        path: result.path.map(idx => spots[idx].name),
                        distance: result.totalDistance,
                        time: result.totalTime
                    });
                }
            }
        }

        return paths;
    }
}

// 导出算法实例
const zooAlgorithms = new ZooAlgorithms(zooData);
