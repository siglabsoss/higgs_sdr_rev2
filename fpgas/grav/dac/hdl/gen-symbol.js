#!/usr/bin/env node
'use strict';
const fs = require('fs');

var channel_max = Math.pow(2, 15)-1;

const cplxRange = length =>
    Array(length).fill({re: 0, im: 0});

const perTone = (freq, amp) => {
    amp = amp || 1;
    const step = 2 * Math.PI * freq;
    return (cur, index) => ({
        re: cur.re + amp * Math.cos(index * step),
        im: cur.im + amp * Math.sin(index * step)
    });
};

function val2hexbounded(value, scale) {
    var calc = value*scale;
    if(calc >= channel_max) {
        throw new Error('Value of ' + (calc) + ' is larger than ' + channel_max);
    }
    var combined =  '00000000' + (Math.round(calc) >>> 0).toString(16);
    return combined.slice(-4);
}

// const val2hex = (value, scale) => (
//     '00000000' +
//     (Math.round(scale * value) >>> 0).toString(16)
// )
// .slice(-4);

// const val2hex = (value, scale) => {
//     const tmp = ('00000000' + (Math.round(scale * value) >>> 0).toString(16)).slice(-4);
//     return tmp.slice(-2) +  tmp.slice(-4,-2);
// };

const perCplx2hex = scale =>
    element =>
        val2hexbounded(element.im, scale, 4) + val2hexbounded(element.re, scale, 4);


const res = cplxRange(1024)
    // .map(perTone(-5 * (1/28), 0.5)) // freq1
    // .map(perTone(10 * (1/28), 0.25)) // freq1
    // .map(perTone(12 * (1/28), 0.125)) // freq1
    .map(perTone(128/1024, 0.01)) // freq1
    // .map(perTone(32 / 1024, 0.25)) // freq2, scale1
    .map(perCplx2hex(1 << 14)) // common scale
    .join('\n');

/*console.log(`unsigned int dmem_sin[] = {
${res}
};`);*/

fs.writeFile("tone.mif", res, function(err) {
    if(err) {
	return console.log(err);
    }
    console.log("tone.mif file was generated");
});

/* eslint no-console:0 */
