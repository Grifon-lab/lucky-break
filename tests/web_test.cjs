// Optional DOM checks: npm install --no-save jsdom, then node tests/web_test.cjs
const {JSDOM}=require('jsdom');
const assert=require('node:assert/strict');
const fs=require('node:fs');
const path=require('node:path');
const root=path.resolve(__dirname,'..');
const html=fs.readFileSync(path.join(root,'web/index.html'),'utf8');
const script=fs.readFileSync(path.join(root,'web/game.js'),'utf8');
const next=()=>new Promise(resolve=>setImmediate(resolve));
function setup(alias){
 const dom=new JSDOM(html,{runScripts:'outside-only',url:'https://example.invalid/web/index.html',pretendToBeVisual:true});
 const w=dom.window;w.LUCKY_BREAK_CONFIG={telegram:alias};
 w.matchMedia=()=>({matches:true});
 w.HTMLDialogElement.prototype.showModal=function(){this.setAttribute('open','');};
 w.HTMLDialogElement.prototype.close=function(){this.removeAttribute('open');};
 w.crypto.getRandomValues=a=>{a[0]=2;return a;};
 w.HTMLElement.prototype.animate=function(){return{finished:next(),cancel(){}};};
 w.eval(script);return dom;
}
(async()=>{
 const dom=setup('@Miss_CyberGirl'),w=dom.window,d=w.document;
 const spin=d.querySelector('#spin');
 assert.equal(d.querySelector('#channel').href,'https://t.me/Miss_CyberGirl');
 assert.equal(d.querySelectorAll('.symbol').length,3);
 for(let i=1;i<=15;i++){
  spin.click();spin.click();
  assert.equal(d.querySelector('#count').textContent,String(i));
  assert(spin.disabled);
  await next();await next();
  assert(!spin.disabled);
  assert.equal(d.querySelector('#win').open,i===15);
 }
 assert.equal(d.querySelector('#balance').textContent.replace(/\s/g,''),'750');
 assert(d.body.classList.contains('won'));
 d.querySelector('#continue').click();assert(!d.querySelector('#win').open);
 d.querySelector('#help').click();assert(d.querySelector('#rules').open);
 spin.click();assert.equal(d.querySelector('#count').textContent,'15');
 d.querySelector('#close-rules').click();assert(!d.querySelector('#rules').open);
 dom.window.close();
 const unsafe=setup('javascript:alert(1)');
 assert.equal(unsafe.window.document.querySelector('#channel').textContent,'@YOUR_CHANNEL');
 assert(!unsafe.window.document.querySelector('#channel').hasAttribute('href'));
 unsafe.window.close();
 console.log('PASS: real DOM events, exact first win on spin 15 despite always matching RNG, balance, duplicate-click lock, rules, win modal, channel validation');
})().catch(e=>{console.error(e);process.exit(1);});
