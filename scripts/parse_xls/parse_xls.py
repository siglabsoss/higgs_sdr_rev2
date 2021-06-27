import xlrd
from string import lower

ddr_fpga_list = ['FPGA00', 'FPGA01', 'FPGA02', 'FPGA03', 'FPGA30', 'FPGA31', 'FPGA32', 'FPGA33']


class lpfrow(object):
    def __init__(self, schematic, port, ball, bank):
        self.schematic = schematic
        self.port = port
        self.ball = ball
        self.bank = bank
        self.commented = True

    def render(self):

        if self.commented:
            precomment = "# "
        else:
            precomment = ""

        return precomment + "LOCATE COMP \"" + str(self.port) + "\" SITE \"" + str(self.ball) + "\"; # " + str(self.schematic)

    def port_prefix(self):
        cut = self.port.index("[")
        return self.port[:cut]

    # returns a unique number for hs ls
    def hs_enum(self):
        if 'hs' in lower(self.port):
            return 1
        if 'ls' in lower(self.port):
            return 2

    # returns a unique number for n,e,s,w
    def dir_enum(self):

        # trim off hs, ls to avoid detection of south below
        trunc = self.port
        if trunc.startswith('HS_'):
            trunc = trunc[3:]
        if trunc.startswith('LS_'):
            trunc = trunc[3:]

        # check long names first so that E doesn't trigger in WEST
        if 'NORTH' in trunc:
            return 3
        if 'EAST' in trunc:
            return 4
        if 'SOUTH' in trunc:
            return 5
        if 'WEST' in trunc:
            return 6

        # check singles if we didn't match fulls above
        if 'N' in trunc:
            return 3
        if 'E' in trunc:
            return 4
        if 'S' in trunc:
            return 5
        if 'W' in trunc:
            return 6

    # def unique_enum(self):
    #     return self.hs_enum() + self.dir_enum()*10


class ddrrow(lpfrow):
    def __init__(self):
        lpfrow.__init__(self, None, None, None, None)
        self.sdrampin = None
        self.iotype = None
        self.slewrate = None
        self.termination = None
        self.vref = None
        self.diffresistor = None

    def render_location(self):

        finalcomment = str(self.sdrampin) + ", " + self.iotype + ", " + self.slewrate + ", " + str(self.termination) + ", " + str(self.diffresistor)

        if self.commented:
            precomment = "# "
        else:
            precomment = ""

        return precomment + "LOCATE COMP \"" + self.port + "\" SITE \"" + self.ball + "\"; # " + finalcomment

class mibrow(ddrrow):
    def __init__(self):
        ddrrow.__init__(self)



def parse_mib_sheet(sheetname, workbook):
    sheet = workbook.sheet_by_name(sheetname)
    maxrows = len(sheet.col(0))

    mibpins = []

    ddr_fpga_enabled = []

    all_configs = []

    for lookrow, lookcol in [(0,4),(0,5),(0,6)]:
        cell = sheet.cell(lookrow, lookcol).value

        this_config = {}
        this_config['name'] = cell
        this_config['col'] = lookcol

        if 'CONFIG' in cell:
            this_config['parsed_name'] = ['CONFIG']
        else:
            this_config['parsed_name'] = cell.split(', ')

        # this_config['balls'] = []
        #
        # for lookdatarow in range(lookrow+1, maxrows):
        #     datacell = sheet.cell(lookdatarow, lookcol).value
        #     if xlrd.empty_cell.value == datacell:
        #         break
        #     this_config['balls'].append(datacell)
        # when all done

        all_configs.append(this_config)


    o2 = {}

    keymap = {}
    keymap[0] = 'schematic'
    keymap[1] = 'port'
    keymap[2] = 'iotype'
    keymap[3] = 'slewrate'

    for cfg in all_configs:
        for fpga in cfg['parsed_name']:

            # return all_configs
            mibpins = []

            for row in range(1, 31):
                # a = sheet.cell(row, 0).value

                mpin = mibrow()

                # Set the the col in which to read the .ball from
                ballcell = sheet.cell(row, cfg['col']).value
                mpin.ball = ballcell

                anyfound = False

                for k in keymap:
                    v = keymap[k]

                    cellval = sheet.cell(row, k).value

                    if xlrd.empty_cell.value == cellval:
                        continue
                    else:
                        anyfound = True

                    mpin.__setattr__(v, cellval)

                if anyfound:
                    if mpin.port is None:
                        mpin.commented = True
                    mibpins.append(mpin)

            o2[fpga] = mibpins

    return all_configs, o2




def parse_ddr_sheet(sheetname, workbook):
    sheet = workbook.sheet_by_name(sheetname)
    maxrows = len(sheet.col(0))

    ddrpins = []

    keymap = {}
    keymap[0] = 'port'
    keymap[1] = 'ball'
    keymap[2] = 'bank'
    keymap[3] = 'sdrampin'
    keymap[4] = 'iotype'
    keymap[5] = 'slewrate'
    keymap[6] = 'termination'
    keymap[7] = 'vref'
    keymap[8] = 'diffresistor'


    ddr_fpga_enabled = []


    for row in range(0, maxrows):
        a = sheet.cell(row, 0).value
        # b = sheet.cell(row, 1).value
        if 'fpga00' in lower(a):
            ddr_fpga_enabled = a.split(', ')

        if 'connection between' in lower(a) or 'fpga00' in lower(a) or 'fpga top level' in lower(a):
            continue

        ddrpin = ddrrow()

        anyfound = False

        for k in keymap:
            v = keymap[k]

            cellval = sheet.cell(row, k).value

            if xlrd.empty_cell.value == cellval:
                continue
            else:
                anyfound = True

            ddrpin.__setattr__(v, cellval)

        if anyfound:
            # push ddrpin object
            ddrpins.append(ddrpin)

    return ddrpins, ddr_fpga_enabled





        #
        #
        #
        #
        # if xlrd.empty_cell.value in [sheet.cell(row, 0).value, sheet.cell(row, 1).value, sheet.cell(row, 2).value, sheet.cell(row, 3).value]:
        #     continue

# open file

def parse_fpga(sheetname, workbook):
    sheet = workbook.sheet_by_name(sheetname)
    maxrows = len(sheet.col(0))

    pins = []

    for row in range(0, maxrows):
        if xlrd.empty_cell.value in [sheet.cell(row, 0).value, sheet.cell(row, 1).value, sheet.cell(row, 2).value, sheet.cell(row, 3).value]:
            continue
        # pull 4 potential columns
        a = sheet.cell(row, 0).value
        b = sheet.cell(row, 1).value
        c = sheet.cell(row, 2).value
        d = sheet.cell(row, 3).value

        rowgood = True

        if 'connection between' in lower(a) or 'schematic' in lower(a):
            rowgood = False

        print row
        if rowgood:
            pins.append(lpfrow(a, b, c, d))
    return pins


def write_fpga(sheetname, pins, all_mib_pins, ddrpins=None):
    # sheet = workbook.sheet_by_name(sheetname)

    if sheetname.startswith("FPGA"):
        fnameout = "outputs/" + "cs" + sheetname.lstrip("FPGA") + "_top.lpf"
    else:
        fnameout = "outputs/" + sheetname + "_output.lpf"

    prev_pin_enum = -1
    group_first_pin = []
    group_names = []
    group_name_suffix = "_GROUP"

    ls_type = "LVCMOS15"
    hs_type = "SSTL15_I"

    skip_ddr = True

    # write file
    with open(fnameout, 'w') as f:

        # write out header
        with open('common_header_snip.txt', 'r') as content_file:
            f.write(content_file.read())
            f.write("######################## CS" + sheetname.lstrip("FPGA") + " configuration only ########################")
        f.write('\n')


        # write out Groups for Normal Pins
        for p in pins:
            # print p.hs_enum(), p.dir_enum()
            # print p.port_prefix()

            if p.port_prefix() not in group_names:
                if skip_ddr and 'ddr' in lower(p.port_prefix()):
                    continue
                group_names.append(p.port_prefix())
                group_first_pin.append(p)

        for i, name in enumerate(group_names):
            firstpin = group_first_pin[i]
            full_group_name = name + group_name_suffix
            f.write('DEFINE PORT GROUP "' + full_group_name + '" "' + name + '[*]";\n')

            if firstpin.hs_enum() == 1:
                speedtype = hs_type
            else:
                speedtype = ls_type

            f.write('IOBUF GROUP "' + full_group_name + '" IO_TYPE=' + speedtype + ';\n')

        # write out normal pins
        for p in pins:
            if skip_ddr and 'ddr' in lower(p.port_prefix()):
                continue
            f.write(p.render() + "\n")

        # mandatory write out MIB
        assert sheetname in all_mib_pins
        mibpins = all_mib_pins[sheetname]
        f.write('\n\n######################## MIB ########################\n')
        with open('mib_groups_snip.txt', 'r') as content_file:
            f.write(content_file.read())
        f.write('\n')
        for p in mibpins:
            f.write(p.render() + "\n")


        # optional write out ddr pins
        if ddrpins is not None:
            f.write('\n\n######################## DDR ########################\n')

            with open('ddr_groups_snip.txt', 'r') as content_file:
                f.write(content_file.read())
            f.write('\n')


            for p in ddrpins:
                f.write(p.render_location() + "\n")





def main():
    workbook = xlrd.open_workbook('rev2_array_fpga_pin_mappings_new.xls')

    ddr, ddr_enabled = parse_ddr_sheet('DDR SDRAM', workbook)

    junk, all_mib_pins = parse_mib_sheet('MIB BUS', workbook)

    fpgas = {}
    fpgas['FPGA00'] = None
    fpgas['FPGA01'] = None
    fpgas['FPGA02'] = None
    fpgas['FPGA03'] = None
    fpgas['FPGA10'] = None
    fpgas['FPGA11'] = None
    fpgas['FPGA12'] = None
    fpgas['FPGA13'] = None
    fpgas['FPGA20'] = None
    fpgas['FPGA21'] = None
    fpgas['FPGA22'] = None
    fpgas['FPGA23'] = None
    fpgas['FPGA30'] = None
    fpgas['FPGA31'] = None
    fpgas['FPGA32'] = None
    fpgas['FPGA33'] = None



    for key in fpgas:
        pins = parse_fpga(key, workbook)


        enabled_for_us = key in ddr_enabled

        if enabled_for_us:
            # write it with ddr
            write_fpga(key, pins, all_mib_pins, ddr)
        else:
            # write it solo
            write_fpga(key, pins, all_mib_pins)

        # save it
        fpgas[key] = pins






if __name__ == '__main__':
    main()