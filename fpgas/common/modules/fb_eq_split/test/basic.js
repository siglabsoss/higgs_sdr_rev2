const dut = require('../build/Release/dut.node');
const {Sim, SimUtils, RisingEdge, RisingEdges, FallingEdge, FallingEdges, Interfaces} = require('signalflip-js');
const { Clock } = SimUtils;
const {Elastic} = Interfaces;
const _ = require('lodash');

const fb_txn = require('../transactions/fb_txn');
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

let randomExclude = (min, max, exclude) => {
    let val = jsc.random(min, max);
    while(exclude.includes(val))
	val = jsc.random(min, max);
    return val;
}

describe('Basic Group', () => {
    let setup = () => {
	// set up the environment
	//dut.init(); // Init dut
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

	target = new Elastic(sim, 0, dut.clk, dut.t_data, dut.t_valid, dut.t_ready, null);
	fb = new Elastic(sim, 1, dut.clk, dut.i_fb_data, dut.i_fb_valid, dut.i_fb_ready, null);
	eq = new Elastic(sim, 1, dut.clk, dut.i_eq_data, dut.i_eq_valid, dut.i_eq_ready, null);
	//target.print = true;
	let din_fb = fb_txn.fb({length: 32, vectorType: randomExclude(0,0xFFFFFFFF,[25,26])});// _.range(10).map(x => x);
	let din_eq = fb_txn.fb({length: 32, type: 2, vectorType: jsc.random(25,26)});
	let din = din_fb.slice();
	din = din.concat(din_eq.slice());
	target.txArray = din.slice();
	//console.log(din_fb);
	//console.log(din_eq);
	//console.log(din.concat(din_eq.slice()));
	//console.log(target.txArray);
	sim.finishTask(() => {
	    //	    assert(_.isEqual(dout, initiator.rxArray));
	    
	    
	    /*dout.map((x,i) => {
		if(x != initiator.rxArray[i])
		    console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
	    });*/

	    try{
		assert.deepEqual(din_fb, fb.rxArray);
		assert.deepEqual(din_eq, eq.rxArray);
	    } catch(e){
		//console.log(e);
		dut.finish();
		throw(e);
	    }
	});
	
    };
    it('CVCR', function () { //Constant valid - Constant ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    dut.init("top_cc");
	    setup();
	    //dut.rstf(0);
	    target.randomizeValid = () =>{ return jsc.random(0,5); };
	    fb.randomizeReady = () =>{ return jsc.random(0,5); };
	    eq.randomizeReady = () =>{ return jsc.random(0,5); };
	    fb.randomize = 0;
	    eq.randomize = 0;
	    target.randomize = 0;
	    
	    target.init();
	    //console.log(target.txArray);
	    fb.init();
	    eq.init();
	    
	    sim.run(2000);
	    return true;
	});
	const props = {size: 2000, tests: 500};
	jsc.check(t, props);
    });
    it('RVCR', function () { //Randomized valid - Constant ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    dut.init("top_rc");
	    setup();
	    //dut.rstf(0);
	    target.randomizeValid = () =>{ return jsc.random(0,5); };
	    fb.randomizeReady = () =>{ return jsc.random(0,5); };
	    eq.randomizeReady = () =>{ return jsc.random(0,5); };
	    fb.randomize = 0;
	    eq.randomize = 0;
	    target.randomize = 1;
	    
	    target.init();
	    //console.log(target.txArray);
	    fb.init();
	    eq.init();
	    
	    sim.run(2000);
	    return true;
	});
	const props = {size: 2000, tests: 500};
	jsc.check(t, props);
    });
    it('CVRR', function () { //Constant valid - Randomized ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    dut.init("top_cr");
	    setup();
	    //dut.rstf(0);
	    target.randomizeValid = () =>{ return jsc.random(0,5); };
	    fb.randomizeReady = () =>{ return jsc.random(0,5); };
	    eq.randomizeReady = () =>{ return jsc.random(0,5); };
	    fb.randomize = 1;
	    eq.randomize = 1;
	    target.randomize = 0;
	    
	    target.init();
	    //console.log(target.txArray);
	    fb.init();
	    eq.init();
	    
	    sim.run(2000);
	    return true;
	});
	const props = {size: 2000, tests: 500};
	jsc.check(t, props);
    });
    it('RVRR', function () { //Randomized valid - Randomized ready
	this.timeout(10000); // test timeout in milliseconds
	let t = jsc.forall(jsc.constant(0), function () {
	    dut.init("top_rr");
	    setup();
	    //dut.rstf(0);
	    target.randomizeValid = () =>{ return jsc.random(0,5); };
	    fb.randomizeReady = () =>{ return jsc.random(0,5); };
	    eq.randomizeReady = () =>{ return jsc.random(0,5); };
	    fb.randomize = 1;
	    eq.randomize = 1;
	    target.randomize = 1;
	    
	    target.init();
	    //console.log(target.txArray);
	    fb.init();
	    eq.init();
	    
	    sim.run(2000);
	    return true;
	});
	const props = {size: 2000, tests: 500};
	jsc.check(t, props);
    });
    
    
});



