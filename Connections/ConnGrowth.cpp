#include "ConnGrowth.h"
#include "ParseParamError.h"
#include "IAllSynapses.h"
#include "XmlGrowthRecorder.h"
#ifdef USE_HDF5
#include "Hdf5GrowthRecorder.h"
#endif

/* ------------- CONNECTIONS STRUCT ------------ *\
 * Below all of the resources for the various
 * connections are instantiated and initialized.
 * All of the allocation for memory is done in the
 * constructor’s parameters and not in the body of
 * the function. Once all memory has been allocated
 * the constructor fills in known information
 * into “radii” and “rates”.
\* --------------------------------------------- */
// TODO comment
/* ------------------- ERROR ------------------- *\
 * terminate called after throwing an instance of 'std::bad_alloc'
 *      what():  St9bad_alloc
 * ------------------- CAUSE ------------------- *|
 * As simulations expand in size the number of
 * neurons in total increases exponentially. When
 * using a MATRIX_TYPE = “complete” the amount of
 * used memory increases by another order of magnitude.
 * Once enough memory is used no more memory can be
 * allocated and a “bsd_alloc” will be thrown.
 * The following members of the connection constructor
 * consume equally vast amounts of memory as the
 * simulation sizes grow:
 *      - W             - radii
 *      - rates         - dist2
 *      - delta         - dist
 *      - areai
 * ----------------- 1/25/14 ------------------- *|
 * Currently when running a simulation of sizes
 * equal to or greater than 100 * 100 the above
 * error is thrown. After some testing we have
 * determined that this is a hardware dependent
 * issue, not software. We are also looking into
 * switching matrix types from "complete" to
 * "sparce". If successful it is possible the
 * problematic matricies mentioned above will use
 * only 1/250 of their current space.
\* --------------------------------------------- */
ConnGrowth::ConnGrowth() : Connections()
{
    radii = NULL;
    rates = NULL;
#if defined(USE_GPU)
    m_barrierUpdateConnections = NULL;
#else // !USE_GPU
    W = NULL;
    delta = NULL;
    area = NULL;
    outgrowth = NULL;
    deltaR = NULL;
#endif // !USE_GPU
}

ConnGrowth::~ConnGrowth()
{
    cleanupConnections();
}

/*
 *  Setup the internal structure of the class (allocate memories and initialize them).
 *
 *  @param  sim_info    SimulationInfo class to read information from.
 *  @param  layout      Layout information of the neunal network.
 *  @param  vtClr       Vector of Cluster class objects.
 *  @param  vtClrInfo   Vector of ClusterInfo.
 */
void ConnGrowth::setupConnections(const SimulationInfo *sim_info, Layout *layout, vector<Cluster *> &vtClr, vector<ClusterInfo *> &vtClrInfo)
{
    int num_neurons = sim_info->totalNeurons;

#if defined(USE_GPU)
    radii = new BGFLOAT[num_neurons];
    rates = new BGFLOAT[num_neurons];

    for (int i = 0; i < num_neurons; i++) {
        radii[i] = m_growth.startRadius;
    }

    // Initialize the Barrier Synchnonize object for updateConnections.
    m_barrierUpdateConnections = new Barrier(vtClr.size() + 1);
#else // !USE_GPU
    W = new CompleteMatrix(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons, 0);
    radii = new VectorMatrix(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons, m_growth.startRadius);
    rates = new VectorMatrix(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons, 0);
    delta = new CompleteMatrix(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons);
    area = new CompleteMatrix(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons, 0);
    outgrowth = new VectorMatrix(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons);
    deltaR = new VectorMatrix(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons);

    // Init connection frontier distance change matrix with the current distances
    (*delta) = (*layout->dist);
#endif // !USE_GPU
}

/*
 *  Cleanup the class (deallocate memories).
 */
void ConnGrowth::cleanupConnections()
{
#if defined(USE_GPU)
    if (m_barrierUpdateConnections != NULL) delete m_barrierUpdateConnections;
    if (radii != NULL) delete[] radii;
    if (rates != NULL) delete[] rates;
#else // !USE_GPU
    if (W != NULL) delete W;
    if (radii != NULL) delete radii;
    if (rates != NULL) delete rates;
    if (delta != NULL) delete delta;
    if (area != NULL) delete area;
    if (outgrowth != NULL) delete outgrowth;
    if (deltaR != NULL) delete deltaR;
#endif // !USE_GPU

    radii = NULL;
    rates = NULL;
#if defined(USE_GPU)
    m_barrierUpdateConnections = NULL;
#else // !UDSE_GPU
    W = NULL;
    delta = NULL;
    area = NULL;
    outgrowth = NULL;
    deltaR = NULL;
#endif // UDSE_GPU
}

/*
 *  Checks the number of required parameters.
 *
 * @return true if all required parameters were successfully read, false otherwise.
 */
bool ConnGrowth::checkNumParameters()
{
    return (nParams >= 1);
}

/*
 *  Attempts to read parameters from a XML file.
 *
 *  @param  element TiXmlElement to examine.
 *  @return true if successful, false otherwise.
 */
bool ConnGrowth::readParameters(const TiXmlElement& element)
{
    if (element.ValueStr().compare("GrowthParams") == 0) {
	nParams++;
	return true;
    }

    if (element.Parent()->ValueStr().compare("GrowthParams") == 0) {
/*
        if (element.QueryFLOATAttribute("epsilon", &m_growth.epsilon) != TIXML_SUCCESS) {
                throw ParseParamError("epsilon", "Growth param 'epsilon' missing in XML.");
        }
        if (m_growth.epsilon < 0) {
                throw ParseParamError("epsilon", "Invalid negative Growth param 'epsilon' value.");
        }

        if (element.QueryFLOATAttribute("beta", &m_growth.beta) != TIXML_SUCCESS) {
                throw ParseParamError("beta", "Growth param 'beta' missing in XML.");
        }
        if (m_growth.beta < 0) {
                throw ParseParamError("beta", "Invalid negative Growth param 'beta' value.");
        }

        if (element.QueryFLOATAttribute("rho", &m_growth.rho) != TIXML_SUCCESS) {
                throw ParseParamError("rho", "Growth param 'rho' missing in XML.");
        }
        if (m_growth.rho < 0) {
                throw ParseParamError("rho", "Invalid negative Growth param 'rho' value.");
        }

        //check if 'beta' is erroneous info
        if (element.QueryFLOATAttribute("targetRate", &m_growth.targetRate) != TIXML_SUCCESS) {
                throw ParseParamError("targetRate", "Growth targetRate 'beta' missing in XML.");
        }
        if (m_growth.targetRate < 0) {
                throw ParseParamError("targetRate", "Invalid negative Growth targetRate.");
        }

        if (element.QueryFLOATAttribute("minRadius", &m_growth.minRadius) != TIXML_SUCCESS) {
                throw ParseParamError("minRadius", "Growth minRadius 'beta' missing in XML.");
        }
        if (m_growth.minRadius < 0) {
                throw ParseParamError("minRadius", "Invalid negative Growth minRadius.");
        }

        if (element.QueryFLOATAttribute("startRadius", &m_growth.startRadius) != TIXML_SUCCESS) {
                throw ParseParamError("startRadius", "Growth startRadius 'beta' missing in XML.");
        }
        if (m_growth.startRadius < 0) {
                throw ParseParamError("startRadius", "Invalid negative Growth startRadius.");
        }

        // initial maximum firing rate
        m_growth.maxRate = m_growth.targetRate / m_growth.epsilon;

        nParams++;
*/
	if(element.ValueStr().compare("epsilon") == 0){
            m_growth.epsilon = atof(element.GetText());
        }
	else if(element.ValueStr().compare("beta") == 0){
            m_growth.beta = atof(element.GetText());
        }
	else if(element.ValueStr().compare("rho") == 0){
            m_growth.rho = atof(element.GetText());
        }
	else if(element.ValueStr().compare("targetRate") == 0){
            m_growth.targetRate = atof(element.GetText());
        }
	else if(element.ValueStr().compare("minRadius") == 0){
            m_growth.minRadius = atof(element.GetText());
        }
	else if(element.ValueStr().compare("startRadius") == 0){
            m_growth.startRadius = atof(element.GetText());
        }
	
	if(m_growth.epsilon != 0){
	    m_growth.maxRate = m_growth.targetRate / m_growth.epsilon;
	}

        return true;
    }

    return false;
}

/*
 *  Prints out all parameters of the connections to ostream.
 *
 *  @param  output  ostream to send output to.
 */
void ConnGrowth::printParameters(ostream &output) const
{
    output << "Growth parameters: " << endl
           << "\tepsilon: " << m_growth.epsilon
           << ", beta: " << m_growth.beta
           << ", rho: " << m_growth.rho
           << ", targetRate: " << m_growth.targetRate << "," << endl
           << "\tminRadius: " << m_growth.minRadius
           << ", startRadius: " << m_growth.startRadius
           << endl;

}

/*
 *  Reads the intermediate connection status from istream.
 *
 *  @param  input    istream to read status from.
 *  @param  sim_info SimulationInfo class to read information from.
 */
void ConnGrowth::deserialize(istream& input, const SimulationInfo *sim_info)
{
    // read the radii
    for (int i = 0; i < sim_info->totalNeurons; i++) {
#if defined(USE_GPU)
            input >> radii[i]; input.ignore();
#else // !USE_GPU
            input >> (*radii)[i]; input.ignore();
#endif // !USE_GPU
    }

    // read the rates
    for (int i = 0; i < sim_info->totalNeurons; i++) {
#if defined(USE_GPU)
            input >> rates[i]; input.ignore();
#else // !USE_GPU
            input >> (*rates)[i]; input.ignore();
#endif // !USE_GPU
    }
}

/*
 *  Writes the intermediate connection status to ostream.
 *
 *  @param  output   ostream to write status to.
 *  @param  sim_info SimulationInfo class to read information from.
 */
void ConnGrowth::serialize(ostream& output, const SimulationInfo *sim_info)
{
    // write the final radii
    for (int i = 0; i < sim_info->totalNeurons; i++) {
#if defined(USE_GPU)
        output << radii[i] << ends;
#else // !USE_GPU
        output << (*radii)[i] << ends;
#endif // !USE_GPU
    }

    // write the final rates
    for (int i = 0; i < sim_info->totalNeurons; i++) {
#if defined(USE_GPU)
        output << rates[i] << ends;
#else // !USE_GPU
        output << (*rates)[i] << ends;
#endif // !USE_GPU
    }
}

#if !defined(USE_GPU)
/*
 *  Update the connections status in every epoch.
 *
 *  @param  sim_info    SimulationInfo class to read information from.
 *  @param  layout      Layout information of the neunal network.
 *  @param  vtClr       Vector of Cluster class objects.
 *  @param  vtClrInfo   Vector of ClusterInfo.
 */
void ConnGrowth::updateConnections(const SimulationInfo *sim_info, Layout *layout, vector<Cluster *> &vtClr, vector<ClusterInfo *> &vtClrInfo)
{
    // Update Connections data
    updateConns(sim_info, vtClr, vtClrInfo);
 
    // Update the distance between frontiers of Neurons
    updateFrontiers(sim_info->totalNeurons, layout);

    // Update the areas of overlap in between Neurons
    updateOverlap(sim_info->totalNeurons, layout);
 
    // Update the weight of the Synapses in the simulation
    updateSynapsesWeights(sim_info, layout, vtClr, vtClrInfo);

    // Create synapse index maps
    SynapseIndexMap::createSynapseImap(sim_info, vtClr, vtClrInfo);
}

/*
 *  Calculates firing rates, neuron radii change and assign new values.
 *
 *  @param  sim_info    SimulationInfo class to read information from.
 *  @param  vtClr       Vector of Cluster class objects.
 *  @param  vtClrInfo   Vector of ClusterInfo.
 */
void ConnGrowth::updateConns(const SimulationInfo *sim_info, vector<Cluster *> &vtClr, vector<ClusterInfo *> &vtClrInfo)
{
    // Calculate growth cycle firing rate for previous period
    int max_spikes = static_cast<int> (sim_info->epochDuration * sim_info->maxFiringRate);

    // for each cluster
    for (CLUSTER_INDEX_TYPE iCluster = 0; iCluster < vtClr.size(); iCluster++) {
        AllSpikingNeurons *neurons = dynamic_cast<AllSpikingNeurons*>(vtClr[iCluster]->m_neurons);
        AllSpikingNeuronsProps *pNeuronsProps = dynamic_cast<AllSpikingNeuronsProps*>(neurons->m_pNeuronsProps);
        int neuronLayoutIndex = vtClrInfo[iCluster]->clusterNeuronsBegin;
        int totalClusterNeurons = vtClrInfo[iCluster]->totalClusterNeurons;
        for (int iNeuron = 0; iNeuron < totalClusterNeurons; iNeuron++, neuronLayoutIndex++) {
            // Calculate firing rate
            assert(pNeuronsProps->spikeCount[iNeuron] < max_spikes);
            (*rates)[neuronLayoutIndex] = pNeuronsProps->spikeCount[iNeuron] / sim_info->epochDuration;
        }
    }

    // compute neuron radii change and assign new values
    (*outgrowth) = 1.0 - 2.0 / (1.0 + exp((m_growth.epsilon - *rates / m_growth.maxRate) / m_growth.beta));
    (*deltaR) = sim_info->epochDuration * m_growth.rho * *outgrowth;
    (*radii) += (*deltaR); 
}

/*
 *  Update the distance between frontiers of Neurons.
 *
 *  @param  num_neurons Number of neurons to update.
 *  @param  layout      Layout information of the neunal network.
 */
void ConnGrowth::updateFrontiers(const int num_neurons, Layout *layout)
{
    DEBUG(cout << "Updating distance between frontiers..." << endl;)
    // Update distance between frontiers
    for (int unit = 0; unit < num_neurons - 1; unit++) {
        for (int i = unit + 1; i < num_neurons; i++) {
            (*delta)(unit, i) = (*layout->dist)(unit, i) - ((*radii)[unit] + (*radii)[i]);
            (*delta)(i, unit) = (*delta)(unit, i);
        }
    }
}

/*
 *  Update the areas of overlap in between Neurons.
 *
 *  @param  num_neurons Number of Neurons to update.
 *  @param  layout      Layout information of the neunal network.
 */
void ConnGrowth::updateOverlap(BGFLOAT num_neurons, Layout *layout)
{
    DEBUG(cout << "computing areas of overlap" << endl;)

    // Compute areas of overlap; this is only done for overlapping units
    for (int i = 0; i < num_neurons; i++) {
        for (int j = 0; j < num_neurons; j++) {
                (*area)(i, j) = 0.0;

                if ((*delta)(i, j) < 0) {
                        BGFLOAT lenAB = (*layout->dist)(i, j);
                        BGFLOAT r1 = (*radii)[i];
                        BGFLOAT r2 = (*radii)[j];

                    if (lenAB + min(r1, r2) <= max(r1, r2)) {
                        (*area)(i, j) = pi * min(r1, r2) * min(r1, r2); // Completely overlapping unit
#ifdef LOGFILE
                        logFile << "Completely overlapping (i, j, r1, r2, area): "
                            << i << ", " << j << ", " << r1 << ", " << r2 << ", " << *pAarea(i, j) << endl;
#endif // LOGFILE
                        } else {
                                // Partially overlapping unit
                                BGFLOAT lenAB2 = (*layout->dist2)(i, j);
                                BGFLOAT r12 = r1 * r1;
                                BGFLOAT r22 = r2 * r2;

                                BGFLOAT cosCBA = (r22 + lenAB2 - r12) / (2.0 * r2 * lenAB);
                                BGFLOAT angCBA = acos(cosCBA);
                                BGFLOAT angCBD = 2.0 * angCBA;

                                BGFLOAT cosCAB = (r12 + lenAB2 - r22) / (2.0 * r1 * lenAB);
                                BGFLOAT angCAB = acos(cosCAB);
                                BGFLOAT angCAD = 2.0 * angCAB;

                                (*area)(i, j) = 0.5 * (r22 * (angCBD - sin(angCBD)) + r12 * (angCAD - sin(angCAD)));
                        }
                }
        }
    }
}

/*
 *  Update the weight of the Synapses in the simulation.
 *
 *  @param  sim_info    SimulationInfo to refer from.
 *  @param  layout      Layout information of the neunal network.
 *  @param  vtClr       Vector of Cluster class objects.
 *  @param  vtClrInfo   Vector of ClusterInfo.
 */
void ConnGrowth::updateSynapsesWeights(const SimulationInfo *sim_info, Layout *layout, vector<Cluster *> &vtClr, vector<ClusterInfo *> &vtClrInfo)
{
    int num_neurons = sim_info->totalNeurons;

    // For now, we just set the weights to equal the areas. We will later
    // scale it and set its sign (when we index and get its sign).
    (*W) = (*area);

    int adjusted = 0;
    int could_have_been_removed = 0; // TODO: use this value
    int removed = 0;
    int added = 0;

    DEBUG(cout << "adjusting weights" << endl;)

    // Scale and add sign to the areas
    // visit each neuron 'a'
    for (int src_neuron = 0; src_neuron < num_neurons; src_neuron++) {

        // destination neurons of each cluster
        for (CLUSTER_INDEX_TYPE iCluster = 0; iCluster < vtClr.size(); iCluster++) {
            AllNeurons *neurons = dynamic_cast<AllNeurons*>(vtClr[iCluster]->m_neurons);
            AllNeuronsProps *pNeuronsProps = neurons->m_pNeuronsProps;
            AllSynapses *synapses = dynamic_cast<AllSynapses*>(vtClr[iCluster]->m_synapses);
            AllSynapsesProps *pSynapsesProps = synapses->m_pSynapsesProps;

            // and each destination neuron 'b'
            int dest_neuron = vtClrInfo[iCluster]->clusterNeuronsBegin;
            int totalClusterNeurons = vtClrInfo[iCluster]->totalClusterNeurons;
            for (int iNeuron = 0; iNeuron < totalClusterNeurons; dest_neuron++, iNeuron++) {
                // visit each synapse at (xa,ya)
                bool connected = false;
                synapseType type = synapses->synType(layout->neuron_type_map, src_neuron, dest_neuron);

                // for each existing synapse
                BGSIZE synapse_counts = pSynapsesProps->synapse_counts[iNeuron];
                BGSIZE synapse_adjusted = 0;
                BGSIZE iSyn = sim_info->maxSynapsesPerNeuron * (iNeuron);
                for (BGSIZE synapse_index = 0; synapse_adjusted < synapse_counts; synapse_index++, iSyn++) {
                    if (pSynapsesProps->in_use[iSyn] == true) {
                        // if there is a synapse between a and b
                        if (pSynapsesProps->sourceNeuronLayoutIndex[iSyn] == src_neuron) {
                            connected = true;
                            adjusted++;
                            // adjust the strength of the synapse or remove
                            // it from the synapse map if it has gone below
                            // zero.
                            if ((*W)(src_neuron, dest_neuron) < 0) {
                                removed++;
                                synapses->eraseSynapse(iNeuron, iSyn);
                            } else {
                                // adjust
                                // SYNAPSE_STRENGTH_ADJUSTMENT is 1.0e-8;
                                pSynapsesProps->W[iSyn] = (*W)(src_neuron, dest_neuron) *
                                    synapses->synSign(type) * AllSynapses::SYNAPSE_STRENGTH_ADJUSTMENT;

                                DEBUG_MID(cout << "weight of rgSynapseMap" <<
                                       "[" <<synapse_index<<"]: " <<
                                       pSynapsesProps->W[iSyn] << endl;);
                            }
                        }
                        synapse_adjusted++;
                    }
                }

                // if not connected and weight(a,b) > 0, add a new synapse from a to b
                if (!connected && ((*W)(src_neuron, dest_neuron) > 0)) {

                    // locate summation point
                    BGFLOAT* sum_point = &( pNeuronsProps->summation_map[iNeuron] );
                    added++;
    
                    BGSIZE iSyn;
                    synapses->addSynapse(iSyn, type, src_neuron, dest_neuron, sum_point, sim_info->deltaT, iNeuron);
                    pSynapsesProps->W[iSyn] = (*W)(src_neuron, dest_neuron) * synapses->synSign(type) * AllSynapses::SYNAPSE_STRENGTH_ADJUSTMENT;

                }
            }
        }
    }

    DEBUG (cout << "adjusted: " << adjusted << endl;)
    DEBUG (cout << "could have been removed (TODO: calculate this): " << could_have_been_removed << endl;)
    DEBUG (cout << "removed: " << removed << endl;)
    DEBUG (cout << "added: " << added << endl << endl << endl;)

}
#endif // !USE_GPU

/*
 *  Creates a recorder class object for the connection.
 *  This function tries to create either Xml recorder or
 *  Hdf5 recorder based on the extension of the file name.
 *
 *  @param  simInfo              SimulationInfo to refer from.
 *  @return Pointer to the recorder class object.
 */
IRecorder* ConnGrowth::createRecorder(const SimulationInfo *simInfo)
{
    // create & init simulation recorder
    IRecorder* simRecorder = NULL;
    if (simInfo->stateOutputFileName.find(".xml") != string::npos) {
        simRecorder = new XmlGrowthRecorder(simInfo);
    }
#ifdef USE_HDF5
    else if (simInfo->stateOutputFileName.find(".h5") != string::npos) {
        simRecorder = new Hdf5GrowthRecorder(simInfo);
    }
#endif // USE_HDF5
    else {
        return NULL;
    }
    if (simRecorder != NULL) {
        simRecorder->init(simInfo->stateOutputFileName);
    }

    return simRecorder;
}
