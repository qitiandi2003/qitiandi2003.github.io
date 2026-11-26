(async function initPet() {
  'use strict';
  if (document.hidden) {
    document.addEventListener('visibilitychange', initPet, {once: true});
    return;
  }
  if (document.querySelector('.web-pet')) return;
  const root = document.createElement('aside');
  root.className = 'web-pet';
  root.setAttribute('aria-label', '小熊熊网页宠物');
  root.innerHTML = `<div id="pet-menu" class="pet-menu" hidden>
    <div class="pet-menu-head"><strong>小熊熊</strong><button type="button" data-pet-dismiss aria-label="关闭宠物菜单">×</button></div>
    <p class="pet-message" role="status" aria-live="polite">拖动小熊换个位置，摸摸头会开心。</p>
    <div class="pet-actions">
      <button type="button" data-pet-action="pat">摸摸</button>
      <button type="button" data-pet-action="feed">喂点心</button>
      <button type="button" data-pet-action="dance">跳个舞</button>
    </div>
    <div class="pet-menu-footer">
      <button type="button" data-pet-collapse class="pet-collapse">收起小熊</button>
      <button type="button" data-pet-reset class="pet-reset">恢复位置</button>
    </div>
  </div>
  <button type="button" class="pet-character" aria-label="和小熊熊互动" aria-expanded="false" aria-controls="pet-menu"
    aria-description="拖动或按方向键移动，轻抚头部可互动，回车打开菜单。">
    <span class="pet-effects" aria-hidden="true"></span>
    <span class="pet-sprite" aria-hidden="true"></span>
    <span class="pet-caption">小熊熊</span>
  </button>`;
  const menu = root.querySelector('.pet-menu');
  const character = root.querySelector('.pet-character');
  const sprite = root.querySelector('.pet-sprite');
  const message = root.querySelector('.pet-message');
  const effects = root.querySelector('.pet-effects');
  try {
    const base = document.documentElement.dataset.base || './';
    const response = await fetch(base + 'assets/pet-bear-rig.svg?v=pet-3', {mode: 'same-origin'});
    if (!response.ok || !/^image\/svg\+xml(?:\s*;|$)/i.test(response.headers.get('content-type') || '')) {
      throw new Error('Artwork unavailable');
    }
    const svg = new DOMParser().parseFromString(await response.text(), 'image/svg+xml').documentElement;
    if (svg.namespaceURI !== 'http://www.w3.org/2000/svg' || svg.localName !== 'svg' || svg.querySelector('parsererror') ||
        !svg.classList.contains('pb-rig') || svg.getAttribute('viewBox')?.trim().split(/[\s,]+/).join(' ') !== '0 0 48 56') {
      throw new Error('Unexpected artwork');
    }
    sprite.replaceChildren(document.importNode(svg, true));
    root.dataset.art = 'rig';
  } catch {
    root.dataset.art = 'fallback';
  }
  if (document.hidden) {
    await new Promise(resolve => {
      function visible() {
        if (document.hidden) return;
        document.removeEventListener('visibilitychange', visible);
        resolve();
      }
      document.addEventListener('visibilitychange', visible);
    });
  }
  if (document.querySelector('.web-pet')) return;
  const header = document.querySelector('.site-header');
  const wide = matchMedia('(min-width: 1280px)');
  const reduced = matchMedia('(prefers-reduced-motion: reduce)');
  const sizeKey = 'qi-butterbear-collapsed', positionKey = 'qi-pet-position';
  const clamp = (value, low, high) => Math.max(low, Math.min(value, high));
  let preference = null, position = null, press = null, ignoreClick = false;
  let frame, timer, layoutFrame, nextPat = 0;
  let idleTimer, idleStep = 0, phaseTimer, acting = false;

  function viewport() {
    const view = window.visualViewport;
    const x = view?.offsetLeft || 0, y = view?.offsetTop || 0;
    const css = getComputedStyle(root);
    const inset = edge => Math.max(10, parseFloat(css.getPropertyValue('--pet-safe-' + edge)) || 0);
    return {
      left: x + inset('left'), right: x + (view?.width || document.documentElement.clientWidth) - inset('right'),
      top: Math.max(y + inset('top'), (header?.getBoundingClientRect().bottom || 0) + 10),
      bottom: y + (view?.height || innerHeight) - inset('bottom')
    };
  }
  function travel() {
    const v = viewport();
    return {x: v.left, y: v.top, w: Math.max(0, v.right - v.left - root.offsetWidth),
      h: Math.max(0, v.bottom - v.top - root.offsetHeight)};
  }
  function place(x, y) {
    Object.assign(root.style, {left: x + 'px', top: y + 'px', right: 'auto', bottom: 'auto'});
  }
  function moveTo(x, y) {
    const b = travel();
    x = clamp(x, b.x, b.x + b.w); y = clamp(y, b.y, b.y + b.h);
    place(x, y);
    return {x: b.w ? (x - b.x) / b.w : 0, y: b.h ? (y - b.y) / b.h : 0};
  }
  function positionMenu() {
    if (menu.hidden) return;
    const v = viewport(), bear = character.getBoundingClientRect(), gap = 8;
    menu.style.maxWidth = Math.max(1, v.right - v.left) + 'px';
    menu.style.maxHeight = Math.max(1, v.bottom - v.top) + 'px';
    const w = menu.offsetWidth, h = menu.offsetHeight;
    const above = bear.top - gap - v.top, below = v.bottom - bear.bottom - gap;
    let x = bear.right - w, y = bear.top - gap - h;
    if (above < h) {
      if (below >= h) y = bear.bottom + gap;
      else if (bear.left - gap - v.left >= w || v.right - bear.right - gap >= w) {
        x = bear.left - gap - v.left >= w ? bear.left - gap - w : bear.right + gap;
        y = (bear.top + bear.bottom - h) / 2;
      } else {
        y = above > below ? bear.top - gap - h : bear.bottom + gap;
      }
    }
    menu.style.left = clamp(x, v.left, v.right - w) + 'px';
    menu.style.top = clamp(y, v.top, v.bottom - menu.offsetHeight) + 'px';
  }
  function layout() {
    if (position) {
      const b = travel();
      place(b.x + position.x * b.w, b.y + position.y * b.h);
    } else {
      for (const side of ['top', 'right', 'bottom', 'left']) root.style.removeProperty(side);
    }
    positionMenu();
  }
  function scheduleLayout(event) {
    if (event.type === 'scroll' && event.currentTarget === window && menu.hidden) return;
    if (root.dataset.idle === 'walk') { stopIdle(); resumeIdle(); }
    if (document.hidden || press || layoutFrame || (!position && menu.hidden)) return;
    layoutFrame = requestAnimationFrame(() => { layoutFrame = 0; if (!press) layout(); });
  }
  function storePosition(value) {
    position = value;
    try {
      if (value) localStorage.setItem(positionKey, JSON.stringify(value));
      else localStorage.removeItem(positionKey);
    } catch {}
  }
  function applySize() {
    const before = root.dataset.collapsed;
    root.dataset.collapsed = preference ?? String(!wide.matches);
    if (root.dataset.collapsed === 'true' && !menu.hidden) closeMenu(root.contains(document.activeElement));
    layout();
    if (before === root.dataset.collapsed) return;
    stopIdle();
    if (root.dataset.collapsed === 'false') phase('enter', 2200, () => resumeIdle(300));
    else clearPhase();
  }
  function readSettings() {
    try {
      const saved = localStorage.getItem(sizeKey);
      preference = saved === 'true' || saved === 'false' ? saved : null;
      position = null;
      const point = JSON.parse(localStorage.getItem(positionKey));
      if (point && [point.x, point.y].every(n => Number.isFinite(n) && n >= 0 && n <= 1)) position = point;
    } catch {}
    applySize();
  }
  function chooseSize(collapsed) {
    preference = String(collapsed);
    try { localStorage.setItem(sizeKey, preference); } catch {}
    if (collapsed) { stopIdle(); phase('exit', 1700, applySize); }
    else applySize();
  }

  function clearPhase() {
    clearTimeout(phaseTimer);
    delete root.dataset.phase;
  }
  function phase(name, ms, done) {
    clearPhase();
    if (reduced.matches || root.dataset.paused === 'true') { done(); return; }
    root.dataset.phase = name;
    phaseTimer = setTimeout(() => { clearPhase(); done(); }, ms);
  }
  function cancelExit() {
    if (root.dataset.phase === 'exit') { clearPhase(); chooseSize(false); }
  }
  const cycle = [['walk', 6000], ['snack', 10000], ['rest', 4500], ['walk', 6000], ['game', 8000],
    ['walk', 6000], ['sleep', 10000], ['rest', 4500]];
  function canIdle() {
    return !document.hidden && !reduced.matches && root.dataset.paused !== 'true' &&
      root.dataset.collapsed === 'false' && !root.dataset.phase && !press && !acting && menu.hidden &&
      !character.matches(':focus-visible');
  }
  function stopIdle() {
    clearTimeout(idleTimer); idleTimer = 0; idleStep = 0;
    const here = root.dataset.idle === 'walk' ? root.getBoundingClientRect() : null;
    root.dataset.idle = 'rest';
    delete root.dataset.direction;
    // Freeze an interrupted walk at the rendered pixel. Only explicit moves write storage.
    if (here) position = moveTo(here.left, here.top);
  }
  function nextIdle() {
    idleTimer = 0;
    if (!canIdle()) { stopIdle(); return; }
    if (root.dataset.idle === 'walk') { root.dataset.idle = 'rest'; delete root.dataset.direction; layout(); }
    const [state, ms] = cycle[idleStep];
    idleStep = (idleStep + 1) % cycle.length;
    if (state === 'walk') {
      const here = root.getBoundingClientRect(), b = travel();
      place(here.left, here.top);
      const start = root.getBoundingClientRect();
      const distance = 64 + Math.round(Math.random() * 48);
      const roomLeft = start.left - b.x, roomRight = b.x + b.w - start.left;
      const right = Math.min(roomLeft, roomRight) >= distance ? Math.random() < .5 : roomRight > roomLeft;
      root.style.setProperty('--pet-walk-leg-duration', ms / 2 + 'ms');
      root.dataset.direction = right ? 'right' : 'left';
      root.dataset.idle = 'walk';
      moveTo(start.left + (right ? distance : -distance), start.top);
      idleTimer = setTimeout(() => {
        root.dataset.direction = right ? 'left' : 'right';
        place(start.left, start.top);
        idleTimer = setTimeout(nextIdle, ms / 2);
      }, ms / 2);
    } else {
      root.dataset.idle = state;
      idleTimer = setTimeout(nextIdle, ms);
    }
  }
  function resumeIdle(delay = 2500) {
    if (!idleTimer && canIdle()) idleTimer = setTimeout(nextIdle, delay);
  }

  function closeMenu(restoreFocus) {
    menu.hidden = true;
    character.setAttribute('aria-expanded', 'false');
    document.removeEventListener('keydown', escape);
    document.removeEventListener('click', outside);
    document.removeEventListener('focusin', outside);
    if (restoreFocus && !document.querySelector('dialog[open]')) character.focus({preventScroll: true});
    resumeIdle();
  }
  function escape(event) {
    if (event.key !== 'Escape' || event.defaultPrevented || document.querySelector('dialog[open]')) return;
    event.preventDefault();
    closeMenu(true);
  }
  function outside(event) {
    if (root.contains(event.target)) return;
    const focus = document.activeElement;
    closeMenu(event.type === 'click' && (focus === document.body || root.contains(focus)));
  }
  character.addEventListener('click', event => {
    if (ignoreClick && event.detail) { ignoreClick = false; return; }
    if (root.dataset.paused === 'true') return;
    cancelExit();
    if (!menu.hidden) { closeMenu(true); return; }
    stopIdle();
    if (root.dataset.collapsed === 'true') chooseSize(false);
    menu.hidden = false;
    positionMenu();
    character.setAttribute('aria-expanded', 'true');
    menu.querySelector('[data-pet-action]').focus({preventScroll: true});
    document.addEventListener('keydown', escape);
    document.addEventListener('click', outside);
    document.addEventListener('focusin', outside);
  });

  const actions = {
    pat: {icon: '♥', message: '小熊熊眯起眼，轻轻蹭了蹭你的手。', duration: 2400},
    feed: {icon: '', message: '小熊熊捧起汉堡，手边还有一杯可乐。', duration: 10000},
    dance: {icon: '♪', message: '小熊熊跟着节拍晃了晃。', duration: 3200}
  };
  function stopEffect() {
    acting = false;
    cancelAnimationFrame(frame); clearTimeout(timer);
    delete root.dataset.action;
    effects.textContent = '';
  }
  function play(action) {
    clearPhase();
    stopIdle();
    stopEffect();
    acting = true;
    const scene = actions[action];
    message.textContent = scene.message;
    positionMenu();
    if (!menu.hidden) {
      const panel = menu.getBoundingClientRect(), bear = sprite.getBoundingClientRect();
      if (panel.left < bear.right && panel.right > bear.left && panel.top < bear.bottom && panel.bottom > bear.top) {
        closeMenu(true);
      }
    }
    frame = requestAnimationFrame(() => {
      frame = requestAnimationFrame(() => {
        root.dataset.action = action;
        effects.textContent = scene.icon;
        timer = setTimeout(() => {
          stopEffect();
          if (action === 'feed') { message.textContent = '吃饱啦，小熊熊打了个小小的饱嗝。'; positionMenu(); }
          resumeIdle();
        }, reduced.matches ? 1200 : scene.duration);
      });
    });
  }
  menu.addEventListener('click', event => {
    const button = event.target.closest('button');
    if (!button) return;
    if (button.dataset.petAction) { play(button.dataset.petAction); return; }
    closeMenu(true);
    if (button.hasAttribute('data-pet-collapse')) { stopEffect(); chooseSize(true); }
    if (button.hasAttribute('data-pet-reset')) { storePosition(null); layout(); }
  });

  function onHead(event) {
    const r = sprite.getBoundingClientRect();
    const x = (event.clientX - r.left) / r.width, y = (event.clientY - r.top) / r.height;
    return x > .15 && x < .85 && y > .09 && y < .50;
  }
  function strokeHead(event) {
    if (event.pointerType === 'touch' || event.buttons || root.dataset.collapsed === 'true' ||
        root.dataset.paused === 'true' || root.dataset.phase === 'exit' || performance.now() < nextPat || !onHead(event)) return;
    nextPat = performance.now() + actions.pat.duration;
    play('pat');
  }
  character.addEventListener('pointerdown', event => {
    if (!event.isPrimary || event.button !== 0 || press || root.dataset.paused === 'true') return;
    cancelExit();
    stopIdle();
    ignoreClick = false;
    const r = root.getBoundingClientRect();
    press = {id: event.pointerId, x: event.clientX, y: event.clientY, left: r.left, top: r.top, dragging: false};
    character.setPointerCapture(event.pointerId);
  });
  character.addEventListener('pointermove', event => {
    if (!press) { strokeHead(event); return; }
    if (event.pointerId !== press.id) return;
    const dx = event.clientX - press.x, dy = event.clientY - press.y;
    if (!press.dragging) {
      if (Math.hypot(dx, dy) < 5) return;
      press.dragging = true; root.dataset.dragging = 'true';
      clearPhase();
      stopEffect(); closeMenu(false);
    }
    moveTo(press.left + dx, press.top + dy);
  });
  function endPress(event) {
    if (!press || (event && event.pointerId !== press.id)) return;
    const p = press; press = null;
    if (p.dragging) {
      ignoreClick = true;
      if (event?.type === 'pointerup') storePosition(moveTo(p.left + event.clientX - p.x, p.top + event.clientY - p.y));
      else layout();
    } else if (event?.type === 'pointerup' && event.pointerType === 'touch' &&
               root.dataset.collapsed === 'false' && onHead(event)) play('pat');
    delete root.dataset.dragging;
    if (character.hasPointerCapture(p.id)) character.releasePointerCapture(p.id);
    resumeIdle();
  }
  for (const type of ['pointerup', 'pointercancel', 'lostpointercapture']) character.addEventListener(type, endPress);
  root.addEventListener('focusin', stopIdle);
  root.addEventListener('focusout', event => {
    if (!root.contains(event.relatedTarget)) queueMicrotask(resumeIdle);
  });
  character.addEventListener('keydown', event => {
    if (event.altKey || event.ctrlKey || event.metaKey || press) return;
    const step = {ArrowLeft: [-16,0], ArrowRight: [16,0], ArrowUp: [0,-16], ArrowDown: [0,16]}[event.key];
    if (!step) return;
    event.preventDefault();
    clearPhase(); stopEffect();
    stopIdle();
    const r = root.getBoundingClientRect();
    storePosition(moveTo(r.left + step[0], r.top + step[1])); positionMenu(); resumeIdle();
  });

  function pause() {
    root.dataset.paused = 'true';
    stopIdle(); clearPhase();
    endPress(); stopEffect(); closeMenu(false);
    applySize();
    cancelAnimationFrame(layoutFrame); layoutFrame = 0;
  }
  function syncPaused() {
    if (document.hidden || document.querySelector('dialog[open]')) pause();
    else { root.dataset.paused = 'false'; layout(); resumeIdle(); }
  }
  window.addEventListener('resize', scheduleLayout);
  window.addEventListener('scroll', scheduleLayout, {passive: true});
  window.visualViewport?.addEventListener('resize', scheduleLayout);
  window.visualViewport?.addEventListener('scroll', scheduleLayout);
  wide.addEventListener('change', applySize);
  reduced.addEventListener('change', () => { stopIdle(); stopEffect(); clearPhase(); applySize(); resumeIdle(); });
  window.addEventListener('storage', event => {
    if (event.key === sizeKey || event.key === positionKey || event.key === null) { endPress(); readSettings(); }
  });
  window.addEventListener('pagehide', pause);
  window.addEventListener('pageshow', event => { if (event.persisted) { readSettings(); syncPaused(); } });
  document.addEventListener('visibilitychange', syncPaused);
  new MutationObserver(records => {
    if (records.some(r => r.target.matches('dialog'))) syncPaused();
  }).observe(document.body, {subtree: true, attributes: true, attributeFilter: ['open']});
  document.body.append(root);
  syncPaused();
  readSettings();
})();
