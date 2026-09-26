const dut = require('../build/Release/dut.node');
const {Sim, RisingEdge, FallingEdge, Interfaces} = require('signalflip-js');
const {Elastic} = Interfaces;
const _ = require('lodash');
const jsc = require("jsverify");
const assert = require('assert');
let range = n => Array.from(Array(n).keys());
const u = x => x >>> 0;
const model = (din_array) => {
  // console.log(din_array);
  let dout = [];
  // console.log('model:');
  // for( let x of din_array ) {
    // console.log(x.toString(16));

    let work = 0;
    for(let j = 0; j < din_array.length; j++) {

        let i = j % 4;

        work |= din_array[j] << (i*8);

        if( i === 3 ) {
          work = work >>> 0;
          dout.push(work);
          // console.log(work.toString(16));
          work = 0;
        }


      //   for(let i = 0; i < 4; i++) {

      //     let mask = 0xff << (i*8);
      //     let val = (x&mask) >>> (i*8);
      //     // console.log(val.toString(16));
      //     dout.push(val);
      // }


    }


  // }
  // while(din_array.length > 0) {
  //   dout.push(din_array[0] << 2);
  //   din_array.shift();
  // }
  return dout;
};

describe('Basic Group', function () {

  before(() => {
  });

  // do not convert this to an arrow function
  it('Basic', function(done) {
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
            dut.reset(1);
        };
        
        init();
        let i = 0;
        clk.on('negedge', (props) => {
            if(i < 10) {
              dut.reset(1);
            } else {
              dut.reset(0);
            }
            i++;
        });

        const target = new Elastic(clk, 0, dut.clk, dut.t0_data, dut.t0_valid, dut.t0_ready, null);
        const initiator = new Elastic(clk, 1, dut.clk, dut.i0_data, dut.i0_valid, dut.i0_ready, null);
        initiator.randomize = 1;
        target.randomize = 1;

        const validChance = jsc.random(1,30);
        const readyChance = jsc.random(1,30);

        console.log( "valid " + validChance +  ", ready " + readyChance );
        
        target.randomizeValid = ()=>{ 
          let valid = jsc.random(0,validChance);
          // console.log("valid: " + valid);
          return valid;
        };
        initiator.randomizeReady = ()=>{ return jsc.random(0,readyChance); };

        target.init();
        initiator.init();

        let asu32 = range(50).map(x => u(0xffeedd00 + x));

        let din = [];
        for(let x of asu32) {
          for(let i = 0; i < 4; i++) {
            let mask = 0xff << (i*8);
            let val = (x&mask) >>> (i*8);
            // console.log(val.toString(16));
            din.push(val);
          }
        }
        
        // let din = range(50).map(x => u(0xffeedd00 + x));
        target.txArray = din.slice();

        clk.finishTask(() => {
            let dout = model(din.slice());
            //        assert(_.isEqual(dout, initiator.rxArray));

            // let match = _.isEqual(dout, initiator.rxArray);

            // if( !match ) {
            //   console.log('did not match');
            // }


            // console.log("output got " + initiator.rxArray);
            // console.log("expected   " + dout);

            assert.deepEqual(dout, initiator.rxArray);
            // try{
            // } catch(e){
            // }
            // dut.finish();
                 
            // dout.map((x,i) => {
            // if(x != initiator.rxArray[i])
            //   console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
            // });
        });

        setImmediate(() => {try{

          clk.run(9000);
          resolve(true);
        }catch(e){reject(e)}});
      }); // promise
    }); // forall
  
    const props = {tests: 2000
      // , rngState:"00561edfca8432b970"
      // , rngState:"80412ad30fe658c7b9"
    };
    jsc.check(t, props).then( r => r === true ? done() : done(new Error(JSON.stringify(r))));
  }); // it

}); // describe
