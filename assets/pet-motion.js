// A fixed plush drawing, deformed by connected bones. No pose-image morphing.
const TAU = Math.PI * 2;
const clamp = (x, a = 0, b = 1) => Math.max(a, Math.min(b, x));
const smooth = t => { t = clamp(t); return t * t * t * (10 + t * (-15 + t * 6)); };
const rise = (t, a, b) => smooth((t - a) / (b - a));
const hold = (t, a, b, c, d) => rise(t, a, b) - rise(t, c, d);
const mix = (a, b, t) => a + (b - a) * t;
const clips = {rest: [5600, true], walk: [900, true], stepOff: [420], settle: [420],
  turn: [700], inspect: [2700], inspectUp: [2700], pat: [2600], feed: [10000],
  dance: [3600], sleep: [10000], game: [8000], enter: [2600], exit: [2000], drag: [1800, true]};
export const motionTiming = {stride: 4, clips};

// The support paw moves back at exactly the body's travel speed. The lifted
// paw follows a Hermite arc with the same tangent at contact and toe-off.
export function footstep(phase) {
  phase = ((phase % 1) + 1) % 1;
  if (phase < .5) return {x: 20 - 80 * phase, y: 0, planted: true};
  const u = (phase - .5) * 2;
  return {x: -20 - 40 * u + 240 * u * u - 160 * u * u * u,
    y: -18 * Math.sin(Math.PI * u) ** 2, planted: false};
}

export function poseAt(name, milliseconds, direction = 1, duration) {
  const timing = clips[name] || clips.rest;
  let t = Math.max(0, milliseconds) / 1000;
  if (timing[1]) t %= (duration || timing[0]) / 1000;
  const p = {yaw: 0, head: 0, headY: 0, body: 0, lift: 0, eye: 1, smile: 1,
    lx: 163, ly: 375, rx: 403, ry: 375, flx: 0, fly: 0, frx: 0, fry: 0,
    table: 0, burger: 0, bx: 359, by: 331, bite: 0,
    cup: 0, cx: 408, cy: 333, controller: 0, pillow: 0,
    parcel: 0, shadow: 1, footLift: 0, sleepy: 0};
  const breathe = Math.sin(t * TAU / (timing[1] ? (duration || timing[0]) / 1000 : 3.6)) ** 2;
  p.headY = -.8 * breathe;
  const blink = hold(t % 5.6, 3.6, 3.69, 3.75, 3.87);
  p.eye = 1 - .96 * blink;
  if (name === 'rest') { p.head = .012 * Math.sin(t * TAU / 5.6); }
  if (['walk', 'stepOff', 'settle', 'turn', 'inspect', 'inspectUp'].includes(name)) {
    p.yaw = .72 * direction;
    if (name === 'walk') {
      const phase = t / ((duration || timing[0]) / 1000) + .25;
      const l = footstep(phase), r = footstep(phase + .5);
      p.flx = l.x * direction; p.fly = l.y;
      p.frx = r.x * direction; p.fry = r.y;
      p.lift = -1.6 * Math.sin(phase * TAU) ** 2;
      p.head = .009 * Math.sin(phase * TAU);
      p.lx += 7 * Math.cos(phase * TAU); p.rx -= 7 * Math.cos(phase * TAU);
      p.ly -= 3 * Math.max(0,-Math.cos(phase * TAU));
    } else if (name === 'stepOff' || name === 'settle') {
      const s = name === 'stepOff' ? rise(t, 0, .42) : 1 - rise(t, 0, .42);
      p.yaw *= s; p.fry = -18 * s; p.lift = -1.6 * s;
    } else if (name === 'turn') {
      p.yaw *= 1 - 2 * rise(t, 0, .7);
      p.fly = -3 * Math.sin(Math.PI * clamp(t / .35)) ** 2;
      p.fry = -3 * Math.sin(Math.PI * clamp((t - .35) / .35)) ** 2;
      p.fry -= 18 * rise(t,.45,.7); p.lift = -1.6 * rise(t,.45,.7);
    } else {
      const s = hold(t, .25, 1.1, 1.8, 2.7);
      p.head = (name === 'inspectUp' ? -.085 : .10) * direction * s;
      p.headY += (name === 'inspectUp' ? -4 : 4) * s;
    }
  }
  if (name === 'pat') {
    const s = hold(t, 0, .75, 1.95, 2.6);
    p.lx = mix(p.lx, 214, s) - 16 * Math.sin(Math.PI*s); p.ly = mix(p.ly, 262, s);
    p.rx = mix(p.rx, 368, s) + 16 * Math.sin(Math.PI*s); p.ry = mix(p.ry, 262, s);
    p.head = .045 * Math.sin(t * 4) * s; p.eye = 1 - .93 * s;
  }
  if (name === 'feed') {
    p.table = hold(t, 0, .5, 9.4, 10); p.burger = p.table; p.cup = p.table;
    const take = rise(t, .8, 1.8), toMouth = hold(t, 1.9, 2.6, 2.9, 3.6), put = rise(t, 3.6, 4.3);
    p.bx = mix(359, 287, take * (1 - put));
    p.by = mix(331, 310 - 68 * toMouth, take * (1 - put));
    p.headY += 13 * toMouth;
    const reach = hold(t, .5, .8, 4.3, 4.6);
    p.rx = mix(p.rx, p.bx + 39, reach); p.ry = mix(p.ry, p.by + 6, reach);
    const support = hold(t, 1.1, 1.8, 3.6, 4.3);
    p.lx = mix(p.lx, p.bx - 36, support); p.ly = mix(p.ly, p.by + 6, support);
    const drink = hold(t, 4.8, 5.6, 6.5, 7.4);
    p.cx = mix(408, 285, drink); p.cy = mix(333, 267, drink);
    const reachCup = hold(t, 4.5, 4.8, 7.4, 7.8);
    p.rx = mix(p.rx, p.cx + 22, reachCup); p.ry = mix(p.ry, p.cy + 14, reachCup);
    p.lx = mix(p.lx, p.cx - 26, drink); p.ly = mix(p.ly, p.cy + 15, drink);
    const chew = hold(t, 2.6, 2.85, 3.9, 4.2);
    p.smile = 1 - .35 * chew * (.5 + .5 * Math.sin((t - 2.6) * TAU * 2.4));
    p.bite = rise(t, 2.6, 2.85);
    p.eye = 1 - .88 * hold(t, 5.5, 5.9, 8.4, 9.1);
    p.head = -.018 * drink + .025 * hold(t, 8.5, 8.68, 8.82, 9.1);
    p.headY += -2 * hold(t, 8.5, 8.68, 8.82, 9.1);
  }
  if (name === 'dance') {
    const s = hold(t, 0, .5, 2.8, 3.6);
    p.body = .035 * Math.sin(t * TAU / 1.25) * s;
    p.lx = mix(p.lx, 152, s); p.ly -= 98 * s;
    p.rx = mix(p.rx, 425, s); p.ry -= 98 * s;
    p.head = -.025 * Math.sin(t * TAU / 1.25) * s; p.eye = 1 - .8 * s;
    const jump = hold(t, 1.15, 1.48, 1.48, 1.95);
    p.lift = -17 * jump; p.shadow = 1 - .5 * jump;
  }
  if (name === 'game' || name === 'sleep') {
    const sleep = name === 'sleep', s = hold(t, 0, .9, sleep ? 8.9 : 7.05, sleep ? 10 : 8);
    p[sleep ? 'pillow' : 'controller'] = s;
    const open = sleep ? 35 * Math.sin(Math.PI*s) : 0;
    p.lx = mix(p.lx, 236, s) - open; p.ly = mix(p.ly, sleep ? 309 : 338, s);
    p.rx = mix(p.rx, 339, s) + open; p.ry = mix(p.ry, sleep ? 309 : 338, s);
    p.eye = sleep ? 1 - .97 * s : p.eye;
    p.sleepy = sleep ? s : 0;
    p.head = (sleep ? .07 : -.016) * s;
    p.headY += (sleep ? 3 : 1.5 * Math.sin(t * TAU / 2.4)) * s;
  }
  if (name === 'drag') { p.lx -= 8; p.rx += 8; p.ly -= 20; p.ry -= 20; p.fly = -8; p.fry = -8; p.head = .018 * Math.sin(t * TAU / 1.8); p.shadow = 0; }
  if (name === 'enter' || name === 'exit') {
    const enter = name === 'enter';
    p.parcel = enter ? 1 - rise(t, 1.1, 1.8) : rise(t, .8, 1.25);
    p.lift = enter ? 485 * (1 - rise(t, .25, 1.4)) : 485 * rise(t, 1.05, 1.95);
    const wave = enter ? hold(t, 1.1, 1.75, 1.95, 2.6) : hold(t, 0, .6, .75, 1.35);
    p.lx = mix(p.lx, 153 + 6 * Math.sin(t * 8), wave); p.ly -= 138 * wave;
  }
  p.footLift = name === 'walk' ? 0 : p.lift;
  return p;
}

// The elbow swings down and forward in depth, like bringing a cup to the face.
// A planar IK solution folded the elbow upward whenever the paw crossed the
// shoulder. The lower length includes the distance from wrist to paw centre.
export function armBones(left, x, y) {
  const outward = left ? -1 : 1, lift = rise(292-y,10,65);
  const s = [(left ? 208 : 366) + outward*2*lift, 292-3*lift, -35];
  const w = [x,y,32+23*rise(292-y,-70,20)], l1 = 57, l2 = 74;
  const delta = w.map((v,i)=>v-s[i]), distance = Math.hypot(...delta);
  const d = clamp(distance,24,l1+l2-.05), n = delta.map(v=>v/distance);
  const guide = [outward*.35,1,.12], dot = guide.reduce((sum,v,i)=>sum+v*n[i],0);
  const pole = guide.map((v,i)=>v-dot*n[i]), norm = Math.hypot(...pole);
  const along = (l1*l1+d*d-l2*l2)/(2*d), radius = Math.sqrt(Math.max(0,l1*l1-along*along));
  const e = s.map((v,i)=>v+n[i]*along+pole[i]/norm*radius);
  return {s,e,w:s.map((v,i)=>v+n[i]*d)};
}
const rotate = (x, y, cx, cy, a) => [cx + (x-cx)*Math.cos(a) - (y-cy)*Math.sin(a), cy + (x-cx)*Math.sin(a) + (y-cy)*Math.cos(a)];
export function createPlushRenderer(canvas, image, data) {
  const gl = canvas.getContext('webgl', {alpha: true, antialias: true, premultipliedAlpha: true, preserveDrawingBuffer: true});
  if (!gl) throw new Error('WebGL unavailable');
  function shader(type, source) {
    const s = gl.createShader(type); gl.shaderSource(s, source); gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(s));
    return s;
  }
  const program = gl.createProgram();
  gl.attachShader(program, shader(gl.VERTEX_SHADER, `attribute vec2 position; attribute vec2 uv; attribute float opacity;
    varying vec2 tex; varying float alpha; void main(){tex=uv;alpha=opacity;gl_Position=vec4(position,0.,1.);}`));
  gl.attachShader(program, shader(gl.FRAGMENT_SHADER, `precision mediump float; varying vec2 tex; varying float alpha;
    uniform sampler2D art;void main(){vec4 c=texture2D(art,tex);gl_FragColor=vec4(c.rgb*c.a,c.a)*alpha;}`));
  gl.linkProgram(program); if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error('Cannot link plush shader');
  gl.useProgram(program);
  const texture = gl.createTexture(); gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR); gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
  gl.enable(gl.BLEND); gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
  const buffer = gl.createBuffer(); gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
  for (const [name, size, offset] of [['position', 2, 0], ['uv', 2, 8], ['opacity', 1, 16]]) {
    const a = gl.getAttribLocation(program, name); gl.enableVertexAttribArray(a); gl.vertexAttribPointer(a, size, gl.FLOAT, false, 20, offset);
  }
  const meshes = {};
  for (const [name, part] of Object.entries(data.parts)) {
    const [ax, ay, w, h] = part.rect, [ox, oy] = part.origin;
    const cols = name.endsWith('Arm') ? 10 : name === 'head' ? 16 : 4;
    const rows = name.endsWith('Arm') ? 16 : name === 'head' ? 12 : 4;
    const vertices = [], levels = Array.from({length:rows+1},(_,i)=>i*h/rows);
    if(name.endsWith('Arm')) levels.push(335-oy,386-oy);
    levels.sort((a,b)=>a-b);
    for (let j = 0; j < levels.length-1; j++) for (let i = 0; i < cols; i++) {
      for (const [dx, dy] of [[0,0],[1,0],[0,1],[1,0],[1,1],[0,1]]) {
        const x = (i+dx)*w/cols, y = levels[j+dy];
        vertices.push([ox+x, oy+y, (ax+x)/data.size[0], (ay+y)/data.size[1]]);
      }
    }
    meshes[name] = vertices;
    if(name.endsWith('Arm')) {
      const upper=[],forearm=[],elbow=[],cx=name==='leftArm'?208:366;
      for(let i=0;i<vertices.length;i+=6) {
        if(vertices[i][1]<335)upper.push(...vertices.slice(i,i+6));
        else forearm.push(...vertices.slice(i,i+6));
      }
      // A round elbow volume covers the shared ends of both sleeve segments.
      // Unlike a bent strip, its inner edge cannot invert or expose a cut edge.
      for(let i=0;i<32;i++)for(const a of [null,i*TAU/32,(i+1)*TAU/32]) {
        const x=cx+(a===null?0:27.8*Math.cos(a)),y=335+(a===null?0:27.8*Math.sin(a));
        elbow.push([x,y,(ax+x-ox)/data.size[0],(ay+y-oy)/data.size[1]]);
      }
      meshes[name]=upper;
      meshes[name.replace('Arm','Forearm')]=forearm;
      meshes[name.replace('Arm','Elbow')]=elbow;
    }
  }
  const vertices = new Float32Array(40000);
  let length = 0, current, lastPose, lastTime, state = {name:'rest', start:0, direction:1}, blend, request = 0, stopped = false;
  function project(x, y, p, head = false) {
    if (head) {
      [x,y] = rotate(x,y,288,248,p.head);
      // Gentle three-quarter turn, using one stable face and one stable bow.
      const face = Math.exp(-(((y-176)/89)**2)) * Math.exp(-(((x-288)/139)**4));
      x = 288 + (x-288)*(1-.10*Math.abs(p.yaw)) + p.yaw*(20+19*face);
      y += p.headY;
    } else x = 288 + (x-288)*(1-.09*Math.abs(p.yaw)) + 13*p.yaw;
    [x,y] = rotate(x,y,288,475,p.body);
    return [x, y+p.lift];
  }
  function add(name, transform, opacity = 1) {
    if (opacity <= .001) return;
    for (const [x,y,u,v] of meshes[name]) {
      const point = transform(x,y);
      vertices[length++] = ((point[0]-288)*.1+24)/24-1;
      vertices[length++] = 1-((point[1]*.1+4)/28);
      vertices[length++] = u; vertices[length++] = v; vertices[length++] = opacity;
    }
  }
  function render(p) {
    const resolution = Math.min(3, devicePixelRatio || 1);
    const width = Math.round((canvas.clientWidth || 96)*resolution), height = Math.round(width*7/6);
    if (canvas.width !== width || canvas.height !== height) { canvas.width=width; canvas.height=height; }
    gl.viewport(0,0,width,height); gl.clear(gl.COLOR_BUFFER_BIT); length=0;
    add('shadow',(x,y)=>[x,y],p.shadow);
    add('table',(x,y)=>[x,y+35*(1-p.table)],p.table);
    for (const [name,dx,dy] of [['leftFoot',p.flx,p.fly],['rightFoot',p.frx,p.fry]]) {
      add(name,(x,y)=>{const q=project(x,y,p);return [q[0]+dx,q[1]+dy+p.footLift-p.lift];});
    }
    function arm(name,left,forearm=false,elbow=false) {
      const centre=left?208:366, target=armBones(left,left?p.lx:p.rx,left?p.ly:p.ry);
      const a=forearm?target.e:target.s,b=forearm?target.w:target.e;
      const start=forearm?335:278,end=forearm?386:335;
      const angle=Math.atan2(b[1]-a[1],b[0]-a[0])-Math.PI/2;
      add(name,(x,y)=>{
        const t=elbow?0:clamp((y-start)/(end-start));
        const across=x-centre,cap=y-(start+t*(end-start));
        return project(mix(a[0],b[0],t)+across*Math.cos(angle)-cap*Math.sin(angle),
          mix(a[1],b[1],t)+across*Math.sin(angle)+cap*Math.cos(angle),p);
      });
    }
    arm('leftArm',true);arm('rightArm',false);
    add('coat',(x,y)=>project(x,y,p));
    add('head',(x,y)=>project(x,y,p,true));
    for (const [name,x,y] of [['leftEye',245,157],['rightEye',329,156]]) {
      add(name,(a,b)=>project(a,y+(b-y)*p.eye,p,true),rise(p.eye,.08,.38));
      add('happyEye',(a,b)=>project(x+(a-245),y+(b-157)*(1-2*p.sleepy),p,true),1-rise(p.eye,.12,.42));
    }
    add('mouth',(x,y)=>project(x,197+(y-197)*p.smile,p,true));
    arm('leftElbow',true,true,true);arm('rightElbow',false,true,true);
    arm('leftForearm',true,true);arm('rightForearm',false,true);
    const prop=(name,x,y,cx,cy,scale,opacity)=>add(name,(a,b)=>project(x+(a-cx)*scale,y+(b-cy)*scale,p),opacity);
    prop('burger',p.bx,p.by,279,284,.90,p.burger*(1-p.bite));
    prop('burgerBitten',p.bx,p.by,279,284,.90,p.burger*p.bite);
    prop('cup',p.cx,p.cy,240,290,.84,p.cup);
    prop('controller',287,328,249,316,.85,p.controller);
    prop('pillow',287,300,256,291,.73,p.pillow);
    add('parcel',(x,y)=>[x,y],p.parcel);
    gl.bufferData(gl.ARRAY_BUFFER,vertices.subarray(0,length),gl.DYNAMIC_DRAW);
    gl.drawArrays(gl.TRIANGLES,0,length/5);
  }
  function sample(time) {
    const p=poseAt(state.name,time-state.start,state.direction,state.duration);
    if (blend) {
      const u=clamp((time-blend.start)/380), w=smooth(u);
      for (const key of Object.keys(p)) {
        const momentum=clamp(blend.velocity[key] || 0,-.15,.15)*(time-blend.start)*(1-u)**2;
        p[key]=mix(blend.from[key]+momentum,p[key],w);
      }
      for(const key of ['eye','smile','table','burger','bite','cup','controller','pillow','parcel','shadow','sleepy'])p[key]=clamp(p[key]);
      if (u>=1) blend=null;
    }
    return p;
  }
  function tick(time) {
    request=0;
    current=sample(time); render(current);
    lastPose=current; lastTime=time;
    if (!stopped) request=requestAnimationFrame(tick);
  }
  return {
    set(name,{direction=1,duration,still=false}={}) {
      const now=performance.now(), from=state.start ? sample(now) : null;
      const matchedStep = name === 'walk' && ['stepOff','turn'].includes(state.name);
      const velocity={};
      if (from&&lastPose&&now>lastTime) for(const key of Object.keys(from)) velocity[key]=(from[key]-lastPose[key])/(now-lastTime);
      state={name,start:now,direction,duration};
      blend=from&&!still&&!matchedStep?{from,velocity,start:now}:null;
      stopped=still;
      if(request)cancelAnimationFrame(request);
      if(still) {current=poseAt('rest',0);render(current);} else request=requestAnimationFrame(tick);
    },
    render,
    pose:()=>current,
    stop(){stopped=true;cancelAnimationFrame(request);request=0;}
  };
}
