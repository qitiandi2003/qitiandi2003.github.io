(() => {
  'use strict';
  const paper = '#f1f5fa', ink = '#24354c', teal = '#256b90', rust = '#af503f', rule = '#c8d3de';
  function line(ctx, x1, y1, x2, y2, color, width = 1) {
    ctx.strokeStyle = color;
    ctx.lineWidth = width;
    ctx.beginPath();
    ctx.moveTo(x1, y1);
    ctx.lineTo(x2, y2);
    ctx.stroke();
  }
  function label(ctx, value, x, y, color = ink, align = 'left') {
    ctx.fillStyle = color;
    ctx.textAlign = align;
    ctx.fillText(value, x, y);
  }
  function dot(ctx, x, y, radius, color, ring = false) {
    ctx.beginPath();
    ctx.arc(x, y, radius, 0, Math.PI * 2);
    if (ring) { ctx.strokeStyle = color; ctx.lineWidth = 1.6; ctx.stroke(); }
    else { ctx.fillStyle = color; ctx.fill(); }
  }
  function gaussian(ctx, w, h, value) {
    const x = w * .50, y = h * .47, a = value / 100;
    function ellipse(angle, rgb, opacity) {
      ctx.save();
      ctx.translate(x, y);
      ctx.rotate(angle);
      ctx.scale(Math.min(w * .31, 147), h * .22);
      const grad = ctx.createRadialGradient(0, 0, 0, 0, 0, 1);
      for (const s of [0, .25, .5, .75, 1]) {
        const weight = s === 1 ? 0 : Math.exp(-4 * s * s);
        grad.addColorStop(s, 'rgba(' + rgb + ',' + opacity * weight + ')');
      }
      ctx.fillStyle = grad;
      ctx.fillRect(-1, -1, 2, 2);
      ctx.beginPath(); ctx.arc(0, 0, .84, 0, Math.PI * 2); ctx.restore();
      ctx.lineWidth = 1; ctx.strokeStyle = 'rgba(' + rgb + ',.4)'; ctx.stroke();
    }
    ellipse(-.29, '45,111,122', .5);
    ellipse(.36, '174,73,44', a);
    dot(ctx, x, y, 4, ink, true);
    label(ctx, '中心像素', x + 12, y - 8);
    const weights = [a, (1 - a) * .5, (1 - a) * .5], tags = ['前景', '后景', '背景'];
    const barW = w - 56, barY = h - 24;
    let pos = 28;
    weights.forEach((weight, i) => {
      ctx.fillStyle = [rust, teal, '#b4b7a8'][i];
      ctx.fillRect(pos, barY, barW * weight, 5);
      pos += barW * weight;
      label(ctx, tags[i] + ' ' + +(weight * 100).toFixed(1) + '%', 28 + i * barW / 3, barY - 8, [rust, teal, ink][i]);
    });
    return [value + '%', '前景贡献 ' + value + '%，后景和背景各贡献 ' + +((100 - value) / 2).toFixed(1) + '%。'];
  }
  function disparity(ctx, w, h, value) {
    const pad = 14, gap = 10, panel = (w - pad * 2 - gap) / 2;
    const top = 38, bottom = h - 40, scale = panel / 145, centerY = (top + bottom) / 2;
    for (let i = 0; i < 2; ++i) {
      const left = pad + i * (panel + gap);
      const pixel = i ? 90 - value : 90, x = left + pixel * scale;
      label(ctx, i ? '右相机' : '左相机', left, 25);
      ctx.fillStyle = '#e7e9df'; ctx.fillRect(left, top, panel, bottom - top);
      for (let n = 20; n < 145; n += 20) {
        const gx = left + n * scale;
        line(ctx, gx, top, gx, bottom, rule);
      }
      ctx.setLineDash([3, 4]); line(ctx, left, centerY, left + panel, centerY, '#82958b'); ctx.setLineDash([]);
      line(ctx, x, top + 8, x, bottom - 8, i ? rust : teal, 2);
      dot(ctx, x, centerY, 5, i ? rust : teal);
      label(ctx, '横坐标 ' + pixel + ' px', left, bottom + 23, i ? rust : teal);
    }
    return [value + ' px', '对应点的横坐标相差 ' + value + ' 像素，深度为 600 × 0.10 ÷ ' + value + ' = ' + (60 / value).toFixed(2) + ' 米。'];
  }
  const target = [[-1.3,-.9],[-.95,-.9],[-.58,-.9],[-.2,-.88],[.14,-.88],[.56,-.8],[.96,-.67],[1.25,-.37],[1.26,.02],[1.25,.41],[1.34,.82],[1.48,1.11]];
  function nearest(point) {
    let best = target[0], distance = Infinity;
    for (const q of target) {
      const d = (point[0] - q[0]) ** 2 + (point[1] - q[1]) ** 2;
      if (d < distance) { best = q; distance = d; }
    }
    return best;
  }
  function rotate(p, angle) {
    const c = Math.cos(angle), s = Math.sin(angle);
    return [p[0] * c - p[1] * s, p[0] * s + p[1] * c];
  }
  function centroid(points) {
    return points.reduce((sum, p) => [sum[0] + p[0] / points.length, sum[1] + p[1] / points.length], [0, 0]);
  }
  function registration(ctx, w, h, steps) {
    let points = target.map(p => { const r = rotate(p, .27); return [r[0] - .54, r[1] + .61]; });
    for (let i = 0; i < steps; ++i) {
      const pairs = points.map(nearest), pc = centroid(points), qc = centroid(pairs);
      let cross = 0, product = 0;
      points.forEach((p, j) => {
        const px = p[0] - pc[0], py = p[1] - pc[1], qx = pairs[j][0] - qc[0], qy = pairs[j][1] - qc[1];
        cross += px * qy - py * qx; product += px * qx + py * qy;
      });
      const angle = Math.atan2(cross, product);
      points = points.map(p => { const r = rotate([p[0] - pc[0], p[1] - pc[1]], angle); return [r[0] + qc[0], r[1] + qc[1]]; });
    }
    const scale = Math.min(w / 4.6, (h - 50) / 3.1), fx = p => w / 2 + p[0] * scale, fy = p => h * .55 - p[1] * scale;
    dot(ctx, 18, 19, 4, teal, true); label(ctx, '目标点', 28, 24);
    dot(ctx, 95, 19, 3, rust); label(ctx, '扫描点', 105, 24);
    let mse = 0;
    ctx.setLineDash([3, 3]);
    points.forEach(p => {
      const q = nearest(p);
      line(ctx, fx(p), fy(p), fx(q), fy(q), '#83948c');
      mse += ((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2) / points.length;
    });
    ctx.setLineDash([]);
    target.forEach(p => dot(ctx, fx(p), fy(p), 5, teal, true));
    points.forEach(p => dot(ctx, fx(p), fy(p), 2.9, rust));
    return [steps + ' 次', '完成 ' + steps + ' 次更新。当前扫描点到各自最近目标点的均方根距离为 ' + (Math.sqrt(mse) * 100).toFixed(1) + ' 厘米。'];
  }
  function framing(ctx, w, h, count) {
    const left = 16, step = (w - 32) / 30, y = h * .47, height = 30;
    for (let n = 0; n < 30; ++n) {
      ctx.fillStyle = n < 15 ? teal : rust;
      ctx.globalAlpha = n < count ? 1 : .22;
      ctx.fillRect(left + n * step, y, step - Math.min(2, step / 5), height);
    }
    ctx.globalAlpha = 1;
    label(ctx, 'A · 15 字节', left, y - 20, teal);
    label(ctx, 'B · 15 字节', left + step * 15, y - 20, rust);
    const x = left + count * step;
    line(ctx, x, y - 9, x, y + height + 12, ink, 1.6);
    const tx = Math.max(48,Math.min(w - 48,x));
    label(ctx, '已读 ' + count + ' 字节', tx, y + height + 35, ink, 'center');
    const complete = Math.floor(count / 15), remainder = count % 15;
    return [count + ' 字节', '第一次可交付 ' + complete + ' 帧，缓冲区保留 ' + remainder + ' 字节。' + (count < 30 ? '再次读入剩余 ' + (30 - count) + ' 字节后，两帧均已交付。' : '这一次就已读完两帧。')];
  }
  function sampling(ctx, w, h, value) {
    const t = value / 10, weights = [0, -1, -2].map(v => Math.exp(v / t)), sum = weights.reduce((a,b) => a+b, 0);
    const percentages = weights.map(v => v / sum * 100);
    percentages.forEach((p, i) => {
      const x = 49, y = h * (.26 + .24 * i), length = w - 134;
      label(ctx, ['茶','水','奶'][i], 19, y + 5);
      ctx.fillStyle = '#dddfd3'; ctx.fillRect(x, y - 5, length, 10);
      ctx.fillStyle = [teal, rust, '#727d64'][i]; ctx.fillRect(x, y - 5, length * p / 100, 10);
      label(ctx, p.toFixed(1) + '%', w - 18, y + 5, ink, 'right');
    });
    return [t.toFixed(1), '温度为 ' + t.toFixed(1) + ' 时，茶、水、奶的概率分别为 ' + percentages.map(p => p.toFixed(1) + '%').join('、') + '。'];
  }
  function queue(ctx, w, h, speed) {
    const x = 48, right = w - 36, y = h - 32, top = 43, end = Math.min(25, speed) * 4;
    const fy = n => y - (y - top) * n / 100;
    for (const n of [0, 50, 100]) { line(ctx, x, fy(n), right, fy(n), rule); label(ctx, String(n), x - 8, fy(n) + 4, ink, 'right'); }
    for (let s = 0; s <= 4; ++s) label(ctx, s + ' s', x + (right - x) * s / 4, y + 20, ink, 'center');
    label(ctx, '累计帧数', 12, 24);
    label(ctx, '到达', w - 114, 24, teal); label(ctx, '写出', w - 57, 24, rust);
    ctx.fillStyle = '#d6dfd3'; ctx.beginPath(); ctx.moveTo(x, y); ctx.lineTo(right, top); ctx.lineTo(right, fy(end)); ctx.closePath(); ctx.fill();
    line(ctx, x, y, right, top, teal, 3);
    ctx.setLineDash([6, 4]); line(ctx, x, y, right, fy(end), rust, 2.4); ctx.setLineDash([]);
    return [speed + ' 帧/秒', '4 秒内共收到 100 帧，写出 ' + end + ' 帧，队列中还有 ' + (100 - end) + ' 帧。'];
  }
  const draw = {gaussian, disparity, registration, framing, sampling, queue};
  for (const figure of document.querySelectorAll('.lesson-diagram')) {
    const canvas = figure.querySelector('canvas'), ctx = canvas.getContext('2d'), plot = figure.querySelector('.plot');
    const input = figure.querySelector('input'), render = draw[figure.dataset.kind];
    if (!ctx || !render) continue;
    function update() {
      const w = plot.clientWidth, h = plot.clientHeight, scale = Math.min(devicePixelRatio || 1, 2);
      if (!w || !h) return;
      const width = Math.round(w * scale), height = Math.round(h * scale);
      if (canvas.width !== width) canvas.width = width;
      if (canvas.height !== height) canvas.height = height;
      ctx.setTransform(scale, 0, 0, scale, 0, 0);
      ctx.setLineDash([]);
      ctx.fillStyle = paper; ctx.fillRect(0, 0, w, h);
      ctx.font = '14px "Noto Sans CJK SC","Microsoft YaHei",sans-serif';
      const [value, result] = render(ctx, w, h, Number(input.value));
      figure.querySelector('output').textContent = value;
      input.setAttribute('aria-valuetext', value);
      figure.querySelector('.figure-result').textContent = result;
      canvas.hidden = false; figure.querySelector('.poster').hidden = true; figure.querySelector('.figure-control').hidden = false;
    }
    input.addEventListener('input', update);
    if ('ResizeObserver' in window) new ResizeObserver(update).observe(plot);
    else {
      update();
      window.addEventListener('resize', update);
    }
  }
})();
