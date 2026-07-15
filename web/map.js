/**
 * 地图组件 - 基于高德地图 JS API 2.0
 * 封装地图初始化、标记、路线绘制、信息窗体等功能
 */

class ZooMap {
    constructor(containerId) {
        this.containerId = containerId;
        this.map = null;
        this.markers = [];
        this.polylines = [];
        this.infoWindow = null;
        this._onSpotClick = null;
    }

    /**
     * 初始化地图
     * @param {Function} onSpotClick 景点点击回调
     */
    async init(onSpotClick) {
        this._onSpotClick = onSpotClick;

        return new Promise((resolve) => {
            AMapLoader.load({
                key: "6d88d6da344cdd56b75f881d92156aab",       // ← 替换为你的高德Key
                version: "2.0",
                plugins: ["AMap.Scale", "AMap.ToolBar"]
            }).then((AMap) => {
                this.map = new AMap.Map(this.containerId, {
                    zoom: 17,
                    center: [113.3050, 23.1420],
                    mapStyle: "amap://styles/fresh",
                    features: ["bg", "road", "building", "point"]
                });

                this.map.addControl(new AMap.Scale());
                this.map.addControl(new AMap.ToolBar({ position: { bottom: "40px", right: "20px" } }));
                this.infoWindow = new AMap.InfoWindow({ autoMove: true, offset: new AMap.Pixel(0, -30) });

                this.renderAll();
                resolve();
            }).catch(e => {
                console.error("高德地图加载失败:", e);
                document.getElementById(this.containerId).innerHTML =
                    '<div style="display:flex;align-items:center;justify-content:center;height:100%;color:#999;">地图加载失败，请检查API Key</div>';
                resolve();
            });
        });
    }

    /* ==================== 渲染 ==================== */

    /** 清除所有覆盖物 */
    clearOverlays() {
        this.markers.forEach(m => this.map.remove(m));
        this.polylines.forEach(p => this.map.remove(p));
        this.markers = [];
        this.polylines = [];
    }

    /** 渲染全部景点和路径 */
    renderAll() {
        this.clearOverlays();
        this._drawEdges();
        this._drawMarkers();
    }

    /** 绘制景点标记 */
    _drawMarkers() {
        zooData.getAllSpots().forEach(spot => {
            const color = SpotTypeColor[spot.type];
            const icon = SpotTypeIcon[spot.type];
            const content = `
                <div class="amap-marker-custom" style="cursor:pointer;">
                    <div class="marker-dot" style="background:${color};">
                        <span class="marker-icon">${icon}</span>
                    </div>
                    <div class="marker-label">${spot.name}</div>
                </div>`;

            const marker = new AMap.Marker({
                position: [spot.lng, spot.lat],
                content: content,
                anchor: "center",
                offset: new AMap.Pixel(0, 0),
                extData: { spotId: spot.id }
            });

            marker.on("click", () => {
                if (this._onSpotClick) this._onSpotClick(spot.id);
                this._showInfoWindow(spot);
            });

            this.map.add(marker);
            this.markers.push(marker);
        });
    }

    /** Draw the curated pedestrian network routes. */
    _drawEdges() {
        zooData.getLandRoadSegments().forEach(path => {
            const polyline = new AMap.Polyline({
                path,
                strokeColor: "#B0BEC5",
                strokeWeight: 4,
                strokeOpacity: 0.6,
                lineJoin: "round"
            });
            this.map.add(polyline);
            this.polylines.push(polyline);
        });
    }

    /** 显示信息窗体 */
    _showInfoWindow(spot) {
        const color = SpotTypeColor[spot.type];
        const icon = SpotTypeIcon[spot.type];
        const typeName = SpotTypeName[spot.type];
        const stars = "★".repeat(spot.popularity) + "☆".repeat(10 - spot.popularity);

        const html = `
            <div class="info-window">
                <div class="info-header" style="background:${color}">
                    <span class="info-icon">${icon}</span>
                    <h3>${spot.name}</h3>
                    <span class="info-type">${typeName}</span>
                </div>
                <div class="info-body">
                    <p>${spot.desc}</p>
                    <div class="info-meta">
                        <span>⏱️ ${spot.visitTime}分钟</span>
                        <span class="stars">${stars}</span>
                    </div>
                </div>
            </div>`;

        this.infoWindow.setContent(html);
        this.infoWindow.open(this.map, [spot.lng, spot.lat]);
    }

    /* ==================== 路线高亮 ==================== */

    /** Highligh the same local pedestrian paths used by the routing algorithm. */
    highlightGeometry(path, startId, endId, color = "#FF4444") {
        this.renderAll();
        if (!path || path.length < 2) return;

        const polyline = new AMap.Polyline({
            path,
            strokeColor: color,
            strokeWeight: 6,
            strokeOpacity: 0.9,
            lineJoin: "round",
            showDir: true
        });
        this.map.add(polyline);
        this.polylines.push(polyline);

        [startId, endId].forEach((id, idx) => {
            const spot = zooData.getSpotById(id);
            if (!spot) return;
            const accessPoint = zooData.getRouteAccessPoint(id) || [spot.lng, spot.lat];
            const marker = new AMap.CircleMarker({
                center: accessPoint,
                radius: 18,
                strokeColor: idx === 0 ? "#4CAF50" : "#FF4444",
                strokeWeight: 3,
                fillColor: idx === 0 ? "#4CAF50" : "#FF4444",
                fillOpacity: 0.3
            });
            this.map.add(marker);
            this.polylines.push(marker);
        });

        this.map.setFitView(null, false, [60, 60, 60, 60]);
    }

    highlightPath(pathIds, color = "#FF4444") {
        const geometry = [];
        pathIds.slice(0, -1).forEach((fromId, index) => {
            const route = zooData.getLandRoute(fromId, pathIds[index + 1]);
            if (!route) return;
            if (geometry.length === 0) geometry.push(...route.path);
            else geometry.push(...route.path.slice(1));
        });
        return this.highlightGeometry(geometry, pathIds[0], pathIds[pathIds.length - 1], color);

        this.renderAll();

        pathIds.slice(0, -1).forEach((fromId, index) => {
            const path = zooData.getEdgePath(fromId, pathIds[index + 1]);
            if (!path) return;
            const polyline = new AMap.Polyline({
                path,
                strokeColor: color,
                strokeWeight: 6,
                strokeOpacity: 0.9,
                lineJoin: "round",
                showDir: true
            });
            this.map.add(polyline);
            this.polylines.push(polyline);
        });

        // 高亮起终点
        [pathIds[0], pathIds[pathIds.length - 1]].forEach((id, idx) => {
            const spot = zooData.getSpotById(id);
            if (!spot) return;
            const accessPoint = zooData.getRouteAccessPoint(id) || [spot.lng, spot.lat];
            const marker = new AMap.CircleMarker({
                center: accessPoint,
                radius: 18,
                strokeColor: idx === 0 ? "#4CAF50" : "#FF4444",
                strokeWeight: 3,
                fillColor: idx === 0 ? "#4CAF50" : "#FF4444",
                fillOpacity: 0.3
            });
            this.map.add(marker);
            this.polylines.push(marker);
        });

        // 自适应视野
        const points = pathIds.map(id => {
            const s = zooData.getSpotById(id);
            return s ? [s.lng, s.lat] : null;
        }).filter(Boolean);
        if (points.length > 1) {
            this.map.setFitView(null, false, [60, 60, 60, 60]);
        }
    }

    /** 销毁地图 */
    destroy() {
        if (this.map) {
            this.map.destroy();
            this.map = null;
        }
    }
}
