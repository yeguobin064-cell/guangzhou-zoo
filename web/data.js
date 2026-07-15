/**
 * 数据层 - 景点和路径数据管理
 * 基于高德地图坐标系，与C语言版本保持一致
 */

/* ==================== 数据版本（坐标修正后递增以自动清除旧缓存） ==================== */
const DATA_VERSION = 2;

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
    { from: 2, to: 3, distance: 350 },
    { from: 2, to: 4, distance: 340 },
    { from: 3, to: 6, distance: 500 },
    { from: 3, to: 7, distance: 320 },
    { from: 4, to: 5, distance: 190 },
    { from: 5, to: 6, distance: 520 },
    { from: 6, to: 7, distance: 180 },
    { from: 0, to: 8, distance: 80  },
    { from: 1, to: 8, distance: 100 },
    { from: 2, to: 8, distance: 120 },
    { from: 4, to: 9, distance: 90  },
    { from: 5, to: 9, distance: 110 },
    { from: 6, to: 9, distance: 150 },
    { from: 3, to: 9, distance: 200 },
    { from: 7, to: 9, distance: 160 }
];

/* ==================== 数据管理类 ==================== */

class ZooData {
    constructor() {
        this._listeners = [];
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
    _notify() { this._save(); this._listeners.forEach(fn => fn()); }

    /* 查询 */
    getAllSpots() { return this._spots; }
    getSpotById(id) { return this._spots.find(s => s.id === id); }
    getSpotByName(name) { return this._spots.find(s => s.name === name); }
    getSpotsByType(type) { return this._spots.filter(s => s.type === type); }
    getAllEdges() { return this._edges; }

    getAdjacencyList() {
        const adj = {};
        this._spots.forEach(s => { adj[s.id] = []; });
        this._edges.forEach(e => {
            const time = Math.ceil(e.distance / 50);
            adj[e.from].push({ to: e.to, distance: e.distance, time });
            adj[e.to].push({ to: e.from, distance: e.distance, time });
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
