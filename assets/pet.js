(function initPet() {
  'use strict';
  if (document.hidden) {
    document.addEventListener('visibilitychange', initPet, {once: true});
    return;
  }
  if (document.querySelector('.web-pet')) return;
  const root = document.createElement('aside');
  root.className = 'web-pet';
  root.setAttribute('aria-label', 'Butterbear 网页宠物');
  root.innerHTML = `<div id="pet-menu" class="pet-menu" hidden>
    <div class="pet-menu-head"><strong>Butterbear</strong><button type="button" data-pet-dismiss aria-label="关闭宠物菜单">×</button></div>
    <p class="pet-message" role="status" aria-live="polite">休息一下，陪你玩一会儿。</p>
    <div class="pet-actions">
      <button type="button" data-pet-action="pat">摸摸</button>
      <button type="button" data-pet-action="feed">喂点心</button>
      <button type="button" data-pet-action="dance">跳个舞</button>
    </div>
    <button type="button" data-pet-collapse class="pet-collapse">收起小熊</button>
  </div>
  <button type="button" class="pet-character" aria-label="和 Butterbear 互动" aria-expanded="false" aria-controls="pet-menu">
    <span class="pet-effects" aria-hidden="true"></span>
    <img width="192" height="259" alt="" draggable="false" decoding="async">
    <span class="pet-caption">Butterbear</span>
  </button>`;
  const menu = root.querySelector('.pet-menu');
  const character = root.querySelector('.pet-character');
  const message = root.querySelector('.pet-message');
  const effects = root.querySelector('.pet-effects');
  const wide = matchMedia('(min-width: 1280px)');
  const storageKey = 'qi-butterbear-collapsed';
  let preference = null;
  let frame, timer;

  function applySize() {
    root.dataset.collapsed = preference ?? String(!wide.matches);
    if (root.dataset.collapsed === 'true' && !menu.hidden) closeMenu(root.contains(document.activeElement));
  }
  function readSize() {
    try {
      const saved = localStorage.getItem(storageKey);
      preference = saved === 'true' || saved === 'false' ? saved : null;
    } catch {}
    applySize();
  }
  function chooseSize(collapsed) {
    preference = String(collapsed);
    try { localStorage.setItem(storageKey, preference); } catch {}
    applySize();
  }

  function closeMenu(restoreFocus) {
    menu.hidden = true;
    character.setAttribute('aria-expanded', 'false');
    document.removeEventListener('keydown', escape);
    document.removeEventListener('click', outside);
    document.removeEventListener('focusin', outside);
    if (restoreFocus && !document.querySelector('dialog[open]')) character.focus({preventScroll: true});
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
  character.addEventListener('click', () => {
    if (!menu.hidden) { closeMenu(true); return; }
    if (root.dataset.collapsed === 'true') chooseSize(false);
    menu.hidden = false;
    character.setAttribute('aria-expanded', 'true');
    menu.querySelector('[data-pet-action]').focus({preventScroll: true});
    document.addEventListener('keydown', escape);
    document.addEventListener('click', outside);
    document.addEventListener('focusin', outside);
  });

  const actions = {
    pat: ['♥', '小熊蹭了蹭你的手。'],
    feed: ['🍪', '小熊接过点心，咬了一小口。'],
    dance: ['♪', '小熊跟着节拍晃了晃。']
  };
  function stopEffect() {
    cancelAnimationFrame(frame);
    clearTimeout(timer);
    delete root.dataset.action;
    effects.textContent = '';
  }
  menu.addEventListener('click', event => {
    const button = event.target.closest('button');
    if (!button) return;
    const action = button.dataset.petAction;
    if (action) {
      stopEffect();
      message.textContent = actions[action][1];
      frame = requestAnimationFrame(() => {
        frame = requestAnimationFrame(() => {
          root.dataset.action = action;
          effects.textContent = actions[action][0];
          timer = setTimeout(stopEffect, 1600);
        });
      });
    } else {
      const collapse = button.hasAttribute('data-pet-collapse');
      closeMenu(true);
      if (collapse) { stopEffect(); chooseSize(true); }
    }
  });

  function suspend() {
    stopEffect();
    closeMenu(false);
  }
  wide.addEventListener('change', applySize);
  window.addEventListener('storage', event => { if (event.key === storageKey || event.key === null) readSize(); });
  window.addEventListener('pagehide', suspend);
  window.addEventListener('pageshow', event => { if (event.persisted) readSize(); });
  document.addEventListener('visibilitychange', () => { if (document.hidden) suspend(); });
  readSize();
  root.querySelector('img').src = (document.documentElement.dataset.base || './') + 'assets/butterbear.webp';
  document.body.append(root);
})();
