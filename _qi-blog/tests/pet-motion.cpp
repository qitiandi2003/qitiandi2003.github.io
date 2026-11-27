// g++ -std=c++17 -O2 _qi-blog/tests/pet-motion.cpp -lcrypto -o /tmp/qi-pet-test
// /tmp/qi-pet-test http://127.0.0.1:8982/
#include "browser.hpp"

int main(int argc,char**argv) {
try {
    const std::string site=argc>1?argv[1]:"http://127.0.0.1:8982/";
    Browser b; b.viewport(1440,900);
    if(site.compare(0,8,"https://")!=0) {
    b.cmd("Page.addScriptToEvaluateOnNewDocument",{{"source",R"JS(
      const upload=WebGLRenderingContext.prototype.bufferData;
      WebGLRenderingContext.prototype.bufferData=function(target,data,...rest){
        if(data instanceof Float32Array)window.__poseMesh=data;
        return upload.call(this,target,data,...rest);
      };
    )JS"}});
    b.cmd("Page.navigate",{{"url",site+"_qi-blog/tests/pet-motion.html"}});
    b.until("document.body?.dataset.ready==='true'");
    const auto logic=b.js(R"JS((()=>{
      const {poseAt,footstep,armBones,motionTiming}=review;
      let boneError=0,reachError=0,loopJump=0,plantDrift=0,samples=0,worstReach;
      let flexMin=180,flexMax=0,elbowStep=0,raisedClearance=Infinity,inwardElbow=0,worstElbow;
      let projectedLength=Infinity,rotationStep=0;
      for(const [name,[duration,loop]] of Object.entries(motionTiming.clips)){
        const previous=[null,null],angles=[null,null];
        for(let t=0;t<=duration;t+=1000/120){
          const p=poseAt(name,t);
          for(const [left,x,y] of [[true,p.lx,p.ly],[false,p.rx,p.ry]]){
            const a=armBones(left,x,y);
            const upper=a.e.map((v,i)=>v-a.s[i]),lower=a.w.map((v,i)=>v-a.e[i]);
            const l1=Math.hypot(...upper),l2=Math.hypot(...lower);
            boneError=Math.max(boneError,Math.abs(l1-57),Math.abs(l2-74));
            const flex=Math.acos(Math.max(-1,Math.min(1,upper.reduce((n,v,i)=>n+v*lower[i],0)/(l1*l2))))*180/Math.PI;
            flexMin=Math.min(flexMin,flex);flexMax=Math.max(flexMax,flex);
            const index=left?0:1;
            const angle=Math.atan2(lower[1],lower[0]);
            projectedLength=Math.min(projectedLength,Math.hypot(lower[0],lower[1]));
            if(angles[index]!==null){
              const turn=angle-angles[index];
              rotationStep=Math.max(rotationStep,Math.abs(Math.atan2(Math.sin(turn),Math.cos(turn)))*180/Math.PI);
            }
            angles[index]=angle;
            if(previous[index]){
              const step=Math.hypot(...a.e.map((v,i)=>v-previous[index][i]));
              if(step>elbowStep){elbowStep=step;worstElbow={name,t,left,from:previous[index],to:a.e};}
            }
            previous[index]=a.e;
            // In a face-level gesture the elbow must stay below the hand and
            // outside it, rather than flipping above or across the chest.
            if((name==='pat'&&t>=750&&t<=1950)||(name==='feed'&&t>=2600&&t<=2900)){
              raisedClearance=Math.min(raisedClearance,a.e[1]-a.w[1]);
              inwardElbow=Math.max(inwardElbow,(a.e[0]-a.w[0])*(left?1:-1));
            }
            const reach=Math.hypot(x-a.w[0],y-a.w[1]);
            if(reach>reachError){reachError=reach;worstReach={name,t,left,x,y};}
          }
          if(!Object.values(p).every(Number.isFinite))throw Error('Nonfinite '+name);
          ++samples;
        }
        if(loop){const a=poseAt(name,0),z=poseAt(name,duration-.001);for(const k of Object.keys(a))loopJump=Math.max(loopJump,Math.abs(a[k]-z[k]));}
      }
      for(let t=0;t<.5;t+=.001)plantDrift=Math.max(plantDrift,Math.abs(80*t+footstep(t).x-20));
      const bite=poseAt('feed',2700),sip=poseAt('feed',6000);
      return {samples,boneError,reachError,worstReach,loopJump,plantDrift,flexMin,flexMax,elbowStep,worstElbow,raisedClearance,inwardElbow,projectedLength,rotationStep,
        mouthInBurger:Math.abs(bite.bx-288)<4&&bite.by-35.1<197+bite.headY&&bite.by>197+bite.headY,
        strawGap:Math.hypot(sip.cx+3.36-288,sip.cy-70.56-197),
        burgerBack:poseAt('feed',4500).bx===359,
        cupBack:poseAt('feed',7900).cx===408,
        neutralEnd:poseAt('feed',10000).lx===163&&poseAt('feed',10000).rx===403};
    })())JS");
    std::cout<<"Motion geometry "<<logic.dump()<<std::endl;
    check(logic["boneError"].get<double>()<1e-6,"bone length changed");
    check(logic["reachError"].get<double>()<1,"hand target beyond reach");
    check(logic["flexMin"].get<double>()>15&&logic["flexMax"].get<double>()<140,"unnatural elbow flexion");
    check(logic["elbowStep"].get<double>()<3,"elbow jumps or flips");
    check(logic["projectedLength"].get<double>()>10&&logic["rotationStep"].get<double>()<10,"forearm projection snaps during a reach");
    check(logic["raisedClearance"].get<double>()>25&&logic["inwardElbow"].get<double>()<1,"raised elbow folds inward");
    check(logic["loopJump"].get<double>()<.01,"loop seam");
    check(logic["plantDrift"].get<double>()<1e-7,"support paw slips");
    check(logic["mouthInBurger"]==true&&logic["strawGap"].get<double>()<2,"food misses mouth");
    check(logic["burgerBack"]==true&&logic["cupBack"]==true&&logic["neutralEnd"]==true,"feeding order");

    const auto sleeves=b.js(R"JS((async()=>{
      const {parts,size}=await(await fetch('../../assets/pet-plush-parts.json')).json();
      const regions=['leftArm','rightArm'].map(name=>{
        const [x,y,w,h]=parts[name].rect;return [x/size[0],y/size[1],(x+w)/size[0],(y+h)/size[1]];
      });
      const area=(a,b,c,d,e,f)=>(c-a)*(f-b)-(d-b)*(e-a);
      let frames=0,triangles=0,inverted=0,worst;
      for(const [name,[duration]] of Object.entries(review.motionTiming.clips))for(let t=0;t<=duration;t+=1000/30){
        review.renderer.render(review.poseAt(name,t));const v=window.__poseMesh;
        for(let i=0;i<v.length;i+=15){
          if(!regions.some(r=>v[i+2]>=r[0]&&v[i+2]<=r[2]&&v[i+3]>=r[1]&&v[i+3]<=r[3]))continue;
          const screen=area(v[i],v[i+1],v[i+5],v[i+6],v[i+10],v[i+11]);
          const texture=area(v[i+2],v[i+3],v[i+7],v[i+8],v[i+12],v[i+13]);
          // Screen Y is reversed. A matching sign means the cloth folded
          // through itself, even if the GPU still renders that triangle.
          if(screen*texture>1e-15){inverted++;worst={name,t,triangle:i/15};}
          triangles++;
        }
        frames++;
      }
      return {frames,triangles,inverted,worst};
    })())JS");
    std::cout<<"Sleeve surface "<<sleeves.dump()<<std::endl;
    check(sleeves["triangles"].get<int>()>100000&&sleeves["inverted"]==0,"sleeve surface folds inside out");
    }

    b.cmd("Page.addScriptToEvaluateOnNewDocument",{{"source",R"JS(
      window.__petFrames=[];window.__petCosts=[];window.__petMesh=null;
      const buffer=WebGLRenderingContext.prototype.bufferData;
      WebGLRenderingContext.prototype.bufferData=function(target,data,...rest){
        if(data instanceof Float32Array&&data.length>1000){window.__petMesh=new Float32Array(data);window.__petFrames.push(performance.now());}
        return buffer.call(this,target,data,...rest);
      };
      const raf=window.requestAnimationFrame;
      window.requestAnimationFrame=callback=>raf.call(window,time=>{
        const start=performance.now();callback(time);
        if(callback.name==='tick')window.__petCosts.push(performance.now()-start);
      });
      Math.random=()=>.1;
    )JS"}});
    b.cmd("Page.navigate",{{"url",site}});
    b.until("document.querySelector('.web-pet[data-art=rig]')?.dataset.idle==='walk'");
    const auto runtime=b.js(R"JS((async()=>{
      const data=await(await fetch((document.documentElement.dataset.base||'./')+'assets/pet-plush-parts.json?v=1')).json();
      const root=document.querySelector('.web-pet'),canvas=root.querySelector('canvas');
      const gaps=[],traces=[[],[]],unique=new Set();let previous,frames=0,previousHash,changedFrames=0;
      const findFoot=(name,mesh)=>{
        const r=data.parts[name].rect,u=(r[0]+r[2]/2)/data.size[0],v=(r[1]+r[3])/data.size[1];
        for(let i=0;i<mesh.length;i+=5)if(Math.abs(mesh[i+2]-u)<.00001&&Math.abs(mesh[i+3]-v)<.00001)return [mesh[i],mesh[i+1]];
        throw Error('Foot vertex missing');
      };
      await new Promise(resolve=>{
        function sample(t){
          if(previous!==undefined)gaps.push(t-previous);previous=t;
          const mesh=window.__petMesh,r=canvas.getBoundingClientRect();
          let hash=2166136261;for(let i=0;i<mesh.length;i+=5)hash=Math.imul(hash^Math.round(mesh[i]*1e5)^Math.round(mesh[i+1]*1e5),16777619);unique.add(hash);
          if(hash!==previousHash)changedFrames++;previousHash=hash;
          ['leftFoot','rightFoot'].forEach((name,i)=>{
            const p=findFoot(name,mesh),part=data.parts[name];
            const x=r.left+(p[0]+1)*r.width/2,y=r.top+(1-p[1])*r.height/2;
            const ground=r.top+((part.origin[1]+part.rect[3])*.1+4)/56*r.height;
            traces[i].push({x,y,plant:Math.abs(y-ground)<.03});
          });
          if(++frames<100)requestAnimationFrame(sample);else resolve();
        }requestAnimationFrame(sample);
      });
      let drift=0,segments=0;
      for(const trace of traces){let planted=[];for(const p of [...trace,{plant:false}]){
        if(p.plant)planted.push(p.x);else{if(planted.length>=6){drift=Math.max(drift,Math.max(...planted)-Math.min(...planted));segments++;}planted=[];}
      }}
      gaps.sort((a,b)=>a-b);const costs=window.__petCosts.slice(-100).sort((a,b)=>a-b);
      return {frames,unique:unique.size,changedFrames,p95:gaps[Math.floor(gaps.length*.95)],long:gaps.filter(t=>t>34).length,
        renderP95:costs[Math.floor(costs.length*.95)],footDrift:drift,plantedSegments:segments,
        files:performance.getEntriesByType('resource').filter(e=>/pet-plush.*\.(webp|avif)/.test(e.name)).map(e=>({url:e.name,bytes:e.decodedBodySize}))};
    })())JS");
    std::cout<<"Browser playback "<<runtime.dump()<<std::endl;
    check(runtime["changedFrames"].get<int>()>=95,"consecutive held walking pictures");
    check(runtime["footDrift"].get<double>()<.8&&runtime["plantedSegments"].get<int>()>=2,"planted paw drift");
    check(runtime["renderP95"].get<double>()<12,"renderer too expensive");
    check(runtime["long"].get<int>()<=3,"playback stalls");
    b.shot("continuous-live-walk");

    b.click(".pet-character");b.click("[data-pet-action=feed]");
    b.until("document.querySelector('.web-pet').dataset.action==='feed'");
    const auto interrupted=b.js(R"JS((async()=>{
      const image=new Image();image.src=(document.documentElement.dataset.base||'./')+'assets/pet-plush-parts.webp?v=1';await image.decode();
      const surface=document.createElement('canvas');surface.width=image.width;surface.height=image.height;
      const context=surface.getContext('2d');context.drawImage(image,0,0);const alpha=context.getImageData(0,0,image.width,image.height).data;
      return new Promise(resolve=>{
      let old,peak=0,frames=0,worst,previousTime;
      const ids=['pat','dance','feed','pat'];
      function sample(time){
        const a=window.__petMesh,map=new Map();
        for(let i=0;i<a.length;i+=5){
          const x=Math.min(image.width-1,Math.floor(a[i+2]*image.width)),y=Math.min(image.height-1,Math.floor(a[i+3]*image.height));
          if(alpha[(y*image.width+x)*4+3]>128&&a[i+4]>.5)map.set(a[i+2].toFixed(5)+':'+a[i+3].toFixed(5),[a[i],a[i+1]]);
        }
        if(old)for(const [key,p] of map){const q=old.get(key);if(q){const delta=Math.hypot((p[0]-q[0])*48,(p[1]-q[1])*56);if(delta>peak){peak=delta;worst={uv:key,frame:frames,gap:time-previousTime,from:q,to:p,action:document.querySelector('.web-pet').dataset.action};}}}
        previousTime=time;
        old=map;
        if(frames%12===0&&frames/12<ids.length)document.querySelector('[data-pet-action='+ids[frames/12]+']').click();
        if(++frames<70)requestAnimationFrame(sample);else resolve({peakCssPixels:peak,frames,worst});
      }requestAnimationFrame(sample);
    });})())JS");
    std::cout<<"Interrupted actions "<<interrupted.dump()<<std::endl;
    check(interrupted["peakCssPixels"].get<double>()<4,"action interruption jumps");
    b.click("[data-pet-collapse]");
    b.until("document.querySelector('.web-pet').dataset.collapsed==='true'");
    b.click(".pet-character");
    b.until("document.querySelector('.web-pet').dataset.phase==='enter'");
    b.until("!document.querySelector('.web-pet').dataset.phase");
    b.key("Escape","Escape",27);
    auto point=b.js("(()=>{const r=document.querySelector('.pet-character').getBoundingClientRect();return [r.left+r.width/2,r.top+r.height*.72]})()");
    b.cmd("Input.dispatchMouseEvent",{{"type","mousePressed"},{"x",point[0]},{"y",point[1]},{"button","left"},{"buttons",1},{"clickCount",1}});
    b.cmd("Input.dispatchMouseEvent",{{"type","mouseMoved"},{"x",point[0].get<double>()-130},{"y",point[1].get<double>()-80},{"button","left"},{"buttons",1}});
    b.cmd("Input.dispatchMouseEvent",{{"type","mouseReleased"},{"x",point[0].get<double>()-130},{"y",point[1].get<double>()-80},{"button","left"},{"buttons",0},{"clickCount",1}});
    check(b.js("!!localStorage.getItem('qi-pet-position') && !document.querySelector('.web-pet').dataset.dragging")==true,"drag and release");
    const auto stored=b.js("localStorage.getItem('qi-pet-position')");
    b.click(".search-button");
    b.until("document.querySelector('.web-pet').dataset.paused==='true'");
    const auto paused=b.js("new Promise(resolve=>{const n=__petFrames.length;setTimeout(()=>resolve(__petFrames.length-n),250)})");
    check(paused==0,"rendering continues behind search");b.key("Escape","Escape",27);
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array({{{"name","prefers-reduced-motion"},{"value","reduce"}}})}});
    std::this_thread::sleep_for(100ms);
    check(b.js("new Promise(resolve=>{const n=__petFrames.length;setTimeout(()=>resolve(__petFrames.length-n),250)})")==0,"reduced motion still rendering");
    b.cmd("Emulation.setEmulatedMedia",{{"features",J::array()}});
    b.viewport(390,844);b.cmd("Page.navigate",{{"url",site+"notes/ros2/"}});
    b.until("!!document.querySelector('.web-pet[data-art=rig]')");
    check(b.js("localStorage.getItem('qi-pet-position')")==stored,"position lost across article navigation");
    b.click(".pet-character");b.click("[data-pet-action=feed]");
    check(b.js("(()=>{let a=document.querySelector('.pet-menu').getBoundingClientRect();return a.left>=0&&a.right<=390&&a.top>=0&&a.bottom<=844})()")==true,"mobile menu outside viewport");
    std::this_thread::sleep_for(1250ms);
    b.shot("continuous-mobile");
    b.cmd("Emulation.setDeviceMetricsOverride",{{"width",390},{"height",844},{"deviceScaleFactor",2},{"mobile",true}});
    b.until("document.querySelector('.pb-canvas').width>=document.querySelector('.pb-canvas').clientWidth*2");
    b.js("document.querySelector('.pb-canvas').getContext('webgl').getExtension('WEBGL_lose_context').loseContext()");
    b.until("document.querySelector('.web-pet').dataset.art==='fallback'");
    check(b.js("!document.querySelector('.pb-canvas') && !!document.querySelector('.pet-character')")==true,"lost context fallback");
    std::cout<<"PASS: contact, playback, interruptions, entry/exit, drag memory, search, reduced motion, mobile, DPR 2 and context fallback"<<std::endl;
}catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 1;}
}
