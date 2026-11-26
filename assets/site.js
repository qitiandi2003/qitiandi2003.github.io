(() => {
  'use strict';

  function setupSearch() {
    const dialog = document.getElementById('search-dialog');
    const input = document.getElementById('search-input');
    const results = document.getElementById('search-results');
    const status = document.getElementById('search-status');
    if (!dialog || !input || !results || !status) return;

    const articles = window.QI_ARTICLES || [];
    const base = document.documentElement.dataset.base || './';
    const compact = value => value.normalize('NFKC').toLowerCase().replace(/[^\p{L}\p{N}]/gu, '');
    let opener;

    function score(article, terms) {
      const fields = [article.title, article.topic, article.slug, article.keywords, article.summary]
        .map(value => compact(value || ''));
      let total = 0;
      for (const term of terms) {
        const index = fields.findIndex(field => field.includes(term));
        if (index === -1) return 0;
        total += fields.length - index;
        if (fields[index] === term) total += 2;
      }
      return total;
    }

    function renderResults() {
      const query = input.value.normalize('NFKC').toLowerCase().trim();
      const terms = query.replace(/(\p{Script=Han}+)/gu, ' $1 ').match(/[\p{L}\p{N}]+/gu) || [];
      const matches = terms.length
        ? articles.map(article => ({article, score: score(article, terms)}))
          .filter(match => match.score > 0)
          .sort((a, b) => b.score - a.score)
          .map(match => match.article)
        : articles;
      const fragment = document.createDocumentFragment();
      for (const article of matches) {
        const link = document.createElement('a');
        link.className = 'result-link';
        link.href = base + 'notes/' + encodeURIComponent(article.slug) + '/';
        const topic = document.createElement('span');
        topic.className = 'result-topic';
        topic.textContent = article.topic;
        const title = document.createElement('span');
        title.className = 'result-title';
        title.textContent = article.title;
        link.append(topic, title);
        if (results.matches('ul, ol')) {
          const item = document.createElement('li');
          item.append(link);
          fragment.append(item);
        } else {
          fragment.append(link);
        }
      }
      results.replaceChildren(fragment);
      results.scrollTop = 0;
      status.textContent = matches.length ? '共 ' + matches.length + ' 篇' : '没有找到相关文章，可以试试主题名或缩写。';
    }

    function openSearch(trigger) {
      if (dialog.open) return;
      opener = trigger;
      if (opener && opener !== document.activeElement) opener.focus();
      input.value = '';
      renderResults();
      dialog.showModal();
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
      if (opener && opener.isConnected) opener.focus();
    });
    dialog.addEventListener('click', event => {
      if (event.target !== dialog) return;
      const rect = dialog.getBoundingClientRect();
      if (event.clientX < rect.left || event.clientX > rect.right ||
          event.clientY < rect.top || event.clientY > rect.bottom) dialog.close();
    });
    input.addEventListener('input', renderResults);
    input.addEventListener('keydown', event => {
      if (event.key !== 'Enter' || event.isComposing || event.keyCode === 229) return;
      event.preventDefault();
      results.querySelector('a')?.click();
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

  function setupFilters() {
    const buttons = Array.from(document.querySelectorAll('[data-filter]'));
    const cards = document.querySelectorAll('[data-group]');
    const count = document.getElementById('filter-count');
    if (!buttons.length) return;

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

  function init() {
    if (document.querySelector('.home') && new URLSearchParams(location.search).has('group')) {
      location.replace('notes/' + location.search + location.hash);
      return;
    }
    const progress = document.querySelector('.reading-progress span');
    if (progress) {
      const toc = document.querySelector('.toc details');
      if (matchMedia('(max-width: 690px)').matches) toc.open = false;
      let pending = false;
      function updateProgress() {
        const distance = document.documentElement.scrollHeight - innerHeight;
        progress.style.transform = 'scaleX(' + (distance > 0 ? Math.min(1, Math.max(0, scrollY / distance)) : 1) + ')';
        pending = false;
      }
      function schedule() {
        if (!pending) { pending = true; requestAnimationFrame(updateProgress); }
      }
      addEventListener('scroll', schedule, {passive: true});
      addEventListener('resize', schedule);
      addEventListener('load', schedule);
      updateProgress();
    }
    setupSearch();
    setupFilters();
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', init, {once: true});
  else init();
})();
