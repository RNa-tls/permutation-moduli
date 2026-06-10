// NEON DEPTHS 헤드리스 스모크 테스트: node test.js
// index.html의 <script>를 추출해 vm에서 실행하고 게임 로직 불변식을 검증한다.
"use strict";
const fs = require("fs");
const vm = require("vm");

const html = fs.readFileSync(__dirname + "/index.html", "utf8");
const m = html.match(/<script>([\s\S]*?)<\/script>/);
if (!m) { console.error("script 블록을 찾지 못했다"); process.exit(1); }

const sandbox = { console, Date, Math };
vm.createContext(sandbox);
vm.runInContext(m[1], sandbox, { filename: "index.html<script>" });
const N = sandbox.__neon;

let pass = 0, fail = 0;
function check(cond, label) {
  if (cond) { pass++; return true; }
  fail++;
  console.error("  FAIL: " + label);
  return false;
}
function section(name) { console.log("\n== " + name); }

const DIRS = [[1, 0], [-1, 0], [0, 1], [0, -1]];

function bfsReach(G, sx, sy, tx, ty) {
  const seen = new Set([sx + "," + sy]);
  const q = [[sx, sy]];
  while (q.length) {
    const [x, y] = q.shift();
    if (x === tx && y === ty) return true;
    for (const [dx, dy] of DIRS) {
      const nx = x + dx, ny = y + dy;
      const k = nx + "," + ny;
      if (seen.has(k)) continue;
      if (nx < 0 || ny < 0 || nx >= N.MAPW || ny >= N.MAPH) continue;
      if (G.map[N.idx(nx, ny)] === N.T_WALL) continue;
      seen.add(k);
      q.push([nx, ny]);
    }
  }
  return false;
}

// ------------------------------------------------------------
section("1. 맵 생성 ×300 — 연결성/배치 불변식");
{
  let ok = true;
  for (let seed = 0; seed < 30 && ok; seed++) {
    for (let depth = 1; depth <= 10 && ok; depth++) {
      N.setSeed(seed * 1000 + depth);
      const G = N.newGame(seed * 1000 + depth);
      G.player = N.makePlayer();
      if (!check(N.genFloor(depth), `genFloor(${depth}) seed=${seed}`)) { ok = false; break; }
      const p = G.player;
      ok = check(G.map[N.idx(p.x, p.y)] !== N.T_WALL, `플레이어가 벽 위 (s${seed} d${depth})`) && ok;
      const goal = depth >= N.MAX_DEPTH ? G.items.find(it => it.kind === "heart") : G.stairs;
      ok = check(!!goal, `목표 지점 존재 (s${seed} d${depth})`) && ok;
      ok = check(bfsReach(G, p.x, p.y, goal.x, goal.y), `플레이어→목표 도달 가능 (s${seed} d${depth})`) && ok;
      const seenPos = new Set();
      for (const mo of G.monsters) {
        ok = check(N.isWalkable(mo.x, mo.y), `몬스터 벽 위 ${mo.type} (s${seed} d${depth})`) && ok;
        const k = mo.x + "," + mo.y;
        ok = check(!seenPos.has(k), `몬스터 중첩 (s${seed} d${depth})`) && ok;
        seenPos.add(k);
        ok = check(Math.abs(mo.x - p.x) + Math.abs(mo.y - p.y) > 8 || mo.boss, `몬스터가 시작점에 너무 가까움 (s${seed} d${depth})`) && ok;
      }
      for (const it of G.items)
        ok = check(N.isWalkable(it.x, it.y), `아이템 벽 위 (s${seed} d${depth})`) && ok;
      if (depth >= N.MAX_DEPTH)
        ok = check(G.monsters.some(mo => mo.boss), `10층 보스 존재 (s${seed})`) && ok;
    }
  }
  if (ok) console.log("  300층 전부 통과");
}

// ------------------------------------------------------------
section("2. FOV — 벽 차단/반경");
{
  const G = N.newGame(42);
  G.player = N.makePlayer();
  N.genFloor(1);
  G.map.fill(N.T_FLOOR);
  const px = 25, py = 15;
  G.player.x = px; G.player.y = py;
  for (let dy = -3; dy <= 3; dy++) G.map[N.idx(px + 2, py + dy)] = N.T_WALL; // 세로 벽
  G.seen.fill(0);
  N.computeFOV(px, py, 8);
  check(G.visible[N.idx(px, py)] === 1, "자기 타일 가시");
  check(G.visible[N.idx(px + 1, py)] === 1, "인접 타일 가시");
  check(G.visible[N.idx(px + 2, py)] === 1, "벽 자체는 보임");
  check(G.visible[N.idx(px + 4, py)] === 0, "벽 뒤는 안 보임");
  check(G.visible[N.idx(px - 8, py)] === 1, "반경 8 가장자리 보임");
  check(G.visible[N.idx(px - 10, py)] === 0, "반경 밖은 안 보임");
  check(G.seen[N.idx(px + 1, py)] === 1, "본 타일은 seen에 기록");
}

// ------------------------------------------------------------
section("3. 전투 경계값 — 데미지 범위/사망 1회");
{
  N.startRunHeadless(7);
  const G = N.G, p = G.player;
  p.maxhp = 1e9; p.hp = 1e9;
  let okRange = true;
  for (let i = 0; i < 2000; i++) {
    const before = p.hp;
    N.damagePlayer(5, null, { cause: "테스트" });
    const d = before - p.hp;
    if (d < 1 || d > (5 - N.pDef(p) + 2) * 2) { okRange = false; break; }
  }
  check(okRange, "피해량이 [1, (atk-def+2)*2] 범위");

  const mo = N.makeMonster("rat", 1, p.x + 1, p.y);
  G.monsters.push(mo);
  mo.hp = 1e9; mo.maxhp = 1e9;
  let okAtk = true;
  for (let i = 0; i < 2000; i++) {
    const before = mo.hp;
    N.playerAttack(mo);
    const d = before - mo.hp;
    if (d < 1 || d > (N.pAtk(p) - mo.def + 2) * 2) { okAtk = false; break; }
  }
  check(okAtk, "플레이어 공격 피해 범위");

  p.maxhp = 30; p.hp = 3;
  N.damagePlayer(50, null, { cause: "낙사" });
  check(G.state === "dying" && p.hp === 0, "사망 시 HP=0, state=dying");
  const info1 = G.deathInfo;
  N.damagePlayer(50, null, { cause: "이중사망" });
  check(G.deathInfo === info1, "사망은 1회만 처리");
}

// ------------------------------------------------------------
section("4. 성장 — 경험치/레벨업");
{
  N.startRunHeadless(11);
  const p = N.G.player;
  const a0 = p.baseAtk, h0 = p.maxhp;
  N.gainXP(N.xpForLevel(1));
  check(p.lv === 2 && p.maxhp === h0 + 5 && p.baseAtk === a0 + 1, "LV2: HP+5, ATK+1(짝수)");
  N.gainXP(N.xpForLevel(2) + N.xpForLevel(3) + 5);
  check(p.lv === 4, "누적 XP로 연속 레벨업");
  check(p.xp === 5, "초과 XP 이월");
  check(p.baseDef === 1, "LV3에서 DEF+1");
}

// ------------------------------------------------------------
section("5. 인벤토리 엣지 케이스");
{
  N.startRunHeadless(13);
  const G = N.G, p = G.player;
  // 빈 인벤 안전성
  check(N.useInvItem(0) === false, "빈 인벤 사용 안전");
  N.dropInvItem(0); // no-op이면 통과
  // 가득 찬 인벤
  for (let i = 0; i < N.INV_CAP; i++) p.inv.push(N.makeItem("potion", "heal", 0, 0));
  G.items.push(N.makeItem("potion", "rage", p.x, p.y));
  const itemCount = G.items.length;
  N.pickUp();
  check(p.inv.length === N.INV_CAP && G.items.length === itemCount, "가득 찬 인벤은 줍기 거부");
  // 만피 회복 포션 미소모
  p.hp = p.maxhp;
  check(N.useInvItem(0) === false && p.inv.length === N.INV_CAP, "만피 회복 포션 미소모");
  // 회복은 maxhp 초과 금지
  p.hp = p.maxhp - 5;
  check(N.useInvItem(0) === true && p.hp === p.maxhp, "회복 클램프");
  // 장착 교체
  p.inv.length = 0;
  p.inv.push(N.makeItem("weapon", 0, 0, 0), N.makeItem("weapon", 1, 0, 0));
  const base = N.pAtk(p);
  N.useInvItem(0);
  check(p.weapon && p.weapon.tier === 0 && N.pAtk(p) === base + 2, "무기 장착 시 ATK 반영");
  N.useInvItem(0); // 이제 0번은 장검
  check(p.weapon.tier === 1 && p.inv.some(it => it.kind === "weapon" && it.tier === 0) && p.inv.length === 1, "장착 교체 시 이전 무기 인벤 복귀");
  // 낙뢰: 대상 없으면 미소모
  G.monsters.length = 0;
  p.inv.length = 0;
  p.inv.push(N.makeItem("scroll", "zap", 0, 0));
  check(N.useInvItem(0) === false && p.inv.length === 1, "낙뢰 대상 없으면 미소모");
  // 순간이동: 걷는 타일로
  p.inv.length = 0;
  p.inv.push(N.makeItem("scroll", "tp", 0, 0));
  check(N.useInvItem(0) === true && N.isWalkable(p.x, p.y), "순간이동은 걷는 타일로");
  // 지도
  p.inv.push(N.makeItem("scroll", "map", 0, 0));
  N.useInvItem(0);
  check(G.seen.every(v => v === 1), "지도 스크롤은 전체 공개");
}

// ------------------------------------------------------------
section("6. 봇 시뮬 — 30시드 × 최대 3000턴 무작위 플레이");
{
  let crashes = 0, deaths = 0, wins = 0, maxDepth = 0, invErr = null;
  for (let seed = 100; seed < 130; seed++) {
    try {
      N.startRunHeadless(seed);
      const G = N.G;
      for (let t = 0; t < 3000; t++) {
        if (G.state === "dying" || G.state === "dead") { deaths++; break; }
        if (G.state === "win") { wins++; break; }
        const p = G.player;
        const r = Math.random();
        if (G.stairs && G.stairs.x === p.x && G.stairs.y === p.y && r < 0.6) N.descend();
        else if (N.itemsAt(p.x, p.y).length && r < 0.5) N.pickUp();
        else if (p.inv.length && r < 0.08) { if (N.useInvItem(Math.floor(Math.random() * p.inv.length))) N.endPlayerTurn(); }
        else if (r < 0.97) { const [dx, dy] = DIRS[Math.floor(Math.random() * 4)]; N.tryMove(dx, dy); }
        else N.waitTurn();

        maxDepth = Math.max(maxDepth, G.depth);
        if (t % 25 === 0 && (G.state === "play" || G.state === "inv")) {
          const bad =
            (!N.isWalkable(p.x, p.y) && "플레이어 벽") ||
            (p.hp > p.maxhp && "HP>max") ||
            ([p.hp, p.maxhp, p.xp, p.x, p.y].some(Number.isNaN) && "NaN") ||
            (G.depth < 1 || G.depth > 10 ? "층수 범위" : null) ||
            (G.monsters.some(mo => !N.isWalkable(mo.x, mo.y)) && "몬스터 벽") ||
            (G.monsters.some(mo => mo.hp <= 0) && "죽은 몬스터 잔존") ||
            (new Set(G.monsters.map(mo => mo.x + "," + mo.y)).size !== G.monsters.length && "몬스터 중첩") ||
            (G.monsters.some(mo => mo.x === p.x && mo.y === p.y) && "몬스터가 플레이어 위") || null;
          if (bad) { invErr = `seed=${seed} t=${t}: ${bad}`; break; }
        }
      }
      if (invErr) break;
    } catch (e) {
      crashes++;
      console.error(`  CRASH seed=${seed}: ${e.stack.split("\n").slice(0, 3).join(" | ")}`);
      break;
    }
  }
  check(crashes === 0, "예외 0건");
  check(!invErr, "불변식 위반 없음" + (invErr ? " — " + invErr : ""));
  check(deaths > 0, "봇이 죽기도 함 (전투 동작, deaths=" + deaths + ")");
  check(maxDepth >= 2, "계단 하강 동작 (maxDepth=" + maxDepth + ")");
  console.log(`  deaths=${deaths} wins=${wins} maxDepth=${maxDepth}`);
}

// ------------------------------------------------------------
section("7. 전체 루프 — 사망 후 재시작");
{
  N.startRunHeadless(55);
  N.damagePlayer(9999, null, { cause: "테스트" });
  check(N.G.state === "dying" && N.G.deathInfo && typeof N.G.deathInfo.epitaph === "string", "사망 화면 데이터 준비");
  N.startRunHeadless(56);
  const G = N.G;
  check(G.state === "play" && G.player.hp === 30 && G.depth === 1 && G.kills === 0, "재시작 시 상태 초기화");
}

// ------------------------------------------------------------
section("8. 브라우저 경로 스모크 — canvas 스텁으로 boot/render/입력 전체 실행");
{
  function makeCtx2d() {
    const gradient = { addColorStop() {} };
    return new Proxy({ canvas: null }, {
      get(t, p) {
        if (p in t) return t[p];
        t[p] = (...args) => (String(p).startsWith("create") ? gradient : undefined);
        return t[p];
      },
      set(t, p, v) { t[p] = v; return true; },
    });
  }
  function makeCanvas() {
    return { width: 960, height: 640, style: {}, getContext: () => makeCtx2d(), addEventListener() {} };
  }
  const listeners = {};
  let rafCb = null;
  const winStub = {
    addEventListener(ev, fn) { listeners[ev] = fn; },
    innerWidth: 1280, innerHeight: 800,
  };
  const sb = {
    console, Date, Math,
    window: winStub,
    document: {
      getElementById: () => makeCanvas(),
      createElement: () => makeCanvas(),
    },
    performance: { now: () => Date.now() },
    requestAnimationFrame: (cb) => { rafCb = cb; },
  };
  sb.window.document = sb.document;
  vm.createContext(sb);
  let err = null;
  try {
    vm.runInContext(m[1], sb, { filename: "index.html<script>(browser)" });
    listeners.load();                       // boot()
    const NG = sb.__neon;
    const frames = (n, step) => { for (let i = 0; i < n; i++) { const cb = rafCb; rafCb = null; cb && cb(i * (step || 16.7)); } };
    const key = (code) => listeners.keydown({ code, preventDefault() {} });
    const keyup = (code) => listeners.keyup({ code });

    check(NG.G.state === "title", "부트 후 타이틀");
    frames(10);
    key("Space");                           // 시작
    check(NG.G.state === "play", "타이틀→플레이");
    frames(10);
    for (const c of ["KeyW", "KeyA", "KeyS", "KeyD", "ArrowUp", "ArrowLeft", "ArrowDown", "ArrowRight"]) {
      for (let i = 0; i < 6; i++) { key(c); frames(3); }
      keyup(c);
    }
    key("KeyG"); key("Space"); key("KeyM"); key("KeyM"); key("KeyE");
    frames(5);
    key("KeyI");
    check(NG.G.state === "inv", "인벤토리 열림");
    NG.G.player.inv.push(NG.makeItem("potion", "heal", 0, 0), NG.makeItem("weapon", 2, 0, 0));
    frames(3);
    key("ArrowDown"); key("ArrowUp"); key("Enter"); frames(3);
    key("KeyI"); key("Escape");
    check(NG.G.state === "play", "인벤토리 닫힘");
    // 사망 → 연출 → 사망 화면 → 재시작
    NG.damagePlayer(9999, null, { cause: "스모크" });
    check(NG.G.state === "dying", "사망 연출 시작");
    let t0 = 1000;
    for (let i = 0; i < 130; i++) { const cb = rafCb; rafCb = null; cb && cb(t0 += 16.7); }
    check(NG.G.state === "dead", "연출 후 사망 화면");
    frames(5, 16.7);
    key("KeyR");
    check(NG.G.state === "play" && NG.G.player.hp === 30, "R로 재시작");
    frames(30);
    // 승리 화면 경로
    NG.winGame();
    frames(10);
    key("KeyR");
    check(NG.G.state === "play", "승리 후 재시작");
    frames(10);
  } catch (e) {
    err = e;
    console.error("  CRASH(browser path): " + e.stack.split("\n").slice(0, 4).join(" | "));
  }
  check(!err, "브라우저 경로 예외 0건");
}

// ------------------------------------------------------------
console.log(`\n결과: ${pass} 통과, ${fail} 실패`);
process.exit(fail ? 1 : 0);
