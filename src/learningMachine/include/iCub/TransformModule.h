/*
 * Copyright (C) 2007-2009 Arjan Gijsberts @ Italian Institute of Technology
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 * Transform module class for wrapping an executable around a Transformer.
 *
 */

#ifndef __ICUB_TRANSFORMMODULE__
#define __ICUB_TRANSFORMMODULE__

#include "iCub/IMachineLearnerModule.h"
#include "iCub/TransformerPortable.h"

using namespace yarp::os;

namespace iCub {
namespace learningmachine {

/**
 * Generic abstract class for transformer based processors.
 *
 * \see iCub::learningmachine::PredictProcessor
 * \see iCub::learningmachine::TrainProcessor
 *
 * \author Arjan Gijsberts
 *
 */
class ITransformProcessor {
protected:
    /**
     * A pointer to a portable transformer.
     */
    TransformerPortable* transformerPortable;

public:
    /**
     * Constructor.
     *
     * @param mp a pointer to a transformer.
     */
    ITransformProcessor(TransformerPortable* tp = (TransformerPortable*) 0) : transformerPortable(tp) {
    }
    
    /**
     * Mutator for the transformer portable.
     *
     * @param tp a pointer to a transformer portable.
     */
    virtual void setTransformerPortable(TransformerPortable* tp) {
        this->transformerPortable = tp;
    }
    
    /**
     * Retrieve the transformer portable wrapper.
     *
     * @return a pointer to the transformer portable
     */
    virtual TransformerPortable* getTransformerPortable() {
        return this->transformerPortable;
    }

    /**
     * Retrieve the transformer.
     *
     * @return a pointer to the transformer.
     */
    virtual ITransformer* getTransformer() {
        return this->getTransformerPortable()->getWrapped();
    }
};

/**
 * Reply processor helper class for predictions. This processor receives 
 * requests for predictions, transforms the samples and relays the request to 
 * the associated port. This architecture allows multiple transformers to be 
 * daisy chained, as long as the chain is terminated by a PredictModule.
 *
 * \see iCub::learningmachine::PredictModule
 * \see iCub::learningmachine::ITransformProcessor
 *
 * \author Arjan Gijsberts
 *
 */
class TransformPredictProcessor : public ITransformProcessor, public PortReader {
protected:
    /**
     * The relay port.
     */
    Port* predict_relay_inout;
    
public:    
    /*
     * Inherited from PortReader.
     */
    virtual bool read(ConnectionReader& connection);

    /**
     * Mutator for the prediction output port.
     *
     * @param t a pointer to a buffered port.
     */
    virtual void setOutputPort(Port* op) {
        this->predict_relay_inout = op;
    }
    
    /**
     * Accessor for the prediction output port.
     *
     * @return a pointer to the output port.
     */
    virtual Port* getOutputPort() {
        return this->predict_relay_inout;
    }

};


/**
 * Port processor helper class for incoming training samples. 
 *
 * \see iCub::learningmachine::TrainModule
 * \see iCub::learningmachine::IMachineProcessor
 *
 * \author Arjan Gijsberts
 *
 */
class TransformTrainProcessor : public ITransformProcessor, public TypedReaderCallback< PortablePair<Vector,Vector> > {
private:
    BufferedPort<PortablePair<Vector,Vector> >* train_out;
public:
    TransformTrainProcessor() {
    }

    /*
     * Inherited from TypedReaderCallback.
     */
    virtual void onRead(PortablePair<Vector,Vector>& input);

    /**
     * Mutator for the training output port.
     *
     * @param t a pointer to a buffered port.
     */
    virtual void setOutputPort(BufferedPort<PortablePair<Vector,Vector> >* op) {
        this->train_out = op;
    }
    
    /**
     * Retrieve the training output port.
     *
     * @return a pointer to the output port.
     */
    virtual BufferedPort<PortablePair<Vector,Vector> >* getOutputPort() {
        return this->train_out;
    }

};




/**
 * A module for transforming vectors. This most common use of this module will 
 * be data preprocessing (e.g. standardization), although it also supports much 
 * more advanced transformations of the data.
 *
 * \author Arjan Gijsberts
 *
 */

class TransformModule : public IMachineLearnerModule {
private:
    /**
     * A pointer to a concrete wrapper around a transformer.
     */
    TransformerPortable* transformerPortable;

    /**
     * Buffered port for the incoming training samples (input and output).
     */
    BufferedPort<PortablePair<Vector,Vector> > train_in;

    /**
     * Buffered port for the outgoing training samples (input and output).
     */
    BufferedPort<PortablePair<Vector,Vector> > train_out;

    /**
     * The processor handling incoming training samples.
     */
    TransformTrainProcessor trainProcessor;

    /**
     * Buffered port for the incoming prediction samples.
     */
    BufferedPort<Vector> predict_inout;

    /**
     * Buffered port for the outgoing prediction samples.
     */
    Port predict_relay_inout;

    /**
     * The processor handling prediction requests.
     */
    TransformPredictProcessor predictProcessor;

    /*
     * Inherited from IMachineLearnerModule.
     */
    void registerAllPorts();

    /*
     * Inherited from IMachineLearnerModule.
     */
    void unregisterAllPorts();

    /*
     * Inherited from IMachineLearnerModule.
     */
    void exitWithHelp(std::string error = "");

public:
    /**
     * Constructor.
     *
     * @param pp the default prefix used for the ports.
     */
    TransformModule(std::string pp = "/lm/transform") : IMachineLearnerModule(pp) {
        this->transformerPortable = new TransformerPortable();
    }
    
    /**
     * Destructor.
     */
    ~TransformModule() {
        delete(this->transformerPortable);
    }

    /*
     * Inherited from IMachineLearnerModule.
     */
    virtual bool open(Searchable& opt);

    /*
     * Inherited from IMachineLearnerModule.
     */
    virtual bool interruptModule();

    /*
     * Inherited from IMachineLearnerModule.
     */
    virtual bool respond(const Bottle& cmd, Bottle& reply);

    /**
     * Retrieve the transformer that is used in this TransformModule.
     *
     * @return a pointer to the transformer
     */
    virtual ITransformer* getTransformer() {
        return this->getTransformerPortable()->getWrapped();
    }


    /**
     * Retrieve the transformer portable.
     *
     * @return a pointer to the transformer portable
     */
    virtual TransformerPortable* getTransformerPortable() {
        return this->transformerPortable;
    }

};

} // learningmachine
} // iCub

#endif
