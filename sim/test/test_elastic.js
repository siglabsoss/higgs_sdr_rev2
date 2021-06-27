const dut = require('../build/Release/dut.node');
const {Sim, RisingEdge, FallingEdge, Interfaces} = require('signalflip-js');
const {Elastic} = Interfaces;
const _ = require('lodash');
const jsc = require("jsverify");
const assert = require('assert');
let range = n => Array.from(Array(n).keys());
const u = x => x >>> 0;
const model = (din_array) => {
	    let dout = [];
	    while(din_array.length > 0) {
		dout.push(din_array[0] << 2);
		din_array.shift();
	    }
	    return dout;
	}

describe('Basic Group', function () {

    before(() => {
	
	;
    });

    it('Basic', (done) => {
	this.timeout(60000);

	let t = jsc.forall(jsc.constant(0), function () {
	    dut.init();

	    return new Promise(function(resolve, reject) {

		const clk = new Sim(dut, dut.eval, dut.clk);

		const init = () => {
		    dut.t0_data(0);
		    dut.t0_valid(0);
		    dut.i0_ready(1);
		    dut.clk(0);
		    dut.rstf(0);
		};
		
		init();
		let i = 0;
		clk.on('negedge', (props) => {
		    if(i < 10) {
			dut.rstf(0);
		    } else {
			dut.rstf(1);
		    }
		    i++;
		});

		const target = new Elastic(clk, 0, dut.clk, dut.t0_data, dut.t0_valid, dut.t0_ready, null);
		const initiator = new Elastic(clk, 1, dut.clk, dut.i0_data, dut.i0_valid, dut.i0_ready, null);
		initiator.randomize = 0;
		target.randomize = 0;
		
		target.randomizeValid = ()=>{ return jsc.random(0,1); };
		initiator.randomizeReady = ()=>{ return jsc.random(0,1); };

		target.init();
		initiator.init();
		
		let din = range(5).map(x => u(x));
		target.txArray = din.slice();

		clk.finishTask(() => {
		    let dout = model(din.slice());
		    
		    
		    //console.log('initator array:: ', initiator.rxArray);
		    //console.log('dou array:: ', dout);
		    //console.log('are equal: ', _.isEqual(dout, initiator.rxArray));
		    //onsole.log('expected #: ', dout.length, ' actual #: ', initiator.rxArray.length);
		    assert(_.isEqual(dout, initiator.rxArray));
		    assert.deepEqual(dout, initiator.rxArray);
		    dout.map((x,i) => {
			if(x != initiator.rxArray[i])
			    console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
		    });
		});

		setImmediate(() => {try{

            clk.run(1000);
  
            resolve(true);
  
          }catch(e){reject(e)}});
        }); // promise
      }); // forall
  
      const props = {tests: 100}; // , rngState:"0084da9315c6bfe072"
      jsc.check(t, props).then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    }); // it


}); // describe

		
