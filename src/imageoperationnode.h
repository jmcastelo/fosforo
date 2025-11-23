#ifndef IMAGEOPERATIONNODE_H
#define IMAGEOPERATIONNODE_H



#include "imageoperation.h"
#include "inputdata.h"

#include <QUuid>
#include <QMap>
#include <QList>



class ImageOperationNode
{
public:
    ImageOperationNode(QUuid id);
    ImageOperationNode(QUuid id, ImageOperation* operation);
    ImageOperationNode(const ImageOperationNode& opNode);
    ~ImageOperationNode();

    QUuid id() const;
    void setId(QUuid newId);

    ImageOperation* operation() const;

    QMap<QUuid, InputData*> inputs() const;

    void addSeedInput(QUuid id, InputData* data);
    void removeSeedInput(QUuid id);

    void addInput(ImageOperationNode* node, InputData* data);
    void removeInput(ImageOperationNode* node);
    void setInputType(QUuid id, InputType type);
    bool isInput(QUuid id);

    void setInputSeedTexId(QUuid id, GLuint *texId);

    int numInputs();
    int numNonNormalInputs();
    int numOutputs();

    QList<InputData*> inputsList();

    bool isBlitConnected();
    void enableBlit(bool set);

    void addOutput(ImageOperationNode* node);
    void removeOutput(ImageOperationNode* node);

    bool computed() const;
    void setComputed(bool done);
    bool allInputsComputed();

    Number<float>* blendFactor(QUuid id);
    void setBlendFactor(QUuid id, float factor);
    void equalizeBlendFactors();

    void setOperation(ImageOperation* newOperation);

    GLuint* pOutTextureId() const;

private:
    QUuid mId;

    ImageOperation* mOperation;

    QMap<QUuid, ImageOperationNode*> mInputNodes;
    QMap<QUuid, ImageOperationNode*> mOutputNodes;

    QMap<QUuid, InputData*> mInputs;

    bool mComputed = false;
};

#endif // IMAGEOPERATIONNODE_H
