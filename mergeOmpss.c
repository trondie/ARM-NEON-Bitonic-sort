#include "utils/timing.h"
#include <omp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <papi.h>
#include <unistd.h>
#include "cmdline.h"
#include <stdint.h>
#include <Python.h>

#include "linkNeon.h"
//#include <algorithm>
#define T float
// N and MIN must be powers of 2
long N;
long MIN_SORT_SIZE;
long MIN_MERGE_SIZE;

#define HLINE "-------------------------------------------------------------\n"

#if _SSGRIND_
char stringMessage[256];
#endif 


static const int NUM_ITERATIONS = 10;
static const int SORT_CUT_OFF = 4;
static const int MERGE_CUT_OFF = 4;

// Merge Sort - See neonBitonic.c for implementation
void merge_sort(float *buffer, float *list, uintptr_t length)
{
    uintptr_t half = length / 2;
    if (length == 16){
        bitonic_4x4(buffer, list);
    } else {
        merge_sort_desc(buffer, list, half);
        merge_sort_desc(buffer + half, list + half, half);
    }
	merge(list, buffer, length);
}
void merge_sort_desc(float *buffer, float *list, uintptr_t length)
{
    uintptr_t half = length / 2;
    if (length == 16){
        bitonic_4x4(list, list);
    } else {
        merge_sort(buffer, list, half);
        merge_sort(buffer + half, list + half, half);
    }
	merge(buffer, list, length);
}

static void initialize(long length, T* data) {
	long i = 0;
	for (i = 0; i < length; i++) {
		if (i==0) {
			data[i] = rand();
		} else {
			data[i] = (float)(rand()%N)+(float)( (rand()%N) * 0.001);
		}
		if (data[i] == 0) data[i] = rand();
	}
}

int check_solution(long length, T* data) {
	int failure=0;
	long i;
	for (i = 0; i < length-1; i++) {
        if (i < 1024)
            printf(" %f ", data[i]);
		if (data[i]>data[i+1]){
		 failure=1; 
         printf (" F \n");
		}
	}
	return (failure);
}

static void touchn(long length, T* data) {
	long i;
	for (i = 0; i < length; i++) {
		data[i] = 0;
	}
}

// Integrate the python scripts for Agilent multimeter sampling and power estimation
float getPowerDissipation(float kernelTime){

	PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs;
	pArgs = PyTuple_New(1);
	pValue = PyFloat_FromDouble(kernelTime);
	PyTuple_SetItem(pArgs, 0, pValue);  
	double E = 0.0;
	int rets = 0;
	//rets = stopMeasure();

    // Initialize the Python Interpreter
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"./Energy")");
    // Build the name object
    pName = PyString_FromString("probe");

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "meanPowerExtern");

    if (PyCallable_Check(pFunc)) 
    {
        PyFloatObject *res = PyObject_CallObject(pFunc, pArgs);
        E = PyFloat_AsDouble(res);
    } else 
    {
        PyErr_Print();
    }

    // Clean up
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
    return (float)E;
}
// Integrate the python scripts for Agilent multimeter sampling and power estimation
float getEnergyConsumed(float kernelTime){
	PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs;
	pArgs = PyTuple_New(1);
	pValue = PyFloat_FromDouble(kernelTime);
	PyTuple_SetItem(pArgs, 0, pValue);  
	double E = 0.0;
	int rets = 0;
	//rets = stopMeasure();
    // Initialize the Python Interpreter
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"./Energy")");
    // Build the name object
    pName = PyString_FromString("probe");

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "energyExtern");

    if (PyCallable_Check(pFunc)) 
    {
        PyFloatObject *res = PyObject_CallObject(pFunc, pArgs);
        E = PyFloat_AsDouble(res);
    } else 
    {
        PyErr_Print();
    }

    // Clean up
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
    return (float)E;
}
// Intergrate Python script for init and request for whole board Telnet Agilent sampling
int startMeasure(){

	PyObject *pName, *pModule, *pDict, *pFunc, *pValue;
	double power = 0.0;
	int ret = 0;
    // Initialize the Python Interpreter
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"./Energy")");
    // Build the name object
    pName = PyString_FromString("startImmediate");

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "init");

    if (PyCallable_Check(pFunc)) 
    {
        PyObject_CallObject(pFunc, NULL);
    } else 
    {
    	ret = 1;
        PyErr_Print();
    }

    // Clean up
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
    return ret;
}

// Stop immediate and compensate for time related errors 
int stopMeasure(){

	PyObject *pName, *pModule, *pDict, *pFunc, *pValue;
	int ret = 0;
	double power = 0.0;
    // Initialize the Python Interpreter
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"./Energy")");
    // Build the name object
    pName = PyString_FromString("stopImmediate");

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "stop");

    if (PyCallable_Check(pFunc)) 
    {
        PyObject_CallObject(pFunc, NULL);
    } else 
    {
    	ret = 1;
        PyErr_Print();
    }

    // Clean up
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
    return ret;
}

float getSampleTime(float offset){
	PyObject *pName, *pModule, *pDict, *pFunc, *pValue, *pArgs;
	pArgs = PyTuple_New(1);
	pValue = PyFloat_FromDouble(offset);
	PyTuple_SetItem(pArgs, 0, pValue);  
	double timeRet = 0.0;
	int rets = 0;
	//rets = stopMeasure();
    // Initialize the Python Interpreter
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append(\"./Energy")");
    // Build the name object
    pName = PyString_FromString("probe");

    // Load the module object
    pModule = PyImport_Import(pName);

    // pDict is a borrowed reference 
    pDict = PyModule_GetDict(pModule);

    // pFunc is also a borrowed reference 
    pFunc = PyDict_GetItemString(pDict, "timeDurationCutoffExtern");

    if (PyCallable_Check(pFunc)) 
    {
        PyFloatObject *res = PyObject_CallObject(pFunc, pArgs);
        timeRet = PyFloat_AsDouble(res);
    } else 
    {
        PyErr_Print();
    }

    // Clean up
    Py_DECREF(pModule);
    Py_DECREF(pName);

    // Finish the Python Interpreter
    Py_Finalize();
    return (float)timeRet;
}

int main(int argc, char **argv) {
#ifdef _CHECK_
 	int failure = 0;
#ifdef CHECKSUM
	int checksum0, checksum1;
#endif
#endif

    size_t i;
	cmd_args args;
	parse_cmdline(argc, argv, &args);
    N = args.N * 1024;
	size_t sizeDataBytes = N * sizeof(T);
	
	T *data = (T*)malloc(sizeDataBytes);
	T* sortedOmpssMerge = (T*)malloc( sizeDataBytes);
	T* tmpOmpssMerge = (T*)malloc( sizeDataBytes );

	initialize( N, data );
	touchn( N, tmpOmpssMerge );
	double elapsedOmpssMergeTime = 0.0;
	memcpy( sortedOmpssMerge, data, sizeDataBytes );

	float energy = 0.0;
	float power = 0.0;
    volatile double sleep = 0.0;

    //Just sleep for one second
    resetAndStartTimer();
    while ((sleep * 1.e-6f) < 1.000){
        sleep += getElapsedTime();
    }

	#ifdef ENERGY_MEASURE
    	while (startMeasure() == 1){
    		printf("START ERROR!\n");
    	}
    #endif
    
    sleep = 0.0;
    //Sleep for 4ms to attempt compensating for latency
	resetAndStartTimer();
    while ((sleep * 1.e-6f) < 0.004){
    	sleep += getElapsedTime();
    }

	//Execute kernel
	volatile double t0 = 0.0;
	int niter = 0;
    while ((niter < 1) || ((t0 * 1.e-6f) < 0.1))
    {
    	memcpy( sortedOmpssMerge, data, sizeDataBytes );
		touchn( N, tmpOmpssMerge );
		resetAndStartTimer();
        #pragma omp task
        merge_sort( tmpOmpssMerge, sortedOmpssMerge, N/2 );
        #pragma omp task
        merge_sort( tmpOmpssMerge + (N / 2), sortedOmpssMerge + (N/2), N/2 );
        #pragma omp taskwait
        merge(tmpOmpssMerge, sortedOmpssMerge, N);
        merge(sortedOmpssMerge, tmpOmpssMerge, N);
        t0 += getElapsedTime();
        niter++;
    }
    //Convert into seconds
    t0 *= 1.e-6f;

    #ifdef ENERGY_MEASURE
    	while (stopMeasure() == 1){
    		printf("STOP ERROR!\n");
    	}
    #endif 

    #ifdef ENERGY_MEASURE
    	power = getPowerDissipation(t0); 
    	energy = getEnergyConsumed(t0); 
    #endif 
    
    //Average the time
    t0 /= (double)niter;
    
    float elems = N/t0;
    elems /= 1.e+6f;

	printf("rt: %le mflops: %le, energy: %le, avg_power: %le, task_size: %d, niters: %d\n", t0, elems, energy/niter, power, 0, niter);

	#ifdef VALIDATE 

	printf( HLINE );
	printf( "Merge Sort NEON:\n" );
	printf( "Vector of size %li elements\n", N );
	printf( "Results after %i iterations\n", niter );
	printf( HLINE );
	printf( "Execution type             Rate (MElement/s)        Avg time (s)\n" );
	printf("%s            %11.4f            %11.4f\n","NEON Mergesort", elems, t0 );
	
	printf( HLINE );

	int solutionValidates = check_solution( N, sortedOmpssMerge );
	if ( solutionValidates == 0 )
	{
		printf( "Mergesort Solution Validates\n" );
	}
	else
	{
		printf( "Mergesort Failed Validation\n" );
	}
	printf( HLINE );
	#endif

	free( data );
	free( sortedOmpssMerge );
	free( tmpOmpssMerge );

    return 0;
}
