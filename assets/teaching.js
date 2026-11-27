(() => {
  'use strict';
  const reduced = matchMedia('(prefers-reduced-motion: reduce)');
  const players = [];

  for (const figure of document.querySelectorAll('[data-explainer]')) {
    const frames = [...figure.querySelectorAll('[data-frame]')];
    const controls = figure.querySelector('.ex-controls');
    const previous = controls.querySelector('[data-ex-prev]'), next = controls.querySelector('[data-ex-next]');
    const play = controls.querySelector('[data-ex-play]'), output = controls.querySelector('output');
    let index = 0, running = false, timer, request = 0;
    function pause() {
      running = false; clearTimeout(timer); play.setAttribute('aria-pressed', 'false');
      play.textContent = index === frames.length - 1 ? '重播' : '播放';
    }
    async function show(value) {
      const ticket = ++request, target = Math.max(0, Math.min(frames.length - 1, value));
      const image = frames[target].querySelector('img');
      image.loading = 'eager';
      try { await image.decode(); } catch { /* The descriptive text still explains the step. */ }
      if (ticket !== request) return;
      index = target;
      frames.forEach((frame, i) => { frame.hidden = i !== index; });
      previous.disabled = index === 0; next.disabled = index === frames.length - 1;
      output.textContent = `${index + 1} / ${frames.length}`;
      figure.dataset.step = String(index);
      if (frames[index + 1]) frames[index + 1].querySelector('img').loading = 'eager';
      if (index === frames.length - 1) pause();
      else if (running) timer = setTimeout(() => show(index + 1), 2600);
      else play.textContent = '播放';
    }
    previous.addEventListener('click', () => { pause(); show(index - 1); });
    next.addEventListener('click', () => { pause(); show(index + 1); });
    play.addEventListener('click', () => {
      if (running) { pause(); return; }
      running = true; play.textContent = '暂停'; play.setAttribute('aria-pressed', 'true');
      show(index === frames.length - 1 ? 0 : index);
    });
    // Reserve the longest explanation so playback does not move the controls.
    function measure() {
      let height = 0;
      const saved = frames.map(frame => frame.hidden);
      figure.classList.add('ex-measuring');
      frames.forEach(frame => { frame.hidden = false; height = Math.max(height, frame.getBoundingClientRect().height); });
      frames.forEach((frame, i) => { frame.hidden = saved[i]; });
      figure.classList.remove('ex-measuring');
      figure.querySelector('.ex-frames').style.minHeight = Math.ceil(height) + 'px';
    }
    frames.forEach((frame, i) => { frame.hidden = i !== 0; });
    figure.dataset.ready = 'true'; controls.hidden = false;
    show(0);
    const resize = new ResizeObserver(measure); resize.observe(figure.querySelector('figcaption'));
    players.push({element: figure, pause});
  }

  const kernel = document.querySelector('[data-kernel]');
  if (kernel) {
    const input = [...kernel.querySelectorAll('.input-grid span')], weights = [...kernel.querySelectorAll('.weight-grid span')];
    const result = [...kernel.querySelectorAll('.output-grid span')], windowBox = kernel.querySelector('.kernel-window');
    const slider = kernel.querySelector('[data-kernel-step]'), mode = kernel.querySelector('[data-kernel-mode]');
    const play = kernel.querySelector('[data-kernel-play]'), flip = kernel.querySelector('[data-kernel-flip]');
    const gx = [-1, 0, 1, -2, 0, 2, -1, 0, 1], gy = [-1, -2, -1, 0, 0, 0, 1, 2, 1];
    let inverted = false, running = false, timer;
    const pixels = () => Array.from({length: 25}, (_, i) => inverted ? 140 - (i % 5 < 3 ? 30 : 110) : (i % 5 < 3 ? 30 : 110));
    function pause() { clearTimeout(timer); running = false; play.textContent = +slider.value === 8 ? '重播' : '播放'; play.setAttribute('aria-pressed', 'false'); }
    function draw() {
      const step = +slider.value, row = Math.floor(step / 3), column = step % 3, data = pixels(), filter = mode.value === 'x' ? gx : gy;
      const terms = [];
      input.forEach((cell, i) => { cell.textContent = data[i]; cell.style.background = `rgb(${data[i]},${data[i]},${data[i]})`; });
      kernel.querySelector('.input-grid').setAttribute('aria-label', '五行相同的灰度值，每行是 ' + data.slice(0, 5).join('、'));
      weights.forEach((cell, i) => { cell.textContent = filter[i]; });
      kernel.querySelector('.weight-grid').setAttribute('aria-label', (mode.value === 'x' ? '横向' : '纵向') + ' Sobel 核，按行排列为 ' + filter.join('、'));
      let current = 0;
      const outputs = result.map((cell, n) => {
        const r = Math.floor(n / 3), c = n % 3;
        const value = filter.reduce((sum, weight, k) => sum + weight * data[(r + Math.floor(k / 3)) * 5 + c + k % 3], 0);
        cell.textContent = n <= step ? value : '·';
        cell.className = n === step ? 'is-current' : '';
        cell.dataset.sign = n > step || value === 0 ? 'zero' : value > 0 ? 'positive' : 'negative';
        if (n === step) current = value;
        return n <= step ? value : '未计算';
      });
      kernel.querySelector('.output-grid').setAttribute('aria-label', '输出按行排列为 ' + outputs.join('、'));
      filter.forEach((weight, k) => {
        const value = data[(row + Math.floor(k / 3)) * 5 + column + k % 3];
        terms.push(`${value} × ${weight < 0 ? '(' + weight + ')' : weight}`);
      });
      windowBox.style.transform = `translate(${column * 100 / 3}%,${row * 100 / 3}%)`;
      kernel.querySelector('[data-products]').textContent = terms.join(' + ') + ' = ' + current;
      kernel.querySelector('.kernel-position output').textContent = `${step + 1} / 9`;
      const meaning = mode.value === 'y' ? '每一行相同，纵向没有亮度变化。' : current === 0 ? '窗口内没有明暗变化，正负权重相互抵消。' : current > 0 ? '窗口跨过由暗到亮的边界，导数为正。' : '边界仍在原处，明暗翻转让导数变成负数。';
      kernel.querySelector('.kernel-result').textContent = `输出 [${row}, ${column}] = ${current}。${meaning}`;
      kernel.dataset.step = String(step); kernel.dataset.result = String(current);
      if (step === 8) pause();
    }
    function tick() { slider.value = String(+slider.value + 1); draw(); if (running) timer = setTimeout(tick, 1300); }
    slider.addEventListener('input', () => { pause(); draw(); });
    mode.addEventListener('change', () => { pause(); draw(); });
    flip.addEventListener('click', () => { pause(); inverted = !inverted; flip.setAttribute('aria-pressed', String(inverted)); draw(); });
    play.addEventListener('click', () => {
      if (running) { pause(); return; }
      if (+slider.value === 8) { slider.value = '0'; draw(); }
      running = true; play.textContent = '暂停'; play.setAttribute('aria-pressed', 'true'); timer = setTimeout(tick, 1300);
    });
    kernel.querySelector('.kernel-controls').hidden = false;
    kernel.dataset.ready = 'true'; draw();
    players.push({element: kernel, pause});
  }

  const visibility = new IntersectionObserver(entries => {
    for (const entry of entries) if (!entry.isIntersecting) players.find(p => p.element === entry.target)?.pause();
  });
  players.forEach(player => visibility.observe(player.element));
  document.addEventListener('visibilitychange', () => { if (document.hidden) players.forEach(p => p.pause()); });
  reduced.addEventListener('change', () => { if (reduced.matches) players.forEach(p => p.pause()); });
})();
