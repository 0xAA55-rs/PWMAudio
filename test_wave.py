import os
import math
import struct

def create_wav(filename, sample_rate, duration, func):
	total_samples = int(duration * sample_rate)
	channels = 1
	fmt_len = 16
	data_len = total_samples * 2
	riff_len = 12 + fmt_len + 8 + data_len
	with open(filename, "wb") as f:
		f.write(b'RIFF')
		f.write(struct.pack('<I', riff_len))
		f.write(b'WAVEfmt ')
		f.write(struct.pack('<I', fmt_len))
		f.write(struct.pack('<H', 1))
		f.write(struct.pack('<H', 1))
		f.write(struct.pack('<I', sample_rate))
		f.write(struct.pack('<I', sample_rate * 2))
		f.write(struct.pack('<H', 2))
		f.write(struct.pack('<H', 16))
		f.write(b'data')
		f.write(struct.pack('<I', data_len))
		for i in range(0, total_samples):
			f.write(struct.pack('<h', int(func(i, total_samples) * 32767.0)))

def create_sine(duration):
	create_wav("test_sine.wav", 48000, duration, lambda x, y: math.cos(x * math.pi * 2.0 / y))

def create_square(duration, interval):
	ivs = int(48000 * interval)
	create_wav("test_square.wav", 48000, duration, lambda x, y: 1.0 if x % ivs > ivs / 2 else -1.0)

if __name__ == '__main__':
	create_sine(6)
	create_square(6, 1.0)
