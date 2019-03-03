#!/usr/bin/env python3

import sys
import argparse
from statistics import mean, median, variance,stdev


def parse(filename) :
    
    with open(filename, 'r') as f:

        iopses = []
        bpses = []

        for line in f:
            if not "INTERIM" in line:
                continue
            s = line.split(" ")
            iops = float(s[1])
            bps = float(s[5])
            iopses.append(iops)
            bpses.append(bps)

        d = {
            "iops" : median(iopses),
            "iops-max" : max(iopses),
            "iops-min" : min(iopses),
            "bps" : median(bpses),
            "bps-max" : max(bpses),
            "bps-min" : min(bpses),
        }
        return d


def variants(files, key):

    vs = set()
    for filename in files:
        filename = filename.strip(".txt")
        for param in filename.split("_"):
            if key in param and "-" in param:
                v = int(param.split("-")[1])
                vs.add(v)
    return sorted(list(vs))
    
def find_file_by_key(files, keys) :

    for filename in files:
        ok = []
        for key in keys:
            if key in filename:
                ok.append(True)
        if len(ok) == len(keys):
            return filename
    print("no keys {} in files \n{}".format(keys, "\n".join(files)),
          file = sys.stderr)
    sys.exit()
    

def main():

    parser = argparse.ArgumentParser()
    parser.add_argument("-x", "--xkey", choices = ["msg", "batch", "cpu"],
                        help = "key for x-axis")
    parser.add_argument("-v", "--vkey", choices = ["msg", "batch", "cpu"],
                        help = "key for variant")
    parser.add_argument("-y", "--yaxis", choices = ["iops", "bps"],
                        help = "value of y-axis")
    parser.add_argument("files", nargs = "+",
                        help = "output files")
    args = parser.parse_args()

    xkeys = variants(args.files, args.xkey)
    vkeys = variants(args.files, args.vkey)
    
    print("#yaxis is {}".format(args.yaxis))
    print("#xkey {} is {}".format(args.xkey, xkeys))
    print("#vkey {} is {}".format(args.vkey, vkeys))
    print("#")
    print("#{}\t{}".format(args.xkey,
                           "\t".join(map(lambda v:
                                         "{}={}".format(args.vkey, v),
                                         vkeys))))
    for x in xkeys:
        xtic = x
        suffix_cnt = 0
        while (xtic >= 1024):
            xtic = int(xtic / 1024)
            suffix_cnt += 1
        if suffix_cnt == 0:
            xtic = x
        elif suffix_cnt == 1:
            xtic = "{}K".format(xtic)
        elif suffix_cnt == 2:
            xtic = "{}M".format(xtic)

        print("{}\t".format(xtic), end = "")
        for v in vkeys:
            filename = find_file_by_key(args.files,
                                        ["{}-{}".format(args.xkey, x),
                                         "{}-{}".format(args.vkey, v)])
            print("{}\t".format(parse(filename)[args.yaxis]), end = "")
        print("")
    

if __name__ == "__main__":
    main()
