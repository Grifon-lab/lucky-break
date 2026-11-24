(() => {
 'use strict';
 const symbols=['codex','claude','deepseek','gemini'];
 const labels=['Codex','Claude','DeepSeek','Gemini'];
 const $=s=>document.querySelector(s);
 const reels=[...document.querySelectorAll('.reel')];
 const spin=$('#spin'),win=$('#win'),rules=$('#rules');
 let balance=1000,spins=0,misses=0,busy=false,audio=null,sound=false;
 const config=window.LUCKY_BREAK_CONFIG||{};
 const alias=/^@[A-Za-z0-9_]{5,32}$/.test(config.telegram||'')?config.telegram:'@YOUR_CHANNEL';
 $('#channel').textContent=alias;
 if(alias!=='@YOUR_CHANNEL')$('#channel').href='https://t.me/'+alias.slice(1);
 const image=i=>{const img=new Image();img.src='../assets/'+symbols[i]+'.png';img.alt=labels[i];img.draggable=false;return img;};
 function setSymbol(reel,i){const strip=reel.querySelector('.strip');strip.replaceChildren();const item=document.createElement('div');item.className='symbol';item.append(image(i));strip.append(item);strip.style.transform='';reel.querySelector('.name').textContent=labels[i];}
 reels.forEach((r,i)=>setSymbol(r,i));
 function random(){const a=new Uint32Array(1);crypto.getRandomValues(a);return a[0]%4;}
 function beep(won){if(!sound)return;try{audio??=new(window.AudioContext||window.webkitAudioContext)();audio.resume();(won?[523.25,659.25,783.99,1046.5]:[220]).forEach((f,i)=>{const o=audio.createOscillator(),g=audio.createGain(),t=audio.currentTime+i*.11;o.type='sine';o.frequency.value=f;g.gain.setValueAtTime(.065,t);g.gain.exponentialRampToValueAtTime(.001,t+.18);o.connect(g);g.connect(audio.destination);o.start(t);o.stop(t+.2);});}catch{}}
 async function rotate(reel,target,column){
  const strip=reel.querySelector('.strip'),height=reel.querySelector('.viewport').clientHeight;
  const last=strip.querySelector('img')?.alt;let first=Math.max(0,labels.indexOf(last));
  strip.replaceChildren();const count=16+column*4;
  for(let j=0;j<=count;j++){const item=document.createElement('div');item.className='symbol';item.append(image(j===count?target:j===0?first:random()));strip.append(item);}
  reel.querySelector('.name').textContent='• • •';
  const reduced=matchMedia('(prefers-reduced-motion: reduce)').matches;
  const anim=strip.animate([{transform:'translateY(0)'},{transform:`translateY(${-count*height}px)`}],{duration:reduced?200:1650+column*380,easing:'cubic-bezier(.12,.6,.18,1)',fill:'forwards'});
  await anim.finished;anim.cancel();setSymbol(reel,target);
 }
 async function run(){
  if(busy||win.open||rules.open||balance<50)return;
  busy=true;spin.disabled=true;document.body.classList.add('spinning');document.body.classList.remove('won');
  spin.textContent='Крутится…';$('#status').textContent='Барабаны крутятся…';
  let outcome=[random(),random(),random()];
  if(spins<14){if(outcome.every(x=>x===outcome[0]))outcome[2]=(outcome[2]+1)%4;}
  else if(spins===14||misses>=7)outcome.fill(outcome[0]);
  balance-=50;spins++;$('#balance').textContent=balance.toLocaleString('ru-RU');$('#count').textContent=spins;beep(false);
  await Promise.all(reels.map((r,i)=>rotate(r,outcome[i],i)));
  const won=outcome.every(x=>x===outcome[0]);
  if(won){balance+=500;misses=0;document.body.classList.add('won');$('#winner').replaceWith(Object.assign(image(outcome[0]),{id:'winner'}));$('#status').textContent='Отличная комбинация. Попробуем ещё?';beep(true);win.showModal();$('#channel').focus();}
  else{misses++;$('#status').textContent='Почти! Следующее вращение может стать твоим.';}
  $('#balance').textContent=balance.toLocaleString('ru-RU');busy=false;spin.disabled=false;spin.innerHTML='Крутить <span>→</span>';document.body.classList.remove('spinning');
 }
 spin.addEventListener('click',run);
 $('#continue').addEventListener('click',()=>{win.close();spin.focus();});
 $('#help').addEventListener('click',()=>{if(!busy)rules.showModal();});
 $('#close-rules').addEventListener('click',()=>rules.close());
 $('#sound').addEventListener('click',()=>{sound=!sound;$('#sound').innerHTML=sound?'♪':'♫<span>×</span>';$('#sound').setAttribute('aria-pressed',String(sound));$('#sound').setAttribute('aria-label',sound?'Выключить звук':'Включить звук');beep(false);});
 document.addEventListener('keydown',e=>{if(e.code==='Space'&&!e.repeat&&!win.open&&!rules.open&&!['BUTTON','A','INPUT','TEXTAREA'].includes(document.activeElement.tagName)){e.preventDefault();run();}});
})();
