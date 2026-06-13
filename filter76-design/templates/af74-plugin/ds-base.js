// ds-base.js — load the Aurora Filter design system into this template.
// In a consuming project, point `base` at the bound _ds/<folder> tree
// relative to this page (e.g. '_ds/aurora-filter', or '../_ds/aurora-filter'
// one level down). Here it points at the design-system root.
(() => {
  const base = '../..';
  for (const p of ['styles.css']) {
    const l = document.createElement('link');
    l.rel = 'stylesheet';
    l.href = base + '/' + p;
    document.head.appendChild(l);
  }
  const s = document.createElement('script');
  s.src = base + '/_ds_bundle.js';
  s.onerror = () => console.error('ds-base.js: failed to load ' + s.src + ' — in a consuming project, point the base line at the bound _ds/<folder> tree relative to this page; in a fresh design system the bundle may not be compiled yet.');
  document.head.appendChild(s);
})();
