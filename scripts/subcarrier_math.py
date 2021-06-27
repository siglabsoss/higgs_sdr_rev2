
# takes a flat list of subcarriers and packs it into the fewest number of
# mover schedules possible
# returns a list of lists which are 16 (or fewer) subcarriers which to not
# conflict
def bin_subcarriers(subcarriers, quiet=False):
    bins = [[] for x in range(16)]

    for sc in subcarriers:
        mod = sc % 16
        # print "sc", sc, "mod", mod
        bins[mod].append(sc)
        # print bins

    worst = 0
    if not quiet:
        print "//   Bins:"
    for i,b in enumerate(bins):
        if not quiet:
            print "// %02d (%02d): " % (i,len(b)) + str(b)
        worst = max(worst,len(b))

    if not quiet:
        print "// Longest bin:", worst

    output = [[] for x in range(worst)]

    schedule_usage = [0] * worst

    for j in range(worst):
        packed = 0
        for i in range(16):
            if j < len(bins[i]):
                # print "// sel", bins[i][j]
                output[j].append(bins[i][j])
                packed += 1
        output[j] = sorted(output[j])
        schedule_usage[j] = packed

    # print "// output", output

    if not quiet:
        print ""
        print "// Schedule usage"
        for j in range(worst):
            print "// %02d: %.2f %%" % (j, (schedule_usage[j]/0.16))

    return output

# takes in a list of words, and
# reorders them to make data loaded into subcarriers "linear"
def reorder_lines(words, subcarriers, bins):
    # print "got", len(words), "lines"

    dic_lookup = {}
    for i, x in enumerate(sorted(subcarriers)):
        # dic_lookup[i] = x
        dic_lookup[x] = i

    bin_lookup = {}

    bin_index = 0


    for i, (x) in enumerate(bins):
        # print "i", i, "x", x
        for j in x:
            # print "x", j
            bin_lookup[bin_index] = j
            # bin_lookup[j] = bin_index
            bin_index+=1

    output_words = []

    # print dic_lookup
    # print bin_lookup


    for i in range(0,len(words)-1023,1024):
        output_chunk = [0] * 1024
        chunk = words[i:i+1024]
        for j in range(1024):
            if j in subcarriers:
                # print "sc", j,
                a = dic_lookup[j]
                # print "a", a,
                b = bin_lookup[a]
                # print "b", b
                # switch
                output_chunk[j] = chunk[b]
            else:
                output_chunk[j] = chunk[j]
        output_words.extend(output_chunk)
    # print [hex(c) for c in chunk]
    # print [hex(c) for c in output_chunk]
    return output_words