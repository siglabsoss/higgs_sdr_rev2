const u = x => x >>> 0;
const neg = x => ~u(x) + 1;
const real = x => u(x) & 0xFFFF;
const imag = x => u((x) >> 16) & 0xFFFF;
const complex = (r,i) => u(i << 16) | r;

const hard_decision = (din_array) => {
    let dout = [];
    while(din_array.length > 0) {
	din = din_array[0];
	din_array.shift();
	dout.push(((imag(din) >> 15) << 1) | (real(din) >> 15));
    }
    return dout;
}
module.exports = hard_decision;
