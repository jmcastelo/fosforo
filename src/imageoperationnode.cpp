


#include "imageoperationnode.h"



ImageOperationNode::ImageOperationNode(QUuid id)
    : mId { id }
{}



ImageOperationNode::ImageOperationNode(QUuid id, ImageOperation* operation)
    : mId { id },
    mOperation { operation }
{}



ImageOperationNode::ImageOperationNode(const ImageOperationNode& opNode)
{
    mId = QUuid::createUuid();
    mOperation = new ImageOperation(*opNode.operation());
}



ImageOperationNode::~ImageOperationNode()
{
    foreach (ImageOperationNode* input, mInputNodes) {
        input->removeOutput(this);
    }

    foreach (ImageOperationNode* output, mOutputNodes) {
        output->removeInput(this);
    }

    // delete mOperation;

    foreach (InputData* data, mInputs) {
        delete data;
    }
}



QUuid ImageOperationNode::id() const
{
    return mId;
}



void ImageOperationNode::setId(QUuid newId)
{
    mId = newId;
}



ImageOperation* ImageOperationNode::operation() const
{
    return mOperation;
}



QMap<QUuid, InputData*> ImageOperationNode::inputs() const
{
    return mInputs;
}



void ImageOperationNode::addSeedInput(QUuid id, InputData* data)
{
    mInputs.insert(id, data);
    mOperation->setInputData(inputsList());
}



void ImageOperationNode::removeSeedInput(QUuid id)
{
    if (mInputs.contains(id))
    {
        delete mInputs.value(id);
        mInputs.remove(id);

        mOperation->setInputData(inputsList());
    }
}



void ImageOperationNode::addInput(ImageOperationNode *node, InputData* data)
{
    mInputNodes.insert(node->id(), node);
    mInputs.insert(node->id(), data);

    mOperation->setInputData(inputsList());
}



void ImageOperationNode::removeInput(ImageOperationNode *node)
{
    mInputNodes.remove(node->id());

    delete mInputs.value(node->id());
    mInputs.remove(node->id());

    mOperation->setInputData(inputsList());
}



void ImageOperationNode::setInputType(QUuid id, InputType type)
{
    if (mInputs.contains(id))
    {
        mInputs[id]->setType(type);

        if (type == InputType::Normal)
        {
            // inputs[id]->setpTextureId(inputNodes.value(id)->operation->pOutTextureId());
            mInputNodes.value(id)->enableBlit(false);
            mInputs[id]->setpTextureId(mInputNodes.value(id)->pOutTextureId());
        }
        else if (type == InputType::Blit)
        {
            // inputs[id]->setpTextureId(inputNodes.value(id)->operation->blitTextureId());
            mInputNodes.value(id)->enableBlit(true);
            mInputs[id]->setpTextureId(mInputNodes.value(id)->pOutTextureId());
        }
    }
}



bool ImageOperationNode::isInput(QUuid id)
{
    return mInputNodes.contains(id);
}



int ImageOperationNode::numInputs()
{
    return mInputs.size();
}



int ImageOperationNode::numNonNormalInputs()
{
    int count = 0;

    foreach (InputData* data, mInputs)
    {
        if (data->type() != InputType::Normal)
        count++;
    }

    return count;
}



int ImageOperationNode::numOutputs()
{
    return mOutputNodes.size();
}



bool ImageOperationNode::isBlitConnected()
{
    foreach (ImageOperationNode* node, mOutputNodes)
    {
        foreach (InputData* input, node->inputsList())
        {
            if (input->type() == InputType::Blit)
                return true;
        }
    }
    return false;
}



void ImageOperationNode::enableBlit(bool set)
{
    mOperation->enableBlit(set);
}



void ImageOperationNode::addOutput(ImageOperationNode *node)
{
    mOutputNodes.insert(node->id(), node);
}



void ImageOperationNode::removeOutput(ImageOperationNode *node)
{
    mOutputNodes.remove(node->id());
}



void ImageOperationNode::setInputSeedTexId(QUuid id, GLuint* texId)
{
    if (mInputs.contains(id))
    {
        mInputs[id]->setpTextureId(texId);
        mOperation->setInputData(inputsList());
    }
}



bool ImageOperationNode::computed() const
{
    return mComputed;
}



bool ImageOperationNode::allInputsComputed()
{
    foreach (ImageOperationNode* node, mInputNodes)
    {
        if(!node->computed() && mInputs.value(node->id())->type() == InputType::Normal)
            return false;
    }

    return true;
}



void ImageOperationNode::setComputed(bool done)
{
    mComputed = done;
}



Number<float>* ImageOperationNode::blendFactor(QUuid id)
{
    return mInputs.value(id)->blendFactor();
}



void ImageOperationNode::setBlendFactor(QUuid id, float factor)
{
    mInputs[id]->setBlendFactor(factor);
    //mOperation->setInputData(inputsList());
}



void ImageOperationNode::equalizeBlendFactors()
{
    int numInputs = mInputs.size();

    foreach (InputData* data, mInputs) {
        data->setBlendFactor(1.0 / numInputs);
    }

    //mOperation->setInputData(inputsList());
}



QList<InputData*> ImageOperationNode::inputsList()
{
    QList<InputData*> inputData;

    foreach(InputData* inData, mInputs) {
        inputData.append(inData);
    }

    return inputData;
}



void ImageOperationNode::setOperation(ImageOperation *newOperation)
{
    // delete mOperation;

    mOperation = newOperation;

    // Set operation's input data

    mOperation->setInputData(inputsList());

    // Set new texture id on output nodes

    foreach (ImageOperationNode* node, mOutputNodes)
    {
        if (node->mInputs.value(mId)->type() == InputType::Normal)
        {
            mOperation->enableBlit(false);
            node->mInputs[mId]->setpTextureId(mOperation->pOutTextureId());
            node->mOperation->setInputData(node->inputsList());

        }
        else if (node->mInputs.value(mId)->type() == InputType::Blit)
        {
            mOperation->enableBlit(true);
            node->mInputs[mId]->setpTextureId(mOperation->pOutTextureId());
            node->mOperation->setInputData(node->inputsList());
        }
    }
}



GLuint* ImageOperationNode::pOutTextureId() const
{
    return mOperation->pOutTextureId();
}
