const dut = require('./build/Release/dut.node');
const {Sim, RisingEdge, FallingEdge, Interfaces} = require('signalflip-js');
const {Elastic} = Interfaces;
const _ = require('lodash');


const clk = new Sim(dut, dut.eval, dut.clk);
dut.init();

const init = () => {
    dut.tx_turnstile_data_in(0);
    dut.tx_turnstile_data_valid(0);
    // dut.t0_data(0);
    // dut.t0_valid(0);
    // dut.i0_ready(1);
    dut.clk(0);
    dut.MIB_MASTER_RESET(1);
};

init();
let i = 0;
clk.on('negedge', (props) => {
    if(i < 10) {
	dut.MIB_MASTER_RESET(1);
    } else {
	dut.MIB_MASTER_RESET(0);
    }
    i++;
});

let range = n => Array.from(Array(n).keys())

const target = new Elastic(clk, 0, dut.clk, dut.tx_turnstile_data_in, dut.tx_turnstile_data_valid, dut.tx_turnstile_data_ready, null);
// const initiator = new Elastic(clk, 1, dut.clk, dut.i0_data, dut.i0_valid, dut.i0_ready, null);
// initiator.randomize = 0;
target.randomize = 0;
target.init();
// initiator.init();


const u = x => x >>> 0;

const fb_packet = (data, enabled_subcarriers, constellation) => {
    let multiplier = (constellation == 2) ? 8:16;
    let type = 2;
    let custom_size = 16 + Math.ceil((data.length*multiplier)/enabled_subcarriers)*1024;;
    let destination0 = 0;
    let destination1 = 0;
    let vtype = 5;
    let timeslot = 0;
    let epoc = 0;
    let input_pkt_size = data.length;
//    let constellation = c;
    let header = [type, custom_size, destination0, destination1, vtype, timeslot, epoc, input_pkt_size, constellation, 9, 10, 11, 12, 13, 14, 15];
    return header.concat(data);
};

let pkt = fb_packet(range(16), 128, 2).slice();

function* send_pkt() {
    yield () => { return i == 30000 };
    target.txArray = pkt;
}

clk.addTask(send_pkt());



// const model = (din_array) => {
//     let dout = [];
//     while(din_array.length > 0) {
// 	dout.push(din_array[0] << 2);
// 	din_array.shift();
//     }
//     return dout;
// };


// let din = range(5).map(x => u(x));
// target.txArray = din.slice();

// clk.finishTask(() => {
//     let dout = model(din.slice());
    

//     console.log('initator array:: ', initiator.rxArray);
//     console.log('dou array:: ', dout);
//     console.log('are equal: ', _.isEqual(dout, initiator.rxArray));
//     console.log('expected #: ', dout.length, ' actual #: ', initiator.rxArray.length);
//     dout.map((x,i) => {
// 	if(x != initiator.rxArray[i])
// 	    console.log('x: ', x, ' i: ', i, 'initiator[i]: ', initiator.rxArray[i]);
//     });
// });

//clk.addTask(txn());
clk.run(100000);



module.exports = dut;
