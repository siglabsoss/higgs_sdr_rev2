const u = x => x >>> 0;
const neg = x => ~u(x) + 1;
const real = x => u(x);
const imag = x => (u(x) >> 16);
const complex = (r,i) => u(u(i << 16) | (u(r) & 0xFFFF) );
const abs = x => ((u(x) >> 15) & 0x1) == 1 ? neg(x):x;

const demapper_model = (din_array, constellation) => {
    let dout = [];
    console.log('----DEMAP----');
    while(din_array.length > 0) {
	let din = din_array[0];
	din_array.shift();
	let din_real = real(din);
	let din_imag = imag(din);
	
	dout.push(din);
	console.log('din: ', u(din).toString(16))
	let stage0_real = u(abs(din_real) + neg(0x2000)) & 0xFFFF;
	let stage0_imag = u(abs(din_imag) + neg(0x2000)) & 0xFFFF;
	dout.push(complex(stage0_real, stage0_imag));
	console.log('real: ', stage0_real.toString(16), ' imag: ', stage0_imag.toString(16));
	console.log(u(complex(stage0_real, stage0_imag)).toString(16));
    }
    return dout;
};

module.exports = demapper_model;
