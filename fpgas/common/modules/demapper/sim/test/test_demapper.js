const dut = require('../build/Release/dut.node');
const {Sim, RisingEdge, FallingEdge, Interfaces} = require('signalflip-js');
const {Elastic} = Interfaces;
const _ = require('lodash');
const jsc = require("jsverify");
const assert = require('assert');

const mapper_model = require('../src/mapper_model');
const demapper_model = require('../src/demapper_model');
const hard_decision = require('../src/hard_decision');

const u = x => x >>> 0;
const neg = x => ~u(x) + 1;
const real = x => u(x) & 0xFFFF;
const imag = x => u((x) >> 16) & 0xFFFF;
const complex = (r,i) => u(i << 16) | r;
let range = n => Array.from(Array(n).keys());
const printHexArray = x => x.map(x => {
    console.log(u(x).toString(16));
});

describe('Basic Group', function () {

    before(()=>{

    })

    // random with delayed pass/fail
    it('constant', function(done) {
	this.timeout(Infinity);

	let t = jsc.forall(jsc.constant(0), function () {
            // console.log('about to require native');
            dut.init();

            return new Promise(function(resolve, reject) {

		const clk = new Sim(dut, dut.eval, dut.clk);

		
		const init = () => {
		    dut.t_data(0);
		    dut.t_valid(0);
		    dut.i_ready(0);
		    dut.clk(0);
		    dut.rstf(0);
		    dut.constellation(2);
		    dut.two_over_sigma_sq(0x1000);
		    
		};

		init();
		
		var i = 0;
		clk.on('negedge', (props) => {
		    if(i < 10) {
			dut.rstf(0);
		    } else {
			dut.rstf(1);
		    }
		    i++;
		});

		const target = new Elastic(clk, 0, dut.clk, dut.t_data, dut.t_valid, dut.t_ready, dut.t_last);
		const initiator = new Elastic(clk, 1, dut.clk, dut.i_data, dut.i_valid, dut.i_ready, dut.i_last);

		target.randomizeValid = ()=>{ return jsc.random(0,3); };
		initiator.randomizeReady = ()=>{ return jsc.random(0,1); };
		initiator.randomize = 1;
		target.randomize = 1;
		
		target.init();
		initiator.init();

		let din = range(10).map(x => jsc.random(0,0xFFFFFFFF));
		target.txArray = mapper_model(din.slice(),2);
		//console.log('-----------START-----------');
		printHexArray(din.slice());
		printHexArray(mapper_model(din.slice(),2));
		//console.log('------MAPPER MODEL---------');
		//console.log(mapper_model(din.slice(),2));
		//printHexArray(mapper_model(din.slice(),2));
		//console.log(demapper_model(mapper_model(din.slice(),2)));

		clk.finishTask(() => {

		    let dout = hard_decision(demapper_model(mapper_model(din.slice(),2)));
		    let dout_demap = demapper_model(mapper_model(din.slice(),2));
		    let actual_hd = hard_decision(initiator.rxArray.slice());
		    console.log('initator array:: ', hard_decision(initiator.rxArray.slice()));
		    console.log('dou array:: ', dout);
		    console.log('are equal: ', _.isEqual(dout, hard_decision(initiator.rxArray.slice())));
		    console.log('expected #: ', dout.length, ' actual #: ', initiator.rxArray.length);
		    dout.map((x,i) => {
			if(x != initiator.rxArray[i])
			    console.log('x: ', x, ' initiator[',i,']: ', initiator.rxArray[i].toString(16), u(dout_demap[i]).toString(16), dout[i], actual_hd[i], dout[i] == actual_hd[i]);
		    });
		    //assert(_.isEqual(dout, initiator.rxArray));
		    assert.deepEqual(dout, hard_decision(initiator.rxArray.slice()));

		});


		setImmediate(() => {try{

		    clk.run(1000);
		    
		    resolve(true);
		    
		}catch(e){reject(e)}});
            }); // promise
	}); // forall
	
	const props = {tests: 1000}; // , rngState:"0084da9315c6bfe072"
	jsc.check(t, props).then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
    }); // it


}); // describe
