const dut = require('../build/Release/dut.node');
const {Sim, SimUtils, RisingEdge, RisingEdges, FallingEdge, FallingEdges, Interfaces} = require('signalflip-js');
const { Clock } = SimUtils;
const {Elastic} = Interfaces;
const _ = require('lodash');

const fb_txn = require('../../fb_eq_split/transactions/fb_txn');
//const chai = require('chai');
//const expect = chai.expect;
const jsc = require('jsverify');
const assert = require('assert');

const model = (din_array) => {
    let dout = [];
    while(din_array.length > 0) {
	dout.push(din_array[0] << 2);
	din_array.shift();
    }
    return dout;
}

let sim;
let target, initiator;

describe('Basic Group', () => {
    let setup = ()  => {
	// set up the environment
	dut.init(); // Init dut
	sim = new Sim(dut, dut.eval);

	function* reset() {
	    //yield* RisingEdge(dut.clk); // required b/c other the initialization of the this generator will move past rstf = 0 before the simulation starts in 'it' (dut.init)
	    dut.rstf(0);
	    yield* RisingEdges(dut.clk,5);
	    dut.rstf(1);
	}
	sim.addTask(reset());

	let clk = new Clock(dut.clk, 1);
	sim.addClock(clk);

	fb = new Elastic(sim, 0, dut.clk, dut.t_fb_data, dut.t_fb_valid, dut.t_fb_ready, dut.t_fb_last);
	eq = new Elastic(sim, 0, dut.clk, dut.t_eq_data, dut.t_eq_valid, dut.t_eq_ready, dut.t_eq_last);
	initiator = new Elastic(sim, 1, dut.clk, dut.i_data, dut.i_valid, dut.i_ready, dut.i_last);
	//initiator.print = true;
	let din = fb_txn.fb({length: 32});// _.range(10).map(x => x);
	fb.txArray = din.slice();
	eq.txArray = din.slice();
	sim.finishTask(() => {
	    let dout = (din.slice()).concat(din.slice());
	    /*assert(_.isEqual(dout, initiator.rxArray));
	      
	      
	      dout.map((x,i) => {
	      if(x != initiator.rxArray[i])
	      console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
	      });
	    */
	    try{
		assert.deepEqual(dout, initiator.rxArray);
		//assert(_.isEqual(dout, initiator.rxArray));
	    } catch(e){
		//console.log(e);
		dut.finish();
		throw(e);
	    }
	});
	
    };
    it('CVCR', function() { // Constant valid - Constant ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    setup();
	    dut.init("top_cc");
	    //dut.rstf(0);
	    initiator.randomizeValid = ()=>{ return jsc.random(0,5); };
	    fb.randomizeReady = ()=>{ return jsc.random(0,5); };
	    eq.randomizeReady = ()=>{ return jsc.random(0,5); };
	    fb.randomize = 0;
	    eq.randomize = 0;
	    initiator.randomize = 0;
	    //initiator.print = true;
	    //console.log(initiator.txArray);
	    fb.init();
	    eq.init();
	    initiator.init();

	    sim.run(8000);
	//assert(true);
	    return true;
	});
	const props = {size: 2000, tests: 200}; // , rngState:"0084da9315c6bfe072"
	jsc.check(t, props);//.then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    });
    it('RVCR', function() { //Randomized valid - Constant ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    setup();
	    dut.init("top_rc");
	    //dut.rstf(0);
	    initiator.randomizeValid = ()=>{ return jsc.random(0,5); };
	    fb.randomizeReady = ()=>{ return jsc.random(0,5); };
	    eq.randomizeReady = ()=>{ return jsc.random(0,5); };
	    fb.randomize = 1;
	    eq.randomize = 1;
	    initiator.randomize = 0;
	    //initiator.print = true;
	    //console.log(initiator.txArray);
	    fb.init();
	    eq.init();
	    initiator.init();

	    sim.run(8000);
	//assert(true);
	    return true;
	});
	const props = {size: 2000, tests: 200}; // , rngState:"0084da9315c6bfe072"
	jsc.check(t, props);//.then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    });
    it('CVRR', function() { //Constant valid - Randmoized ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    setup();
	    dut.init("top_cr");
	    //dut.rstf(0);
	    initiator.randomizeValid = ()=>{ return jsc.random(0,5); };
	    fb.randomizeReady = ()=>{ return jsc.random(0,5); };
	    eq.randomizeReady = ()=>{ return jsc.random(0,5); };
	    fb.randomize = 0;
	    eq.randomize = 0;
	    initiator.randomize = 1;
	    //initiator.print = true;
	    //console.log(initiator.txArray);
	    fb.init();
	    eq.init();
	    initiator.init();

	    sim.run(8000);
	//assert(true);
	    return true;
	});
	const props = {size: 2000, tests: 200}; // , rngState:"0084da9315c6bfe072"
	jsc.check(t, props);//.then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    });
    it('RVRR', function() { //Randomized valid - Randomized ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    setup();
	    dut.init("top_rr");

	    initiator.randomizeValid = ()=>{ return jsc.random(0,5); };
	    fb.randomizeReady = ()=>{ return jsc.random(0,5); };
	    eq.randomizeReady = ()=>{ return jsc.random(0,5); };
	    fb.randomize = 1;
	    eq.randomize = 1;
	    initiator.randomize = 1;
	    //initiator.print = true;
	    //console.log(initiator.txArray);
	    fb.init();
	    eq.init();
	    initiator.init();

	    sim.run(8000);
	//assert(true);
	    return true;
	});
	const props = {size: 2000, tests: 200}; // , rngState:"0084da9315c6bfe072"
	jsc.check(t, props);//.then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    });
    
    
});


