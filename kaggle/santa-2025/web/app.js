/**
 * Kaggle Santa 2025: Christmas Tree Packing Visualizer
 * Core interactive canvas engine, geometry, collision detection, and sandbox editor.
 */

// ================= Constants & Geometry =================
const BASE_VERTICES = [
  [0.0, 0.8],          // 0: Tip
  [0.125, 0.5],        // 1: Right tier 1 outer (0.25 / 2)
  [0.0625, 0.5],       // 2: Right tier 1 inner (0.25 / 4)
  [0.2, 0.25],         // 3: Right tier 2 outer (0.4 / 2)
  [0.1, 0.25],         // 4: Right tier 2 inner (0.4 / 4)
  [0.35, 0.0],         // 5: Right base outer (0.7 / 2)
  [0.075, 0.0],        // 6: Right trunk top (0.15 / 2)
  [0.075, -0.2],       // 7: Right trunk bottom (-0.2)
  [-0.075, -0.2],      // 8: Left trunk bottom
  [-0.075, 0.0],       // 9: Left trunk top
  [-0.35, 0.0],        // 10: Left base outer
  [-0.1, 0.25],        // 11: Left tier 2 inner
  [-0.2, 0.25],        // 12: Left tier 2 outer
  [-0.0625, 0.5],      // 13: Left tier 1 inner
  [-0.125, 0.5]        // 14: Left tier 1 outer
];

const TREE_AREA = 0.245625; // Exact analytical area of a single tree

// Color Palettes
const PALETTES = {
  emerald: {
    name: 'Festive Emerald',
    getColor: (idx, total) => {
      const hue = 145 + ((idx * 17) % 30);
      const sat = 65 + ((idx * 7) % 25);
      const lum = 30 + ((idx * 13) % 25);
      return `hsl(${hue}, ${sat}%, ${lum}%)`;
    },
    stroke: '#105230',
    trunk: '#5c3a21'
  },
  viridis: {
    name: 'Official Viridis',
    getColor: (idx, total) => {
      const t = total > 1 ? idx / (total - 1) : 0.5;
      // Viridis colormap polynomial approximation
      const r = Math.round(255 * (0.28 + 0.5 * Math.sin(Math.PI * (t - 0.2))));
      const g = Math.round(255 * (0.1 + 0.85 * Math.sin(Math.PI * t)));
      const b = Math.round(255 * (0.4 + 0.55 * Math.cos(Math.PI * (t * 0.8 - 0.2))));
      return `rgb(${Math.max(0, Math.min(255, r))}, ${Math.max(0, Math.min(255, g))}, ${Math.max(0, Math.min(255, b))})`;
    },
    stroke: '#1a1a2e',
    trunk: '#4a3b32'
  },
  rainbow: {
    name: 'Rainbow Spectrum',
    getColor: (idx, total) => {
      const hue = (idx * 360 / Math.max(1, total)) % 360;
      return `hsl(${hue}, 70%, 45%)`;
    },
    stroke: '#222',
    trunk: '#443322'
  },
  pine: {
    name: 'Natural Pine',
    getColor: () => '#1b4d3e',
    stroke: '#0d2820',
    trunk: '#543d2b'
  },
  status: {
    name: 'Collision Status',
    getColor: (idx, total, isColliding) => isColliding ? '#e74c3c' : '#27ae60',
    stroke: '#111',
    trunk: '#333'
  }
};

// ================= Geometry Functions =================

/**
 * Transforms base tree vertices by rotation (deg) and translation (cx, cy).
 */
function transformVertices(cx, cy, deg) {
  const rad = (deg * Math.PI) / 180;
  const cosA = Math.cos(rad);
  const sinA = Math.sin(rad);
  return BASE_VERTICES.map(([vx, vy]) => [
    vx * cosA - vy * sinA + cx,
    vx * sinA + vy * cosA + cy
  ]);
}

/**
 * Computes Axis-Aligned Bounding Box for an array of 2D points [[x, y], ...].
 */
function getAABB(points) {
  let minX = Infinity, maxX = -Infinity;
  let minY = Infinity, maxY = -Infinity;
  for (let i = 0; i < points.length; i++) {
    const p = points[i];
    if (p[0] < minX) minX = p[0];
    if (p[0] > maxX) maxX = p[0];
    if (p[1] < minY) minY = p[1];
    if (p[1] > maxY) maxY = p[1];
  }
  return { minX, maxX, minY, maxY, width: maxX - minX, height: maxY - minY };
}

/**
 * Strict segment-segment intersection test (cross products must have strictly opposite signs).
 */
function intersectSegmentsStrict(p1, p2, p3, p4) {
  const ccw = (a, b, c) => (c[1] - a[1]) * (b[0] - a[0]) - (b[1] - a[1]) * (c[0] - a[0]);
  const cp1 = ccw(p1, p2, p3);
  const cp2 = ccw(p1, p2, p4);
  const cp3 = ccw(p3, p4, p1);
  const cp4 = ccw(p3, p4, p2);

  const eps = 1e-11;
  const proper1 = (cp1 > eps && cp2 < -eps) || (cp1 < -eps && cp2 > eps);
  const proper2 = (cp3 > eps && cp4 < -eps) || (cp3 < -eps && cp4 > eps);
  return proper1 && proper2;
}

/**
 * Ray casting algorithm to check if a point lies strictly inside a polygon.
 */
function isPointStrictlyInsidePoly(pt, poly) {
  const x = pt[0], y = pt[1];
  const n = poly.length;
  let inside = false;
  const eps = 1e-11;

  for (let i = 0; i < n; i++) {
    const p1 = poly[i];
    const p2 = poly[(i + 1) % n];

    // Check if point is on the segment boundary
    const cp = (p2[0] - p1[0]) * (y - p1[1]) - (p2[1] - p1[1]) * (x - p1[0]);
    if (Math.abs(cp) < eps) {
      const dot = (x - p1[0]) * (p2[0] - p1[0]) + (y - p1[1]) * (p2[1] - p1[1]);
      const sq = (p2[0] - p1[0]) ** 2 + (p2[1] - p1[1]) ** 2;
      if (dot >= -eps && dot <= sq + eps) {
        return false; // Point is on boundary, not strictly inside
      }
    }

    if ((p1[1] > y) !== (p2[1] > y)) {
      const xInt = ((y - p1[1]) * (p2[0] - p1[0])) / (p2[1] - p1[1]) + p1[0];
      if (x < xInt - eps) {
        inside = !inside;
      }
    }
  }
  return inside;
}

/**
 * Detects if two tree polygons overlap with non-zero area (matching Santa 2025 metric).
 */
function checkTreeCollision(polyA, polyB, aabbA, aabbB) {
  // 1. Fast AABB bounding box rejection
  if (
    aabbA.maxX <= aabbB.minX ||
    aabbB.maxX <= aabbA.minX ||
    aabbA.maxY <= aabbB.minY ||
    aabbB.maxY <= aabbA.minY
  ) {
    return false;
  }

  // 2. Strict segment intersection
  const nA = polyA.length;
  const nB = polyB.length;
  for (let i = 0; i < nA; i++) {
    const a1 = polyA[i];
    const a2 = polyA[(i + 1) % nA];
    for (let j = 0; j < nB; j++) {
      const b1 = polyB[j];
      const b2 = polyB[(j + 1) % nB];
      if (intersectSegmentsStrict(a1, a2, b1, b2)) {
        return true;
      }
    }
  }

  // 3. Strict point-in-polygon containment
  for (let i = 0; i < nA; i++) {
    if (isPointStrictlyInsidePoly(polyA[i], polyB)) {
      return true;
    }
  }
  for (let j = 0; j < nB; j++) {
    if (isPointStrictlyInsidePoly(polyB[j], polyA)) {
      return true;
    }
  }

  return false;
}

// ================= Application State =================
class VisualizerState {
  constructor() {
    this.datasets = {};
    this.currentDatasetName = '';
    this.currentN = 1;
    this.selectedTreeIndex = null;
    this.hoveredTreeIndex = null;

    // Undo / Redo history for editing
    this.history = [];
    this.historyIndex = -1;

    // Viewport transform
    this.panX = 0;
    this.panY = 0;
    this.zoom = 1;

    // Mouse interaction states
    this.isDragging = false;
    this.dragMode = null; // 'pan', 'moveTree', 'rotateTree'
    this.dragStart = { x: 0, y: 0, wx: 0, wy: 0 };
    this.dragTreeInitial = { x: 0, y: 0, deg: 0 };

    // Display settings
    this.settings = {
      colorScheme: 'emerald',
      showSquare: true,
      showBBox: true,
      showLabels: true,
      showCenters: true,
      showTipStars: true,
      showGrid: true,
      showCollisions: true,
      opacity: 0.75
    };

    // Compare Mode
    this.compareMode = false;
    this.compareDatasetA = '';
    this.compareDatasetB = '';
    this.compareRenderMode = 'overlay'; // 'overlay' or 'sideBySide'

    // Cached computations
    this.treePolygons = [];
    this.treeAABBs = [];
    this.collisions = new Set();
    this.collidingPairs = [];
    this.boundingSquare = { x: 0, y: 0, side: 1 };
    this.tightBBox = { minX: 0, maxX: 0, minY: 0, maxY: 0, width: 0, height: 0 };
    this.configScore = 0;
    this.configDensity = 0;
    this.totalDatasetScore = 0;
    this.datasetSummary = {}; // per N: { side, score, density, collisions }
  }

  loadPreloadedDatasets() {
    if (window.PRELOADED_DATASETS) {
      for (const [name, data] of Object.entries(window.PRELOADED_DATASETS)) {
        this.datasets[name] = data;
      }
      const names = Object.keys(this.datasets);
      if (names.length > 0) {
        this.currentDatasetName = names[0];
      }
    }
  }

  getCurrentTrees() {
    const ds = this.datasets[this.currentDatasetName];
    if (!ds) return [];
    return ds[String(this.currentN)] || [];
  }

  setCurrentTrees(trees, recordHistory = true) {
    if (recordHistory) {
      this.pushHistoryState();
    }
    const ds = this.datasets[this.currentDatasetName];
    if (ds) {
      ds[String(this.currentN)] = trees;
      this.recalculateCurrentConfig();
      this.recalculateTotalScore();
    }
  }

  pushHistoryState() {
    const current = JSON.parse(JSON.stringify(this.getCurrentTrees()));
    this.history = this.history.slice(0, this.historyIndex + 1);
    this.history.push(current);
    this.historyIndex++;
    updateHistoryButtons();
  }

  undo() {
    if (this.historyIndex > 0) {
      this.historyIndex--;
      const trees = JSON.parse(JSON.stringify(this.history[this.historyIndex]));
      const ds = this.datasets[this.currentDatasetName];
      ds[String(this.currentN)] = trees;
      this.recalculateCurrentConfig();
      this.recalculateTotalScore();
      updateHistoryButtons();
      render();
    }
  }

  redo() {
    if (this.historyIndex < this.history.length - 1) {
      this.historyIndex++;
      const trees = JSON.parse(JSON.stringify(this.history[this.historyIndex]));
      const ds = this.datasets[this.currentDatasetName];
      ds[String(this.currentN)] = trees;
      this.recalculateCurrentConfig();
      this.recalculateTotalScore();
      updateHistoryButtons();
      render();
    }
  }

  resetCurrentN() {
    // Reload from original preloaded if available
    if (window.PRELOADED_DATASETS && window.PRELOADED_DATASETS[this.currentDatasetName]) {
      const orig = window.PRELOADED_DATASETS[this.currentDatasetName][String(this.currentN)];
      if (orig) {
        this.setCurrentTrees(JSON.parse(JSON.stringify(orig)), true);
        render();
      }
    }
  }

  recalculateCurrentConfig() {
    const trees = this.getCurrentTrees();
    const numTrees = trees.length;
    this.treePolygons = [];
    this.treeAABBs = [];

    if (numTrees === 0) {
      this.tightBBox = { minX: 0, maxX: 0, minY: 0, maxY: 0, width: 0, height: 0 };
      this.boundingSquare = { x: 0, y: 0, side: 0 };
      this.configScore = 0;
      this.configDensity = 0;
      this.collisions.clear();
      this.collidingPairs = [];
      return;
    }

    // 1. Transform each tree and compute AABBs
    let allMinX = Infinity, allMaxX = -Infinity;
    let allMinY = Infinity, allMaxY = -Infinity;

    for (let i = 0; i < numTrees; i++) {
      const [cx, cy, deg] = trees[i];
      const poly = transformVertices(cx, cy, deg);
      const aabb = getAABB(poly);
      this.treePolygons.push(poly);
      this.treeAABBs.push(aabb);

      if (aabb.minX < allMinX) allMinX = aabb.minX;
      if (aabb.maxX > allMaxX) allMaxX = aabb.maxX;
      if (aabb.minY < allMinY) allMinY = aabb.minY;
      if (aabb.maxY > allMaxY) allMaxY = aabb.maxY;
    }

    const width = allMaxX - allMinX;
    const height = allMaxY - allMinY;
    const side = Math.max(width, height);

    const sqX = width >= height ? allMinX : allMinX - (side - width) / 2;
    const sqY = height >= width ? allMinY : allMinY - (side - height) / 2;

    this.tightBBox = { minX: allMinX, maxX: allMaxX, minY: allMinY, maxY: allMaxY, width, height };
    this.boundingSquare = { x: sqX, y: sqY, side };

    // Metric score: side^2 / N
    this.configScore = (side * side) / numTrees;
    this.configDensity = (numTrees * TREE_AREA) / (side * side);

    // 2. Collision detection
    this.collisions.clear();
    this.collidingPairs = [];

    if (this.settings.showCollisions) {
      for (let i = 0; i < numTrees; i++) {
        for (let j = i + 1; j < numTrees; j++) {
          if (checkTreeCollision(this.treePolygons[i], this.treePolygons[j], this.treeAABBs[i], this.treeAABBs[j])) {
            this.collisions.add(i);
            this.collisions.add(j);
            this.collidingPairs.push([i, j]);
          }
        }
      }
    }
  }

  recalculateTotalScore() {
    const ds = this.datasets[this.currentDatasetName];
    if (!ds) {
      this.totalDatasetScore = 0;
      return;
    }

    let total = 0;
    this.datasetSummary = {};

    for (let n = 1; n <= 200; n++) {
      const trees = ds[String(n)];
      if (!trees || trees.length === 0) continue;

      let allMinX = Infinity, allMaxX = -Infinity;
      let allMinY = Infinity, allMaxY = -Infinity;

      for (let i = 0; i < trees.length; i++) {
        const [cx, cy, deg] = trees[i];
        const poly = transformVertices(cx, cy, deg);
        for (let k = 0; k < poly.length; k++) {
          const px = poly[k][0], py = poly[k][1];
          if (px < allMinX) allMinX = px;
          if (px > allMaxX) allMaxX = px;
          if (py < allMinY) allMinY = py;
          if (py > allMaxY) allMaxY = py;
        }
      }

      const side = Math.max(allMaxX - allMinX, allMaxY - allMinY);
      const score = (side * side) / n;
      const density = (n * TREE_AREA) / (side * side);

      total += score;
      this.datasetSummary[n] = { side, score, density, collisions: 0 };
    }

    this.totalDatasetScore = total;
  }
}

const state = new VisualizerState();

// ================= Canvas & Rendering Setup =================
const canvas = document.getElementById('packingCanvas');
const ctx = canvas.getContext('2d');
const container = document.getElementById('canvasContainer');

function resizeCanvas() {
  const dpr = window.devicePixelRatio || 1;
  const rect = container.getBoundingClientRect();
  canvas.width = rect.width * dpr;
  canvas.height = rect.height * dpr;
  ctx.scale(dpr, dpr);
  render();
}

// Convert World (tree space) -> Screen (pixel space)
function worldToScreen(wx, wy) {
  const rect = container.getBoundingClientRect();
  const cx = rect.width / 2 + state.panX;
  const cy = rect.height / 2 + state.panY;
  // Notice Y is inverted: standard math coordinates have +Y up, canvas has +Y down
  const sx = cx + wx * state.zoom;
  const sy = cy - wy * state.zoom;
  return [sx, sy];
}

// Convert Screen (pixel space) -> World (tree space)
function screenToWorld(sx, sy) {
  const rect = container.getBoundingClientRect();
  const cx = rect.width / 2 + state.panX;
  const cy = rect.height / 2 + state.panY;
  const wx = (sx - cx) / state.zoom;
  const wy = (cy - sy) / state.zoom;
  return [wx, wy];
}

/**
 * Fits the view nicely to the current bounding square.
 */
function fitViewToSquare() {
  const rect = container.getBoundingClientRect();
  const sq = state.boundingSquare;
  if (!sq || sq.side <= 0) {
    state.panX = 0;
    state.panY = 0;
    state.zoom = 100;
    render();
    return;
  }

  const padding = 60; // pixels padding
  const availW = Math.max(100, rect.width - padding * 2);
  const availH = Math.max(100, rect.height - padding * 2);

  const zoomW = availW / sq.side;
  const zoomH = availH / sq.side;
  state.zoom = Math.min(zoomW, zoomH);

  // Center of the bounding square
  const sqCenterX = sq.x + sq.side / 2;
  const sqCenterY = sq.y + sq.side / 2;

  // Pan offsets so square center is at screen center
  state.panX = -sqCenterX * state.zoom;
  state.panY = sqCenterY * state.zoom;

  render();
}

// ================= Main Render Loop =================
function render() {
  const rect = container.getBoundingClientRect();
  const w = rect.width;
  const h = rect.height;

  ctx.clearRect(0, 0, w, h);

  // 1. Draw Grid and Coordinates
  if (state.settings.showGrid) {
    drawGrid(w, h);
  }

  // 2. Draw Comparison Mode Ghost (if active)
  if (state.compareMode && state.compareDatasetB) {
    drawCompareOverlay();
  }

  // 3. Draw Bounding Square & Tight Bounding Box
  drawBoundingBoxes();

  // 4. Draw Trees
  drawTrees();

  // 5. Draw Transform Gizmo for selected tree
  if (state.selectedTreeIndex !== null) {
    drawSelectedGizmo();
  }

  // 6. Update UI elements and statistics
  updateUI();
}

function drawGrid(w, h) {
  const [minWx, maxWy] = screenToWorld(0, 0);
  const [maxWx, minWy] = screenToWorld(w, h);

  // Dynamic grid step based on zoom
  const targetPixelSpacing = 80;
  const roughStep = targetPixelSpacing / state.zoom;
  const pow10 = Math.pow(10, Math.floor(Math.log10(roughStep)));
  let step = pow10;
  if (roughStep / pow10 > 5) step = pow10 * 5;
  else if (roughStep / pow10 > 2) step = pow10 * 2;

  const isLight = document.body.classList.contains('theme-light');
  ctx.lineWidth = 1;
  ctx.strokeStyle = isLight ? 'rgba(0, 0, 0, 0.06)' : 'rgba(255, 255, 255, 0.06)';
  ctx.fillStyle = isLight ? 'rgba(0, 0, 0, 0.35)' : 'rgba(255, 255, 255, 0.35)';
  ctx.font = '10px monospace';

  const startX = Math.floor(minWx / step) * step;
  const endX = Math.ceil(maxWx / step) * step;
  const startY = Math.floor(minWy / step) * step;
  const endY = Math.ceil(maxWy / step) * step;

  // Vertical lines
  for (let x = startX; x <= endX; x += step) {
    const [sx, _] = worldToScreen(x, 0);
    ctx.beginPath();
    ctx.moveTo(sx, 0);
    ctx.lineTo(sx, h);
    ctx.stroke();

    const [__, sy0] = worldToScreen(0, 0);
    const labelY = Math.max(14, Math.min(h - 4, sy0 + 12));
    ctx.fillText(Number(x.toFixed(4)).toString(), sx + 3, labelY);
  }

  // Horizontal lines
  for (let y = startY; y <= endY; y += step) {
    const [_, sy] = worldToScreen(0, y);
    ctx.beginPath();
    ctx.moveTo(0, sy);
    ctx.lineTo(w, sy);
    ctx.stroke();

    const [sx0, __] = worldToScreen(0, 0);
    const labelX = Math.max(4, Math.min(w - 30, sx0 + 4));
    if (Math.abs(y) > 1e-6) {
      ctx.fillText(Number(y.toFixed(4)).toString(), labelX, sy - 3);
    }
  }

  // Axes (0,0)
  ctx.lineWidth = 1.5;
  ctx.strokeStyle = isLight ? 'rgba(0, 0, 0, 0.25)' : 'rgba(255, 255, 255, 0.25)';
  const [axisSx, axisSy] = worldToScreen(0, 0);
  ctx.beginPath();
  ctx.moveTo(axisSx, 0);
  ctx.lineTo(axisSx, h);
  ctx.moveTo(0, axisSy);
  ctx.lineTo(w, axisSy);
  ctx.stroke();
}

function drawBoundingBoxes() {
  const sq = state.boundingSquare;
  if (!sq || sq.side <= 0) return;

  // 1. Tight Bounding Box (cyan dashed)
  if (state.settings.showBBox) {
    const tb = state.tightBBox;
    const [tbX, tbY] = worldToScreen(tb.minX, tb.maxY);
    const tbW = tb.width * state.zoom;
    const tbH = tb.height * state.zoom;

    ctx.save();
    ctx.setLineDash([4, 4]);
    ctx.strokeStyle = 'rgba(52, 152, 219, 0.6)';
    ctx.lineWidth = 1.5;
    ctx.strokeRect(tbX, tbY, tbW, tbH);
    ctx.restore();
  }

  // 2. Official Bounding Square (gold/red dashed)
  if (state.settings.showSquare) {
    const [sqSx, sqSy] = worldToScreen(sq.x, sq.y + sq.side);
    const sqPixelSide = sq.side * state.zoom;

    ctx.save();
    ctx.setLineDash([8, 6]);
    ctx.strokeStyle = '#e74c3c';
    ctx.lineWidth = 2.5;
    ctx.strokeRect(sqSx, sqSy, sqPixelSide, sqPixelSide);

    // Corner brackets
    ctx.setLineDash([]);
    ctx.strokeStyle = '#f0c034';
    ctx.lineWidth = 3.5;
    const cLen = Math.min(20, sqPixelSide * 0.2);

    // Top-left
    ctx.beginPath();
    ctx.moveTo(sqSx, sqSy + cLen);
    ctx.lineTo(sqSx, sqSy);
    ctx.lineTo(sqSx + cLen, sqSy);
    ctx.stroke();

    // Top-right
    ctx.beginPath();
    ctx.moveTo(sqSx + sqPixelSide - cLen, sqSy);
    ctx.lineTo(sqSx + sqPixelSide, sqSy);
    ctx.lineTo(sqSx + sqPixelSide, sqSy + cLen);
    ctx.stroke();

    // Bottom-left
    ctx.beginPath();
    ctx.moveTo(sqSx, sqSy + sqPixelSide - cLen);
    ctx.lineTo(sqSx, sqSy + sqPixelSide);
    ctx.lineTo(sqSx + cLen, sqSy + sqPixelSide);
    ctx.stroke();

    // Bottom-right
    ctx.beginPath();
    ctx.moveTo(sqSx + sqPixelSide - cLen, sqSy + sqPixelSide);
    ctx.lineTo(sqSx + sqPixelSide, sqSy + sqPixelSide);
    ctx.lineTo(sqSx + sqPixelSide, sqSy + sqPixelSide - cLen);
    ctx.stroke();

    // Side dimension badge
    ctx.font = 'bold 12px monospace';
    ctx.fillStyle = '#f0c034';
    const label = `L = ${sq.side.toFixed(6)}`;
    ctx.fillText(label, sqSx + 6, sqSy - 8);

    ctx.restore();
  }
}

function drawTrees() {
  const trees = state.getCurrentTrees();
  const numTrees = trees.length;
  const palette = PALETTES[state.settings.colorScheme] || PALETTES.emerald;

  for (let i = 0; i < numTrees; i++) {
    const [cx, cy, deg] = trees[i];
    const poly = state.treePolygons[i];
    if (!poly) continue;

    const isSelected = state.selectedTreeIndex === i;
    const isHovered = state.hoveredTreeIndex === i;
    const isColliding = state.collisions.has(i);

    ctx.save();

    // Convert polygon vertices to screen coordinates
    const screenPts = poly.map(([px, py]) => worldToScreen(px, py));

    // Fill Color
    let fillColor;
    if (isColliding && state.settings.showCollisions) {
      fillColor = 'rgba(231, 76, 60, 0.7)';
    } else {
      fillColor = palette.getColor(i, numTrees, isColliding);
    }

    ctx.globalAlpha = isSelected ? 1.0 : state.settings.opacity;
    ctx.fillStyle = fillColor;

    // Draw tree polygon
    ctx.beginPath();
    ctx.moveTo(screenPts[0][0], screenPts[0][1]);
    for (let k = 1; k < screenPts.length; k++) {
      ctx.lineTo(screenPts[k][0], screenPts[k][1]);
    }
    ctx.closePath();
    ctx.fill();

    // Stroke outline
    ctx.globalAlpha = 1.0;
    if (isSelected) {
      ctx.lineWidth = 3;
      ctx.strokeStyle = '#f0c034';
    } else if (isColliding && state.settings.showCollisions) {
      ctx.lineWidth = 2.5;
      ctx.strokeStyle = '#e74c3c';
    } else if (isHovered) {
      ctx.lineWidth = 2;
      ctx.strokeStyle = '#ffffff';
    } else {
      ctx.lineWidth = 1;
      ctx.strokeStyle = palette.stroke;
    }
    ctx.stroke();

    // Tip Star / Arrow
    if (state.settings.showTipStars) {
      const tipPt = screenPts[0]; // Tip is vertex 0
      drawTipStar(tipPt[0], tipPt[1], isSelected);
    }

    // Center Crosshair Marker
    const [scX, scY] = worldToScreen(cx, cy);
    if (state.settings.showCenters || isSelected) {
      ctx.beginPath();
      ctx.arc(scX, scY, isSelected ? 4 : 2.5, 0, Math.PI * 2);
      ctx.fillStyle = isSelected ? '#f0c034' : 'rgba(255, 255, 255, 0.8)';
      ctx.fill();
    }

    // Tree Index Label
    if (state.settings.showLabels) {
      ctx.font = 'bold 10px sans-serif';
      ctx.fillStyle = '#ffffff';
      ctx.strokeStyle = '#000000';
      ctx.lineWidth = 2.5;
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.strokeText(String(i), scX, scY);
      ctx.fillText(String(i), scX, scY);
    }

    ctx.restore();
  }
}

function drawTipStar(sx, sy, isSelected) {
  ctx.save();
  ctx.beginPath();
  ctx.arc(sx, sy, isSelected ? 4.5 : 3, 0, Math.PI * 2);
  ctx.fillStyle = '#f1c40f';
  ctx.fill();
  ctx.lineWidth = 1;
  ctx.strokeStyle = '#d4ac0d';
  ctx.stroke();
  ctx.restore();
}

function drawSelectedGizmo() {
  const idx = state.selectedTreeIndex;
  const trees = state.getCurrentTrees();
  if (idx === null || !trees[idx]) return;

  const [cx, cy, deg] = trees[idx];
  const [scX, scY] = worldToScreen(cx, cy);

  const radius = Math.max(30, 0.85 * state.zoom); // Dynamic gizmo radius

  ctx.save();
  // Draw subtle dashed rotation circle
  ctx.beginPath();
  ctx.arc(scX, scY, radius, 0, Math.PI * 2);
  ctx.setLineDash([4, 4]);
  ctx.strokeStyle = 'rgba(240, 192, 52, 0.6)';
  ctx.lineWidth = 1.5;
  ctx.stroke();

  // Rotation Handle at current angle
  const rad = (deg * Math.PI) / 180;
  // Remember screen coordinate inversion for Y: screenAngle is -rad
  const hX = scX + radius * Math.sin(rad); // Tip is along +Y initially
  const hY = scY - radius * Math.cos(rad);

  ctx.setLineDash([]);
  ctx.beginPath();
  ctx.arc(hX, hY, 7, 0, Math.PI * 2);
  ctx.fillStyle = '#f0c034';
  ctx.fill();
  ctx.lineWidth = 2;
  ctx.strokeStyle = '#ffffff';
  ctx.stroke();

  // Connect handle to center
  ctx.beginPath();
  ctx.moveTo(scX, scY);
  ctx.lineTo(hX, hY);
  ctx.strokeStyle = 'rgba(240, 192, 52, 0.4)';
  ctx.lineWidth = 1.5;
  ctx.stroke();

  ctx.restore();
}

function drawCompareOverlay() {
  const dsB = state.datasets[state.compareDatasetB];
  if (!dsB) return;
  const treesB = dsB[String(state.currentN)] || [];

  ctx.save();
  ctx.globalAlpha = 0.35;
  ctx.fillStyle = '#3498db';
  ctx.strokeStyle = '#2980b9';
  ctx.lineWidth = 1.5;

  for (let i = 0; i < treesB.length; i++) {
    const [cx, cy, deg] = treesB[i];
    const poly = transformVertices(cx, cy, deg);
    const screenPts = poly.map(([px, py]) => worldToScreen(px, py));

    ctx.beginPath();
    ctx.moveTo(screenPts[0][0], screenPts[0][1]);
    for (let k = 1; k < screenPts.length; k++) {
      ctx.lineTo(screenPts[k][0], screenPts[k][1]);
    }
    ctx.closePath();
    ctx.fill();
    ctx.stroke();
  }
  ctx.restore();
}

// ================= UI Updates =================
function updateUI() {
  document.getElementById('statN').textContent = state.currentN;
  document.getElementById('statSide').textContent = state.boundingSquare.side.toFixed(6);
  document.getElementById('statScore').textContent = state.configScore.toFixed(6);
  document.getElementById('statDensity').textContent = `${(state.configDensity * 100).toFixed(1)}%`;
  document.getElementById('statTotalScore').textContent = state.totalDatasetScore.toFixed(6);

  // Collision Pill
  const colPill = document.getElementById('statCollision');
  const numCollisions = state.collisions.size;
  if (numCollisions === 0) {
    colPill.textContent = '✓ Valid (0 overlaps)';
    colPill.className = 'val status-ok';
  } else {
    colPill.textContent = `⚠️ ${state.collidingPairs.length} Overlaps!`;
    colPill.className = 'val status-error';
  }

  // Navigation inputs
  document.getElementById('nSlider').value = state.currentN;
  document.getElementById('nInput').value = state.currentN;

  // Problem Metrics Table
  document.getElementById('metricN').textContent = state.currentN;
  document.getElementById('metricSide').textContent = state.boundingSquare.side.toFixed(6);
  document.getElementById('metricScore').textContent = state.configScore.toFixed(6);
  document.getElementById('metricSqArea').textContent = (state.boundingSquare.side ** 2).toFixed(6);
  document.getElementById('metricTreeArea').textContent = (state.currentN * TREE_AREA).toFixed(6);
  document.getElementById('metricDensity').textContent = `${(state.configDensity * 100).toFixed(2)}%`;
  document.getElementById('metricOverlap').textContent = `${state.collidingPairs.length} collision pairs`;

  // Selected Tree Inspector
  const inspectorSection = document.getElementById('inspectorContent');
  const badge = document.getElementById('selectedTreeBadge');
  const banner = document.getElementById('treeBanner');

  if (state.selectedTreeIndex !== null) {
    const idx = state.selectedTreeIndex;
    const trees = state.getCurrentTrees();
    const tree = trees[idx];
    if (tree) {
      inspectorSection.classList.remove('disabled-section');
      badge.textContent = `Tree #${idx}`;
      document.getElementById('treeXInput').value = tree[0].toFixed(5);
      document.getElementById('treeYInput').value = tree[1].toFixed(5);
      document.getElementById('treeDegInput').value = tree[2].toFixed(2);
      document.getElementById('treeDegSlider').value = (tree[2] % 360 + 360) % 360;

      // Floating banner
      banner.classList.remove('hidden');
      document.getElementById('bannerTreeTitle').textContent = `Tree #${idx} of ${state.currentN}`;
      document.getElementById('bannerTreeCoords').textContent = `(x: ${tree[0].toFixed(4)}, y: ${tree[1].toFixed(4)}, deg: ${tree[2].toFixed(1)}°)`;
    }
  } else {
    inspectorSection.classList.add('disabled-section');
    badge.textContent = 'None Selected';
    banner.classList.add('hidden');
  }

  // Compare banner
  const cmpBanner = document.getElementById('compareBanner');
  if (state.compareMode) {
    cmpBanner.classList.remove('hidden');
    document.getElementById('cmpNameA').textContent = state.currentDatasetName;
    document.getElementById('cmpNameB').textContent = state.compareDatasetB;

    const dsA = state.datasets[state.currentDatasetName];
    const dsB = state.datasets[state.compareDatasetB];
    const treesA = dsA ? dsA[String(state.currentN)] || [] : [];
    const treesB = dsB ? dsB[String(state.currentN)] || [] : [];

    const sideA = state.boundingSquare.side;
    let sideB = 0;
    if (treesB.length > 0) {
      let bMinX = Infinity, bMaxX = -Infinity, bMinY = Infinity, bMaxY = -Infinity;
      for (let t of treesB) {
        const poly = transformVertices(t[0], t[1], t[2]);
        for (let p of poly) {
          if (p[0] < bMinX) bMinX = p[0];
          if (p[0] > bMaxX) bMaxX = p[0];
          if (p[1] < bMinY) bMinY = p[1];
          if (p[1] > bMaxY) bMaxY = p[1];
        }
      }
      sideB = Math.max(bMaxX - bMinX, bMaxY - bMinY);
    }
    const scoreA = (sideA * sideA) / Math.max(1, treesA.length);
    const scoreB = (sideB * sideB) / Math.max(1, treesB.length);
    const diff = scoreA - scoreB;
    const sign = diff >= 0 ? '+' : '';
    document.getElementById('cmpDiffBadge').textContent = `Δ Score: ${sign}${diff.toFixed(6)}`;
  } else {
    cmpBanner.classList.add('hidden');
  }
}

function updateHistoryButtons() {
  document.getElementById('undoBtn').disabled = state.historyIndex <= 0;
  document.getElementById('redoBtn').disabled = state.historyIndex >= state.history.length - 1;
}

// ================= Mouse & Interaction Handlers =================

function findTreeAtScreenPoint(sx, sy) {
  const [wx, wy] = screenToWorld(sx, sy);
  const pt = [wx, wy];
  const trees = state.getCurrentTrees();

  // Check in reverse order so top-drawn tree is picked first
  for (let i = trees.length - 1; i >= 0; i--) {
    const poly = state.treePolygons[i];
    if (poly && isPointStrictlyInsidePoly(pt, poly)) {
      return i;
    }
  }
  return null;
}

function isOverRotationHandle(sx, sy) {
  if (state.selectedTreeIndex === null) return false;
  const trees = state.getCurrentTrees();
  const tree = trees[state.selectedTreeIndex];
  if (!tree) return false;

  const [cx, cy, deg] = tree;
  const [scX, scY] = worldToScreen(cx, cy);
  const radius = Math.max(30, 0.85 * state.zoom);

  const rad = (deg * Math.PI) / 180;
  const hX = scX + radius * Math.sin(rad);
  const hY = scY - radius * Math.cos(rad);

  const dist = Math.hypot(sx - hX, sy - hY);
  return dist <= 12; // 12px hit radius
}

container.addEventListener('mousedown', (e) => {
  if (e.button !== 0 && e.button !== 1) return; // Left or middle click

  const rect = container.getBoundingClientRect();
  const sx = e.clientX - rect.left;
  const sy = e.clientY - rect.top;
  const [wx, wy] = screenToWorld(sx, sy);

  state.isDragging = true;
  state.dragStart = { x: sx, y: sy, wx, wy };

  // Check 1: Rotation handle of selected tree
  if (isOverRotationHandle(sx, sy)) {
    state.dragMode = 'rotateTree';
    const tree = state.getCurrentTrees()[state.selectedTreeIndex];
    state.dragTreeInitial = { x: tree[0], y: tree[1], deg: tree[2] };
    state.pushHistoryState();
    return;
  }

  // Check 2: Clicked on a tree
  const clickedTree = findTreeAtScreenPoint(sx, sy);
  if (clickedTree !== null && e.button === 0) {
    state.selectedTreeIndex = clickedTree;
    state.dragMode = 'moveTree';
    const tree = state.getCurrentTrees()[clickedTree];
    state.dragTreeInitial = { x: tree[0], y: tree[1], deg: tree[2] };
    state.pushHistoryState();
    render();
    return;
  }

  // Check 3: Clicked empty canvas -> pan
  state.dragMode = 'pan';
  if (e.button === 0 && !e.shiftKey) {
    state.selectedTreeIndex = null;
  }
  render();
});

window.addEventListener('mousemove', (e) => {
  const rect = container.getBoundingClientRect();
  const sx = e.clientX - rect.left;
  const sy = e.clientY - rect.top;

  // Update coordinate display
  const [wx, wy] = screenToWorld(sx, sy);
  document.getElementById('coordDisplay').textContent = `X: ${wx.toFixed(3)}, Y: ${wy.toFixed(3)}`;

  if (!state.isDragging) {
    // Hover check
    if (isOverRotationHandle(sx, sy)) {
      container.style.cursor = 'crosshair';
    } else {
      const hTree = findTreeAtScreenPoint(sx, sy);
      if (hTree !== state.hoveredTreeIndex) {
        state.hoveredTreeIndex = hTree;
        container.style.cursor = hTree !== null ? 'pointer' : 'grab';
        render();
      }
    }
    return;
  }

  // Active Dragging
  if (state.dragMode === 'pan') {
    const dx = sx - state.dragStart.x;
    const dy = sy - state.dragStart.y;
    state.panX += dx;
    state.panY += dy;
    state.dragStart.x = sx;
    state.dragStart.y = sy;
    container.style.cursor = 'grabbing';
    render();
  } else if (state.dragMode === 'moveTree') {
    const dx = wx - state.dragStart.wx;
    const dy = wy - state.dragStart.wy;
    const trees = state.getCurrentTrees();
    const tree = trees[state.selectedTreeIndex];
    if (tree) {
      tree[0] = state.dragTreeInitial.x + dx;
      tree[1] = state.dragTreeInitial.y + dy;
      state.recalculateCurrentConfig();
      state.recalculateTotalScore();
      render();
    }
  } else if (state.dragMode === 'rotateTree') {
    const trees = state.getCurrentTrees();
    const tree = trees[state.selectedTreeIndex];
    if (tree) {
      const [scX, scY] = worldToScreen(tree[0], tree[1]);
      // Angle from center to mouse position
      const angleRad = Math.atan2(sx - scX, scY - sy);
      let angleDeg = (angleRad * 180) / Math.PI;
      if (angleDeg < 0) angleDeg += 360;

      // Snap to 15 deg if Shift is held
      if (e.shiftKey) {
        angleDeg = Math.round(angleDeg / 15) * 15;
      }

      tree[2] = angleDeg;
      state.recalculateCurrentConfig();
      state.recalculateTotalScore();
      render();
    }
  }
});

window.addEventListener('mouseup', () => {
  if (state.isDragging) {
    state.isDragging = false;
    state.dragMode = null;
    container.style.cursor = 'grab';
    render();
  }
});

// Zoom via Wheel
container.addEventListener('wheel', (e) => {
  e.preventDefault();
  const rect = container.getBoundingClientRect();
  const sx = e.clientX - rect.left;
  const sy = e.clientY - rect.top;

  // If hovering over selected tree and Alt/Ctrl key is held, rotate tree instead
  if (state.selectedTreeIndex !== null && (e.altKey || e.ctrlKey)) {
    const trees = state.getCurrentTrees();
    const tree = trees[state.selectedTreeIndex];
    const step = e.shiftKey ? 15 : 1;
    tree[2] = (tree[2] + (e.deltaY < 0 ? step : -step) + 360) % 360;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
    return;
  }

  const zoomFactor = e.deltaY < 0 ? 1.15 : 0.85;
  const [wx, wy] = screenToWorld(sx, sy);

  state.zoom *= zoomFactor;
  state.zoom = Math.max(5, Math.min(20000, state.zoom)); // limits

  // Keep mouse cursor pointing at same world coordinate
  const [newSx, newSy] = worldToScreen(wx, wy);
  state.panX += sx - newSx;
  state.panY += sy - newSy;

  render();
}, { passive: false });

// ================= Keyboard Shortcuts =================
window.addEventListener('keydown', (e) => {
  // Don't trigger shortcuts if user is typing in an input
  if (['INPUT', 'TEXTAREA', 'SELECT'].includes(e.target.tagName)) return;

  if (e.key === 'ArrowLeft') {
    e.preventDefault();
    changeN(state.currentN - 1);
  } else if (e.key === 'ArrowRight') {
    e.preventDefault();
    changeN(state.currentN + 1);
  } else if (e.key === '[') {
    e.preventDefault();
    changeN(state.currentN - 10);
  } else if (e.key === ']') {
    e.preventDefault();
    changeN(state.currentN + 10);
  } else if (e.key === ' ' || e.key === 'f') {
    e.preventDefault();
    fitViewToSquare();
  } else if (e.key === 'r' || e.key === 'R') {
    if (state.selectedTreeIndex !== null) {
      e.preventDefault();
      state.pushHistoryState();
      const trees = state.getCurrentTrees();
      const step = e.shiftKey ? -15 : 15;
      trees[state.selectedTreeIndex][2] = (trees[state.selectedTreeIndex][2] + step + 360) % 360;
      state.recalculateCurrentConfig();
      state.recalculateTotalScore();
      render();
    }
  } else if (e.key === 'Escape') {
    state.selectedTreeIndex = null;
    render();
  } else if (e.key === 'Delete' || e.key === 'Backspace') {
    if (state.selectedTreeIndex !== null) {
      deleteSelectedTree();
    }
  } else if ((e.ctrlKey || e.metaKey) && (e.key === 'z' || e.key === 'Z')) {
    e.preventDefault();
    if (e.shiftKey) state.redo();
    else state.undo();
  } else if ((e.ctrlKey || e.metaKey) && (e.key === 'y' || e.key === 'Y')) {
    e.preventDefault();
    state.redo();
  }
});

// ================= Navigation & Inspector Bindings =================

function changeN(newN) {
  state.currentN = Math.max(1, Math.min(200, newN));
  state.selectedTreeIndex = null;
  state.history = [JSON.parse(JSON.stringify(state.getCurrentTrees()))];
  state.historyIndex = 0;
  updateHistoryButtons();
  state.recalculateCurrentConfig();
  fitViewToSquare();
}

document.getElementById('nSlider').addEventListener('input', (e) => changeN(parseInt(e.target.value)));
document.getElementById('nInput').addEventListener('change', (e) => changeN(parseInt(e.target.value)));
document.getElementById('prevNBtn').addEventListener('click', () => changeN(state.currentN - 1));
document.getElementById('nextNBtn').addEventListener('click', () => changeN(state.currentN + 1));

document.querySelectorAll('.jump-pill').forEach(btn => {
  btn.addEventListener('click', () => changeN(parseInt(btn.getAttribute('data-n'))));
});

document.getElementById('zoomInBtn').addEventListener('click', () => {
  state.zoom *= 1.25;
  render();
});
document.getElementById('zoomOutBtn').addEventListener('click', () => {
  state.zoom *= 0.8;
  render();
});
document.getElementById('fitViewBtn').addEventListener('click', fitViewToSquare);
document.getElementById('resetOriginBtn').addEventListener('click', () => {
  state.panX = 0;
  state.panY = 0;
  render();
});

// Inspector Inputs
document.getElementById('treeXInput').addEventListener('change', (e) => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    state.getCurrentTrees()[state.selectedTreeIndex][0] = parseFloat(e.target.value) || 0;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

document.getElementById('treeYInput').addEventListener('change', (e) => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    state.getCurrentTrees()[state.selectedTreeIndex][1] = parseFloat(e.target.value) || 0;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

document.getElementById('treeDegInput').addEventListener('change', (e) => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    state.getCurrentTrees()[state.selectedTreeIndex][2] = parseFloat(e.target.value) || 0;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

document.getElementById('treeDegSlider').addEventListener('input', (e) => {
  if (state.selectedTreeIndex !== null) {
    state.getCurrentTrees()[state.selectedTreeIndex][2] = parseFloat(e.target.value);
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

document.querySelectorAll('.angle-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    if (state.selectedTreeIndex !== null) {
      state.pushHistoryState();
      state.getCurrentTrees()[state.selectedTreeIndex][2] = parseFloat(btn.getAttribute('data-angle'));
      state.recalculateCurrentConfig();
      state.recalculateTotalScore();
      render();
    }
  });
});

document.getElementById('rotateMinus15Btn').addEventListener('click', () => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    const tree = state.getCurrentTrees()[state.selectedTreeIndex];
    tree[2] = (tree[2] - 15 + 360) % 360;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

document.getElementById('rotatePlus15Btn').addEventListener('click', () => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    const tree = state.getCurrentTrees()[state.selectedTreeIndex];
    tree[2] = (tree[2] + 15) % 360;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

function deleteSelectedTree() {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    const trees = state.getCurrentTrees();
    trees.splice(state.selectedTreeIndex, 1);
    state.selectedTreeIndex = null;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
}

document.getElementById('deleteTreeBtn').addEventListener('click', deleteSelectedTree);
document.getElementById('deselectTreeBtn').addEventListener('click', () => {
  state.selectedTreeIndex = null;
  render();
});
document.getElementById('bannerDeselectBtn').addEventListener('click', () => {
  state.selectedTreeIndex = null;
  render();
});

document.getElementById('duplicateTreeBtn').addEventListener('click', () => {
  if (state.selectedTreeIndex !== null) {
    state.pushHistoryState();
    const trees = state.getCurrentTrees();
    const src = trees[state.selectedTreeIndex];
    trees.push([src[0] + 0.2, src[1] + 0.2, src[2]]);
    state.selectedTreeIndex = trees.length - 1;
    state.recalculateCurrentConfig();
    state.recalculateTotalScore();
    render();
  }
});

// Sandbox Tools
document.getElementById('addTreeBtn').addEventListener('click', () => {
  state.pushHistoryState();
  const trees = state.getCurrentTrees();
  const sq = state.boundingSquare;
  const cx = sq.x + sq.side / 2;
  const cy = sq.y + sq.side / 2;
  trees.push([cx, cy, 0]);
  state.selectedTreeIndex = trees.length - 1;
  state.recalculateCurrentConfig();
  state.recalculateTotalScore();
  render();
});

document.getElementById('centerAllTreesBtn').addEventListener('click', () => {
  state.pushHistoryState();
  const trees = state.getCurrentTrees();
  const sq = state.boundingSquare;
  const sqCenterX = sq.x + sq.side / 2;
  const sqCenterY = sq.y + sq.side / 2;
  for (let t of trees) {
    t[0] -= sqCenterX;
    t[1] -= sqCenterY;
  }
  state.recalculateCurrentConfig();
  state.recalculateTotalScore();
  fitViewToSquare();
});

document.getElementById('resetConfigBtn').addEventListener('click', () => state.resetCurrentN());
document.getElementById('undoBtn').addEventListener('click', () => state.undo());
document.getElementById('redoBtn').addEventListener('click', () => state.redo());

// Display Toggles
document.getElementById('colorSchemeSelect').addEventListener('change', (e) => {
  state.settings.colorScheme = e.target.value;
  render();
});
document.getElementById('toggleSquare').addEventListener('change', (e) => {
  state.settings.showSquare = e.target.checked;
  render();
});
document.getElementById('toggleBBox').addEventListener('change', (e) => {
  state.settings.showBBox = e.target.checked;
  render();
});
document.getElementById('toggleLabels').addEventListener('change', (e) => {
  state.settings.showLabels = e.target.checked;
  render();
});
document.getElementById('toggleCenters').addEventListener('change', (e) => {
  state.settings.showCenters = e.target.checked;
  render();
});
document.getElementById('toggleTipStars').addEventListener('change', (e) => {
  state.settings.showTipStars = e.target.checked;
  render();
});
document.getElementById('toggleGrid').addEventListener('change', (e) => {
  state.settings.showGrid = e.target.checked;
  render();
});
document.getElementById('toggleCollisions').addEventListener('change', (e) => {
  state.settings.showCollisions = e.target.checked;
  state.recalculateCurrentConfig();
  render();
});
document.getElementById('treeOpacitySlider').addEventListener('input', (e) => {
  state.settings.opacity = parseFloat(e.target.value);
  document.getElementById('opacityVal').textContent = `${Math.round(state.settings.opacity * 100)}%`;
  render();
});

// Jump Tools
document.getElementById('jumpWorstBtn').addEventListener('click', () => {
  let worstN = 1;
  let worstDensity = Infinity;
  for (let n = 1; n <= 200; n++) {
    const summary = state.datasetSummary[n];
    if (summary && summary.density < worstDensity) {
      worstDensity = summary.density;
      worstN = n;
    }
  }
  changeN(worstN);
});

document.getElementById('jumpCollisionBtn').addEventListener('click', () => {
  const ds = state.datasets[state.currentDatasetName];
  if (!ds) return;

  for (let step = 1; step <= 200; step++) {
    const n = ((state.currentN - 1 + step) % 200) + 1;
    const trees = ds[String(n)] || [];
    const polys = trees.map(t => transformVertices(t[0], t[1], t[2]));
    const aabbs = polys.map(p => getAABB(p));

    for (let i = 0; i < polys.length; i++) {
      for (let j = i + 1; j < polys.length; j++) {
        if (checkTreeCollision(polys[i], polys[j], aabbs[i], aabbs[j])) {
          changeN(n);
          return;
        }
      }
    }
  }
  alert('No collisions found across all 200 configurations! Great job! 🎉');
});

// Theme Toggle
document.getElementById('themeToggleBtn').addEventListener('click', () => {
  const body = document.body;
  if (body.classList.contains('theme-dark')) {
    body.classList.remove('theme-dark');
    body.classList.add('theme-light');
    document.getElementById('themeToggleBtn').textContent = '🌙';
  } else {
    body.classList.remove('theme-light');
    body.classList.add('theme-dark');
    document.getElementById('themeToggleBtn').textContent = '☀️';
  }
  render();
});

// Dataset Selection
function populateDatasetDropdown() {
  const select = document.getElementById('datasetSelect');
  select.innerHTML = '';
  for (const name of Object.keys(state.datasets)) {
    const opt = document.createElement('option');
    opt.value = name;
    opt.textContent = name;
    select.appendChild(opt);
  }
  select.value = state.currentDatasetName;
}

document.getElementById('datasetSelect').addEventListener('change', (e) => {
  state.currentDatasetName = e.target.value;
  state.recalculateTotalScore();
  state.recalculateCurrentConfig();
  changeN(state.currentN);
});

// ================= CSV Parsing & Exporting =================

function parseCsv(csvText) {
  const lines = csvText.split(/\r?\n/).map(l => l.trim()).filter(l => l.length > 0);
  if (lines.length === 0) throw new Error('File is empty');

  let startIndex = 0;
  if (lines[0].toLowerCase().includes('id') && lines[0].toLowerCase().includes('x')) {
    startIndex = 1; // skip header
  }

  const result = {};
  for (let i = 1; i <= 200; i++) result[String(i)] = [];

  for (let i = startIndex; i < lines.length; i++) {
    const parts = lines[i].split(',').map(s => s.trim());
    if (parts.length < 4) continue;

    const id = parts[0];
    const nStr = id.split('_')[0];
    const n = parseInt(nStr, 10);
    if (isNaN(n) || n < 1 || n > 200) continue;

    let x = parts[1];
    let y = parts[2];
    let deg = parts[3];

    if (x.startsWith('s')) x = x.substring(1);
    if (y.startsWith('s')) y = y.substring(1);
    if (deg.startsWith('s')) deg = deg.substring(1);

    const xNum = parseFloat(x);
    const yNum = parseFloat(y);
    const degNum = parseFloat(deg);

    if (!isNaN(xNum) && !isNaN(yNum) && !isNaN(degNum)) {
      result[String(n)].push([xNum, yNum, degNum]);
    }
  }

  return result;
}

function exportCsvText(includeS = true) {
  const ds = state.datasets[state.currentDatasetName];
  if (!ds) return '';

  const rows = ['id,x,y,deg'];
  const prefix = includeS ? 's' : '';

  for (let n = 1; n <= 200; n++) {
    const trees = ds[String(n)] || [];
    for (let t = 0; t < trees.length; t++) {
      const id = `${String(n).padStart(3, '0')}_${t}`;
      const x = `${prefix}${trees[t][0].toFixed(8)}`;
      const y = `${prefix}${trees[t][1].toFixed(8)}`;
      const deg = `${prefix}${trees[t][2].toFixed(6)}`;
      rows.push(`${id},${x},${y},${deg}`);
    }
  }
  return rows.join('\n');
}

function downloadFile(content, filename, type = 'text/plain') {
  const blob = new Blob([content], { type });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

// Export Dropdown
const exportMenu = document.getElementById('exportMenu');
document.getElementById('exportDropdownBtn').addEventListener('click', (e) => {
  e.stopPropagation();
  exportMenu.classList.toggle('show');
});
window.addEventListener('click', () => exportMenu.classList.remove('show'));

document.getElementById('exportPngBtn').addEventListener('click', (e) => {
  e.preventDefault();
  const url = canvas.toDataURL('image/png');
  const a = document.createElement('a');
  a.href = url;
  a.download = `santa_2025_N${state.currentN}.png`;
  a.click();
});

document.getElementById('exportSvgBtn').addEventListener('click', (e) => {
  e.preventDefault();
  const sq = state.boundingSquare;
  const padding = sq.side * 0.1;
  const minX = sq.x - padding;
  const minY = sq.y - padding;
  const viewBoxW = sq.side + padding * 2;
  const viewBoxH = sq.side + padding * 2;

  let svg = `<?xml version="1.0" standalone="no"?>\n`;
  svg += `<svg xmlns="http://www.w3.org/2000/svg" viewBox="${minX} ${-(minY + viewBoxH)} ${viewBoxW} ${viewBoxH}" width="800" height="800">\n`;
  svg += `  <rect x="${minX}" y="${-(minY + viewBoxH)}" width="${viewBoxW}" height="${viewBoxH}" fill="#0d1411" />\n`;

  // Bounding Square
  svg += `  <rect x="${sq.x}" y="${-(sq.y + sq.side)}" width="${sq.side}" height="${sq.side}" fill="none" stroke="#e74c3c" stroke-width="${sq.side * 0.005}" stroke-dasharray="${sq.side * 0.02}" />\n`;

  // Trees
  const trees = state.getCurrentTrees();
  for (let i = 0; i < trees.length; i++) {
    const poly = state.treePolygons[i];
    const pointsStr = poly.map(([px, py]) => `${px.toFixed(5)},${(-py).toFixed(5)}`).join(' ');
    svg += `  <polygon points="${pointsStr}" fill="#2bb368" stroke="#105230" stroke-width="${sq.side * 0.002}" opacity="0.85" />\n`;
  }
  svg += `</svg>`;
  downloadFile(svg, `santa_2025_N${state.currentN}.svg`, 'image/svg+xml');
});

document.getElementById('exportFullCsvBtn').addEventListener('click', (e) => {
  e.preventDefault();
  const csv = exportCsvText(true);
  downloadFile(csv, 'submission.csv', 'text/csv');
});

document.getElementById('exportConfigCsvBtn').addEventListener('click', (e) => {
  e.preventDefault();
  const ds = state.datasets[state.currentDatasetName];
  const trees = ds ? ds[String(state.currentN)] || [] : [];
  const rows = ['id,x,y,deg'];
  for (let t = 0; t < trees.length; t++) {
    const id = `${String(state.currentN).padStart(3, '0')}_${t}`;
    rows.push(`${id},s${trees[t][0].toFixed(8)},s${trees[t][1].toFixed(8)},s${trees[t][2].toFixed(6)}`);
  }
  downloadFile(rows.join('\n'), `santa_2025_config_N${state.currentN}.csv`, 'text/csv');
});

// ================= Upload Modal =================
const uploadModal = document.getElementById('uploadModal');
document.getElementById('uploadBtn').addEventListener('click', () => {
  uploadModal.classList.remove('hidden');
});
document.getElementById('closeUploadBtn').addEventListener('click', () => {
  uploadModal.classList.add('hidden');
});
document.getElementById('cancelUploadBtn').addEventListener('click', () => {
  uploadModal.classList.add('hidden');
});

const dropzone = document.getElementById('dropzone');
const fileInput = document.getElementById('csvFileInput');
document.getElementById('browseFileBtn').addEventListener('click', () => fileInput.click());

dropzone.addEventListener('dragover', (e) => {
  e.preventDefault();
  dropzone.classList.add('dragover');
});
dropzone.addEventListener('dragleave', () => dropzone.classList.remove('dragover'));
dropzone.addEventListener('drop', (e) => {
  e.preventDefault();
  dropzone.classList.remove('dragover');
  if (e.dataTransfer.files.length > 0) {
    handleFileUpload(e.dataTransfer.files[0]);
  }
});
fileInput.addEventListener('change', () => {
  if (fileInput.files.length > 0) {
    handleFileUpload(fileInput.files[0]);
  }
});

function handleFileUpload(file) {
  const reader = new FileReader();
  reader.onload = (e) => {
    document.getElementById('csvPasteArea').value = e.target.result;
    document.getElementById('customDatasetName').value = file.name.replace(/\.[^/.]+$/, '');
  };
  reader.readAsText(file);
}

document.getElementById('confirmUploadBtn').addEventListener('click', () => {
  const text = document.getElementById('csvPasteArea').value;
  const name = document.getElementById('customDatasetName').value.trim() || 'Custom Submission';
  const statusEl = document.getElementById('uploadStatusMsg');

  try {
    const parsed = parseCsv(text);
    state.datasets[name] = parsed;
    state.currentDatasetName = name;
    populateDatasetDropdown();
    state.recalculateTotalScore();
    changeN(state.currentN);
    uploadModal.classList.add('hidden');
  } catch (err) {
    statusEl.textContent = `Error: ${err.message}`;
    statusEl.className = 'upload-status error';
    statusEl.classList.remove('hidden');
  }
});

// ================= Compare Modal =================
const compareModal = document.getElementById('compareModal');
document.getElementById('compareModeBtn').addEventListener('click', () => {
  const selA = document.getElementById('cmpSelectA');
  const selB = document.getElementById('cmpSelectB');
  selA.innerHTML = '';
  selB.innerHTML = '';

  const names = Object.keys(state.datasets);
  for (let n of names) {
    selA.appendChild(new Option(n, n));
    selB.appendChild(new Option(n, n));
  }
  selA.value = state.currentDatasetName;
  if (names.length > 1) {
    selB.value = names[1];
  }

  compareModal.classList.remove('hidden');
});

document.getElementById('closeCompareModalBtn').addEventListener('click', () => compareModal.classList.add('hidden'));
document.getElementById('cancelCompareBtn').addEventListener('click', () => compareModal.classList.add('hidden'));

document.getElementById('startCompareBtn').addEventListener('click', () => {
  state.currentDatasetName = document.getElementById('cmpSelectA').value;
  state.compareDatasetB = document.getElementById('cmpSelectB').value;
  state.compareMode = true;
  compareModal.classList.add('hidden');
  state.recalculateCurrentConfig();
  render();
});

document.getElementById('closeCompareBtn').addEventListener('click', () => {
  state.compareMode = false;
  render();
});

// ================= Overview & Analytics Modal =================
const overviewModal = document.getElementById('overviewModal');
document.getElementById('overviewBtn').addEventListener('click', () => {
  state.recalculateTotalScore();
  renderOverviewChart('score');
  renderOverviewGallery();
  renderOverviewTable();
  overviewModal.classList.remove('hidden');
});
document.getElementById('closeOverviewBtn').addEventListener('click', () => {
  overviewModal.classList.add('hidden');
});

// Tabs in Overview Modal
document.querySelectorAll('.tab-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
    btn.classList.add('active');
    document.getElementById(btn.getAttribute('data-tab')).classList.add('active');
  });
});

let currentChartMetric = 'score';
document.querySelectorAll('.chart-opt-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.chart-opt-btn').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');
    currentChartMetric = btn.getAttribute('data-metric');
    renderOverviewChart(currentChartMetric);
  });
});

function renderOverviewChart(metric) {
  const svg = document.getElementById('overviewChartSvg');
  svg.innerHTML = '';

  const w = 900;
  const h = 300;
  const padL = 60, padR = 20, padT = 30, padB = 40;
  const chartW = w - padL - padR;
  const chartH = h - padT - padB;

  const data = [];
  let minY = Infinity, maxY = -Infinity;

  for (let n = 1; n <= 200; n++) {
    const s = state.datasetSummary[n] || { score: 0, side: 0, density: 0 };
    let val = s.score;
    if (metric === 'side') val = s.side;
    else if (metric === 'density') val = s.density * 100;

    data.push({ n, val });
    if (val < minY) minY = val;
    if (val > maxY) maxY = val;
  }

  minY = Math.max(0, minY * 0.9);
  maxY = maxY * 1.05;

  // Grid lines
  for (let step = 0; step <= 5; step++) {
    const yVal = minY + (step / 5) * (maxY - minY);
    const py = padT + chartH - (step / 5) * chartH;

    const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
    line.setAttribute('x1', padL);
    line.setAttribute('y1', py);
    line.setAttribute('x2', padL + chartW);
    line.setAttribute('y2', py);
    line.setAttribute('stroke', 'rgba(255,255,255,0.08)');
    svg.appendChild(line);

    const txt = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    txt.setAttribute('x', padL - 8);
    txt.setAttribute('y', py + 4);
    txt.setAttribute('text-anchor', 'end');
    txt.setAttribute('fill', '#9db5a7');
    txt.setAttribute('font-size', '10px');
    txt.setAttribute('font-family', 'monospace');
    txt.textContent = yVal.toFixed(2);
    svg.appendChild(txt);
  }

  // X Axis labels (N = 1, 50, 100, 150, 200)
  for (let n of [1, 50, 100, 150, 200]) {
    const px = padL + ((n - 1) / 199) * chartW;
    const txt = document.createElementNS('http://www.w3.org/2000/svg', 'text');
    txt.setAttribute('x', px);
    txt.setAttribute('y', padT + chartH + 20);
    txt.setAttribute('text-anchor', 'middle');
    txt.setAttribute('fill', '#9db5a7');
    txt.setAttribute('font-size', '11px');
    txt.textContent = `N=${n}`;
    svg.appendChild(txt);
  }

  // Plot line
  let pathD = '';
  for (let i = 0; i < data.length; i++) {
    const d = data[i];
    const px = padL + ((d.n - 1) / 199) * chartW;
    const py = padT + chartH - ((d.val - minY) / (maxY - minY)) * chartH;
    pathD += `${i === 0 ? 'M' : 'L'} ${px.toFixed(2)} ${py.toFixed(2)} `;
  }

  const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
  path.setAttribute('d', pathD);
  path.setAttribute('fill', 'none');
  path.setAttribute('stroke', '#2bb368');
  path.setAttribute('stroke-width', '2.5');
  svg.appendChild(path);

  // Plot interactive dots
  for (let i = 0; i < data.length; i++) {
    const d = data[i];
    const px = padL + ((d.n - 1) / 199) * chartW;
    const py = padT + chartH - ((d.val - minY) / (maxY - minY)) * chartH;

    const circle = document.createElementNS('http://www.w3.org/2000/svg', 'circle');
    circle.setAttribute('cx', px);
    circle.setAttribute('cy', py);
    circle.setAttribute('r', d.n === state.currentN ? '5' : '2.5');
    circle.setAttribute('fill', d.n === state.currentN ? '#f0c034' : '#2bb368');
    circle.style.cursor = 'pointer';

    circle.addEventListener('click', () => {
      overviewModal.classList.add('hidden');
      changeN(d.n);
    });

    const title = document.createElementNS('http://www.w3.org/2000/svg', 'title');
    title.textContent = `N=${d.n}: ${d.val.toFixed(6)}`;
    circle.appendChild(title);

    svg.appendChild(circle);
  }
}

function renderOverviewGallery() {
  const grid = document.getElementById('galleryGrid');
  grid.innerHTML = '';

  for (let n = 1; n <= 200; n++) {
    const s = state.datasetSummary[n] || { score: 0, side: 0, density: 0 };
    const card = document.createElement('div');
    card.className = `gallery-card ${n === state.currentN ? 'active' : ''}`;
    card.innerHTML = `
      <div class="g-n">N = ${n}</div>
      <div class="g-score">${s.score.toFixed(4)}</div>
      <div class="g-density" style="font-size:9px; color:#6b8577;">${(s.density * 100).toFixed(0)}%</div>
    `;
    card.addEventListener('click', () => {
      overviewModal.classList.add('hidden');
      changeN(n);
    });
    grid.appendChild(card);
  }
}

function renderOverviewTable() {
  const tbody = document.getElementById('overviewTableBody');
  tbody.innerHTML = '';

  for (let n = 1; n <= 200; n++) {
    const s = state.datasetSummary[n] || { score: 0, side: 0, density: 0 };
    const tr = document.createElement('tr');
    tr.innerHTML = `
      <td><b>${n}</b></td>
      <td>${s.side.toFixed(6)}</td>
      <td>${s.score.toFixed(6)}</td>
      <td>${(s.density * 100).toFixed(2)}%</td>
      <td><span class="val status-ok">0</span></td>
      <td><button class="btn btn-xs btn-outline jump-table-btn" data-n="${n}">View</button></td>
    `;
    tbody.appendChild(tr);
  }

  document.querySelectorAll('.jump-table-btn').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const n = parseInt(btn.getAttribute('data-n'));
      overviewModal.classList.add('hidden');
      changeN(n);
    });
  });
}

// ================= App Initialization =================
window.addEventListener('load', () => {
  state.loadPreloadedDatasets();
  populateDatasetDropdown();
  state.recalculateTotalScore();
  changeN(1);
  resizeCanvas();
  window.addEventListener('resize', resizeCanvas);
});
