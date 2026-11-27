(() => {
  'use strict';

  function setupSearch() {
    const dialog = document.getElementById('search-dialog');
    const input = document.getElementById('search-input');
    const results = document.getElementById('search-results');
    const status = document.getElementById('search-status');
    if (!dialog || !input || !results || !status) return;

    const suggestions = document.getElementById('search-suggestions');
    const more = dialog.querySelector('[data-more-search]');
    const clear = dialog.querySelector('[data-clear-search]');
    const retry = dialog.querySelector('[data-retry-search]');
    const body = dialog.querySelector('.search-body');
    const base = document.documentElement.dataset.base || './';
    const normalize = value => value.normalize('NFKC').toLowerCase()
      .replace(/\bros\s*2\b/gu, 'ros2').replace(/\bc\+\+/gu, 'cplusplus');
    let opener;
    let visibleCount = 6;
    let indexState = 'idle';
    let entries = [];
    let pendingEnter;

    function finishIndex() {
      if (Array.isArray(window.QI_ARTICLES)) {
        entries = window.QI_ARTICLES.map(article => {
          const fields = [article.title, article.topic, article.slug, article.keywords, article.summary].map(value => {
            const text = normalize(value || '');
            return {text, words: text.match(/[\p{Script=Latin}\p{N}]+/gu) || []};
          });
          return {article, fields};
        });
        indexState = 'ready';
      } else {
        indexState = 'error';
      }
      if (dialog.open) {
        renderResults();
        if (pendingEnter === input.value && document.activeElement === input && !document.hidden) {
          results.querySelector('a')?.click();
        }
      }
      pendingEnter = undefined;
    }

    function loadIndex() {
      if (indexState === 'ready' || indexState === 'loading') return;
      if (Array.isArray(window.QI_ARTICLES)) {
        finishIndex();
        return;
      }
      indexState = 'loading';
      const script = document.createElement('script');
      script.async = true;
      script.src = dialog.dataset.indexSrc || base + 'assets/search-index.js';
      script.onload = script.onerror = () => {
        script.remove();
        finishIndex();
      };
      document.head.append(script);
    }

    function score(fields, terms) {
      let total = 0;
      for (const term of terms) {
        const index = fields.findIndex(field => term.han
          ? field.text.includes(term.text)
          : field.words.some(word => word.startsWith(term.text)));
        if (index === -1) return 0;
        total += fields.length - index;
        if (fields[index].text === term.text) total += 2;
      }
      return total;
    }

    function renderResults(resetScroll = true) {
      const query = normalize(input.value).trim();
      const words = query.replace(/(\p{Script=Han}+)/gu, ' $1 ').match(/[\p{L}\p{N}]+/gu) || [];
      const terms = words.map(text => ({text, han: /\p{Script=Han}/u.test(text)}));
      const matches = terms.length
        ? entries.map(({article, fields}) => ({article, score: score(fields, terms)}))
          .filter(match => match.score > 0)
          .sort((a, b) => b.score - a.score)
          .map(match => match.article)
        : [];
      const fragment = document.createDocumentFragment();
      for (const article of matches.slice(0, visibleCount)) {
        const link = document.createElement('a');
        link.className = 'result-link';
        link.href = base + 'notes/' + encodeURIComponent(article.slug) + '/';
        const topic = document.createElement('span');
        topic.className = 'result-topic';
        topic.textContent = article.topic;
        const title = document.createElement('span');
        title.className = 'result-title';
        title.textContent = article.title;
        link.append(title, topic);
        const item = document.createElement('li');
        item.append(link);
        fragment.append(item);
      }
      results.replaceChildren(fragment);
      if (resetScroll) body.scrollTop = 0;
      suggestions.hidden = terms.length > 0;
      results.hidden = !matches.length;
      if (retry) retry.hidden = indexState !== 'error';
      clear.hidden = !input.value.length;
      more.hidden = matches.length <= visibleCount;
      more.textContent = more.hidden ? '' : '再显示 ' + Math.min(6, matches.length - visibleCount) + ' 篇';
      if (indexState !== 'ready') {
        status.textContent = indexState === 'error' ? '搜索暂时不可用，请重试。' : (terms.length ? '正在搜索…' : '');
        return;
      }
      status.textContent = !terms.length
        ? (query ? '试试文章名称或主题关键词。' : '')
        : matches.length
          ? '找到 ' + matches.length + ' 篇文章' + (matches.length > visibleCount ? ' · 显示前 ' + visibleCount + ' 篇' : '')
          : '没有找到相关文章，换个关键词试试。';
    }

    function updateQuery(value) {
      pendingEnter = undefined;
      input.value = value;
      visibleCount = 6;
      renderResults();
      input.focus();
    }

    function openSearch(trigger) {
      if (dialog.open) return;
      opener = trigger;
      if (opener && opener !== document.activeElement) opener.focus();
      input.value = '';
      visibleCount = 6;
      pendingEnter = undefined;
      loadIndex();
      renderResults();
      dialog.showModal();
      document.documentElement.classList.add('search-open');
      input.focus();
    }

    status.setAttribute('role', 'status');
    document.querySelectorAll('[data-open-search]').forEach(button => {
      button.addEventListener('click', event => {
        event.preventDefault();
        openSearch(button);
      });
    });
    dialog.querySelectorAll('[data-close-search]').forEach(button => {
      button.addEventListener('click', () => dialog.close());
    });
    dialog.addEventListener('close', () => {
      document.documentElement.classList.remove('search-open');
      if (opener?.closest('#pet-menu[hidden]')) opener = document.querySelector('.pet-character');
      if (opener && opener.isConnected) opener.focus();
    });
    dialog.addEventListener('click', event => {
      if (event.target !== dialog) return;
      const rect = dialog.getBoundingClientRect();
      if (event.clientX < rect.left || event.clientX > rect.right ||
          event.clientY < rect.top || event.clientY > rect.bottom) dialog.close();
    });
    dialog.querySelectorAll('[data-search-query]').forEach(button => {
      button.addEventListener('click', () => updateQuery(button.dataset.searchQuery));
    });
    clear.addEventListener('click', () => updateQuery(''));
    retry?.addEventListener('click', () => {
      loadIndex();
      renderResults();
      input.focus();
    });
    more.addEventListener('click', () => {
      const previousCount = results.children.length;
      visibleCount += 6;
      renderResults(false);
      results.querySelectorAll('a')[previousCount]?.focus();
    });
    input.addEventListener('input', () => {
      pendingEnter = undefined;
      visibleCount = 6;
      renderResults();
    });
    input.addEventListener('compositionstart', () => { pendingEnter = undefined; });
    input.addEventListener('blur', () => { pendingEnter = undefined; });
    input.addEventListener('keydown', event => {
      if (event.isComposing || event.keyCode === 229) return;
      if (event.key === 'Escape') {
        event.preventDefault();
        dialog.close();
        return;
      }
      if (event.key !== 'Enter') return;
      event.preventDefault();
      if (indexState === 'loading') pendingEnter = input.value;
      else results.querySelector('a')?.click();
    });
    document.addEventListener('keydown', event => {
      if (event.defaultPrevented || event.isComposing || event.repeat || event.altKey) return;
      const commandK = (event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'k';
      const editing = event.target instanceof Element &&
        (event.target.closest('input, textarea, select') || event.target.isContentEditable);
      const slash = event.key === '/' && !event.ctrlKey && !event.metaKey && !editing;
      if (commandK || slash) {
        event.preventDefault();
        if (dialog.open) input.focus();
        else openSearch(document.activeElement);
      }
    });
  }

  function setupFilters(animateIn) {
    const buttons = Array.from(document.querySelectorAll('[data-filter]'));
    const cards = document.querySelectorAll('[data-group]');
    const count = document.getElementById('filter-count');
    if (!buttons.length) return;
    const list = document.querySelector('.note-list');
    let previousGroup;
    let feedback;

    function renderFilters() {
      const requested = new URL(location.href).searchParams.get('group');
      const group = buttons.some(button => button.dataset.filter === requested) ? requested : 'all';
      let visible = 0;
      buttons.forEach(button => button.setAttribute('aria-pressed', String(button.dataset.filter === group)));
      cards.forEach(card => {
        card.hidden = group !== 'all' && card.dataset.group !== group;
        if (!card.hidden) visible++;
      });
      if (count) count.textContent = '共 ' + visible + ' 篇';
      if (previousGroup !== undefined && previousGroup !== group && list) {
        feedback?.cancel();
        feedback = animateIn(list, .7, 4, 150);
      }
      previousGroup = group;
    }

    buttons.forEach(button => {
      button.addEventListener('click', event => {
        event.preventDefault();
        const url = new URL(location.href);
        const group = button.dataset.filter;
        if (group === 'all') url.searchParams.delete('group');
        else url.searchParams.set('group', group);
        if (url.href !== location.href) history.pushState(null, '', url);
        renderFilters();
      });
    });
    window.addEventListener('popstate', renderFilters);
    renderFilters();
  }

  function setupMotion() {
    const motion = matchMedia('(prefers-reduced-motion: no-preference)');
    const pointer = matchMedia('(hover: hover) and (pointer: fine)');
    const surfaces = document.querySelectorAll('.note-row > a, .path-card, .next-note, .course-hero');
    const waiting = new Set(document.querySelectorAll('.note-row, .path-card, .article-image, .lesson-diagram, .chapter-links'));
    const animations = new Set();
    let surface, clientX, clientY;
    let frame = 0;
    let active = false;
    surfaces.forEach(element => element.classList.add('light-surface'));

    function animateIn(element, opacity, distance, duration) {
      if (!active || document.hidden || !element.animate) return;
      const animation = element.animate([
        {opacity, transform: 'translateY(' + distance + 'px)'},
        {opacity: 1, transform: 'none'}
      ], {duration, easing: 'cubic-bezier(.2,.7,.2,1)'});
      animations.add(animation);
      const forget = () => animations.delete(animation);
      animation.finished.then(forget, forget);
      return animation;
    }

    const observer = 'IntersectionObserver' in window ? new IntersectionObserver(entries => {
      if (!active || document.hidden) return;
      for (const entry of entries) {
        if (!entry.isIntersecting || !waiting.delete(entry.target)) continue;
        observer.unobserve(entry.target);
        animateIn(entry.target, .85, 6, 260);
      }
    }) : null;

    function clearLight() {
      cancelAnimationFrame(frame);
      frame = 0;
      surface?.style.removeProperty('--light-x');
      surface?.style.removeProperty('--light-y');
      surface = null;
    }

    function moveLight(event) {
      if (event.pointerType === 'touch' || document.hidden) return;
      if (surface !== event.currentTarget) clearLight();
      surface = event.currentTarget;
      clientX = event.clientX;
      clientY = event.clientY;
      if (frame) return;
      frame = requestAnimationFrame(() => {
        frame = 0;
        const rect = surface.getBoundingClientRect();
        surface.style.setProperty('--light-x', clientX - rect.left + 'px');
        surface.style.setProperty('--light-y', clientY - rect.top + 'px');
      });
    }

    function suspend() {
      active = false;
      clearLight();
      observer?.disconnect();
      animations.forEach(animation => animation.cancel());
      surfaces.forEach(element => {
        element.removeEventListener('pointermove', moveLight);
        element.removeEventListener('pointerleave', clearLight);
      });
    }

    function refresh() {
      suspend();
      if (document.hidden || !motion.matches) return;
      active = true;
      if (observer) waiting.forEach(element => observer.observe(element));
      if (pointer.matches) surfaces.forEach(element => {
        element.addEventListener('pointermove', moveLight, {passive: true});
        element.addEventListener('pointerleave', clearLight);
      });
    }
    motion.addEventListener('change', refresh);
    pointer.addEventListener('change', refresh);
    document.addEventListener('visibilitychange', refresh);
    window.addEventListener('pagehide', suspend);
    window.addEventListener('pageshow', event => { if (event.persisted) refresh(); });
    refresh();
    return animateIn;
  }

  function setupIllustration() {
    const link = document.querySelector('.figure-zoom');
    if (!link || typeof HTMLDialogElement === 'undefined') return;
    const dialog = document.createElement('dialog');
    dialog.className = 'figure-dialog';
    dialog.setAttribute('aria-label', '放大图解');
    const close = document.createElement('button');
    close.type = 'button';
    close.className = 'quiet-button';
    close.textContent = '关闭图解';
    const image = document.createElement('img');
    image.alt = link.closest('figure').querySelector('.caption-text').textContent;
    const viewport = document.createElement('div');
    viewport.className = 'figure-viewport';
    viewport.tabIndex = 0;
    viewport.setAttribute('role', 'region');
    viewport.setAttribute('aria-label','放大的教学图');
    const hint = document.createElement('p');
    hint.className = 'pan-hint';
    hint.textContent = '左右滑动查看图解细节';
    viewport.append(image);
    dialog.append(close, hint, viewport);
    document.body.append(dialog);
    link.addEventListener('click', event => {
      if (event.ctrlKey || event.metaKey || event.shiftKey || event.altKey) return;
      event.preventDefault();
      image.src = link.href;
      dialog.showModal();
      viewport.scrollLeft = 0;
      close.focus();
    });
    close.addEventListener('click', () => dialog.close());
    dialog.addEventListener('click', event => { if (event.target === dialog) dialog.close(); });
    dialog.addEventListener('close', () => link.focus());
  }

  function setupPrefetch() {
    if (location.protocol !== 'https:' && location.protocol !== 'http:') return;
    const root = new URL(document.documentElement.dataset.base || './', location.href).pathname;
    const sent = new Set();
    let timer, pendingLink;

    function clear() {
      clearTimeout(timer);
      pendingLink = null;
    }

    function destination(link) {
      const connection = navigator.connection;
      if (document.hidden || sent.size >= 2 || connection?.saveData ||
          ['slow-2g', '2g'].includes(connection?.effectiveType)) return;
      if (link.hasAttribute('download') || (link.target && link.target !== '_self') ||
          link.origin !== location.origin || link.username || link.password || /[?#]/.test(link.href) ||
          link.pathname === location.pathname || !link.pathname.startsWith(root) || sent.has(link.href)) return;
      const route = link.pathname.slice(root.length);
      if (!route || route === 'about/' || /^(?:notes|topics)\/(?:[a-z0-9-]+\/)?$/.test(route)) return link.href;
    }

    function prefetch(link) {
      clear();
      const href = destination(link);
      if (!href) return;
      sent.add(href);
      if (HTMLScriptElement.supports?.('speculationrules')) {
        const rules = document.createElement('script');
        rules.type = 'speculationrules';
        rules.textContent = JSON.stringify({prefetch: [{source: 'list', urls: [href], eagerness: 'immediate'}]});
        document.head.append(rules);
      } else {
        const hint = document.createElement('link');
        hint.rel = 'prefetch';
        hint.href = href;
        document.head.append(hint);
      }
    }

    function intent(event) {
      const link = event.target.closest?.('a[href]');
      if (!link || link.contains(event.relatedTarget) || !destination(link)) return;
      if (event.type === 'pointerdown') {
        if (!event.button && !event.altKey && !event.ctrlKey && !event.metaKey && !event.shiftKey) prefetch(link);
      } else if (event.pointerType !== 'touch' && pendingLink !== link) {
        clear();
        pendingLink = link;
        timer = setTimeout(() => prefetch(link), 90);
      }
    }
    function leave(event) {
      if (pendingLink?.contains(event.target) && !pendingLink.contains(event.relatedTarget)) clear();
    }
    for (const type of ['pointerover', 'focusin', 'pointerdown']) document.addEventListener(type, intent);
    for (const type of ['pointerout', 'focusout']) document.addEventListener(type, leave);
    document.addEventListener('visibilitychange', () => { if (document.hidden) clear(); });
    window.addEventListener('pagehide', clear);
  }

  function schedulePet() {
    const base = document.documentElement.dataset.base || './';
    function load() {
      if (document.hidden) {
        document.addEventListener('visibilitychange', idle, {once: true});
        return;
      }
      if (document.getElementById('pet-styles')) return;
      const css = document.createElement('link');
      css.id = 'pet-styles';
      css.rel = 'stylesheet';
      css.href = base + 'assets/pet.css?v=plush-3';
      css.fetchPriority = 'low';
      css.onload = function loadPetScript() {
        if (document.hidden) {
          document.addEventListener('visibilitychange', loadPetScript, {once: true});
          return;
        }
        const script = document.createElement('script');
        script.async = true;
        script.fetchPriority = 'low';
        script.src = base + 'assets/pet.js?v=plush-4';
        document.head.append(script);
      };
      document.head.append(css);
    }
    function idle() {
      if ('requestIdleCallback' in window) window.requestIdleCallback(load, {timeout: 1500});
      else setTimeout(load, 250);
    }
    if (document.readyState === 'complete') idle();
    else window.addEventListener('load', idle, {once: true});
  }

  function init() {
    if (document.querySelector('.home') && new URLSearchParams(location.search).has('group')) {
      location.replace('notes/' + location.search + location.hash);
      return;
    }
    const progress = document.querySelector('.reading-progress span');
    if (progress) {
      let pending = 0;
      function updateProgress() {
        const distance = document.documentElement.scrollHeight - innerHeight;
        progress.style.transform = 'scaleX(' + (distance > 0 ? Math.min(1, Math.max(0, scrollY / distance)) : 1) + ')';
        pending = 0;
      }
      function schedule() {
        if (!pending && !document.hidden) pending = requestAnimationFrame(updateProgress);
      }
      function cancelProgress() {
        cancelAnimationFrame(pending);
        pending = 0;
      }
      addEventListener('scroll', schedule, {passive: true});
      addEventListener('resize', schedule);
      addEventListener('load', schedule);
      addEventListener('pagehide', cancelProgress);
      addEventListener('pageshow', schedule);
      document.addEventListener('visibilitychange', () => document.hidden ? cancelProgress() : schedule());
      schedule();
    }
    setupIllustration();
    setupSearch();
    setupFilters(setupMotion());
    setupPrefetch();
    schedulePet();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init, {once: true});
  else init();
})();
