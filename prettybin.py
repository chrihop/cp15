#!/usr/bin/env python3
import sys

class color:
	HEADER = '\033[95m'
	OKBLUE = '\033[94m'
	OKGREEN = '\033[92m'
	WARNING = '\033[93m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'

def number(s, end='\n'):
	if s == "  1":
		print(color.BOLD + color.FAIL + s + color.ENDC, end=end)
	else:
		print(color.BOLD + s + color.ENDC, end=end)

def index(s, end='\n'):
	print(color.OKBLUE + s + color.ENDC, end=end)


def gap(i, d):
	if i % d == 0:
		print("   ", end='')

def pretty_bin_group(l, s, length, max):
	for i in range(s, s + length):
		gap(i, 8)
		number("  {0}".format(l[i]), end = '')
	print("")
	for i in range(s, s + length):
		gap(i, 8)
		index(" {:>2}".format(max - i - 1), end = '')
	print("")


def pretty_bin64(x):
	l = "{:064b}".format(x)
	pretty_bin_group(l, 0, 32, 64)
	pretty_bin_group(l, 32, 32, 64)


def main():
	if len(sys.argv) > 0:
		pretty_bin64(int(sys.argv[1], 0))

main()