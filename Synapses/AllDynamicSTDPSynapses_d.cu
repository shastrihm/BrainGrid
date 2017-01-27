/*
 * AllDynamicSTDPSynapses_d.cu
 *
 */

#include "AllDynamicSTDPSynapses.h"
#include "AllSynapsesDeviceFuncs.h"
#include <helper_cuda.h>

/*
 *  Allocate GPU memories to store all synapses' states,
 *  and copy them from host to GPU memory.
 *
 *  @param  allSynapsesDevice  Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                             on device memory.
 *  @param  sim_info           SimulationInfo to refer from.
 *  @param  clr_info           ClusterInfo to refer from.
 */
void AllDynamicSTDPSynapses::allocSynapseDeviceStruct( void** allSynapsesDevice, const SimulationInfo *sim_info, const ClusterInfo *clr_info ) {
	allocSynapseDeviceStruct( allSynapsesDevice, clr_info->totalClusterNeurons, sim_info->maxSynapsesPerNeuron );
}

/*
 *  Allocate GPU memories to store all synapses' states,
 *  and copy them from host to GPU memory.
 *
 *  @param  allSynapsesDevice     Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                                on device memory.
 *  @param  num_neurons           Number of neurons.
 *  @param  maxSynapsesPerNeuron  Maximum number of synapses per neuron.
 */
void AllDynamicSTDPSynapses::allocSynapseDeviceStruct( void** allSynapsesDevice, int num_neurons, int maxSynapsesPerNeuron ) {
	AllDynamicSTDPSynapsesDeviceProperties allSynapses;

	allocDeviceStruct( allSynapses, num_neurons, maxSynapsesPerNeuron );

	checkCudaErrors( cudaMalloc( allSynapsesDevice, sizeof( AllDynamicSTDPSynapsesDeviceProperties ) ) );
	checkCudaErrors( cudaMemcpy ( *allSynapsesDevice, &allSynapses, sizeof( AllDynamicSTDPSynapsesDeviceProperties ), cudaMemcpyHostToDevice ) );
}

/*
 *  Allocate GPU memories to store all synapses' states,
 *  and copy them from host to GPU memory.
 *  (Helper function of allocSynapseDeviceStruct)
 *
 *  @param  allSynapsesDevice     Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                                on device memory.
 *  @param  num_neurons           Number of neurons.
 *  @param  maxSynapsesPerNeuron  Maximum number of synapses per neuron.
 */
void AllDynamicSTDPSynapses::allocDeviceStruct( AllDynamicSTDPSynapsesDeviceProperties &allSynapses, int num_neurons, int maxSynapsesPerNeuron ) {
        AllSTDPSynapses::allocDeviceStruct( allSynapses, num_neurons, maxSynapsesPerNeuron );

        BGSIZE max_total_synapses = maxSynapsesPerNeuron * num_neurons;

        checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.lastSpike, max_total_synapses * sizeof( uint64_t ) ) );
	checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.r, max_total_synapses * sizeof( BGFLOAT ) ) );
	checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.u, max_total_synapses * sizeof( BGFLOAT ) ) );
	checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.D, max_total_synapses * sizeof( BGFLOAT ) ) );
	checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.U, max_total_synapses * sizeof( BGFLOAT ) ) );
	checkCudaErrors( cudaMalloc( ( void ** ) &allSynapses.F, max_total_synapses * sizeof( BGFLOAT ) ) );
}

/*
 *  Delete GPU memories.
 *
 *  @param  allSynapsesDevice  Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                             on device memory.
 *  @param  sim_info           SimulationInfo to refer from.
 */
void AllDynamicSTDPSynapses::deleteSynapseDeviceStruct( void* allSynapsesDevice ) {
	AllDynamicSTDPSynapsesDeviceProperties allSynapses;

	checkCudaErrors( cudaMemcpy ( &allSynapses, allSynapsesDevice, sizeof( AllDynamicSTDPSynapsesDeviceProperties ), cudaMemcpyDeviceToHost ) );

	deleteDeviceStruct( allSynapses );

	checkCudaErrors( cudaFree( allSynapsesDevice ) );
}

/*
 *  Delete GPU memories.
 *  (Helper function of deleteSynapseDeviceStruct)
 *
 *  @param  allSynapsesDevice  Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                             on device memory.
 */
void AllDynamicSTDPSynapses::deleteDeviceStruct( AllDynamicSTDPSynapsesDeviceProperties& allSynapses ) {
        checkCudaErrors( cudaFree( allSynapses.lastSpike ) );
	checkCudaErrors( cudaFree( allSynapses.r ) );
	checkCudaErrors( cudaFree( allSynapses.u ) );
	checkCudaErrors( cudaFree( allSynapses.D ) );
	checkCudaErrors( cudaFree( allSynapses.U ) );
	checkCudaErrors( cudaFree( allSynapses.F ) );

        AllSTDPSynapses::deleteDeviceStruct( allSynapses );
}

/*
 *  Copy all synapses' data from host to device.
 *
 *  @param  allSynapsesDevice  Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                             on device memory.
 *  @param  sim_info           SimulationInfo to refer from.
 *  @param  clr_info           ClusterInfo to refer from.
 */
void AllDynamicSTDPSynapses::copySynapseHostToDevice( void* allSynapsesDevice, const SimulationInfo *sim_info, const ClusterInfo *clr_info ) { // copy everything necessary
	copySynapseHostToDevice( allSynapsesDevice, clr_info->totalClusterNeurons, sim_info->maxSynapsesPerNeuron );	
}

/*
 *  Copy all synapses' data from host to device.
 *
 *  @param  allSynapsesDevice     Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                                on device memory.
 *  @param  num_neurons           Number of neurons.
 *  @param  maxSynapsesPerNeuron  Maximum number of synapses per neuron.
 */
void AllDynamicSTDPSynapses::copySynapseHostToDevice( void* allSynapsesDevice, int num_neurons, int maxSynapsesPerNeuron ) { // copy everything necessary
	AllDynamicSTDPSynapsesDeviceProperties allSynapses;

        checkCudaErrors( cudaMemcpy ( &allSynapses, allSynapsesDevice, sizeof( AllDynamicSTDPSynapsesDeviceProperties ), cudaMemcpyDeviceToHost ) );

	copyHostToDevice( allSynapsesDevice, allSynapses, num_neurons, maxSynapsesPerNeuron );	
}

/*
 *  Copy all synapses' data from host to device.
 *  (Helper function of copySynapseHostToDevice)
 *
 *  @param  allSynapsesDevice     Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                                on device memory.
 *  @param  num_neurons           Number of neurons.
 *  @param  maxSynapsesPerNeuron  Maximum number of synapses per neuron.
 */
void AllDynamicSTDPSynapses::copyHostToDevice( void* allSynapsesDevice, AllDynamicSTDPSynapsesDeviceProperties& allSynapses, int num_neurons, int maxSynapsesPerNeuron ) { // copy everything necessary 
        AllSTDPSynapses::copyHostToDevice( allSynapsesDevice, allSynapses, num_neurons, maxSynapsesPerNeuron );

        BGSIZE max_total_synapses = maxSynapsesPerNeuron * num_neurons;
        
        checkCudaErrors( cudaMemcpy ( allSynapses.lastSpike, lastSpike,
                max_total_synapses * sizeof( uint64_t ), cudaMemcpyHostToDevice ) );
        checkCudaErrors( cudaMemcpy ( allSynapses.r, r,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyHostToDevice ) );
        checkCudaErrors( cudaMemcpy ( allSynapses.u, u,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyHostToDevice ) );
        checkCudaErrors( cudaMemcpy ( allSynapses.D, D,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyHostToDevice ) );
        checkCudaErrors( cudaMemcpy ( allSynapses.U, U,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyHostToDevice ) );
        checkCudaErrors( cudaMemcpy ( allSynapses.F, F,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyHostToDevice ) );
}

/*
 *  Copy all synapses' data from device to host.
 *
 *  @param  allSynapsesDevice  Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                             on device memory.
 *  @param  sim_info           SimulationInfo to refer from.
 *  @param  clr_info           ClusterInfo to refer from.
 */
void AllDynamicSTDPSynapses::copySynapseDeviceToHost( void* allSynapsesDevice, const SimulationInfo *sim_info, const ClusterInfo *clr_info ) {
	// copy everything necessary
	AllDynamicSTDPSynapsesDeviceProperties allSynapses;

        checkCudaErrors( cudaMemcpy ( &allSynapses, allSynapsesDevice, sizeof( AllDynamicSTDPSynapsesDeviceProperties ), cudaMemcpyDeviceToHost ) );

	copyDeviceToHost( allSynapses, sim_info, clr_info );
}

/*
 *  Copy all synapses' data from device to host.
 *  (Helper function of copySynapseDeviceToHost)
 *
 *  @param  allSynapsesDevice     Reference to the AllDynamicSTDPSynapsesDeviceProperties struct 
 *                                on device memory.
 *  @param  sim_info              SimulationInfo to refer from.
 *  @param  clr_info              ClusterInfo to refer from.
 */
void AllDynamicSTDPSynapses::copyDeviceToHost( AllDynamicSTDPSynapsesDeviceProperties& allSynapses, const SimulationInfo *sim_info, const ClusterInfo *clr_info ) {
        AllSTDPSynapses::copyDeviceToHost( allSynapses, sim_info, clr_info ) ;

	int num_neurons = clr_info->totalClusterNeurons;
	BGSIZE max_total_synapses = sim_info->maxSynapsesPerNeuron * num_neurons;

        checkCudaErrors( cudaMemcpy ( lastSpike, allSynapses.lastSpike,
                max_total_synapses * sizeof( uint64_t ), cudaMemcpyDeviceToHost ) );
        checkCudaErrors( cudaMemcpy ( r, allSynapses.r,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyDeviceToHost ) );
        checkCudaErrors( cudaMemcpy ( u, allSynapses.u,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyDeviceToHost ) );
        checkCudaErrors( cudaMemcpy ( D, allSynapses.D,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyDeviceToHost ) );
        checkCudaErrors( cudaMemcpy ( U, allSynapses.U,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyDeviceToHost ) );
        checkCudaErrors( cudaMemcpy ( F, allSynapses.F,
                max_total_synapses * sizeof( BGFLOAT ), cudaMemcpyDeviceToHost ) );
}

/**     
 *  Set synapse class ID defined by enumClassSynapses for the caller's Synapse class.
 *  The class ID will be set to classSynapses_d in device memory,
 *  and the classSynapses_d will be referred to call a device function for the
 *  particular synapse class.
 *  Because we cannot use virtual function (Polymorphism) in device functions,
 *  we use this scheme.
 *  Note: we used to use a function pointer; however, it caused the growth_cuda crash
 *  (see issue#137).
 */
void AllDynamicSTDPSynapses::setSynapseClassID()
{
    enumClassSynapses classSynapses_h = classAllDynamicSTDPSynapses;

    checkCudaErrors( cudaMemcpyToSymbol(classSynapses_d, &classSynapses_h, sizeof(enumClassSynapses)) );
}
