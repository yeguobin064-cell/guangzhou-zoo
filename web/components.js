/**
 * UI组件层 - 界面渲染与交互
 * 基于模块化组件设计
 */

/* ==================== 通知组件 ==================== */

class Toast {
    static _container = null;

    static _getContainer() {
        if (!this._container) {
            this._container = document.getElementById("messages");
        }
        return this._container;
    }

    static show(msg, type = "info", duration = 3000) {
        const container = this._getContainer();
        const el = document.createElement("div");
        el.className = `toast toast-${type}`;
        el.innerHTML = `<span class="toast-icon">${type === "success" ? "✓" : type === "error" ? "✗" : "ℹ"}</span><span>${msg}</span>`;
        container.appendChild(el);
        setTimeout(() => { el.classList.add("toast-out"); setTimeout(() => el.remove(), 300); }, duration);
    }

    static success(msg) { this.show(msg, "success"); }
    static error(msg) { this.show(msg, "error"); }
    static info(msg) { this.show(msg, "info"); }
}

/* ==================== 模态框组件 ==================== */

class Modal {
    static open(title, bodyHtml, onSave) {
        const modal = document.getElementById("modal");
        document.getElementById("modal-title").textContent = title;
        document.getElementById("modal-body").innerHTML = bodyHtml;
        modal.classList.add("active");

        const saveBtn = document.getElementById("btn-save");
        const handler = () => { if (onSave) onSave(); saveBtn.removeEventListener("click", handler); };
        saveBtn.onclick = handler;
    }

    static close() {
        document.getElementById("modal").classList.remove("active");
    }
}

/* ==================== 景点卡片组件 ==================== */

function renderSpotCard(spot) {
    const color = SpotTypeColor[spot.type];
    const icon = SpotTypeIcon[spot.type];
    const typeName = SpotTypeName[spot.type];
    return `
        <div class="spot-card" data-id="${spot.id}">
            <div class="spot-card-header" style="background:linear-gradient(135deg,${color},${color}dd)">
                <span class="spot-card-icon">${icon}</span>
                <span class="spot-card-type">${typeName}</span>
            </div>
            <div class="spot-card-body">
                <h3>${spot.name}</h3>
                <p>${spot.desc}</p>
                <div class="spot-card-meta">
                    <span>⏱️ ${spot.visitTime}分钟</span>
                    <span>⭐ ${spot.popularity}/10</span>
                </div>
            </div>
        </div>`;
}

/* ==================== 路径结果组件 ==================== */

function renderPathResult(result, isRandom = false) {
    if (!result) return '<div class="result-card result-error">❌ 无法找到路径</div>';

    const spots = zooData.getAllSpots();
    const pathNames = result.path.map(idx => spots[idx].name);
    const restrooms = result.restrooms || [];

    const pathHtml = pathNames.map((name, i) => {
        const isRc = restrooms.includes(result.path[i]);
        const cls = isRc ? "path-node restroom" : "path-node";
        return `<span class="${cls}">${name}</span>${i < pathNames.length - 1 ? '<span class="path-arrow">→</span>' : ""}`;
    }).join("");

    let statsHtml = `
        <div class="stat"><span class="stat-label">总距离</span><span class="stat-value">${result.totalDistance} 米</span></div>
        <div class="stat"><span class="stat-label">步行时间</span><span class="stat-value">${result.totalTime || result.walkTime} 分钟</span></div>
        <div class="stat"><span class="stat-label">途经景点</span><span class="stat-value">${result.spotCount} 个</span></div>`;

    if (isRandom) {
        statsHtml += `
            <div class="stat"><span class="stat-label">游览时间</span><span class="stat-value">${result.visitTime} 分钟</span></div>
            <div class="stat"><span class="stat-label">总时间</span><span class="stat-value">${result.totalTime} 分钟</span></div>
            <div class="stat"><span class="stat-label">公共厕所</span><span class="stat-value">${restrooms.map(idx => spots[idx].name).join(", ")}</span></div>`;
    }

    return `
        <div class="result-card result-success">
            <h3>${isRandom ? "🎲 随机游览路线" : "✅ 最短路径"}</h3>
            <div class="path-display">${pathHtml}</div>
            <div class="stats-grid">${statsHtml}</div>
        </div>`;
}

/* ==================== 管理表格组件 ==================== */

function renderManageTable(spots) {
    const rows = spots.map(s => `
        <tr>
            <td>${s.id}</td>
            <td><span class="type-badge" style="background:${SpotTypeColor[s.type]}">${SpotTypeIcon[s.type]} ${SpotTypeName[s.type]}</span></td>
            <td>${s.name}</td>
            <td>${s.visitTime}分钟</td>
            <td>${"⭐".repeat(s.popularity)}</td>
            <td class="actions">
                <button class="btn btn-sm btn-primary" onclick="app.editSpot(${s.id})">编辑</button>
                <button class="btn btn-sm btn-danger" onclick="app.deleteSpot(${s.id})">删除</button>
            </td>
        </tr>`).join("");

    return `
        <table class="data-table">
            <thead><tr><th>ID</th><th>类型</th><th>名称</th><th>时间</th><th>人气</th><th>操作</th></tr></thead>
            <tbody>${rows}</tbody>
        </table>`;
}

/* ==================== 景点编辑表单 ==================== */

function renderSpotForm(spot) {
    const s = spot || { name: "", desc: "", type: 1, visitTime: 20, popularity: 5, lng: "", lat: "" };
    const opts = Object.entries(SpotTypeName).map(([v, t]) =>
        `<option value="${v}" ${parseInt(v) === s.type ? "selected" : ""}>${t}</option>`).join("");
    return `
        <input type="hidden" id="form-id" value="${s.id || ""}">
        <div class="form-row"><div class="form-group"><label>景点名称</label><input id="form-name" class="form-control" value="${s.name}" placeholder="请输入景点名称"></div></div>
        <div class="form-group"><label>景点描述</label><textarea id="form-desc" class="form-control" placeholder="请输入景点描述">${s.desc}</textarea></div>
        <div class="form-row">
            <div class="form-group"><label>景点类型</label><select id="form-type" class="form-control">${opts}</select></div>
            <div class="form-group"><label>游览时间(分钟)</label><input id="form-time" type="number" class="form-control" value="${s.visitTime}" min="1" max="120"></div>
            <div class="form-group"><label>受欢迎程度</label><input id="form-pop" type="number" class="form-control" value="${s.popularity}" min="1" max="10"></div>
        </div>
        <div class="form-row">
            <div class="form-group"><label>经度(lng)</label><input id="form-lng" type="number" step="0.0001" class="form-control" value="${s.lng}" placeholder="可选"></div>
            <div class="form-group"><label>纬度(lat)</label><input id="form-lat" type="number" step="0.0001" class="form-control" value="${s.lat}" placeholder="可选"></div>
        </div>`;
}

/* ==================== 景点详情面板 ==================== */

function renderSpotDetail(spotId) {
    const spot = zooData.getSpotById(spotId);
    if (!spot) return "";
    const color = SpotTypeColor[spot.type];
    const icon = SpotTypeIcon[spot.type];
    const typeName = SpotTypeName[spot.type];

    const edges = zooData.getAllEdges();
    const adj = [];
    edges.forEach(e => {
        if (e.from === spotId) { const s = zooData.getSpotById(e.to); if (s) adj.push({ name: s.name, dist: e.distance }); }
        if (e.to === spotId)   { const s = zooData.getSpotById(e.from); if (s) adj.push({ name: s.name, dist: e.distance }); }
    });
    const adjHtml = adj.map(a => `<span class="adj-tag">${a.name} (${a.dist}m)</span>`).join("");

    return `
        <div class="detail-panel">
            <div class="detail-header" style="background:linear-gradient(135deg,${color},${color}dd)">
                <span class="detail-icon">${icon}</span>
                <div><h3>${spot.name}</h3><span class="detail-type">${typeName}</span></div>
            </div>
            <div class="detail-body">
                <p>${spot.desc}</p>
                <div class="detail-meta"><span>⏱️ ${spot.visitTime}分钟</span><span>⭐ ${spot.popularity}/10</span></div>
                <h4>相邻景点</h4>
                <div class="adj-list">${adjHtml || "无"}</div>
            </div>
        </div>`;
}
