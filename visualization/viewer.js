/**
 * viewer.js
 *
 * Draws the Tango Tree and plays the steps of a search as an animation.
 *
 * The tree itself runs in WebAssembly: tango.js is the TangoVisualization class compiled with Emscripten (see CMakeLists.txt in this directory). A search
 * returns its frames as JSON. Each frame is the whole forest after one step: the children, color and mark of every key, the root of every separate tree, and
 * a sentence that describes the step. This file only places the nodes and moves them from one frame to the next.
 *
 * Placement: the column of a node is its key, so a node never moves sideways. Its row is its depth inside its tree, plus the row of the node its tree is
 * drawn below (the anchor of the tree), if it has one.
 */

'use strict';

(function () {
  const SVG_NS = 'http://www.w3.org/2000/svg';
  const WIDTH = 1200;      // The width of the drawing, in SVG units. The page scales it to fit.
  const MARGIN = 30;

  const $ = (id) => document.getElementById(id);
  const stage = $('stage');
  const edgesLayer = $('edges');
  const nodesLayer = $('nodes');

  let api = null;          // The functions exported by the WebAssembly module.
  let n = 15;              // The number of keys.
  let geometry = null;     // The sizes used to draw a tree with n keys.
  let rows = 0;            // The number of rows the drawing has room for.
  let trace = null;        // The frames of the last search, as returned by the module.
  let layouts = [];        // The placement of every node, for each frame.
  let index = 0;           // The frame on screen, when nothing is moving.
  let motion = null;       // The move in progress: { from, to, start, duration }.
  let playing = false;
  let holdUntil = 0;       // When playing, the time to start the next move.
  let frameRequest = 0;
  let speed = 1;
  let searched = [];       // The keys searched so far.
  let nodeElements = [];
  let edgeElements = [];
  let phaseElements = [];
  const xs = [];           // The current position of each node, while drawing.
  const ys = [];

  /* Sizes and placement. */

  function makeGeometry(count) {
    const column = (WIDTH - 2 * MARGIN) / count;
    const radius = Math.max(8, Math.min(20, column * 0.46));
    return {
      column,
      radius,
      row: Math.max(44, radius * 3.2),
      top: radius + 12,
      font: radius >= 18 ? 15 : radius >= 13 ? 12.5 : 9.5,
    };
  }

  const columnX = (key) => MARGIN + (key - 0.5) * geometry.column;
  const rowY = (level) => geometry.top + level * geometry.row;

  /**
   * Computes the row and the parent of every key in a frame.
   */
  function layoutFrame(frame) {
    const level = new Array(n + 1).fill(-1);
    const parent = new Array(n + 1).fill(0);

    const place = (root, rootLevel) => {
      const stack = [[root, rootLevel]];
      while (stack.length > 0) {
        const [key, keyLevel] = stack.pop();
        level[key] = keyLevel;
        for (const child of [frame.l[key - 1], frame.r[key - 1]]) {
          if (child !== 0) {
            parent[child] = key;
            stack.push([child, keyLevel + 1]);
          }
        }
      }
    };

    // A tree can only be placed after the tree of its anchor, so repeat until nothing is left.
    const pending = frame.roots.map((root, i) => ({ root, anchor: frame.anchors[i] }));
    for (let round = 0; round <= frame.roots.length && pending.length > 0; round++) {
      for (let i = pending.length - 1; i >= 0; i--) {
        const { root, anchor } = pending[i];
        if (anchor !== 0 && level[anchor] < 0)
          continue;
        place(root, anchor === 0 ? 0 : level[anchor] + 1);
        pending.splice(i, 1);
      }
    }

    return {
      level,
      parent,
      maxLevel: Math.max(0, ...level),
      red: frame.c,
      external: frame.e,
      focus: new Set(frame.focus),
      path: new Set(frame.path),
    };
  }

  /* Drawing. */

  function svg(tag, attributes) {
    const element = document.createElementNS(SVG_NS, tag);
    for (const name in attributes)
      element.setAttribute(name, attributes[name]);
    return element;
  }

  /**
   * Creates one group and one link for each key. They are reused for every frame.
   */
  function buildStage() {
    edgesLayer.textContent = '';
    nodesLayer.textContent = '';
    nodeElements = [null];
    edgeElements = [null];
    const r = geometry.radius;
    const glow = Math.max(4, r * 0.35);     // How far the marks around a node reach. Smaller in the big tree, where the columns are close together.

    for (let key = 1; key <= n; key++) {
      const line = svg('line', { class: 'edge' });
      line.style.display = 'none';
      edgesLayer.appendChild(line);
      edgeElements.push(line);

      const group = svg('g', { class: 'node' });
      group.appendChild(svg('circle', { class: 'halo focus', r: r + glow }));
      group.appendChild(svg('circle', { class: 'halo path', r: r + glow - 1 }));
      group.appendChild(svg('circle', { class: 'ring target', r: r + glow - 0.5 }));
      group.appendChild(svg('circle', { class: 'ring ext', r: r + glow / 2 }));
      group.appendChild(svg('circle', { class: 'hit', r: r + 4 }));
      group.appendChild(svg('circle', { class: 'disc', r }));
      const label = svg('text', { class: 'key', 'font-size': geometry.font });
      label.textContent = key;
      group.appendChild(label);
      group.appendChild(svg('text', { class: 'depth', y: r + 4, 'font-size': Math.max(8, geometry.font - 3) }));
      const title = svg('title', {});
      title.textContent = 'Search for ' + key;
      group.appendChild(title);
      group.addEventListener('click', () => search(key));
      nodesLayer.appendChild(group);
      nodeElements.push(group);
    }
  }

  function setRows(count) {
    rows = count;
    const height = geometry.top + (rows - 1) * geometry.row + geometry.radius + 26;
    stage.setAttribute('viewBox', '0 0 ' + WIDTH + ' ' + height);
  }

  /**
   * Draws the tree somewhere between two frames. At amount 0 it is the frame a, and at 1 it is the frame b.
   *
   * A link that exists in both frames follows its two nodes. A new link grows from the child up to its parent, and a link that was cut shrinks back into the
   * child. A link that changes parent swings from the old parent to the new one.
   */
  function draw(a, b, amount) {
    for (let key = 1; key <= n; key++) {
      xs[key] = columnX(key);
      ys[key] = rowY(a.level[key] + (b.level[key] - a.level[key]) * amount);
      nodeElements[key].setAttribute('transform', 'translate(' + xs[key].toFixed(2) + ',' + ys[key].toFixed(2) + ')');
    }

    for (let key = 1; key <= n; key++) {
      const line = edgeElements[key];
      const before = a.parent[key];
      const after = b.parent[key];
      if ((before === 0 && after === 0) || (after === 0 && amount >= 1)) {
        line.style.display = 'none';
        continue;
      }

      const fromX = before !== 0 ? xs[before] : xs[key];
      const fromY = before !== 0 ? ys[before] : ys[key];
      const toX = after !== 0 ? xs[after] : xs[key];
      const toY = after !== 0 ? ys[after] : ys[key];
      line.setAttribute('x1', xs[key].toFixed(2));
      line.setAttribute('y1', ys[key].toFixed(2));
      line.setAttribute('x2', (fromX + (toX - fromX) * amount).toFixed(2));
      line.setAttribute('y2', (fromY + (toY - fromY) * amount).toFixed(2));
      line.style.display = '';
      line.classList.toggle('weak', (amount < 0.5 && before !== 0 ? a : b).external[key - 1] === '1');
    }
  }

  /**
   * Shows the description of a frame, and the colors and marks of its nodes. The CSS fades them in.
   */
  function describe(frameIndex) {
    const frame = trace.frames[frameIndex];
    const layout = layouts[frameIndex];

    const crumbs = $('crumbs');
    crumbs.textContent = '';
    frame.ctx.forEach((operation, i) => {
      if (i > 0) {
        const separator = document.createElement('span');
        separator.className = 'sep';
        separator.textContent = '›';
        crumbs.appendChild(separator);
      }
      const crumb = document.createElement('span');
      crumb.className = 'crumb ' + operation.split(' ')[0];
      crumb.textContent = operation;
      crumbs.appendChild(crumb);
    });
    $('text').textContent = frame.text;

    for (let key = 1; key <= n; key++) {
      const classes = nodeElements[key].classList;
      classes.toggle('is-red', layout.red[key - 1] === 'r');
      classes.toggle('is-ext', layout.external[key - 1] === '1');
      classes.toggle('is-focus', layout.focus.has(key));
      classes.toggle('is-path', layout.path.has(key));
      classes.toggle('is-target', trace.key === key);
    }
  }

  /* Playing. */

  const ease = (t) => (t < 0.5 ? 4 * t * t * t : 1 - Math.pow(-2 * t + 2, 3) / 2);

  /**
   * How long the move between two frames takes, in milliseconds at normal speed. Steps that move nothing are quick.
   */
  function moveTime(a, b) {
    for (let key = 1; key <= n; key++) {
      if (a.level[key] !== b.level[key] || a.parent[key] !== b.parent[key])
        return 480;
    }
    return a.red !== b.red || a.external !== b.external ? 260 : 110;
  }

  /**
   * How long a frame stays still before the next move. It is short on purpose, so playing looks like a video: there is no time to read every description, but
   * the player can be paused at any step.
   */
  const holdTime = (frameIndex) => Math.min(420, 220 + 2 * trace.frames[frameIndex].text.length);

  function startMotion(to, now) {
    motion = { from: index, to, start: now, duration: moveTime(layouts[index], layouts[to]) / speed };
    stage.style.setProperty('--dur', Math.round(260 / speed) + 'ms');
    describe(to);
    syncControls();
  }

  /**
   * Ends the move in progress at once, leaving its last frame on screen.
   */
  function settle() {
    if (motion === null)
      return;
    index = motion.to;
    motion = null;
    draw(layouts[index], layouts[index], 1);
  }

  function tick(now) {
    frameRequest = 0;
    if (motion !== null) {
      const t = Math.min(1, (now - motion.start) / motion.duration);
      draw(layouts[motion.from], layouts[motion.to], ease(t));
      if (t >= 1) {
        index = motion.to;
        motion = null;
        holdUntil = now + holdTime(index) / speed;
        syncControls();
      }
    } else if (playing) {
      if (index >= layouts.length - 1) {
        playing = false;
        syncControls();
      } else if (now >= holdUntil) {
        startMotion(index + 1, now);
      }
    }
    if (motion !== null || playing)
      requestTick();
  }

  function requestTick() {
    if (frameRequest === 0)
      frameRequest = requestAnimationFrame(tick);
  }

  /**
   * Shows a frame at once, with no move.
   */
  function show(frameIndex) {
    motion = null;
    index = Math.max(0, Math.min(layouts.length - 1, frameIndex));
    describe(index);
    draw(layouts[index], layouts[index], 1);
    syncControls();
  }

  /**
   * Moves to a frame next to the current one, and stops there.
   */
  function stepTo(frameIndex) {
    playing = false;
    settle();
    if (frameIndex < 0 || frameIndex >= layouts.length || frameIndex === index) {
      syncControls();
      return;
    }
    startMotion(frameIndex, performance.now());
    requestTick();
  }

  function play() {
    settle();
    if (index >= layouts.length - 1)
      show(0);
    playing = true;
    holdUntil = performance.now() + (index === 0 ? 350 / speed : 0);
    syncControls();
    requestTick();
  }

  function pause() {
    playing = false;
    syncControls();
  }

  function syncControls() {
    const shown = motion !== null ? motion.to : index;
    const last = layouts.length - 1;
    $('counter').textContent = shown + 1 + ' / ' + layouts.length;
    $('scrub').max = last;
    $('scrub').value = shown;
    $('play').textContent = playing ? 'Pause' : shown >= last && last > 0 ? 'Replay' : 'Play';
    $('play').disabled = last === 0;
    $('first').disabled = $('prev').disabled = shown === 0;
    $('next').disabled = $('last').disabled = shown >= last;
    phaseElements.forEach((element, i) => element.classList.toggle('done', i <= shown));
  }

  /* Loading a tree or a search. */

  /**
   * Builds the colored strip above the slider: one piece per frame, colored by the operation the frame belongs to.
   */
  function buildPhases() {
    const strip = $('phases');
    strip.textContent = '';
    phaseElements = trace.frames.map((frame) => {
      const piece = document.createElement('i');
      if (frame.ctx.some((operation) => operation.startsWith('cut')))
        piece.className = 'cut';
      else if (frame.ctx.some((operation) => operation.startsWith('paste')))
        piece.className = 'paste';
      strip.appendChild(piece);
      return piece;
    });
  }

  function load(json, autoplay) {
    trace = JSON.parse(json);
    layouts = trace.frames.map(layoutFrame);
    setRows(Math.max(rows, ...layouts.map((layout) => layout.maxLevel + 1)));
    nodeElements.forEach((group, key) => {
      if (group !== null)
        group.querySelector('.depth').textContent = trace.depth[key - 1];
    });
    playing = false;
    buildPhases();
    show(0);
    if (autoplay && layouts.length > 1)
      play();
  }

  function search(key) {
    if (api === null || !Number.isInteger(key))
      return;
    playing = false;
    motion = null;
    searched.push(key);
    $('history').textContent = 'Searched: ' + searched.join(', ');
    $('key').value = key;
    load(api.search(key), true);
  }

  function reset(count) {
    n = count;
    api.init(n);
    geometry = makeGeometry(n);
    searched = [];
    $('history').textContent = '';
    $('key').value = '';
    $('key').max = n + 1;
    $('key').min = 0;
    $('key').placeholder = '1–' + n;
    document.querySelectorAll('#sizes button').forEach((button) => button.setAttribute('aria-pressed', String(Number(button.dataset.n) === n)));
    buildStage();
    rows = 0;
    setRows(Math.round(Math.log2(n + 1)) + 2);
    load(api.state(), false);
  }

  /* Controls. */

  function bindControls() {
    document.querySelectorAll('#sizes button').forEach((button) => button.addEventListener('click', () => reset(Number(button.dataset.n))));
    $('reset').addEventListener('click', () => reset(n));
    $('random').addEventListener('click', () => search(1 + Math.floor(Math.random() * n)));
    $('search-form').addEventListener('submit', (event) => {
      event.preventDefault();
      const key = Number($('key').value);
      if ($('key').value !== '' && Number.isInteger(key))
        search(key);
    });

    $('play').addEventListener('click', () => (playing ? pause() : play()));
    $('prev').addEventListener('click', () => stepTo((motion !== null ? motion.to : index) - 1));
    $('next').addEventListener('click', () => stepTo((motion !== null ? motion.to : index) + 1));
    $('first').addEventListener('click', () => { playing = false; show(0); });
    $('last').addEventListener('click', () => { playing = false; show(layouts.length - 1); });
    $('scrub').addEventListener('input', () => { playing = false; show(Number($('scrub').value)); });
    $('speed').addEventListener('change', () => { speed = Number($('speed').value); });
    $('depths').addEventListener('change', () => stage.classList.toggle('show-depth', $('depths').checked));

    // The keys work anywhere in the page, except where they already mean something else: typing a key, moving the slider, choosing a speed.
    document.addEventListener('keydown', (event) => {
      if (event.target.closest('input, select') || event.ctrlKey || event.metaKey || event.altKey)
        return;
      if (event.key === ' ') {
        event.preventDefault();
        $('play').click();
      } else if (event.key === 'ArrowRight') {
        event.preventDefault();
        $('next').click();
      } else if (event.key === 'ArrowLeft') {
        event.preventDefault();
        $('prev').click();
      } else if (event.key === 'Home') {
        event.preventDefault();
        $('first').click();
      } else if (event.key === 'End') {
        event.preventDefault();
        $('last').click();
      }
    });
  }

  /**
   * Reads the options given in the address of the page. For example: index.html?n=31&search=5,20&speed=2
   *  - n: the number of keys (15, 31 or 63).
   *  - search: one or more keys, separated by commas. All are searched, and the last search is played.
   *  - speed: the playing speed.
   *  - autoplay=0: do not start playing.
   */
  function applyAddressOptions() {
    const options = new URLSearchParams(location.search);
    const count = Number(options.get('n'));
    reset([15, 31, 63].includes(count) ? count : 15);

    if (options.has('speed') && Number(options.get('speed')) > 0) {
      speed = Number(options.get('speed'));
      $('speed').value = String(speed);
    }
    const keys = (options.get('search') || '').split(',').filter((text) => text !== '').map(Number).filter(Number.isInteger);
    keys.forEach((key) => search(key));
    if (options.get('autoplay') === '0')
      pause();
  }

  function start() {
    if (typeof createTangoModule !== 'function') {
      $('error').style.display = 'block';
      for (const id of ['caption', 'stage-wrap', 'transport', 'legend'])
        $(id).style.display = 'none';
      return;
    }
    createTangoModule().then((module) => {
      api = {
        init: module.cwrap('tvInit', 'number', ['number']),
        search: module.cwrap('tvSearch', 'string', ['number']),
        state: module.cwrap('tvState', 'string', []),
      };
      bindControls();
      applyAddressOptions();
    });
  }

  // For tests and for the browser console: lets a script read the frames and show any of them.
  window.tangoViewer = {
    frames: () => (trace === null ? [] : trace.frames),
    layouts: () => layouts,
    current: () => (motion !== null ? motion.to : index),
    isPlaying: () => playing || motion !== null,
    show,
    showBetween: (from, to, amount) => { describe(to); draw(layouts[from], layouts[to], ease(amount)); },
    search,
    reset,
  };

  start();
})();
