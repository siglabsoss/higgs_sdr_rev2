const dut = require('./build/Release/dut.node');
const {Sim, RisingEdge, FallingEdge, Interfaces} = require('signalflip-js');
const {Elastic} = Interfaces;
const _ = require('lodash');


const clk = new Sim(dut, dut.eval, dut.clk);
dut.init();



module.exports = dut;
