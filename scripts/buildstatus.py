import argparse
import os
from pathlib import Path
from collections import namedtuple
import subprocess

## This script shows what fpgas have been modified.
# can be run with
#
#    make buildstatus
#
#    USE_COLOR=no make buildstatus
#

# helpers

# a pretty-print for hex strings
def get_rose(data):
    ret = ' '.join("{:02x}".format(ord(c)) for c in data)
    return ret



def timing_grep_slack(path):
    # print("in tail file:",path)
    output = subprocess.check_output("cat " + path + " | tail -n 20 | grep slack | tail -n 1", shell=True)
    dcd = output.decode("utf-8").rstrip('\r\n')
    return dcd

def timing_error_count(path):
    output = subprocess.check_output("cat " + path + " | grep \"items scored\"  -B1", shell=True)
    dcd = output.decode("utf-8").rstrip('\r\n')
    lines = dcd.splitlines()

    output = ""

    for i in range(0,len(lines),3):
        x = lines[i]
        y = lines[i+1]

        # output += y + "\n"

        if(", 0 timing errors detected" not in y and "0 items scored." not in y):
            output += x + "\n" + y + "\n"
            # output += "\n\nasdfjasdlkfj"

    return output.rstrip('\r\n')

def timing_error_worst_path(path):
    output = subprocess.check_output("cat " + path + " | grep \"path exceeds requirements\" -A5 | grep -v \"Logical Details\" | grep . | head -3", shell=True)
    dcd = output.decode("utf-8").rstrip('\r\n')
    return dcd


def grep_build_log_errors(path):
    output = subprocess.check_output('cat ' + path + ' | grep -i @E | grep -i -v @END || true', shell=True)
    dcd = output.decode("utf-8").rstrip('\r\n')
    return dcd


def grep_build_log_error_map(path):
    output = subprocess.check_output('cat ' + path + '| grep -i "ERROR - map" || true', shell=True)
    dcd = output.decode("utf-8").rstrip('\r\n')
    return dcd

def grep_build_log_error_crash(path):
    outputcheck= subprocess.check_output('cat ' + path + '| grep -i "Software Tool error" || true', shell=True)
    found_text = outputcheck.decode("utf-8").rstrip('\r\n')
    
    if found_text is None:
        llen = 0
    else:
        llen = len(found_text)

    if llen != 0:
        output = subprocess.check_output('cat ' + path + '| tail -20 || true', shell=True)
        dcd = output.decode("utf-8").rstrip('\r\n')
        return dcd
    else:
        return ""


def grep_placing(pathin):
    search ="Finished Placer Phase 0.  REAL time"
    print(repr(pathin))
    # path = 'fpgas\\grav\\adc\\build\\build.log'
    path = 'fpgas/cs/cs00/build/build.log'
    print(repr(path))
    output = subprocess.check_output("pwd", shell=True)
    print(output.decode("utf-8"))
    output = subprocess.check_output("cat " + path + " | grep Placer | grep REAL", shell=True)
    # output = subprocess.check_output("cat " + path + " | grep \"" + search + "\"", shell=True)
    # output = subprocess.check_output("echo " + path, shell=True)
    # output = subprocess.check_output("cat Makefile", shell=True)
    return output.decode("utf-8")



# a bunch of classes put together to subclass unique features of all types of findable files
# Each class should have unique features to that file, inheritance is encouraged
class BuildPath(object):
    name = ""

    def name(self, something=""):
        return self.name

    def __init__(self, name):
        self.name = name

    def status(self):
        return None

    def path(self):
        return self.name

    def filefound(self):
        po = Path(self.path())
        if po.is_file():
            return True
        return False
        pass


# FINDABLE FILES
#

# obj0
class BuildLog(BuildPath):
    def status(self):
        global cc
        if self.filefound():
            checks = [None] * 3

            checks[0] = grep_build_log_errors(self.path())
            checks[1] = grep_build_log_error_map(self.path())
            checks[2] = grep_build_log_error_crash(self.path())

            formatpre = "\n"+cc.red
            formatpost = cc.normal

            for status_txt in checks:
                if len(status_txt) != 0:
                    print(formatpre + status_txt + formatpost)


# obj2
class BitFile(BuildPath):
    pass


# obj1
class BuildSt(BuildPath):
    pass


# obj3
class TimingReport(BuildPath):
    def status(self):
        global cc
        if self.filefound():
            print ("")
            print(cc.teal + timing_grep_slack(self.path()) + cc.normal + '\n')  # there was a  + '\x1B[0m' here (and not anywhere else) but it's not needed when timing_grep_slack() does not return newlines
            print(cc.teal + timing_error_count(self.path()) + cc.normal + '\n')
            print(cc.teal + timing_error_worst_path(self.path()) + cc.normal)

# obj4
class MapReport(BuildPath):
    pass


def main():
    # global sugar for colors throughout the script
    global cc

    # Begin Argument Parsing
    option_color = os.environ.get('USE_COLOR')
    if option_color is not None:
        option_color = option_color.lower()
    # End Argument PArsing


    # Begin color on/off feature
    Colors = namedtuple('Colors', 'red green teal normal')
    if option_color is None or (option_color is not None and option_color == 'yes' ):
        cc = Colors(red='\x1B[31m', green='\x1B[32m', teal='\x1B[36m', normal='\x1B[0m')
    else:
        cc = Colors(red='', green='', teal='', normal='')
    # End color feature


    cs_fpgas = "cs00 cs01 cs02 cs03 cs10 cs11 cs12 cs13 cs20 cs21 cs22 cs23 cs30 cs31 cs32 cs33 cscfg".split(" ")
    grav_fpgas = "adc cfg dac eth".split(" ")

    cs_fpga_with_prefix = ['cs' + os.path.sep + x for x in cs_fpgas]

    grav_fpga_with_prefix = ['grav' + os.path.sep + x for x in grav_fpgas]

    folders = cs_fpga_with_prefix + grav_fpga_with_prefix

    # sugar path names
    build = 'build'
    reports = build + os.path.sep + 'reports'
    tmp = build + os.path.sep + 'tmp'

    print("")

    #####################
    #
    #  Loop through each fpga
    #
    for folder in folders:

        obj0 = BuildLog('fpgas' + os.path.sep + folder + os.path.sep + build + os.path.sep + 'build.log')
        obj1 = BuildSt('fpgas' + os.path.sep + folder + os.path.sep + tmp + os.path.sep + '.build_status')
        obj2 = BitFile('fpgas' + os.path.sep + folder + os.path.sep + build + os.path.sep + folder.split(os.path.sep)[1] + '_top.bit')
        obj3 = TimingReport('fpgas' + os.path.sep + folder + os.path.sep + reports + os.path.sep + 'timing_report.txt')
        obj4 = MapReport('fpgas' + os.path.sep + folder + os.path.sep + reports + os.path.sep + 'map_report.txt')


        reportsaslist = [obj0, obj1, obj2, obj3, obj4]

        print("--------------------------")

        #####################
        #
        #  For this FPGA, set 2 flags
        #
        trueifonefound = False
        trueifallfound = True

        for reportnum, thisreport in enumerate(reportsaslist):
            if thisreport.filefound():
                trueifonefound = True
            else:
                trueifallfound = False

        if not trueifonefound:
            print(folder + ': all files ' + cc.red + 'missing' + cc.normal)  # every file was missing, print this and bail
        else:
            #####################
            #
            #  Loop through each report and print
            #
            for i, thisreport in enumerate(reportsaslist):
                if not thisreport.filefound():
                    print(folder + ': missing ' + cc.red + thisreport.path() + cc.normal)

            # special if just for bitfile


            if trueifallfound:
                # every file found
                print(folder + ': all files ' + cc.green + 'exist' + cc.normal)
            else:
                # not every file found..
                #
                # check for bitfile specifically
                if obj2.filefound():
                    print(folder + ': has bitfile ' + cc.green + obj2.path() + cc.normal)

            #################
            #
            #  Individual printing
            #
            #    now that the presence or absence of each files have been conveyed to the user
            #    each file gets a chance to print any greped or important status
            #
            for i, thisreport in enumerate(reportsaslist):
                thisreport.status()

    print ("")


if __name__ == "__main__":
    main()
