/*
 * Copyright (C) 2007-2009 Arjan Gijsberts @ Italian Institute of Technology
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 * Transformer prototype to preprocess data samples.
 *
 */

#ifndef __ICUB_ITRANSFORMER__
#define __ICUB_ITRANSFORMER__

#include <yarp/sig/Vector.h>
#include <yarp/os/IConfig.h>
#include <yarp/os/Bottle.h>

#include <string>
#include <sstream>

using namespace yarp::os;
using namespace yarp::sig;

namespace iCub {
namespace learningmachine {


/**
 *
 * A class that provides a preprocessing interface, which can be used
 * to preprocess the data samples that have been received by the 
 * MachineLearner.
 *
 * \see iCub::learningmachine::Standardizer
 * \see iCub::learningmachine::Normalizer
 *
 * \author Arjan Gijsberts
 *
 */

class ITransformer : public IConfig, public Portable {
protected:
    /**
     * The name of this type of transformer.
     */
    std::string name;
    
    /**
     * Number of samples transformed since initialization.
     */
    int sampleCount;
    
    /**
     * Writes a serialization of the transformer into a bottle. This method is 
     * internally referenced by the write method. Typically, subclasses should
     * override this method instead of overriding the write method directly.
     *
     * @param bot the bottle containing the transformer serialization
     */
    virtual void writeBottle(Bottle& bot) = 0;

    /**
     * Unserializes a transformer from a bottle. This method is internally 
     * referenced by the read method. Typically, subclasses should override this 
     * method instead of overriding the read method directly.
     *
     * @param bot the bottle
     */
    virtual void readBottle(Bottle& bot) = 0;

public:
    /**
     * Constructor.
     */
    ITransformer() {
        this->setName("");
        this->sampleCount = 0;
    }

    /**
     * Destructor.
     */
    ~ITransformer() {}

    /**
     * Transforms an input vector.
     *
     * @param input the input vector
     * @param output the output vector
     */
    virtual void transform(const Vector& input, Vector& output) {
        this->sampleCount++;
    }

    /**
     * Asks the transformer to return a string containing statistics on its 
     * operation so far.
     *
     * @return the statistics of the transformer
     */
    virtual std::string getInfo() {
	    std::ostringstream buffer;
	    buffer << "Type: " << this->getName() << ", ";
    	buffer << "Sample Count: " << this->sampleCount << std::endl;
	    return buffer.str();
    }

    /**
     * Forget everything and start over.
     */
    virtual void reset() {
        this->sampleCount = 0;
    }

    /**
     * Retrieve the name of this transformer.
     *
     * @return the name of this transformer
     */
    std::string getName() const {
        return this->name;
    }

    /**
     * Set the name of this transformer.
     *
     * @param name the new name
     */
    void setName(std::string name) {
        this->name = name;
    }

    /**
     * Asks the transformer to return a string containing the list of 
     * configuration options that it supports.
     *
     * @return an informative description of the configuration options
     */
    virtual std::string getConfigHelp() {
        return std::string("Transformer configuration options for '") + 
               this->getName() + "'\n"; 
    }

    /**
     * Asks the transformer to return a new object of its type.
     *
     * @return a fresh instance of the current class
     */
    virtual ITransformer* create() = 0;

    /*
     * Inherited from IConfig.
     */
    virtual bool configure(Searchable& config) { return true; }

    /*
     * Inherited from Portable.
     */
    bool write(ConnectionWriter& connection) {
        Bottle model;
        this->writeBottle(model);
        model.write(connection);
        return true;
    }

    /*
     * Inherited from Portable.
     */
    bool read(ConnectionReader& connection) {
        Bottle model;
        model.read(connection);
        this->readBottle(model);
        return true;
    }

    /**
     * Asks the transformer to return a string serialization.
     *
     * @return a string serialization of the transformer
     */
    virtual std::string toString() {
        Bottle model;
        this->writeBottle(model);
        return model.toString().c_str();
    }

    /**
     * Asks the transformer to initialize from a string serialization.
     *
     * @return true on succes
     */
    virtual bool fromString(const std::string& str) {
        Bottle model(str.c_str()); 
        this->readBottle(model);
        return true;
    }

};

} // learningmachine
} // iCub

#endif
