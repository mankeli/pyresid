"""PyAudio Example: Play a wave file (callback version)."""

import pyaudio
import time
import sys

import sys
sys.path.append('../Release')
import resid

def callback(in_data, frame_count, time_info, status):
	eh = resid.gen(0, frame_count)
	data = bytes(eh[0])
	return (data, pyaudio.paContinue)

try:

	resid.say_hello("MIXERI OF ORIGO")

	resid.init(1000000, 44100, 20000)
	resid.wr(0x17, 0xF1)
	resid.wr(0x18, 0x1F)

	resid.wr(0x16, 0x40)

	resid.wr(0x00, 0x04)
	resid.run(5);
	resid.wr(0x01, 0x04)
	resid.run(5);
	resid.wr(0x05, 0xFF)
	resid.run(5);
	resid.wr(0x06, 0x0F)
	resid.run(5);
	resid.wr(0x04, 0x21)
	resid.run(5);

	eh = resid.gen(0, 10)
	print(eh)

	p = pyaudio.PyAudio()

	format_def = p.get_format_from_width(2, False)
	print("data format (should be 8): "+str(format_def))

	stream = p.open(format=format_def,
					channels=1,
					rate=44100,
					output=True,
					stream_callback=callback)

	# start the stream (4)
	stream.start_stream()

	# wait for stream to finish (5)
	while True:
		time.sleep(0.1)

except:
	try:
		print("shutting down!")
		# stop stream (6)
		stream.stop_stream()
		stream.close()

		# close PyAudio (7)
		p.terminate()
	except:
		print("oh forget it :-D")