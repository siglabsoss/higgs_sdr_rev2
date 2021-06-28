const u = x => x >>> 0;
const neg = x => ~x + 1;
const mapper_model = (din_array, constellation) => {
    let qam8_mem = [1,2,3,4,5,6,7,8];
    let qam16_mem = [0x30003000,
		 0x10003000,
		 0x30001000,
		 0x10001000,
		 u(neg(0x3000) << 16)|0x3000,
		 u(neg(0x1000) << 16)|0x3000,
		 u(neg(0x3000) << 16)|0x1000,
		 u(neg(0x1000) << 16)|0x1000,
		 (0x3000 << 16)|u(neg(0x3000) & 0xFFFF),
		 (0x1000 << 16)|u(neg(0x3000) & 0xFFFF),
		 (0x3000 << 16)|u(neg(0x1000) & 0xFFFF),
		 (0x1000 << 16)|u(neg(0x1000) & 0xFFFF),
		 u(neg(0x3000) << 16) | u(neg(0x3000) & 0xFFFF),
		 u(neg(0x1000) << 16) | u(neg(0x3000) & 0xFFFF),
		 u(neg(0x3000) << 16) | u(neg(0x1000) & 0xFFFF),
		 u(neg(0x3000) << 16) | u(neg(0x1000) & 0xFFFF)
		];
    let rem = 0;
    let dout = [];
    let cnt = 0;
    let done = false;
    din = BigInt(din_array[0]);
    din_array.shift();
    while(!done) {
	switch(constellation) {
	case 1:
	    dout.push(qam8_mem[din & 7n]);
	    cnt++;
	    if(rem == 0 && cnt == 10) {
		cnt = 0;
		rem++;
		if(din_array.length > 0) {
		    din = din >> 3n | BigInt(din_array[0]) << 2n;
		    din_array.shift();
		}
		else {
		    dout.push(qam8_mem[din >> 3n]);
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
		    dout.push(qam8_mem[din >> 3n]);
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
	    break;
	case 2:
	    //console.log('i: ', din & 15n, ' val: ', u(qam16_mem[din & 15n]).toString(16), u(qam16_mem[6]).toString(16), u(u(neg(0x3000) << 16)|0x1000).toString(16) );
	    dout.push(u(qam16_mem[din & 15n]));
	    cnt++;
	    if(cnt == 8) {
		cnt = 0;
		if(din_array.length > 0) {
		    din = din >> 4n | BigInt(din_array[0]);
		    din_array.shift();
		} else {
		    done = true;
		}
	    } else {
		din = din >> 4n;
	    }
	    break;
	    
	}
    }
    
    return dout;
    
};

module.exports = mapper_model;
