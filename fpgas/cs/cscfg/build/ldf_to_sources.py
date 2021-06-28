import xml.etree.ElementTree
import sys

fd = open('output.tcl', 'w+')
fd.write('set macro_search_path "./tmp;"\n')
fd.write('\n')

for arg in sys.argv[1:]:
    print arg
    root = xml.etree.ElementTree.parse(arg).getroot()

sources = root.findall("./Implementation/Source")

for source in sources:
    if source.get('type') in ['Verilog', 'sbx']:
        fd.write('prj_src add ' + source.get('name') + '\n')

