/**
 * 应用控制器 - 主业务逻辑
 * 支持用户端/管理端双角色切换
 */

class ZooApp {
    constructor() {
        this.map = null;
        this.role = null;         // "user" | "admin"
        this.currentView = "map";
        this.mapInitialized = false;
    }

    /* ==================== 初始化 ==================== */

    init() {
        this._bindRoleSelection();
        this._bindSwitchRole();
    }

    /* ==================== 角色选择 ==================== */

    _bindRoleSelection() {
        document.querySelectorAll(".role-card").forEach(card => {
            card.querySelector("button").addEventListener("click", () => {
                this.enterApp(card.dataset.role);
            });
        });
    }

    _bindSwitchRole() {
        document.getElementById("btn-switch-role")?.addEventListener("click", () => {
            if (this.map) { this.map.destroy(); this.map = null; }
            this.mapInitialized = false;
            document.getElementById("page-app").style.display = "none";
            document.getElementById("page-role").style.display = "flex";
        });
    }

    async enterApp(role) {
        this.role = role;

        // 切换页面
        document.getElementById("page-role").style.display = "none";
        document.getElementById("page-app").style.display = "flex";

        // 角色标签
        document.getElementById("role-label").textContent = role === "admin" ? "🛡️ 管理端" : "👤 游客端";

        // 管理端专属功能
        const navManage = document.getElementById("nav-manage");
        const adminStats = document.getElementById("admin-stats");
        if (role === "admin") {
            navManage.style.display = "flex";
            adminStats.style.display = "block";
        } else {
            navManage.style.display = "none";
            adminStats.style.display = "none";
        }

        // 初始化地图（只初始化一次）
        if (!this.mapInitialized) {
            this.map = new ZooMap("amap-container");
            await this.map.init((id) => this.onSpotClick(id));
            this.mapInitialized = true;
        } else {
            this.map.renderAll();
        }

        this._bindNav();
        this._bindEvents();
        this._showView("map");
    }

    /* ==================== 导航 ==================== */

    _bindNav() {
        document.querySelectorAll(".nav-item").forEach(el => {
            el.onclick = () => this._showView(el.dataset.view);
        });
    }

    _showView(view) {
        // 用户端不能进入管理视图
        if (view === "manage" && this.role !== "admin") return;

        this.currentView = view;
        document.querySelectorAll(".view").forEach(v => v.classList.remove("active"));
        document.querySelectorAll(".nav-item").forEach(n => n.classList.remove("active"));
        document.getElementById(`view-${view}`)?.classList.add("active");
        document.querySelector(`[data-view="${view}"]`)?.classList.add("active");

        if (view === "map")    { this.map.renderAll(); if (this.role === "admin") this._renderStats(); }
        if (view === "spots")  this._renderSpots();
        if (view === "path")   this._renderPathPanel();
        if (view === "manage") this._renderManage();
    }

    /* ==================== 事件绑定 ==================== */

    _bindEvents() {
        document.getElementById("btn-search-path")?.addEventListener("click", () => this._searchPath());
        document.getElementById("btn-random-route")?.addEventListener("click", () => this._randomRoute());
        document.getElementById("btn-add-spot")?.addEventListener("click", () => this._showAddForm());
        document.getElementById("btn-search")?.addEventListener("click", () => this._searchSpots());
        document.querySelector(".modal-close")?.addEventListener("click", () => Modal.close());
        document.getElementById("modal-overlay")?.addEventListener("click", (e) => { if (e.target === e.currentTarget) Modal.close(); });
        document.getElementById("search-keyword")?.addEventListener("keyup", (e) => { if (e.key === "Enter") this._searchSpots(); });
    }

    /* ==================== 数据统计（管理端） ==================== */

    _renderStats() {
        const spots = zooData.getAllSpots();
        const edges = zooData.getAllEdges();
        const typeCounts = {};
        Object.keys(SpotTypeName).forEach(t => typeCounts[t] = 0);
        spots.forEach(s => typeCounts[s.type]++);

        const avgTime = spots.length ? Math.round(spots.reduce((a, s) => a + s.visitTime, 0) / spots.length) : 0;
        const avgPop = spots.length ? (spots.reduce((a, s) => a + s.popularity, 0) / spots.length).toFixed(1) : 0;

        document.getElementById("stats-grid").innerHTML = `
            <div class="stat"><span class="stat-label">景点总数</span><span class="stat-value">${spots.length}</span></div>
            <div class="stat"><span class="stat-label">路径总数</span><span class="stat-value">${edges.length}</span></div>
            <div class="stat"><span class="stat-label">展馆数量</span><span class="stat-value">${typeCounts[SpotType.ANIMAL_HOUSE]}</span></div>
            <div class="stat"><span class="stat-label">设施/服务</span><span class="stat-value">${typeCounts[SpotType.FACILITY]}</span></div>
            <div class="stat"><span class="stat-label">平均游览</span><span class="stat-value">${avgTime}分钟</span></div>
            <div class="stat"><span class="stat-label">平均人气</span><span class="stat-value">${avgPop}/10</span></div>`;
    }

    /* ==================== 景点列表 ==================== */

    _renderSpots() {
        const spots = zooData.getAllSpots();
        document.getElementById("spots-grid").innerHTML = spots.map(s => renderSpotCard(s)).join("");
        document.querySelectorAll(".spot-card").forEach(card => {
            card.addEventListener("click", () => this.onSpotClick(parseInt(card.dataset.id)));
        });
    }

    onSpotClick(id) {
        if (this.currentView === "spots") {
            Modal.open("景点详情", renderSpotDetail(id));
        } else {
            this.map._showInfoWindow(zooData.getSpotById(id));
        }
    }

    /* ==================== 路径查询 ==================== */

    _renderPathPanel() {
        const opts = zooData.getAllSpots().map(s => `<option value="${s.id}">${s.name}</option>`).join("");
        const startSel = document.getElementById("start-spot");
        const endSel = document.getElementById("end-spot");
        if (startSel) startSel.innerHTML = opts;
        if (endSel) endSel.innerHTML = opts;
    }

    _searchPath() {
        const startId = parseInt(document.getElementById("start-spot")?.value);
        const endId = parseInt(document.getElementById("end-spot")?.value);
        if (startId === endId) { Toast.error("起点和终点不能相同"); return; }

        const result = zooAlgorithms.dijkstra(startId, endId);
        document.getElementById("path-result").innerHTML = renderPathResult(result);
        if (result) this.map.highlightGeometry(result.geometry, startId, endId);
    }

    _randomRoute() {
        const maxTime = parseInt(document.getElementById("max-time")?.value) || 0;
        const result = zooAlgorithms.generateRandomRoute(maxTime);
        document.getElementById("random-result").innerHTML = renderPathResult(result, true);
        if (result) this.map.highlightGeometry(result.geometry, result.path[0], result.path[result.path.length - 1], "#FF9800");
    }

    /* ==================== 景点管理（管理端） ==================== */

    _renderManage() {
        this._renderManageTable(zooData.getAllSpots());
    }

    _renderManageTable(spots) {
        document.getElementById("manage-table").innerHTML = renderManageTable(spots);
    }

    _searchSpots() {
        const kw = document.getElementById("search-keyword")?.value?.trim() || "";
        const type = document.getElementById("search-type")?.value;
        let spots = zooData.getAllSpots();
        if (kw) spots = spots.filter(s => s.name.includes(kw) || s.desc.includes(kw));
        if (type && type !== "all") spots = spots.filter(s => s.type === parseInt(type));
        this._renderManageTable(spots);
        if (spots.length === 0) Toast.info("未找到匹配的景点");
    }

    _showAddForm() {
        Modal.open("添加景点", renderSpotForm(null), () => this._saveSpot());
    }

    editSpot(id) {
        Modal.open("编辑景点", renderSpotForm(zooData.getSpotById(id)), () => this._saveSpot());
    }

    _saveSpot() {
        const id = document.getElementById("form-id")?.value;
        const data = {
            name: document.getElementById("form-name")?.value?.trim(),
            desc: document.getElementById("form-desc")?.value?.trim(),
            type: document.getElementById("form-type")?.value,
            visitTime: document.getElementById("form-time")?.value,
            popularity: document.getElementById("form-pop")?.value,
            lng: document.getElementById("form-lng")?.value,
            lat: document.getElementById("form-lat")?.value
        };
        if (!data.name) { Toast.error("请输入景点名称"); return; }

        const result = id
            ? zooData.updateSpot(parseInt(id), data)
            : zooData.addSpot(data.name, data.desc, data.type, data.visitTime, data.popularity, data.lng, data.lat);
        Toast[result.success ? "success" : "error"](result.message);
        if (result.success) { Modal.close(); this._renderManage(); }
    }

    deleteSpot(id) {
        if (!confirm("确定要删除这个景点吗？")) return;
        const result = zooData.deleteSpot(id);
        Toast[result.success ? "success" : "error"](result.message);
        if (result.success) this._renderManage();
    }
}

/* ==================== 启动 ==================== */

const app = new ZooApp();
document.addEventListener("DOMContentLoaded", () => app.init());
