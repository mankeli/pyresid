#include <Python.h>
#include "sid.h"

using namespace reSIDfp;

static SID *resid = NULL;

static struct sidinfo
{
	double clockrate;
	double samplerate;
} sidinfo;

#define RES_BUF_SIZE (65536)
static short residual_buf[RES_BUF_SIZE];
static int residual_buf_fill;

static bool be_loud = false;

static PyObject* say_hello(PyObject *self, PyObject *args)
{
	const char* name;
 
	if (!PyArg_ParseTuple(args, "s", &name))
		return NULL;
 
	printf("Hello %s!\n", name);
 
	Py_RETURN_NONE;
}

static PyObject* init(PyObject *self, PyObject *args)
{
	double clockrate;
	double samplerate;
	double passband;
	if (!PyArg_ParseTuple(args, "ddd", &clockrate, &samplerate, &passband))
		return NULL;

	fprintf(stderr, "resid init clock %.2f, sr %.2f pb %.2f\n", clockrate, samplerate, passband);

	if (resid)
		delete resid;

	sidinfo.clockrate = clockrate;
	sidinfo.samplerate = samplerate;

	resid = new SID;
	resid->setChipModel(MOS6581);
	resid->setSamplingParameters(clockrate, RESAMPLE, samplerate, passband);
	resid->clockSilent(10);

	residual_buf_fill = 0;

	Py_RETURN_NONE;
}

static PyObject *wr(PyObject *self, PyObject *args)
{
	short int reg, val;

	if (!PyArg_ParseTuple(args, "hh", &reg, &val))
		return NULL;

	fprintf(stderr, "write: %02X = %02X\n", reg, val);

	resid->write((int)reg, (unsigned char)val);

	Py_RETURN_NONE;
}

static inline int min(int v1, int v2)
{
	return v1 < v2 ? v1 : v2;
}

#define SAFE_EXTRA_CYCLECOUNT (100)

static inline int estimate_cycles(int frames_left)
{
	return (int)((frames_left * sidinfo.clockrate) / sidinfo.samplerate) + SAFE_EXTRA_CYCLECOUNT;
}

static PyObject *gen(PyObject *self, PyObject *args)
{
	int i;
	int maxcycles, maxframes;
	int fr_cycles;

	if (!PyArg_ParseTuple(args, "ii", &maxcycles, &maxframes))
		return NULL;

	if (be_loud)
		fprintf(stderr, "START GEN: max cycles %i, max frames %i. residual buf size %i\n", maxcycles, maxframes, residual_buf_fill);


	if (maxcycles == 0)
	{
		maxcycles = estimate_cycles(maxframes);
		if (be_loud)
			fprintf(stderr, "estimated cycles to be %i\n", maxcycles);
	}

	int this_frames = 0;

	if (maxframes <= residual_buf_fill)
	{
		this_frames = residual_buf_fill;
		fr_cycles = 0;

		if (be_loud)
			fprintf(stderr, "we don't want to emulate at all. enough residual data\n");
	}
	else
	{
		this_frames = residual_buf_fill;

		fr_cycles = estimate_cycles(maxframes - this_frames);
		fr_cycles = min(fr_cycles, maxcycles);

		if (be_loud)
			fprintf(stderr, "we want to generate %i cycles\n", fr_cycles);

		residual_buf_fill += resid->clock(fr_cycles, &(residual_buf[this_frames]));

		this_frames = min(residual_buf_fill, maxframes);
	}

	if (this_frames > residual_buf_fill)
	{
		fprintf(stderr, "trying to give %i frames out, but we only have %i\n", this_frames, residual_buf_fill);
		this_frames = residual_buf_fill;
	}

	if (be_loud)
		fprintf(stderr, "so results are: %i/%i cycles, %i/%i frames\n", fr_cycles, maxcycles, this_frames, maxframes);

	PyObject *ret;
	ret = Py_BuildValue("(y#ii)", residual_buf, this_frames * sizeof(short), fr_cycles, this_frames);

	for (i = 0; i < (residual_buf_fill - this_frames); i++)
	{
		residual_buf[i] = residual_buf[this_frames + i];
	}
	residual_buf_fill = residual_buf_fill - this_frames;
	if (residual_buf_fill < 0)
	{
		fprintf(stderr, "heh residual buf size meni alle nollan.\n");
		residual_buf_fill = 0;
	}


	return ret;
}

static PyObject *run(PyObject *self, PyObject *args)
{
	int maxcycles;

	if (!PyArg_ParseTuple(args, "i", &maxcycles))
		return NULL;

	residual_buf_fill += resid->clock(maxcycles, &(residual_buf[residual_buf_fill]));

	Py_RETURN_NONE;
}

static PyObject *beloud(PyObject *self, PyObject *args)
{
	be_loud = true;
	Py_RETURN_NONE;
}

//generated_count = resid->clock(cycles_to_run, buf);
 
static PyMethodDef HelloMethods[] =
{
	 {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},
	 {"init", init, METH_VARARGS, "init resid"},
	 {"wr", wr, METH_VARARGS, "write to resid"},
	 {"gen", gen, METH_VARARGS, "gen audio"},
	 {"run", run, METH_VARARGS, "run sid"},
	 {"beloud", beloud, METH_VARARGS, "be loud"},
	 {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"resid",     /* m_name */
	"This is RESID",  /* m_doc */
	-1,                  /* m_size */
	HelloMethods,    /* m_methods */
	NULL,                /* m_reload */
	NULL,                /* m_traverse */
	NULL,                /* m_clear */
	NULL,                /* m_free */
};

PyMODINIT_FUNC PyInit_resid(void)
{
	return PyModule_Create(&moduledef);
}

