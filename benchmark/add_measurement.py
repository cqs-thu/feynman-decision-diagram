import re
import argparse


parser = argparse.ArgumentParser()
parser.add_argument("-i", "--input", help="Input file name", type=str, required=True)
parser.add_argument("-o", "--output", help="Output file name", type=str, required=True)
args = parser.parse_args()
input_fn = args.input
output_fn = args.output

input_f = open(input_fn, "r", encoding="utf-8")
output_f = open(output_fn, "w", encoding="utf-8")

for line in input_f:
    output_f.write(line)
    line = line.strip().split(' ')
    if line[0] == "qreg":
        n = re.findall(r".*\[([0-9]+)\].*", line[1])[0]
        output_f.write(f"creg c[{n}];\n")

for i in range(int(n)):
    output_f.write(f"measure q[{i}] -> c[{i}];\n")
