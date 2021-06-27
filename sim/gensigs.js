const fs = require('fs');
const _ = require('lodash');
const util = require('util');
const exec = util.promisify(require('child_process').exec);

//const replace = require('./getsignature.js');
const get_signal_names = require('./get_signal_names');

const dut_file = require('./config.json').dut_file;
const dut_name = require('./config.json').dut_name;

async function get_pinlist(options={}) {
    const { topfile, verilatorArgs=[] } = options;

    let cmd;

    if( verilatorArgs.length !== 0 ) {
      cmd = 'verilator -E -P ' + verilatorArgs.join(' ') + ' ' + topfile;
    } else {
      cmd = 'verilator -E -P ' + topfile;
    }

    console.log('running: ' + cmd);

    const { stdout, stderr } = await exec(cmd);
    console.log(get_signal_names(stdout));
    return get_signal_names(stdout);
}
//const getsig = replace(dut_file);
async function gen(options={}) {

    const {verilatorArgs=[],prefix=''} = options;

    let sigs = await get_pinlist({ topfile: dut_file, verilatorArgs:verilatorArgs, prefix:prefix });
    console.log(sigs);
    const val = {'sigs': sigs, //['in_quad', 'fastclk', 'clk', 'reset_l'],
		 'dutName': dut_name };

    const mkdirSync = function (dirPath) {
	try {
	    fs.mkdirSync(dirPath)
	} catch (err) {
	    if (err.code !== 'EEXIST') throw err
	}
    };

    let sigs_header_file = fs.readFileSync(prefix + './node_modules/signalflip-js/templates/signals.h');
    let sigs_header_compiled = _.template(sigs_header_file)(val);

    let sigs_src_file = fs.readFileSync(prefix + './node_modules/signalflip-js/templates/signals.cpp');
    let sigs_src_compiled = _.template(sigs_src_file)(val);

    console.log('----------Creating signals.cpp and signals.h------------');
    fs.writeFile(prefix + 'cppsrc/signals.h',sigs_header_compiled, (err) => {
	if(err) {
	    return console.log(err);
	}
    });

    fs.writeFile(prefix + 'cppsrc/signals.cpp',sigs_src_compiled, (err) => {
	if(err) {
	    return console.log(err);
	}
    });

    console.log('--------------------Done--------------------------------');
    //replace('./src/top.sv');

}

// program
//   .version('1.0.0')
//   .option('-D --drink [drink]', 'Drink', /^(coke|pepsi|izze)$/i)
//   // .option('-B, --baz', 'enable some baz')
//   .action(function (dir, cmd) {
//     console.log('remove ' + dir + " " + cmd)
//   })
//   .parse(process.argv);

//   console.log('options');
//   console.log(program.drink);

// console.log(process.argv);

let dargs = [];

let passthrough = ['-D', '+define+', '-I'];

// for( x of process.argv) {
//   if( x.indexOf('-D') === 0 ) {
//     // console.log(x + ' matched');
//     dargs.push(x);
//   } else if ( x.indexOf('+define+') === 0) {
//     dargs.push(x);
//   }
// }


for( x of process.argv) {
  for( y of passthrough ) {
    if( x.indexOf(y) === 0 ) {
      dargs.push(x);
      break;
    }
  }
}

let prefix = '';

for( x of process.argv) {
  let search = '-PREFIX=';
  if( x.indexOf(search) === 0 ) {
    prefix = x.slice(search.length);
  }
}

// console.log(prefix);


// console.log(dargs);



gen({verilatorArgs:dargs,prefix:prefix});
