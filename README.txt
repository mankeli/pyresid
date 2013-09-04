PyReSID
=======

Python wrapper for your favourite sid emulator!
Shitty & simplified & incomplete.
But seems to work on my compy!

Build system is for MSVC 2010 SP1, but it should compile on
OSX/Linux with a custom g++ command line.

anyway:
- Usage example in python/pyaudiotest.py
- Requires pyaudio. The author says that it's alpha-quality software,
  but it seems to work fine here. It might support ASIO, but dunno.
- SID emu hides a global state. This should upgraded to use Python object
  system, but I'm too lazy right now.
  Instead remember that you can only have one (1) 6581. (for now)
- by "frame" we mean one 16bit mono sample (2 bytes)


API:
  resid.init(sid_clock_frequency, sample_rate, passband_freq)

    Initializes ReSID. pass c64 clock freq, samplerate and passband for resampling.
    First two are obvious, and definition of passband freq can be found in
    pyresid/residfp/SID.h. For 44100, 20000 seems to work quite fine.

  resid.wr(reg, value)
    poke $D400+reg, value

  resid.run(cycles)
    Advance sid emulation. This generates sound data, but it will be stored
    into an internal buffer and outputted later.
    Don't use too much. (buffer can hold about 1.5 c64 frames)

  resid.gen(max_cycles, max_frames)
    This runs the emulator and generates audio until it hits max_cycles or max_frames.
    After the generation, it returns a tuple:
      data = (bytearray sampledata, int cycles_run, int frames_generated)

    "cycles_run" is how many cycles the emulator advanced. "frames_generated" is
    the number of frames. And "sampledata" is an array of bytes, which is
    (frames_generated*2) bytes long. In best case, it can be directly passed
    into portaudio.

   resid.say_hello(name)
     This is used to send scene-style greetings.

