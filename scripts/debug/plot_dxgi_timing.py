import argparse
import matplotlib.pyplot as plt
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument('--logs', required=True)
parser.add_argument('--output', required=True)
parser.add_argument('--min', type=int, required=True)
parser.add_argument('--max', type=int, required=True)
parser.add_argument('--bucket', type=int, required=True)
args = parser.parse_args()

tmAcquire = []
tmCopyResource = []
tmMap = []
tmMemcpy = []
tmEncode = []

with open(args.logs, 'r') as f:
    for ln in f:
        arrToModify = None
        split = ''
        if 'Acquire:' in ln:
            arrToModify = tmAcquire
            split = 'Acquire:'
        elif 'Copy Resource:' in ln:
            arrToModify = tmCopyResource
            split = 'Copy Resource:'
        elif 'Map:' in ln:
            arrToModify = tmMap
            split = 'Map:'
        elif 'Memcpy:' in ln:
            arrToModify = tmMemcpy
            split = 'Memcpy:'
        elif 'Encode:' in ln:
            arrToModify = tmEncode
            split = 'Encode:'

        if arrToModify is None:
            continue

        tokens = ln.split(split)
        try:
            arrToModify.append(float(tokens[1].strip()))
        except:
            pass

bins = [i for i in range(args.min, args.max, args.bucket)]
plt.hist([tmAcquire, tmCopyResource, tmMap, tmMemcpy, tmEncode], label=['Acquire', 'Copy Resource', 'Map', 'Memcpy', 'Encode'], bins=bins)
plt.legend(loc='best')
plt.savefig(args.output, dpi=128)