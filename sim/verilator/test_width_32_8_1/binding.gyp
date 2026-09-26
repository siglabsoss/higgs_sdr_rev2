{
  'targets': [
    {
      'target_name': 'dut',
#      'sources': [
#      		 'cppsrc/main.cpp',
#		 'cppsrc/signals.cpp',
#		 'obj_dir/Vtop_elastic.cpp',
#		 'obj_dir/Vtop_elastic__Syms.cpp',
#		 'obj_dir/Vtop_elastic__Trace.cpp',
#		 'obj_dir/Vtop_elastic__Trace__Slow.cpp'
#		 '/usr/local/share/verilator/include/verilated.cpp'
#     		 ],
      'sources': ["<!@(node -p \"require('./getconfig.js').sources\")"],
#      		 "<!@(ls -1 obj_dir/*.cpp)"],
#      'sources': '<!(["node", ])',
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")",
      "<!@(node -p \"require('./getconfig.js').libraries\")"
#      "/usr/local/share/verilator/include",
#      "./obj_dir"
      ],
      'libraries': [
	  "-lpthread",
	  "-latomic",
     	  "../obj_dir/Vtop_elastic__ALL.a",
	  "../obj_dir/verilator_global_libs.a",
	  "-lm"],
      'ld_flags': ["-pthread", "-latomic", "-lpthread", "-lm", "-lstdc++", "-std=c++11"],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
#      'cflags!': [ '-fno-exceptions', '-fPIC'],
#      'cflags_cc!': [ '-fno-exceptions', '-fPIC'],
       'cflags' : [ '-std=c++11', '-pthread -w' ],
#       'cflags_cc' : [ '-pthread', '-lpthread', '-latomic', '-lm', '-lstdc++' ],
	'CXXFLAGS' : [ "-std=c++11" ],
	'link_settings': {
    			 'libraries': [
      		     	   	      "-lm",
      		     	   	      "-latomic",
				      "-lpthread"
    		     	 	      ],
    			'library_dirs': [
      					"/usr/lib",
    					],
	},
#      'xcode_settings': {
#      			'CXXFLAGS': [
#			"-std=c++14",
#			"-stdlib=libc++"
#			]
#      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}
