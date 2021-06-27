const dut = require('./build/Release/dut.node');
const clock = require('./clock.js').clock;
const elastic = require('./interfaces/elastic/elastic.js');
const _ = require('lodash');
const RisingEdge = require('./clock.js').RisingEdge;


const clk = new clock(dut, dut.clk, dut.eval);
console.log('dut: ', dut);
console.log(clk);

const init = () => {
    dut.t_data(0);
    dut.t_last(0);
    dut.t_valid(0);
    dut.i_ready(1);
    dut.clk(0);
    dut.rstf(0);
};

init();
var i = 0;
clk.on('posedge', (props) => {
    if(i < 10) {
	dut.rstf(0);
    } else {
	dut.rstf(1);
    }
    //console.log('t_data: ', dut.t_data(), ' i0_valid: ', dut.t_valid(), ' rstf: ', dut.rstf());
    //console.log('t_valid: ', dut.t_valid(), ' i: ', i);
    i++;
});

function* print_ringbus() {
    yield* FallingEdge(dut.clk);
    if(dut.ringbus_out_data_vld() == 1)
	console.log('Ringbus out: ' + dut.ringbus_out_data.toString(16));
}

clk.addTask(print_ringbus());
/*
function* drive_t0() {
    while(true) {
	yield function() {return dut.rstf() == 1};
	dut.t0_data(dut.t0_data() + 2);
	dut.t0_valid(1);
    }
    }*/
let range = n => Array.from(Array(n).keys())

const target = new elastic(clk, 0, dut.clk, dut.t_data, dut.t_valid, dut.t_ready, dut.t_last);
const initiator = new elastic(clk, 1, dut.clk, dut.i_data, dut.i_valid, dut.i_ready, null);
initiator.randomize = 1;
target.randomize = 1;
target.init();
initiator.init();


const u = x => x >>> 0;

const qam8 = (din_array) => {
    let mem = [1,2,3,4,5,6,7,8];
    let rem = 0;
    let dout = [];
    let cnt = 0;
    let done = false;
    din = BigInt(din_array[0]);
    din_array.shift();
    while(!done) {
	dout.push(mem[din & 7n]);
	cnt++;
	//console.log('din: ', din.toString(2), ' mem: ', mem[din & 0x7n], ' rem: ', rem, ' cnt: ', cnt, ' dout: ', dout.length);
	if(rem == 0 && cnt == 10) {
	    cnt = 0;
	    rem++;
	    if(din_array.length > 0) {
		//console.log('din >> 3: ', din >> 3, 'din_array[0] << 2', din_array[0] << 2, 'result: ',  din >> 3 | din_array[0] << 2);
		din = din >> 3n | BigInt(din_array[0]) << 2n;
		din_array.shift();
	    }
	    else {
		dout.push(mem[din >> 3n]);
		done = true;
	    }
	}
	else if (rem == 1 && cnt == 11) {
	    cnt = 0;
	    rem++;
	    if(din_array.length > 0) {
		din = din >> 3n | BigInt(din_array[0]) << 1n;
		din_array.shift();
	    } else {
		//console.log('here:: din: ', din.toString(2), 'last: ', dout);
		dout.push(mem[din >> 3n]);
		//console.log('here:: din: ', din.toString(2), 'last: ', dout);
		done = true;
	    }
	}
	else if (cnt == 11) {
	    cnt = 0;
	    rem = 0;
	    if(din_array.length > 0) {
		din = din >> 3n | BigInt(din_array[0]);
		din_array.shift();
	    } else {
		done = true;
	    }
	} else {

	    din = din >> 3n;
	}
	
	//console.log('din: ', din.toString(2), 'last: ', dout[dout.length-1]);
    }
    return dout;
    
};

//qam8_in = range(10).map(x => u(x ^ 0xFFA3A5FFF));
//qam8_out = qam8(qam8_in.slice());
//console.log('expected #: ', Math.ceil(qam8_in.length*32/3), ' actual #: ', qam8_out.length);
//console.log(qam8_in.slice());
let qam8_in = range(5).map(x => u(x ^ 0xFFFFFFFF));
target.txArray = qam8_in.slice();
/*const txn = function* () {

    
    
    target.txArray = qam8_in.slice();


    yield () => { return target.txArray == 0};

    for(let j = 0; j <30; j++) {
	yield *RisingEdge(dut.clk);
    }
    


    yield () => { return target.txArray == 0};
    for(let j = 0; j < 150; j++) {
	yield *RisingEdge(dut.clk);
    }

    qam8_out = qam8(qam8_in.slice());
    

    console.log('initator array:: ', initiator.rxArray);
    console.log('qam8_out array:: ', qam8_out);
    console.log('are equal: ', _.isEqual(qam8_out, initiator.rxArray));
    console.log('expected #: ', qam8_out.length, ' actual #: ', initiator.rxArray.length);
    qam8_out.map((x,i) => {
	if(x != initiator.rxArray[i])
	    console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
    });
}*/

clk.finishTask(() => {
    let qam8_out = qam8(qam8_in.slice());
    

    console.log('initator array:: ', initiator.rxArray);
    console.log('qam8_out array:: ', qam8_out);
    console.log('are equal: ', _.isEqual(qam8_out, initiator.rxArray));
    console.log('expected #: ', qam8_out.length, ' actual #: ', initiator.rxArray.length);
    qam8_out.map((x,i) => {
	if(x != initiator.rxArray[i])
	    console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
    });
});


//clk.addTask(txn());
clk.run(1000);



module.exports = dut;
