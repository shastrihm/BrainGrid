#include "LIFModel.h"

#include "include/tinyxml.h"

#include "ParseParamError.h"
#include "Util.h"

LIFModel::LIFModel() :
     m_read_params(0)
    ,m_fixed_layout(false)
	,m_conns(NULL)
{

}

LIFModel::~LIFModel()
{
    if (m_conns != NULL) {
        delete m_conns;
        m_conns = NULL;
    }
}

bool LIFModel::readParameters(TiXmlElement *source)
{
    m_read_params = 0;
    try {
        source->Accept(this);
    } catch (ParseParamError &error) {
        error.printError(cerr);
        cerr << endl;
        return false;
    }

    // initial maximum firing rate
    m_growth.maxRate = m_growth.targetRate / m_growth.epsilon;

    return m_read_params == 9;
}

// Visit an element.
bool LIFModel::VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
    if (element.ValueStr().compare("LsmParams") == 0) {
        if (element.QueryFLOATAttribute("frac_EXC", &m_frac_excititory_neurons) != TIXML_SUCCESS) {
            throw ParseParamError("frac_EXC", "Fraction Excitatory missing in XML.");
        }
        if (element.QueryFLOATAttribute("starter_neurons", &m_frac_starter_neurons) != TIXML_SUCCESS) {
            throw ParseParamError("starter_neurons", "Fraction endogenously active missing in XML.");
        }
    }
    
    if (element.ValueStr().compare("Iinject") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Iinject[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Iinject min", "Iinject missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Iinject[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Iinject min", "Iinject missing maximum value in XML.");
        }
        m_read_params++;
    }
    
    if (element.ValueStr().compare("Inoise") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Inoise[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Inoise min", "Inoise missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Inoise[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Inoise max", "Inoise missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("Vthresh") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Vthresh[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Vthresh min", "Vthresh missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Vthresh[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Vthresh max", "Vthresh missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("Vresting") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Vresting[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Vresting min", "Vresting missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Vresting[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Vresting max", "Vresting missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("Vreset") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Vreset[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Vreset min", "Vreset missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Vreset[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Vreset max", "Vreset missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("Vinit") == 0) {
        if (element.QueryFLOATAttribute("min", &m_Vinit[0]) != TIXML_SUCCESS) {
            throw ParseParamError("Vinit min", "Vinit missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_Vinit[1]) != TIXML_SUCCESS) {
            throw ParseParamError("Vinit max", "Vinit missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("starter_vthresh") == 0) {
        if (element.QueryFLOATAttribute("min", &m_starter_Vthresh[0]) != TIXML_SUCCESS) {
            throw ParseParamError("starter_vthresh min", "starter_vthresh missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_starter_Vthresh[1]) != TIXML_SUCCESS) {
            throw ParseParamError("starter_vthresh max", "starter_vthresh missing maximum value in XML.");
        }
        m_read_params++;
    }

    if (element.ValueStr().compare("starter_vreset") == 0) {
        if (element.QueryFLOATAttribute("min", &m_starter_Vreset[0]) != TIXML_SUCCESS) {
            throw ParseParamError("starter_vreset min", "starter_vreset missing minimum value in XML.");
        }
        if (element.QueryFLOATAttribute("max", &m_starter_Vreset[1]) != TIXML_SUCCESS) {
            throw ParseParamError("starter_vreset max", "starter_vreset missing maximum value in XML.");
        }
        m_read_params++;
    }
    
    if (element.ValueStr().compare("GrowthParams") == 0) {
		if (element.QueryFLOATAttribute("epsilon", &m_growth.epsilon) != TIXML_SUCCESS) {
			throw ParseParamError("epsilon", "Growth param 'epsilon' missing in XML.");
		}
		if (element.QueryFLOATAttribute("beta", &m_growth.beta) != TIXML_SUCCESS) {
			throw ParseParamError("beta", "Growth param 'beta' missing in XML.");
		}
		if (element.QueryFLOATAttribute("rho", &m_growth.rho) != TIXML_SUCCESS) {
			throw ParseParamError("rho", "Growth param 'rho' missing in XML.");
		}
		if (element.QueryFLOATAttribute("targetRate", &m_growth.targetRate) != TIXML_SUCCESS) {
			throw ParseParamError("targetRate", "Growth targetRate 'beta' missing in XML.");
		}
		if (element.QueryFLOATAttribute("minRadius", &m_growth.minRadius) != TIXML_SUCCESS) {
			throw ParseParamError("minRadius", "Growth minRadius 'beta' missing in XML.");
		}
		if (element.QueryFLOATAttribute("startRadius", &m_growth.startRadius) != TIXML_SUCCESS) {
			throw ParseParamError("startRadius", "Growth startRadius 'beta' missing in XML.");
		}
    }

    // Parse fixed layout (overrides random layouts)
    if (element.ValueStr().compare("FixedLayout") == 0) {
        m_fixed_layout = true;

        const TiXmlNode* pNode = NULL;
        while ((pNode = element.IterateChildren(pNode)) != NULL) {
            if (strcmp(pNode->Value(), "A") == 0) {
                getValueList(pNode->ToElement()->GetText(), &m_endogenously_active_neuron_list);
            } else if (strcmp(pNode->Value(), "I") == 0) {
                getValueList(pNode->ToElement()->GetText(), &m_inhibitory_neuron_layout);
            }
        }
    }
    
    return true;
}

/*
// Visit an element.
bool VisitExit( const TiXmlElement& element )
{
    return true;
}
*/

// TODO(derek) : comment
void LIFModel::printParameters(ostream &output) const
{
    output << "frac_EXC:" << m_frac_excititory_neurons
    	   << " starter_neurons:" << m_frac_starter_neurons
    	   << endl;
    output << "Interval of constant injected current: ["
    	   << m_Iinject[0] << ", " << m_Iinject[1] << "]"
           << endl;
    output << "Interval of STD of (gaussian) noise current: ["
           << m_Inoise[0] << ", " << m_Inoise[1] << "]"
           << endl;
    output << "Interval of firing threshold: ["
    	   << m_Vthresh[0] << ", "<< m_Vthresh[1] << "]"
           << endl;
    output << "Interval of asymptotic voltage (Vresting): [" << m_Vresting[0]
           << ", " << m_Vresting[1] << "]"
           << endl;
    output << "Interval of reset voltage: [" << m_Vreset[0]
           << ", " << m_Vreset[1] << "]"
           << endl;
    output << "Interval of initial membrance voltage: [" << m_Vinit[0]
           << ", " << m_Vinit[1] << "]"
           << endl;
    output << "Starter firing threshold: [" << m_starter_Vthresh[0]
           << ", " << m_starter_Vthresh[1] << "]"
           << endl;
    output << "Starter reset threshold: [" << m_starter_Vreset[0]
           << ", " << m_starter_Vreset[1] << "]"
           << endl;
    output << "Growth parameters: " << endl
    	   << "\tepsilon: " << m_growth.epsilon
    	   << ", beta: " << m_growth.beta
    	   << ", rho: " << m_growth.rho
    	   << ", targetRate: " << m_growth.targetRate << "," << endl
    	   << "\tminRadius: " << m_growth.minRadius
    	   << ", startRadius: " << m_growth.startRadius
    	   << endl;
    if (m_fixed_layout) {
    	output << "Layout parameters:" << endl;

        cout << "\tEndogenously active neuron positions: ";
        for (size_t i = 0; i < m_endogenously_active_neuron_list.size(); i++) {
        	output << m_endogenously_active_neuron_list[i] << " ";
        }

        cout << endl;

        cout << "\tInhibitory neuron positions: ";
        for (size_t i = 0; i < m_inhibitory_neuron_layout.size(); i++) {
        	output << m_inhibitory_neuron_layout[i] << " ";
        }

        output << endl;
    }
}

/**
 * @return the complete state of the neuron.
 */
string LIFModel::neuronToString(AllNeurons &neurons, const int i) const
{
    stringstream ss;
    ss << "Cm: " << neurons.Cm[i] << " "; // membrane capacitance
    ss << "Rm: " << neurons.Rm[i] << " "; // membrane resistance
    ss << "Vthresh: " << neurons.Vthresh[i] << " "; // if Vm exceeds, Vthresh, a spike is emitted
    ss << "Vrest: " << neurons.Vrest[i] << " "; // the resting membrane voltage
    ss << "Vreset: " << neurons.Vreset[i] << " "; // The voltage to reset Vm to after a spike
    ss << "Vinit: " << neurons.Vinit[i] << endl; // The initial condition for V_m at t=0
    ss << "Trefract: " << neurons.Trefract[i] << " "; // the number of steps in the refractory period
    ss << "Inoise: " << neurons.Inoise[i] << " "; // the stdev of the noise to be added each delta_t
    ss << "Iinject: " << neurons.Iinject[i] << " "; // A constant current to be injected into the LIF neuron
    ss << "nStepsInRefr: " << neurons.nStepsInRefr[i] << endl; // the number of steps left in the refractory period
    ss << "Vm: " << neurons.Vm[i] << " "; // the membrane voltage
    ss << "hasFired: " << neurons.hasFired[i] << " "; // it done fired?
    ss << "C1: " << neurons.C1[i] << " ";
    ss << "C2: " << neurons.C2[i] << " ";
    ss << "I0: " << neurons.I0[i] << " ";
    return ss.str( );
}

void LIFModel::loadMemory(istream& input, AllNeurons &neurons, AllSynapses &synapses, const SimulationInfo &sim_info)
{
    for (int i = 0; i < neurons.size; i++) {
        readNeuron(input, neurons, i);
    }

    int read_synapses_counts[neurons.size];
    for (int i = 0; i < neurons.size; i++) {
        read_synapses_counts[i] = 0;
    }

    // read the synapse data & create synapses
    int synapse_count;
    input >> synapse_count;
    for (int i = 0; i < synapse_count; i++) {
    // read the synapse data and add it to the list
        // create synapse
        Coordinate summation_coord;
        input >> summation_coord.x;
        input >> summation_coord.y;

        int neuron_index = summation_coord.x + summation_coord.y * sim_info.width;
        int synapses_index = read_synapses_counts[neuron_index];

        synapses.summationCoord[neuron_index][synapses_index] = summation_coord;

        readSynapse(input, synapses, neuron_index, synapses_index);

        synapses.summationPoint[neuron_index][synapses_index] = &(neurons.summation_map[summation_coord.x + summation_coord.y * sim_info.width]);

        read_synapses_counts[neuron_index]++;

    }

    // read the radii
    for (int i = 0; i < neurons.size; i++)
        input >> m_conns->radii[i];

    // read the rates
    for (int i = 0; i < neurons.size; i++) {
        input >> m_conns->rates[i];
    }

    for (int i = 0; i < neurons.size; i++) {
        m_conns->radiiHistory(0, i) = m_conns->radii[i]; // NOTE: Radii Used for read.
        m_conns->ratesHistory(0, i) = m_conns->rates[i]; // NOTE: Rates Used for read.
    }
}

void LIFModel::readNeuron(istream &input, AllNeurons &neurons, const int index)
{
    input >> neurons.deltaT[index]; input.ignore();
    input >> neurons.Cm[index]; input.ignore();
    input >> neurons.Rm[index]; input.ignore();
    input >> neurons.Vthresh[index]; input.ignore();
    input >> neurons.Vrest[index]; input.ignore();
    input >> neurons.Vreset[index]; input.ignore();
    input >> neurons.Vinit[index]; input.ignore();
    input >> neurons.Trefract[index]; input.ignore();
    input >> neurons.Inoise[index]; input.ignore();
    input >> neurons.Iinject[index]; input.ignore();
    input >> neurons.Isyn[index]; input.ignore();
    input >> neurons.nStepsInRefr[index]; input.ignore();
    input >> neurons.C1[index]; input.ignore();
    input >> neurons.C2[index]; input.ignore();
    input >> neurons.I0[index]; input.ignore();
    input >> neurons.Vm[index]; input.ignore();
    input >> neurons.hasFired[index]; input.ignore();
    input >> neurons.Tau[index]; input.ignore();
}

void LIFModel::readSynapse(istream &input, AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    // initialize spike queue
    initSpikeQueue(synapses, neuron_index, synapse_index);
    resetSynapse(synapses, neuron_index, synapse_index);

    int synapse_type(0);

    input >> synapses.synapseCoord[neuron_index][synapse_index].x; input.ignore();
    input >> synapses.synapseCoord[neuron_index][synapse_index].y; input.ignore();
    input >> synapses.deltaT[neuron_index][synapse_index]; input.ignore();
    input >> synapses.W[neuron_index][synapse_index]; input.ignore();
    input >> synapses.psr[neuron_index][synapse_index]; input.ignore();
    input >> synapses.decay[neuron_index][synapse_index]; input.ignore();
    input >> synapses.total_delay[neuron_index][synapse_index]; input.ignore();
    input >> synapses.delayQueue[neuron_index][synapse_index][0]; input.ignore();
    input >> synapses.delayIdx[neuron_index][synapse_index]; input.ignore();
    input >> synapses.ldelayQueue[neuron_index][synapse_index]; input.ignore();
    input >> synapse_type; input.ignore();
    input >> synapses.tau[neuron_index][synapse_index]; input.ignore();
    input >> synapses.r[neuron_index][synapse_index]; input.ignore();
    input >> synapses.u[neuron_index][synapse_index]; input.ignore();
    input >> synapses.D[neuron_index][synapse_index]; input.ignore();
    input >> synapses.U[neuron_index][synapse_index]; input.ignore();
    input >> synapses.F[neuron_index][synapse_index]; input.ignore();
    input >> synapses.lastSpike[neuron_index][synapse_index]; input.ignore();

    synapses.type[neuron_index][synapse_index] = get_synapse_type(synapse_type);
}

void LIFModel::initSpikeQueue(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    int &total_delay = synapses.total_delay[neuron_index][synapse_index];
    uint32_t &delayQueue = synapses.delayQueue[neuron_index][synapse_index][0];
    int &delayIdx = synapses.delayIdx[neuron_index][synapse_index];
    int &ldelayQueue = synapses.ldelayQueue[neuron_index][synapse_index];

    size_t size = total_delay / ( sizeof(uint8_t) * 8 ) + 1;
    assert( size <= BYTES_OF_DELAYQUEUE );
    delayQueue = 0;
    delayIdx = 0;
    ldelayQueue = LENGTH_OF_DELAYQUEUE;
}

/**
 * Reset time varying state vars and recompute decay.
 */
void LIFModel::resetSynapse(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    synapses.psr[neuron_index][synapse_index] = 0.0;
    assert( updateDecay(synapses, neuron_index, synapse_index) );
    synapses.u[neuron_index][synapse_index] = DEFAULT_U;
    synapses.r[neuron_index][synapse_index] = 1.0;
    synapses.lastSpike[neuron_index][synapse_index] = ULONG_MAX;
}

bool LIFModel::updateDecay(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    FLOAT &tau = synapses.tau[neuron_index][synapse_index];
    FLOAT &deltaT = synapses.deltaT[neuron_index][synapse_index];
    FLOAT &decay = synapses.decay[neuron_index][synapse_index];

    if (tau > 0) {
        decay = exp( -deltaT / tau );
        return true;
    }
    return false;
}

/**
* Write the simulation memory image
*
* @param os The filestream to write
*/
void LIFModel::saveMemory(ostream& output, AllNeurons &neurons, AllSynapses &synapses, FLOAT simulation_step)
{
    // write the neurons data
    output << neurons.size;
    for (int i = 0; i < neurons.size; i++) {
        writeNeuron(output, neurons, i);
    }

    // write the synapse data
    int synapse_count = 0;
    for (int i = 0; i < neurons.size; i++) {
        synapse_count += synapses.synapse_counts[i];
    }
    output << synapse_count;

    for (int i = 0; i < neurons.size; i++) {
        for (int j = 0; j < synapses.synapse_counts[i]; j++) {
            writeSynapse(output, synapses, i, j);
        }
    }

    // write the final radii
    for (int i = 0; i < neurons.size; i++) {
        output << m_conns->radiiHistory(simulation_step, i);
    }

    // write the final rates
    for (int i = 0; i < neurons.size; i++) {
        output << m_conns->ratesHistory(simulation_step, i);
    }

    output << flush;
}

void LIFModel::writeNeuron(ostream& output, AllNeurons &neurons, const int index) const {
    output << neurons.deltaT[index] << ends;
    output << neurons.Cm[index] << ends;
    output << neurons.Rm[index] << ends;
    output << neurons.Vthresh[index] << ends;
    output << neurons.Vrest[index] << ends;
    output << neurons.Vreset[index] << ends;
    output << neurons.Vinit[index] << ends;
    output << neurons.Trefract[index] << ends;
    output << neurons.Inoise[index] << ends;
    output << neurons.Iinject[index] << ends;
    output << neurons.Isyn[index] << ends;
    output << neurons.nStepsInRefr[index] << ends;
    output << neurons.C1[index] << ends;
    output << neurons.C2[index] << ends;
    output << neurons.I0[index] << ends;
    output << neurons.Vm[index] << ends;
    output << neurons.hasFired[index] << ends;
    output << neurons.Tau[index] << ends;
}

/**
 * Write the synapse data to the stream
 * @param[in] os    The filestream to write
 */
void LIFModel::writeSynapse(ostream& output, AllSynapses &synapses, const int neuron_index, const int synapse_index) const {
    output << synapses.summationCoord[neuron_index][synapse_index].x << ends;
    output << synapses.summationCoord[neuron_index][synapse_index].y << ends;
    output << synapses.synapseCoord[neuron_index][synapse_index].x << ends;
    output << synapses.synapseCoord[neuron_index][synapse_index].y << ends;
    output << synapses.deltaT[neuron_index][synapse_index] << ends;
    output << synapses.W[neuron_index][synapse_index] << ends;
    output << synapses.psr[neuron_index][synapse_index] << ends;
    output << synapses.decay[neuron_index][synapse_index] << ends;
    output << synapses.total_delay[neuron_index][synapse_index] << ends;
    output << synapses.delayQueue[neuron_index][synapse_index] << ends;
    output << synapses.delayIdx[neuron_index][synapse_index] << ends;
    output << synapses.ldelayQueue[neuron_index][synapse_index] << ends;
    output << synapses.type[neuron_index][synapse_index] << ends;
    output << synapses.tau[neuron_index][synapse_index] << ends;
    output << synapses.r[neuron_index][synapse_index] << ends;
    output << synapses.u[neuron_index][synapse_index] << ends;
    output << synapses.D[neuron_index][synapse_index] << ends;
    output << synapses.U[neuron_index][synapse_index] << ends;
    output << synapses.F[neuron_index][synapse_index] << ends;
    output << synapses.lastSpike[neuron_index][synapse_index] << ends;
}

void LIFModel::saveState(ostream &output, const AllNeurons &neurons, const SimulationInfo &sim_info)
{
    output << "   " << m_conns->radiiHistory.toXML("radiiHistory") << endl;
    output << "   " << m_conns->ratesHistory.toXML("ratesHistory") << endl;

#ifdef STORE_SPIKEHISTORY
    get_spike_history(m_conns->burstinessHist, m_conns->spikesHistory);
#endif // STORE_SPIKEHISTORY
    output << "   " << m_conns->burstinessHist.toXML("burstinessHist") << endl;
    output << "   " << m_conns->spikesHistory.toXML("spikesHistory") << endl;

    output << "   " << m_conns->xloc.toXML("xloc") << endl;
    output << "   " << m_conns->yloc.toXML("yloc") << endl;

    //Write Neuron Types
    VectorMatrix neuronTypes("complete", "const", 1, neurons.size, EXC);
    for (int i = 0; i < neurons.size; i++) {
        neuronTypes[i] = neurons.neuron_type_map[i];
    }
    output << "   " << neuronTypes.toXML("neuronTypes") << endl;

    int num_starter_neurons = m_frac_starter_neurons * neurons.size;
    if (num_starter_neurons > 0) {
        VectorMatrix starterNeuronsM("complete", "const", 1, num_starter_neurons);
        getStarterNeuronMatrix(starterNeuronsM, neurons.starter_map, sim_info);
        output << "   " << starterNeuronsM.toXML("starterNeurons") << endl;
    }

    // Write neuron threshold
    // neuron threshold
    VectorMatrix neuronThresh("complete", "const", 1, neurons.size, 0);
    for (int i = 0; i < neurons.size; i++) {
        neuronThresh[i] = neurons.Vthresh[i];
    }
    output << "   " << neuronThresh.toXML("neuronThresh") << endl;
}

/**
* Get starter neuron matrix
*
* @param matrix [out] Starter neuron matrix
*/
void LIFModel::getStarterNeuronMatrix(VectorMatrix& matrix, const bool* starter_map, const SimulationInfo &sim_info)
{
    int cur = 0;
    for (int x = 0; x < sim_info.width; x++) {
        for (int y = 0; y < sim_info.height; y++) {
            if (starter_map[x + y * sim_info.width]) {
                matrix[cur] = x + y * sim_info.height;
                cur++;
            }
        }
    }
}

void LIFModel::createAllNeurons(AllNeurons &neurons, const SimulationInfo &sim_info)
{
    DEBUG(cout << "\nAllocating neurons..." << endl;)

    generateNeuronTypeMap(neurons.neuron_type_map, neurons.size);
    initStarterMap(neurons.starter_map, neurons.size, neurons.neuron_type_map);
    
    /* set their specific types */
    for (int i = 0; i < neurons.size; i++) {
        setNeuronDefaults(neurons, i);

        neurons.Iinject[i] = rng.inRange(m_Iinject[0], m_Iinject[1]);
        neurons.Inoise[i] = rng.inRange(m_Inoise[0], m_Inoise[1]);
        neurons.Vthresh[i] = rng.inRange(m_Vthresh[0], m_Vthresh[1]);
        neurons.Vrest[i] = rng.inRange(m_Vresting[0], m_Vresting[1]);
        neurons.Vreset[i] = rng.inRange(m_Vreset[0], m_Vreset[1]);
        neurons.Vinit[i] = rng.inRange(m_Vinit[0], m_Vinit[1]);
        neurons.deltaT[i] = sim_info.deltaT;
        
        neurons.spike_history[i] = new uint64_t[sim_info.stepDuration * growth.maxFiringRate * sim_info.max_steps];
        // init
        
        
        DEBUG2(cout << "neuron" << i << " as " << neuronTypeToString(neurons.neuron_type_map[i]) << endl;);
        
        switch (neurons.neuron_type_map[i]) {
            case INH:
                DEBUG2(cout << "setting inhibitory neuron: "<< i << endl;)
                // set inhibitory absolute refractory period
                neurons.Trefract[i] = DEFAULT_InhibTrefract; // TODO(derek): move defaults inside model.
                break;

            case EXC:
                DEBUG2(cout << "setting exitory neuron: " << i << endl;)
                // set excitory absolute refractory period
                neurons.Trefract[i] = DEFAULT_ExcitTrefract;
                break;

            default:
                DEBUG2(cout << "ERROR: unknown neuron type: " << m_rgNeuronTypeMap[i] << "@" << i << endl;)
                assert(false);
                break;
        }

        // endogenously_active_neuron_map -> Model State
        if (neurons.starter_map[i]) {
            DEBUG2(cout << "setting endogenously active neuron properties" << endl;)
            // set endogenously active threshold voltage, reset voltage, and refractory period
            neurons.Vthresh[i] = rng.inRange(m_starter_Vthresh[0], m_starter_Vthresh[1]);
            neurons.Vreset[i] = rng.inRange(m_starter_Vreset[0], m_starter_Vreset[1]);
            neurons.Trefract[i] = DEFAULT_ExcitTrefract; // TODO(derek): move defaults inside model.
        }
        DEBUG2(cout << neuronToString(neurons, i) << endl;)
    }
    
    DEBUG(cout << "Done initializing neurons..." << endl;)
}

/**
 *  Creates a randomly ordered distribution with the specified numbers of neuron types.
 *  @returns A flat vector (to map to 2-d [x,y] = [i % m_width, i / m_width])
 */
void LIFModel::generateNeuronTypeMap(neuronType neuron_types[], int num_neurons)
{
    //TODO: m_pInhibitoryNeuronLayout
    
    DEBUG(cout << "\nInitializing neuron type map"<< endl;);
    
    neuronType types[num_neurons];
    for (int i = 0; i < num_neurons; i++) {
        types[i] = EXC;
    }
    
    if (m_fixed_layout) {
        int num_inhibitory_neurons = m_inhibitory_neuron_layout.size();
        int num_excititory_neurons = num_neurons - num_inhibitory_neurons;
        DEBUG(cout << "Total neurons: " << num_neurons << endl;)
        DEBUG(cout << "Inhibitory Neurons: " << num_inhibitory_neurons << endl;)
        DEBUG(cout << "Excitatory Neurons: " << num_excititory_neurons << endl;)
        
        for (int i = 0; i < num_inhibitory_neurons; i++) {
            types[m_inhibitory_neuron_layout.at(i)] = INH;
        }
    } else {
        int num_excititory_neurons = (int) (m_frac_excititory_neurons * num_neurons + 0.5);
        int num_inhibitory_neurons = num_neurons - num_excititory_neurons;
        DEBUG(cout << "Total neurons: " << num_neurons << endl;)
        DEBUG(cout << "Inhibitory Neurons: " << num_inhibitory_neurons << endl;)
        DEBUG(cout << "Excitatory Neurons: " << num_inhibitory_neurons << endl;)
        
        DEBUG(cout << endl << "Randomly selecting inhibitory neurons..." << endl;)
        
        int rg_inhibitory_layout[num_inhibitory_neurons];
        
        for (int i = 0; i < num_inhibitory_neurons; i++) {
            rg_inhibitory_layout[i] = i;
        }
        
        for (int i = num_inhibitory_neurons; i < num_neurons; i++) {
            int j = static_cast<int>(rng() * num_neurons);
            if (j < num_inhibitory_neurons) {
                rg_inhibitory_layout[j] = i;
            }
        }
        
        for (int i = 0; i < num_inhibitory_neurons; i++) {
            types[rg_inhibitory_layout[i]] = INH;
        }
    }
    
    DEBUG(cout << "Done initializing neuron type map" << endl;);
}

/**
 * Populates the starter map.
 * Selects \e numStarter excitory neurons and converts them into starter neurons.
 * @pre m_rgNeuronTypeMap must already be properly initialized
 * @post m_pfStarterMap is populated.
 */
void LIFModel::initStarterMap(bool *starter_map, const int num_neurons, const neuronType neuron_type_map[])
{
    starter_map = new bool[num_neurons];
    for (int i = 0; i < num_neurons; i++) {
        starter_map[i] = false;
    }
    
    if (!starter_flag) {
        for (int i = 0; i < num_neurons; i++) {
            starter_map[i] = false;
        }
        return;
    }
    
    if (m_fixed_layout) {
        size_t num_endogenously_active_neurons = m_endogenously_active_neuron_list.size();
        for (size_t i = 0; i < num_endogenously_active_neurons; i++) {
            starter_map[m_endogenously_active_neuron_list.at(i)] = true;
        }
    } else {
        int num_starter_neurons = (int) (m_frac_starter_neurons * num_neurons + 0.5);
        int starters_allocated = 0;

        DEBUG(cout << "\nRandomly initializing starter map\n";);
        DEBUG(cout << "Total neurons: " << num_neurons << endl;)
        DEBUG(cout << "Starter neurons: " << num_starter_neurons << endl;)

        // randomly set neurons as starters until we've created enough
        while (starters_allocated < num_starter_neurons) {
            // Get a random integer
            int i = static_cast<int>(rng.inRange(0, num_neurons));

            // If the neuron at that index is excitatory and a starter map
            // entry does not already exist, add an entry.
            if (neuron_type_map[i] == EXC && starter_map[i] == false) {
                starter_map[i] = true;
                starters_allocated++;
                DEBUG(cout << "allocated EA neuron at random index [" << i << "]" << endl;);
            }
        }

        DEBUG(cout <<"Done randomly initializing starter map\n\n";)
    }
}

void LIFModel::setNeuronDefaults(AllNeurons &neurons, const int index)
{
    neurons.deltaT[index] = DEFAULT_dt;
    neurons.Cm[index] = DEFAULT_Cm;
    neurons.Rm[index] = DEFAULT_Rm;
    neurons.Vthresh[index] = DEFAULT_Vthresh;
    neurons.Vrest[index] = DEFAULT_Vrest;
    neurons.Vreset[index] = DEFAULT_Vreset;
    neurons.Vinit[index] = DEFAULT_Vreset;
    neurons.Trefract[index] = DEFAULT_Trefract;
    neurons.Inoise[index] = DEFAULT_Inoise;
    neurons.Iinject[index] = DEFAULT_Iinject;
    neurons.Tau[index] = DEFAULT_Cm * DEFAULT_Rm;
}

void LIFModel::setupSim(const int num_neurons, const SimulationInfo &sim_info)
{
    if (m_conns != NULL) {
        delete m_conns;
        m_conns = NULL;
    }

    m_conns = new Connections(num_neurons, m_growth.startRadius, sim_info.stepDuration, sim_info.maxSteps);
    // Initialize neuron locations
    for (int i = 0; i < num_neurons; i++) {
        m_conns->xloc[i] = i % sim_info.width;
        m_conns->yloc[i] = i / sim_info.width;
    }
}

void LIFModel::advance(AllNeurons &neurons, AllSynapses &synapses)
{
    advanceNeurons(neurons, synapses);
    advanceSynapses(neurons.size, synapses);
}

/**
 * Notify outgoing synapses if neuron has fired.
 * @param[in] psi - Pointer to the simulation information.
 */
void LIFModel::advanceNeurons(AllNeurons &neurons, AllSynapses &synapses, const SimulationInfo &sim_info)
{
    // TODO: move this code into a helper class - it's being used in multiple places.
    // For each neuron in the network
    for (int i = neurons.size - 1; i >= 0; --i) {
        // advance neurons
        advanceNeuron(neurons, i);

        // notify outgoing synapses if neuron has fired
        if (neurons.hasFired[i]) {
            DEBUG2(cout << " !! Neuron" << i << "has Fired @ t: " << g_simulationStep * sim_info.deltaT << endl;)

            for (int z = synapses.synapse_counts[i] - 1; z >= 0; --z) {
                preSpikeHit(synapses, i, z);
            }

            neurons.hasFired[i] = false;
        }
    }

#ifdef DUMP_VOLTAGES
    // ouput a row with every voltage level for each time step
    cout << g_simulationStep * psi->deltaT;

    for (int i = 0; i < psi->cNeurons; i++) {
        cout << "\t i: " << i << " " << m_neuronList[i].toStringVm();
    }
    
    cout << endl;
#endif /* DUMP_VOLTAGES */
}

void LIFModel::advanceNeuron(AllNeurons &neurons, const int index)
{
    FLOAT &Vm = neurons.Vm[index];
    FLOAT &Vthresh = neurons.Vthresh[index];
    FLOAT &summationPoint = neurons.summation_map[index];
    FLOAT &I0 = neurons.I0[index];
    FLOAT &Inoise = neurons.Inoise[index];
    FLOAT &C1 = neurons.C1[index];
    FLOAT &C2 = neurons.C2[index];
    int &nStepsInRefr = neurons.nStepsInRefr[index];

    if (nStepsInRefr > 0) {
        // is neuron refractory?
        --nStepsInRefr;
    } else if (Vm >= Vthresh) {
        // should it fire?
        fire(neurons, index);
    } else {
        summationPoint += I0; // add IO
        // add noise
        summationPoint += ((*rgNormrnd[0])() * Inoise); // add noise
        Vm = C1 * Vm + C2 * summationPoint; // decay Vm and add inputs
    }
    // clear synaptic input for next time step
    summationPoint = 0;

    DEBUG2(cout << i << " " << Vm << endl;)
}

void LIFModel::fire(AllNeurons &neurons, const int index) const
{
    // Note that the neuron has fired!
    neurons.hasFired[index] = true;

#ifdef STORE_SPIKEHISTORY
    // record spike time
    neurons.spikeHistory[index] = g_simulationStep;
#endif // STORE_SPIKEHISTORY

    // increment spike count
    neurons.spikeCount[index]++;

    // calculate the number of steps in the absolute refractory period
    neurons.nStepsInRefr[index] = static_cast<int> ( neurons.Trefract[index] / neurons.deltaT[index] + 0.5 );

    // reset to 'Vreset'
    neurons.Vm[index] = neurons.Vreset[index];
}

void LIFModel::preSpikeHit(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    uint32_t *delay_queue = synapses.delayQueue[neuron_index][synapse_index];
    int &delayIdx = synapses.delayIdx[neuron_index][synapse_index];
    int &ldelayQueue = synapses.ldelayQueue[neuron_index][synapse_index];
    int &total_delay = synapses.total_delay[neuron_index][synapse_index];

    // Add to spike queue

    // calculate index where to insert the spike into delayQueue
    int idx = delayIdx +  total_delay;
    if ( idx >= ldelayQueue ) {
        idx -= ldelayQueue;
    }

    // set a spike
    assert( !(delay_queue[0] & (0x1 << idx)) );
    delay_queue[0] |= (0x1 << idx);

    delay_queue = NULL;
}

/**
 * @param[in] psi - Pointer to the simulation information.
 */
void LIFModel::advanceSynapses(const int num_neurons, AllSynapses &synapses)
{
    for (int i = num_neurons - 1; i >= 0; --i) {
        for (int z = synapses.synapse_counts[i] - 1; z >= 0; --z) {
            // Advance Synapse
            advanceSynapse(synapses, i, z);
        }
    }
}

void LIFModel::advanceSynapse(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    uint64_t &lastSpike = synapses.lastSpike[neuron_index][synapse_index];
    FLOAT &deltaT = synapses.deltaT[neuron_index][synapse_index];
    FLOAT &r = synapses.r[neuron_index][synapse_index];
    FLOAT &u = synapses.u[neuron_index][synapse_index];
    FLOAT &D = synapses.D[neuron_index][synapse_index];
    FLOAT &F = synapses.F[neuron_index][synapse_index];
    FLOAT &U = synapses.U[neuron_index][synapse_index];
    FLOAT &W = synapses.W[neuron_index][synapse_index];
    FLOAT &decay = synapses.decay[neuron_index][synapse_index];
    FLOAT &psr = synapses.psr[neuron_index][synapse_index];
    FLOAT &summationPoint = *(synapses.summationPoint[neuron_index][synapse_index]);

    // is an input in the queue?
    if (isSpikeQueue(synapses, neuron_index, synapse_index)) {
        // adjust synapse parameters
        if (lastSpike != ULONG_MAX) {
            FLOAT isi = (g_simulationStep - lastSpike) * deltaT ;
            r = 1 + ( r * ( 1 - u ) - 1 ) * exp( -isi / D );
            u = U + u * ( 1 - U ) * exp( -isi / F );
        }
        psr += ( ( W / decay ) * u * r );// calculate psr
        lastSpike = g_simulationStep; // record the time of the spike
    }

    // decay the post spike response
    psr *= decay;
    // and apply it to the summation point
#ifdef USE_OMP
#pragma omp atomic
#endif
    summationPoint += psr;
#ifdef USE_OMP
    //PAB: atomic above has implied flush (following statement generates error -- can't be member variable)
    //#pragma omp flush (summationPoint)
#endif
}

/**
 * Check if there is an input spike in the queue.
 * @post The queue index is incremented.
 * @return true if there is an input spike event.
 */
bool LIFModel::isSpikeQueue(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    uint32_t *delay_queue = synapses.delayQueue[neuron_index][synapse_index];
    int &delayIdx = synapses.delayIdx[neuron_index][synapse_index];
    int &ldelayQueue = synapses.ldelayQueue[neuron_index][synapse_index];

    bool r = delay_queue[0] & (0x1 << delayIdx);
    delay_queue[0] &= ~(0x1 << delayIdx);
    if ( ++delayIdx >= ldelayQueue ) {
        delayIdx = 0;
    }
    delay_queue = NULL;
    return r;
}

void LIFModel::updateConnections(const int currentStep, AllNeurons &neurons, AllSynapses &synapses, const SimulationInfo &sim_info)
{
	updateHistory(currentStep, sim_info.stepDuration, neurons);
	updateFrontiers(neurons.size);
	updateOverlap(neurons.size);
	updateWeights(neurons.size, neurons, synapses, sim_info);
}

void LIFModel::updateHistory(const int currentStep, FLOAT stepDuration, AllNeurons &neurons)
{
	// Calculate growth cycle firing rate for previous period
	getSpikeCounts(neurons, m_conns->spikeCounts);

	// Calculate growth cycle firing rate for previous period
	for (int i = 0; i < neurons.size; i++) {
		// Calculate firing rate
		m_conns->rates[i] = m_conns->spikeCounts[i] / stepDuration;
		// record firing rate to history matrix
		m_conns->ratesHistory(currentStep, i) = m_conns->rates[i];
	}

	// clear spike count
	clearSpikeCounts(neurons);

	// compute neuron radii change and assign new values
	m_conns->outgrowth = 1.0 - 2.0 / (1.0 + exp((m_growth.epsilon - m_conns->rates / m_growth.maxRate) / m_growth.beta));
	m_conns->deltaR = stepDuration * m_growth.rho * m_conns->outgrowth;
	m_conns->radii += m_conns->deltaR;

	// Cap minimum radius size and record radii to history matrix
	for (int i = 0; i < m_conns->radii.Size(); i++) {
		// TODO: find out why we cap this here.
		if (m_conns->radii[i] < m_growth.minRadius) {
			m_conns->radii[i] = m_growth.minRadius;
		}

		// record radius to history matrix
		m_conns->radiiHistory(currentStep, i) = m_conns->radii[i];

		DEBUG2(cout << "radii[" << i << ":" << m_conns->radii[i] << "]" << endl;);
	}
}

void LIFModel::getSpikeCounts(const AllNeurons &neurons, int *spikeCounts)
{
    for (int i = 0; i < neurons.size; i++) {
        spikeCounts[i] = neurons.spikeCount[i];
    }
}

//! Clear spike count of each neuron.
void LIFModel::clearSpikeCounts(AllNeurons &neurons)
{
    for (int i = 0; i < neurons.size; i++) {
        neurons.spikeCount[i] = 0;
    }
}

void LIFModel::updateFrontiers(const int num_neurons)
{
	DEBUG(cout << "Updating distance between frontiers..." << endl;)
	// Update distance between frontiers
	for (int unit = 0; unit < num_neurons - 1; unit++) {
		for (int i = unit + 1; i < num_neurons; i++) {
			m_conns->delta(unit, i) = m_conns->dist(unit, i) - (m_conns->radii[unit] + m_conns->radii[i]);
			m_conns->delta(i, unit) = m_conns->delta(unit, i);
		}
	}
}

void LIFModel::updateOverlap(FLOAT num_neurons)
{
	DEBUG(cout << "computing areas of overlap" << endl;)

	// Compute areas of overlap; this is only done for overlapping units
	for (int i = 0; i < num_neurons; i++) {
		for (int j = 0; j < num_neurons; j++) {
			m_conns->area(i, j) = 0.0;

			if (m_conns->delta(i, j) < 0) {
				FLOAT lenAB = m_conns->dist(i, j);
				FLOAT r1 = m_conns->radii[i];
				FLOAT r2 = m_conns->radii[j];

				if (lenAB + min(r1, r2) <= max(r1, r2)) {
					m_conns->area(i, j) = pi * min(r1, r2) * min(r1, r2); // Completely overlapping unit
#ifdef LOGFILE
					logFile << "Completely overlapping (i, j, r1, r2, area): "
							<< i << ", " << j << ", " << r1 << ", " << r2 << ", " << *pAarea(i, j) << endl;
#endif // LOGFILE
				} else {
					// Partially overlapping unit
					FLOAT lenAB2 = m_conns->dist2(i, j);
					FLOAT r12 = r1 * r1;
					FLOAT r22 = r2 * r2;

					FLOAT cosCBA = (r22 + lenAB2 - r12) / (2.0 * r2 * lenAB);
					FLOAT angCBA = acos(cosCBA);
					FLOAT angCBD = 2.0 * angCBA;

					FLOAT cosCAB = (r12 + lenAB2 - r22) / (2.0 * r1 * lenAB);
					FLOAT angCAB = acos(cosCAB);
					FLOAT angCAD = 2.0 * angCAB;

					m_conns->area(i, j) = 0.5 * (r22 * (angCBD - sin(angCBD)) + r12 * (angCAD - sin(angCAD)));
				}
			}
		}
	}
}

/**
 * Platform Dependent
 */
void LIFModel::updateWeights(const int num_neurons, AllNeurons &neurons, AllSynapses &synapses, const SimulationInfo &sim_info)
{

	// For now, we just set the weights to equal the areas. We will later
	// scale it and set its sign (when we index and get its sign).
	m_conns->W = m_conns->area;

	int adjusted = 0;
	int could_have_been_removed = 0; // TODO: use this value
	int removed = 0;
	int added = 0;

	DEBUG(cout << "adjusting weights" << endl;)

	// Scale and add sign to the areas
	// visit each neuron 'a'
	for (int src_neuron = 0; src_neuron < num_neurons; src_neuron++) {
		int xa = src_neuron % sim_info.width;
		int ya = src_neuron / sim_info.width;
		Coordinate src_coord(xa, ya);

		// and each destination neuron 'b'
		for (int dest_neuron = 0; dest_neuron < num_neurons; dest_neuron++) {
			int xb = dest_neuron % sim_info.width;
			int yb = dest_neuron / sim_info.width;
			Coordinate dest_coord(xb, yb);

			// visit each synapse at (xa,ya)
			bool connected = false;
			synapseType type = synType(neurons, src_neuron, dest_neuron);

			// for each existing synapse
			for (size_t synapse_index = 0; synapse_index < synapses.synapse_counts[src_neuron]; synapse_index++) {
				// if there is a synapse between a and b
				if (synapses.summationCoord[src_neuron][synapse_index] == dest_coord) {
					connected = true;
					adjusted++;

					// adjust the strength of the synapse or remove
					// it from the synapse map if it has gone below
					// zero.
					if (m_conns->W(src_neuron, dest_neuron) < 0) {
						removed++;
						eraseSynapse(synapses, src_neuron, synapse_index);
						//sim_info->rgSynapseMap[a].erase(sim_info->rgSynapseMap[a].begin() + syn);
					} else {
						// adjust
						// g_synapseStrengthAdjustmentConstant is 1.0e-8;
					    synapses.W[src_neuron][synapse_index] = m_conns->W(src_neuron, dest_neuron) *
							synSign(type) * g_synapseStrengthAdjustmentConstant; // TODO( synSign in HostSim )

						DEBUG2(cout << "weight of rgSynapseMap" <<
							   coordToString(xa, ya)<<"[" <<synapse_index<<"]: " <<
							   sim_info->rgSynapseMap[src_neuron][synapse_index].W << endl;);
					}
				}
			}

			// if not connected and weight(a,b) > 0, add a new synapse from a to b
			if (!connected && (m_conns->W(src_neuron, dest_neuron) > 0)) {

			    // locate summation point
			    FLOAT* sum_point = &( neurons.summation_map[dest_neuron] );
				added++;

				addSynapse(synapses, type, src_neuron, dest_neuron, src_coord, dest_coord, sum_point, sim_info.deltaT);

			}
		}
	}

	DEBUG (cout << "adjusted: " << adjusted << endl;)
	DEBUG (cout << "could have been removed (TODO: calculate this): " << could_have_been_removed << endl;)
	DEBUG (cout << "removed: " << removed << endl;)
	DEBUG (cout << "added: " << added << endl << endl << endl;)
}

/**
* Remove a synapse from the network.
* @param neuron_i   Index of a neuron.
* @param syn_i      Index of a synapse.
*/
void LIFModel::eraseSynapse(AllSynapses &synapses, const int neuron_index, const int synapse_index)
{
    synapses.synapse_counts[neuron_index]--;
    synapses.in_use[neuron_index][synapse_index] = false;
    synapses.summationPoint[neuron_index][synapse_index] = NULL;
}

void LIFModel::addSynapse(AllSynapses &synapses, synapseType type, const int src_neuron, const int dest_neuron, Coordinate &source, Coordinate &dest, FLOAT *sum_point, FLOAT deltaT)
{
    if (synapses.synapse_counts[src_neuron] >= synapses.max_synapses) {
        return; // TODO: ERROR!
    }

    // add it to the list
    int synapse_index;
    for (synapse_index = 0; synapse_index < synapses.max_synapses; synapse_index++) {
        if (!synapses.in_use[src_neuron][synapse_index]) {
            break;
        }
    }

    synapses.synapse_counts[src_neuron]++;

    // create a synapse
    createSynapse(synapses, src_neuron, synapse_index, source, dest, sum_point, deltaT, type );
    synapses.W[src_neuron][synapse_index] = m_conns->W(src_neuron, dest_neuron) * synSign(type) * g_synapseStrengthAdjustmentConstant;
}

void LIFModel::createSynapse(AllSynapses &synapses, const int neuron_index, const int synapse_index, Coordinate source, Coordinate dest, FLOAT *sum_point, FLOAT deltaT, synapseType type)
{
    FLOAT delay;

    synapses.in_use[neuron_index][synapse_index] = true;
    synapses.summationPoint[neuron_index][synapse_index] = sum_point;
    synapses.summationCoord[neuron_index][synapse_index] = dest;
    synapses.synapseCoord[neuron_index][synapse_index] = source;
    synapses.deltaT[neuron_index][synapse_index] = deltaT;
    synapses.W[neuron_index][synapse_index] = 10.0e-9;
    synapses.psr[neuron_index][synapse_index] = 0.0;
    synapses.delayQueue[neuron_index][synapse_index] = 0;
    synapses.ldelayQueue[neuron_index][synapse_index] = LENGTH_OF_DELAYQUEUE;
    synapses.r[neuron_index][synapse_index] = 1.0;
    synapses.u[neuron_index][synapse_index] = 0.4;     // DEFAULT_U
    synapses.lastSpike[neuron_index][synapse_index] = ULONG_MAX;
    synapses.type[neuron_index][synapse_index] = type;

    FLOAT tau;
    switch ( type ) {
        case II:
            tau = 6e-3;
            delay = 0.8e-3;
            break;
        case IE:
            tau = 6e-3;
            delay = 0.8e-3;
            break;
        case EI:
            tau = 3e-3;
            delay = 0.8e-3;
            break;
        case EE:
            tau = 3e-3;
            delay = 1.5e-3;
            break;
        case STYPE_UNDEF:
            // TODO: error;
            break;
    }

    synapses.tau[neuron_index][synapse_index] = tau;
    synapses.total_delay[neuron_index][synapse_index] = static_cast<int>( delay / deltaT ) + 1;
    synapses.decay[neuron_index][synapse_index] = exp( -deltaT / tau );
}

synapseType LIFModel::synType(AllNeurons &neurons, Coordinate src_coord, Coordinate dest_coord, const int width)
{
    return synType(neurons, src_coord.x + src_coord.y * width, dest_coord.x + dest_coord.y * width);
}

synapseType LIFModel::synapseOrdinalToType(const int type_ordinal)
{
    switch (type_ordinal) {
        case 0:
            return II;
        case 1:
            return IE;
        case 2:
            return EI;
        case 3:
            return EE;
        default:
            return STYPE_UNDEF;
    }
}

/**
* Returns the type of synapse at the given coordinates
* @param rgNeuronTypeMap_d  The neuron type map (INH, EXC).
* @param ax Source coordinate(x).
* @param ay Source coordinate(y).
* @param bx Destination coordinate(x).
* @param by Destination coordinate(y).
* @param width  Width of neuron map (assumes square).
* @return type of synapse at the given coordinate or -1 on error
*/
synapseType LIFModel::synType(AllNeurons &neurons, const int src_neuron, const int dest_neuron)
{
    if ( neurons.neuron_type_map[src_neuron] == INH && neurons.neuron_type_map[dest_neuron] == INH )
        return II;
    else if ( neurons.neuron_type_map[src_neuron] == INH && neurons.neuron_type_map[dest_neuron] == EXC )
        return IE;
    else if ( neurons.neuron_type_map[src_neuron] == EXC && neurons.neuron_type_map[dest_neuron] == INH )
        return EI;
    else if ( neurons.neuron_type_map[src_neuron] == EXC && neurons.neuron_type_map[dest_neuron] == EXC )
        return EE;

    return STYPE_UNDEF;
}

/**
* Return 1 if originating neuron is excitatory, -1 otherwise.
* @param[in] t  synapseType I to I, I to E, E to I, or E to E
* @return 1 or -1
*/
int LIFModel::synSign(const synapseType type)
{
    switch ( type ) {
        case II:
        case IE:
            return -1;
        case EI:
        case EE:
            return 1;
    }

    return 0;
}

void LIFModel::cleanupSim(AllNeurons &neurons, SimulationInfo &sim_info)
{
    // output spikes
    for (int i = 0; i < sim_info.width; i++) {
        for (int j = 0; j < sim_info.height; j++) {
            vector<uint64_t>* pSpikes = m_neuronList[i + j * sim_info.width]->getSpikes();

            DEBUG2 (cout << endl << coordToString(i, j) << endl);

            for (unsigned int i = 0; i < neurons.size(); i++) {
                DEBUG2 (cout << i << " ");
                int idx1 = (*pSpikes)[i] * sim_info.deltaT;
                m_conns->burstinessHist[idx1] = burstinessHist[idx1] + 1.0;
                int idx2 = (*pSpikes)[i] * sim_info.deltaT * 100;
                m_conns->spikesHistory[idx2] = spikesHistory[idx2] + 1.0;
            }
        }
    }
}

void LIFModel::logSimStep(const AllNeurons &neurons, const AllSynapses &synapses, const SimulationInfo &sim_info) const
{
    cout << "format:\ntype,radius,firing rate" << endl;

    for (int y = 0; y < sim_info.height; y++) {
        stringstream ss;
        ss << fixed;
        ss.precision(1);

        for (int x = 0; x < sim_info.width; x++) {
            switch (neurons.neuron_type_map[x + y * sim_info.width]) {
                case EXC:
                    if (neurons.starter_map[x + y * sim_info.width])
                        ss << "s";
                    else
                        ss << "e";
                    break;
                case INH:
                    ss << "i";
                    break;
                case NTYPE_UNDEF:
                    assert(false);
                    break;
            }

            ss << " " << m_conns->radii[x + y * sim_info.width];
            ss << " " << m_conns->radii[x + y * sim_info.width];

            if (x + 1 < sim_info.width) {
                ss.width(2);
                ss << "|";
                ss.width(2);
            }
        }

        ss << endl;

        for (int i = ss.str().length() - 1; i >= 0; i--) {
            ss << "_";
        }

        ss << endl;
        cout << ss.str();
    }
}

const string LIFModel::Connections::MATRIX_TYPE = "complete";
const string LIFModel::Connections::MATRIX_INIT = "const";

LIFModel::Connections::Connections(const int num_neurons, const FLOAT start_radius, const FLOAT growthStepDuration, const FLOAT maxGrowthSteps) :
    xloc(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons),
    yloc(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons),
	W(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons, 0),
	radii(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons, start_radius),
	rates(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons, 0),
	dist2(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons),
	delta(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons),
	dist(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons),
	area(MATRIX_TYPE, MATRIX_INIT, num_neurons, num_neurons, 0),
	outgrowth(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons),
	deltaR(MATRIX_TYPE, MATRIX_INIT, 1, num_neurons),
    radiiHistory(MATRIX_TYPE, MATRIX_INIT, static_cast<int>(maxGrowthSteps + 1), num_neurons),
    ratesHistory(MATRIX_TYPE, MATRIX_INIT, static_cast<int>(maxGrowthSteps + 1), num_neurons),
    burstinessHist(MATRIX_TYPE, MATRIX_INIT, 1, (int)(growthStepDuration * maxGrowthSteps), 0),
    spikesHistory(MATRIX_TYPE, MATRIX_INIT, 1, (int)(growthStepDuration * maxGrowthSteps), 0)
{
    // Init radii and rates history matrices with current radii and rates
    for (int i = 0; i < num_neurons; i++) {
        radiiHistory(0, i) = start_radius;
        ratesHistory(0, i) = 0;
    }
}