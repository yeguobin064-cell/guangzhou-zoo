/**
 * 数据层 - 景点和路径数据管理
 * 基于高德地图坐标系，与C语言版本保持一致
 */

/* ==================== 数据版本（坐标修正后递增以自动清除旧缓存） ==================== */
const DATA_VERSION = 8;

/* ==================== 常量定义 ==================== */

const SpotType = {
    ENTRANCE: 0,
    ANIMAL_HOUSE: 1,
    SCENIC_SPOT: 2,
    FACILITY: 3,
    EDUCATION: 4
};

const SpotTypeName = {
    [SpotType.ENTRANCE]: '入口',
    [SpotType.ANIMAL_HOUSE]: '展馆',
    [SpotType.SCENIC_SPOT]: '景点',
    [SpotType.FACILITY]: '设施',
    [SpotType.EDUCATION]: '科普'
};

const SpotTypeColor = {
    [SpotType.ENTRANCE]: '#4CAF50',
    [SpotType.ANIMAL_HOUSE]: '#FF9800',
    [SpotType.SCENIC_SPOT]: '#2196F3',
    [SpotType.FACILITY]: '#9C27B0',
    [SpotType.EDUCATION]: '#00BCD4'
};

const SpotTypeIcon = {
    [SpotType.ENTRANCE]: '🚪',
    [SpotType.ANIMAL_HOUSE]: '🐾',
    [SpotType.SCENIC_SPOT]: '🏞️',
    [SpotType.FACILITY]: '🚻',
    [SpotType.EDUCATION]: '📚'
};

/* ==================== 水域与陆地绕行 ==================== */

/*
 * 路线只能走陆地。水禽湖的范围使用地图坐标框表示，外扩的缓冲距离让
 * 路线与湖岸保持一点间隔，避免折线刚好贴着或穿过水面。
 */
const WATER_AREAS = [
    {
        name: '水禽湖',
        minLng: 113.30485,
        maxLng: 113.30625,
        minLat: 23.14180,
        maxLat: 23.14345
    }
];
const LAND_ROUTE_CLEARANCE = 0.00012;
const ROUTE_EPSILON = 1e-10;

function pointInWater(point, water) {
    return point[0] >= water.minLng - ROUTE_EPSILON && point[0] <= water.maxLng + ROUTE_EPSILON &&
        point[1] >= water.minLat - ROUTE_EPSILON && point[1] <= water.maxLat + ROUTE_EPSILON;
}

function orientation(a, b, c) {
    const value = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]);
    if (Math.abs(value) <= ROUTE_EPSILON) return 0;
    return value > 0 ? 1 : -1;
}

function pointOnSegment(a, b, point) {
    return Math.min(a[0], b[0]) - ROUTE_EPSILON <= point[0] && point[0] <= Math.max(a[0], b[0]) + ROUTE_EPSILON &&
        Math.min(a[1], b[1]) - ROUTE_EPSILON <= point[1] && point[1] <= Math.max(a[1], b[1]) + ROUTE_EPSILON;
}

function segmentsIntersect(a, b, c, d) {
    const o1 = orientation(a, b, c);
    const o2 = orientation(a, b, d);
    const o3 = orientation(c, d, a);
    const o4 = orientation(c, d, b);
    if (o1 !== o2 && o3 !== o4) return true;
    return (o1 === 0 && pointOnSegment(a, b, c)) ||
        (o2 === 0 && pointOnSegment(a, b, d)) ||
        (o3 === 0 && pointOnSegment(c, d, a)) ||
        (o4 === 0 && pointOnSegment(c, d, b));
}

function segmentCrossesWater(start, end, water) {
    if (pointInWater(start, water) || pointInWater(end, water)) return true;
    const corners = [
        [water.minLng, water.minLat], [water.maxLng, water.minLat],
        [water.maxLng, water.maxLat], [water.minLng, water.maxLat]
    ];
    return corners.some((corner, index) => segmentsIntersect(start, end, corner, corners[(index + 1) % corners.length]));
}

function routeLength(a, b) {
    return Math.hypot(a[0] - b[0], a[1] - b[1]);
}

/* 使用湖区四个外扩角点构建可见图，选择最短的陆地绕行折线。 */
function detourAroundWater(start, end, water) {
    if (!segmentCrossesWater(start, end, water)) return [start, end];

    const c = LAND_ROUTE_CLEARANCE;
    const vertices = [
        start,
        [water.minLng - c, water.minLat - c],
        [water.maxLng + c, water.minLat - c],
        [water.maxLng + c, water.maxLat + c],
        [water.minLng - c, water.maxLat + c],
        end
    ];
    const distance = Array(vertices.length).fill(Infinity);
    const previous = Array(vertices.length).fill(-1);
    const visited = Array(vertices.length).fill(false);
    distance[0] = 0;

    for (let step = 0; step < vertices.length; step++) {
        let current = -1;
        for (let i = 0; i < vertices.length; i++) {
            if (!visited[i] && (current === -1 || distance[i] < distance[current])) current = i;
        }
        if (current === -1 || current === vertices.length - 1) break;
        visited[current] = true;

        for (let next = 0; next < vertices.length; next++) {
            if (visited[next] || segmentCrossesWater(vertices[current], vertices[next], water)) continue;
            const candidate = distance[current] + routeLength(vertices[current], vertices[next]);
            if (candidate < distance[next]) {
                distance[next] = candidate;
                previous[next] = current;
            }
        }
    }

    if (!Number.isFinite(distance[vertices.length - 1])) return [start, end];
    const route = [];
    for (let current = vertices.length - 1; current !== -1; current = previous[current]) {
        route.unshift(vertices[current]);
    }
    return route;
}

function getLandRoute(path) {
    return WATER_AREAS.reduce((safePath, water) => {
        const nextPath = [];
        for (let i = 0; i < safePath.length - 1; i++) {
            const segment = detourAroundWater(safePath[i], safePath[i + 1], water);
            if (i === 0) nextPath.push(segment[0]);
            nextPath.push(...segment.slice(1));
        }
        return nextPath;
    }, path);
}

/* ==================== 景点数据（高德坐标） ==================== */

const spots = [
    { id: 0, name: '正门',     desc: '广州动物园正门入口，设有售票处和游客服务中心', type: SpotType.ENTRANCE,     visitTime: 10, popularity: 8,  lng: 113.3032, lat: 23.1436 },
    { id: 1, name: '熊猫馆',   desc: '国宝大熊猫的家园，可近距离观赏可爱的大熊猫', type: SpotType.ANIMAL_HOUSE, visitTime: 30, popularity: 10, lng: 113.3062, lat: 23.1408 },
    { id: 2, name: '大象馆',   desc: '亚洲象和非洲象的栖息地，定时有大象表演',     type: SpotType.ANIMAL_HOUSE, visitTime: 25, popularity: 9,  lng: 113.3048, lat: 23.1400 },
    { id: 3, name: '海洋馆',   desc: '海豚、海狮等海洋动物的表演和展览',           type: SpotType.ANIMAL_HOUSE, visitTime: 40, popularity: 10, lng: 113.3048, lat: 23.1437 },
    { id: 4, name: '猴山',     desc: '各种灵长类动物的乐园，活泼可爱的猴子们',     type: SpotType.ANIMAL_HOUSE, visitTime: 25, popularity: 8,  lng: 113.3062, lat: 23.1414 },
    { id: 5, name: '虎山',     desc: '东北虎和华南虎的领地，威风凛凛的百兽之王',   type: SpotType.ANIMAL_HOUSE, visitTime: 20, popularity: 9,  lng: 113.3034, lat: 23.1415 },
    { id: 6, name: '水禽湖',   desc: '天鹅、鸳鸯等水鸟的栖息地，湖光山色',         type: SpotType.SCENIC_SPOT,  visitTime: 20, popularity: 8,  lng: 113.3064, lat: 23.1427 },
    { id: 7, name: '科普馆',   desc: '动物知识科普教育基地，设有互动展览',         type: SpotType.EDUCATION,    visitTime: 30, popularity: 7,  lng: 113.3052, lat: 23.1451 },
    { id: 8, name: '公共厕所A', desc: '正门附近公共厕所，靠近熊猫馆和大象馆',      type: SpotType.FACILITY,     visitTime: 5,  popularity: 5,  lng: 113.3046, lat: 23.1404 },
    { id: 9, name: '公共厕所B', desc: '园区中部公共厕所，靠近猴山和虎山',          type: SpotType.FACILITY,     visitTime: 5,  popularity: 5,  lng: 113.3064, lat: 23.1417 }
];

/* ==================== 路径数据 ==================== */

const edges = [
    { from: 0, to: 1, distance: 150 },
    { from: 0, to: 2, distance: 200 },
    { from: 1, to: 2, distance: 120 },
    { from: 1, to: 4, distance: 420 },
    { from: 2, to: 3, distance: 350,
      waypoints: [[113.3046,23.1404],[113.3062,23.1414]] },      // 经公共厕所A、猴山绕开水域
    { from: 2, to: 4, distance: 340 },
    { from: 3, to: 6, distance: 500,
      waypoints: [[113.3052,23.1451],[113.3064,23.1442]] },      // 经科普馆北侧绕开水域
    { from: 3, to: 7, distance: 320 },
    { from: 4, to: 5, distance: 190 },
    { from: 5, to: 6, distance: 520,
      waypoints: [[113.3034,23.1417],[113.3064,23.1417]] },      // 经南侧绕行至公共厕所B
    { from: 6, to: 7, distance: 180 },
    { from: 0, to: 8, distance: 80  },
    { from: 1, to: 8, distance: 100 },
    { from: 2, to: 8, distance: 120 },
    { from: 4, to: 9, distance: 90  },
    { from: 5, to: 9, distance: 110 },
    { from: 6, to: 9, distance: 150 },
    { from: 3, to: 9, distance: 200,
      waypoints: [[113.3048,23.1445],[113.3064,23.1445]] },      // 经北侧绕开水域
    { from: 7, to: 9, distance: 160 }
];

/*
 * Curated pedestrian network for the zoo. The intermediate nodes follow the
 * visible west, north, east, and south land corridors around the lake system.
 * Water is deliberately absent from this graph, so a route can never cross it.
 * Attraction marker positions are intentionally not used as walking nodes.
 */
const LAND_ROAD_NODES = {
    // West-side pedestrian corridor.
    gateWest: [113.30318, 23.14330],
    westNorth: [113.30348, 23.14390],
    westUpper: [113.30328, 23.14295],
    westMiddle: [113.30328, 23.14225],
    westLower: [113.30332, 23.14178],
    tigerNorth: [113.30348, 23.14128],
    tigerSouth: [113.30362, 23.14072],
    southWest: [113.30402, 23.14022],

    // Middle land corridor: west of the inner ponds and the main lake.
    middleEntry: [113.30370, 23.14302],
    middleNorth: [113.30395, 23.14288],
    middleWest: [113.30410, 23.14258],
    middleCenter: [113.30398, 23.14226],
    middleLower: [113.30392, 23.14190],
    middleSouth: [113.30398, 23.14152],
    middleExit: [113.30405, 23.14114],
    middleSouthExit: [113.30406, 23.14074],

    // Northern path.
    northWest: [113.30412, 23.14400],
    oceanWest: [113.30448, 23.14392],
    northArc: [113.30488, 23.14447],
    scienceWest: [113.30508, 23.14478],
    scienceEast: [113.30556, 23.14492],
    northEast: [113.30602, 23.14460],

    // East-side path along the lake shore.
    eastUpper: [113.30655, 23.14382],
    waterfowlNorth: [113.30658, 23.14315],
    waterfowlAccess: [113.30658, 23.14272],
    eastMiddle: [113.30656, 23.14210],
    restroomBAccess: [113.30650, 23.14172],
    eastLower: [113.30648, 23.14132],
    pandaAccess: [113.30635, 23.14092],

    // Southern pedestrian corridor.
    southEast: [113.30582, 23.14048],
    restroomAAccess: [113.30472, 23.14040],
    elephantAccess: [113.30496, 23.14016]
};

/*
 * A marker can represent a building or a lake rather than a point visitors
 * can stand on. Routes end at its nearest land-side entrance instead.
 */
const LAND_ACCESS_POINTS = {
    6: [113.30658, 23.14272]
};

/*
 * These are curated display vertices for walkable paths, not connections
 * between attraction markers. Every link stays on a surveyed land corridor;
 * the middle branch passes west of the ponds rather than across the lake.
 */
const LAND_ROAD_LINKS = [
    ["spot-0", "gateWest"], ["gateWest", "westNorth"],
    ["westNorth", "westUpper"], ["westUpper", "westMiddle"],
    ["westMiddle", "westLower"], ["westLower", "tigerNorth"],
    ["tigerNorth", "spot-5"], ["spot-5", "tigerSouth"],
    ["tigerSouth", "southWest"],

    ["westUpper", "middleEntry"], ["middleEntry", "middleNorth"],
    ["middleNorth", "middleWest"], ["middleWest", "middleCenter"],
    ["middleCenter", "middleLower"], ["middleLower", "middleSouth"],
    ["middleSouth", "middleExit"], ["middleExit", "middleSouthExit"],
    ["middleSouthExit", "southWest"],

    ["westNorth", "northWest"], ["northWest", "oceanWest"],
    ["oceanWest", "spot-3"],
    ["northWest", "northArc"], ["northArc", "scienceWest"],
    ["scienceWest", "spot-7"], ["spot-7", "scienceEast"],
    ["scienceEast", "northEast"],

    ["northEast", "eastUpper"], ["eastUpper", "waterfowlNorth"],
    ["waterfowlNorth", "waterfowlAccess"], ["waterfowlAccess", "spot-6"],
    ["waterfowlNorth", "eastMiddle"], ["eastMiddle", "restroomBAccess"],
    ["restroomBAccess", "spot-9"], ["restroomBAccess", "eastLower"],
    ["eastLower", "spot-4"], ["eastLower", "pandaAccess"],
    ["pandaAccess", "spot-1"], ["pandaAccess", "southEast"],

    ["southEast", "restroomAAccess"], ["restroomAAccess", "spot-8"],
    ["restroomAAccess", "elephantAccess"], ["elephantAccess", "spot-2"],
    ["restroomAAccess", "southWest"]
];

function landDistance(a, b) {
    const rad = Math.PI / 180;
    const lat1 = a[1] * rad;
    const lat2 = b[1] * rad;
    const deltaLat = (b[1] - a[1]) * rad;
    const deltaLng = (b[0] - a[0]) * rad;
    const h = Math.sin(deltaLat / 2) ** 2 + Math.cos(lat1) * Math.cos(lat2) * Math.sin(deltaLng / 2) ** 2;
    return 6371000 * 2 * Math.atan2(Math.sqrt(h), Math.sqrt(1 - h));
}

/* ==================== 数据管理类 ==================== */

class ZooData {
    constructor() {
        this._listeners = [];
        this._routeDistances = new Map();
        this._landRouteCache = new Map();
        this._load();
    }

    /* localStorage 持久化 */
    _save() {
        try {
            localStorage.setItem('zoo_spots', JSON.stringify(this._spots));
            localStorage.setItem('zoo_edges', JSON.stringify(this._edges));
            localStorage.setItem('zoo_nextId', this._nextId);
        } catch (e) { console.warn('保存失败:', e); }
    }

    _load() {
        try {
            const savedVersion = parseInt(localStorage.getItem('zoo_data_version')) || 0;
            if (savedVersion !== DATA_VERSION) {
                localStorage.removeItem('zoo_spots');
                localStorage.removeItem('zoo_edges');
                localStorage.removeItem('zoo_nextId');
                localStorage.setItem('zoo_data_version', DATA_VERSION);
            }
            const savedSpots = localStorage.getItem('zoo_spots');
            const savedEdges = localStorage.getItem('zoo_edges');
            if (savedSpots && savedEdges) {
                this._spots = JSON.parse(savedSpots);
                this._edges = JSON.parse(savedEdges);
                this._nextId = parseInt(localStorage.getItem('zoo_nextId')) || this._spots.length;
            } else {
                this._spots = JSON.parse(JSON.stringify(spots));
                this._edges = JSON.parse(JSON.stringify(edges));
                this._nextId = spots.length;
            }
        } catch (e) {
            this._spots = JSON.parse(JSON.stringify(spots));
            this._edges = JSON.parse(JSON.stringify(edges));
            this._nextId = spots.length;
        }
    }

    /* 重置为默认数据 */
    reset() {
        localStorage.removeItem('zoo_spots');
        localStorage.removeItem('zoo_edges');
        localStorage.removeItem('zoo_nextId');
        this._load();
        this._notify();
    }

    /* 事件订阅 */
    onChange(fn) { this._listeners.push(fn); }
    _notify() {
        this._routeDistances.clear();
        this._landRouteCache.clear();
        this._save();
        this._listeners.forEach(fn => fn());
    }

    /* 查询 */
    getAllSpots() { return this._spots; }
    getSpotById(id) { return this._spots.find(s => s.id === id); }
    getSpotByName(name) { return this._spots.find(s => s.name === name); }
    getSpotsByType(type) { return this._spots.filter(s => s.type === type); }
    getAllEdges() { return this._edges; }

    _edgeKey(fromId, toId) {
        return fromId < toId ? `${fromId}-${toId}` : `${toId}-${fromId}`;
    }

    setRouteDistance(fromId, toId, distance) {
        if (!Number.isFinite(distance) || distance <= 0) return;
        this._routeDistances.set(this._edgeKey(fromId, toId), Math.round(distance));
    }

    getRouteDistance(fromId, toId) {
        const key = this._edgeKey(fromId, toId);
        if (this._routeDistances.has(key)) return this._routeDistances.get(key);
        const route = this.getLandRoute(fromId, toId);
        return route ? route.distance : null;
    }

    _roadCoordinates() {
        const coordinates = { ...LAND_ROAD_NODES };
        this._spots.forEach(spot => {
            coordinates[`spot-${spot.id}`] = this.getRouteAccessPoint(spot.id);
        });
        return coordinates;
    }

    getRouteAccessPoint(spotId) {
        const accessPoint = LAND_ACCESS_POINTS[spotId];
        if (accessPoint) return [...accessPoint];
        const spot = this.getSpotById(spotId);
        return spot ? [spot.lng, spot.lat] : null;
    }

    getLandRoadSegments() {
        const coordinates = this._roadCoordinates();
        return LAND_ROAD_LINKS
            .filter(([from, to]) => coordinates[from] && coordinates[to])
            .map(([from, to]) => [coordinates[from], coordinates[to]]);
    }

    /**
     * Find a route on the curated pedestrian network. Its vertices are all
     * land-side paths, so the returned polyline is also the routing geometry.
     */
    getLandRoute(fromId, toId) {
        const cacheKey = `${fromId}-${toId}`;
        if (this._landRouteCache.has(cacheKey)) return this._landRouteCache.get(cacheKey);

        const coordinates = this._roadCoordinates();
        const start = `spot-${fromId}`;
        const end = `spot-${toId}`;
        if (!coordinates[start] || !coordinates[end]) return null;

        const adjacency = Object.fromEntries(Object.keys(coordinates).map(key => [key, []]));
        LAND_ROAD_LINKS.forEach(([from, to]) => {
            if (!coordinates[from] || !coordinates[to]) return;
            const distance = landDistance(coordinates[from], coordinates[to]);
            adjacency[from].push({ to, distance });
            adjacency[to].push({ to: from, distance });
        });

        const distance = Object.fromEntries(Object.keys(coordinates).map(key => [key, Infinity]));
        const previous = {};
        const visited = new Set();
        distance[start] = 0;

        while (visited.size < Object.keys(coordinates).length) {
            let current = null;
            Object.keys(coordinates).forEach(key => {
                if (!visited.has(key) && (current === null || distance[key] < distance[current])) current = key;
            });
            if (current === null || !Number.isFinite(distance[current])) break;
            if (current === end) break;
            visited.add(current);
            adjacency[current].forEach(link => {
                const nextDistance = distance[current] + link.distance;
                if (nextDistance < distance[link.to]) {
                    distance[link.to] = nextDistance;
                    previous[link.to] = current;
                }
            });
        }
        if (!Number.isFinite(distance[end])) return null;

        const nodePath = [];
        for (let current = end; current; current = previous[current]) {
            nodePath.unshift(current);
            if (current === start) break;
        }
        const route = {
            path: nodePath.map(key => coordinates[key]),
            distance: Math.round(distance[end])
        };
        this._landRouteCache.set(cacheKey, route);
        this.setRouteDistance(fromId, toId, route.distance);
        return route;
    }

    getEdgePath(fromId, toId) {
        const edge = this._edges.find(e =>
            (e.from === fromId && e.to === toId) || (e.from === toId && e.to === fromId)
        );
        if (!edge) return null;
        return this.getLandRoute(fromId, toId)?.path ?? null;
    }

    getAdjacencyList() {
        const adj = {};
        this._spots.forEach(s => { adj[s.id] = []; });
        this._edges.forEach(e => {
            // 只采用步行导航返回的实际陆地距离；没有道路数据的边不参与最短路径。
            const distance = this.getRouteDistance(e.from, e.to);
            if (distance === null) return;
            const time = Math.ceil(distance / 50);
            adj[e.from].push({ to: e.to, distance, time });
            adj[e.to].push({ to: e.from, distance, time });
        });
        return adj;
    }

    /* 新增 */
    addSpot(name, desc, type, visitTime, popularity, lng, lat) {
        if (this._spots.find(s => s.name === name)) {
            return { success: false, message: '景点名称已存在' };
        }
        const spot = {
            id: this._nextId++,
            name, desc,
            type: parseInt(type),
            visitTime: parseInt(visitTime),
            popularity: parseInt(popularity),
            lng: parseFloat(lng) || 113.3050 + (Math.random() - 0.5) * 0.004,
            lat: parseFloat(lat) || 23.1425 + (Math.random() - 0.5) * 0.006
        };
        this._spots.push(spot);
        this._notify();
        return { success: true, message: '添加成功', spot };
    }

    /* 删除 */
    deleteSpot(id) {
        const idx = this._spots.findIndex(s => s.id === id);
        if (idx === -1) return { success: false, message: '景点不存在' };
        this._spots.splice(idx, 1);
        this._edges = this._edges.filter(e => e.from !== id && e.to !== id);
        this._notify();
        return { success: true, message: '删除成功' };
    }

    /* 修改 */
    updateSpot(id, updates) {
        const spot = this._spots.find(s => s.id === id);
        if (!spot) return { success: false, message: '景点不存在' };
        if (updates.name && updates.name !== spot.name) {
            if (this._spots.find(s => s.name === updates.name))
                return { success: false, message: '景点名称已存在' };
            spot.name = updates.name;
        }
        if (updates.desc) spot.desc = updates.desc;
        if (updates.type !== undefined) spot.type = parseInt(updates.type);
        if (updates.visitTime !== undefined) spot.visitTime = parseInt(updates.visitTime);
        if (updates.popularity !== undefined) spot.popularity = parseInt(updates.popularity);
        if (updates.lng !== undefined) spot.lng = parseFloat(updates.lng);
        if (updates.lat !== undefined) spot.lat = parseFloat(updates.lat);
        this._notify();
        return { success: true, message: '修改成功', spot };
    }
}

const zooData = new ZooData();
