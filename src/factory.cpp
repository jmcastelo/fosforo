


#include "factory.h"
#include "operationparser.h"

#include <QDir>
#include <QStringList>



Factory::Factory(VideoInputControl *videoInCtrl, QObject *parent)
    : QObject{parent},
    mVideoInputControl { videoInCtrl }
{}



Factory::~Factory()
{
    qDeleteAll(mAvailOps);
    qDeleteAll(mOperations);
    qDeleteAll(mSeeds);
}


QList<ImageOperation*> Factory::operations()
{
    return mOperations;
}



QList<Seed*> Factory::seeds()
{
    return mSeeds;
}



void Factory::createNewOperation()
{
    QUuid id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation();
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, mMidiEnabled, true, this);
    emit newOpWidgetCreated(widget);
    emit newOpWidgetCreated(id, widget);
    widget->toggleEditMode(true);
}



void Factory::createNewSeed()
{
    QUuid id = QUuid::createUuid();

    Seed* seed = new Seed();
    mSeeds.append(seed);
    emit newSeedCreated(id, seed);

    SeedWidget* widget = new SeedWidget(id, seed, mVideoInputControl);
    emit newSeedWidgetCreated(id, widget);
}



void Factory::addAvailableOperation(int index)
{
    QUuid id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation(*mAvailOps[index]);
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, mMidiEnabled, false, this);
    emit newOpWidgetCreated(widget);
    emit newOpWidgetCreated(id, widget);
    widget->toggleEditMode(false);
}



void Factory::addAvailableOperation(int index, QUuid& id)
{
    id = QUuid::createUuid();

    ImageOperation* operation = new ImageOperation(*mAvailOps[index]);
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, mMidiEnabled, false, this);
    emit newOpWidgetCreated(widget);
    emit newOpWidgetCreated(id, widget);
    widget->toggleEditMode(false);
}



void Factory::addOperation(QUuid id, ImageOperation* operation, QPointF position)
{
    mOperations.append(operation);
    emit newOperationCreated(id, operation);

    OperationWidget* widget = new OperationWidget(id, operation, mMidiEnabled, false, this);
    emit newOpWidgetCreated(widget);
    emit newOpWidgetCreated(id, widget, position);
    widget->toggleEditMode(false);
}



void Factory::addSeed(QUuid id, Seed* seed)
{
    mSeeds.append(seed);
    emit newSeedCreated(id, seed);

    SeedWidget* widget = new SeedWidget(id, seed, mVideoInputControl);
    emit newSeedWidgetCreated(id, widget);
}



ImageOperation* Factory::createReplaceOp(QUuid id, ImageOperation* oldOperation, int index)
{
    ImageOperation* operation = new ImageOperation(*mAvailOps[index], *oldOperation);
    mOperations.append(operation);

    emit replaceOpCreated(id, operation);

    return operation;
}



void Factory::deleteOperation(ImageOperation* operation)
{
    mOperations.removeOne(operation);
    delete operation;
}



void Factory::deleteSeed(Seed* seed)
{
    mSeeds.removeOne(seed);
    delete seed;
}



QStringList Factory::availableOperationNames()
{
    QStringList opNames;

    foreach (ImageOperation* operation, mAvailOps) {
        opNames.append(operation->name());
    }

    return opNames;
}



ImageOperation* Factory::availableOperation(int index)
{
    if (index >= 0 && index < mAvailOps.size()) {
        return mAvailOps[index];
    }

    return nullptr;
}



template<>
Number<float>* Factory::number(QUuid id)
{
    Number<float>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<float>(id);
        if (number) {
            return number;
        }
    }

    return nullptr;
}



template<>
Number<int>* Factory::number(QUuid id)
{
    Number<int>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<int>(id);
        if (number) {
            return number;
        }
    }

    return nullptr;
}



template<>
Number<unsigned int>* Factory::number(QUuid id)
{
    Number<unsigned int>* number = nullptr;

    foreach (ImageOperation* operation, mOperations)
    {
        number = operation->number<unsigned int>(id);
        if (number) {
            return number;
        }
    }

    return nullptr;
}



void Factory::scan()
{
    // Select only operation files

    QStringList filters;
    filters << "*.op";

    // Get operations from the resources system

    QDir opsResDir = QDir(":/operations");

    QStringList resFileNames = opsResDir.entryList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
    QStringList allFileNames = resFileNames;

    // And from the ./operations directory

    QDir opsDir = QDir(QDir::currentPath() + "/operations");

    QStringList fileNames;

    if (opsDir.exists()) {
        fileNames = opsDir.entryList(filters, QDir::Files | QDir::NoSymLinks, QDir::Name);
        allFileNames += fileNames;
    }

    // Add found operations

    qDeleteAll(mAvailOps);
    mAvailOps.clear();

    OperationParser opParser;

    foreach (QString fileName, allFileNames)
    {
        QString filePath = resFileNames.contains(fileName) ? opsResDir.absoluteFilePath(fileName) : opsDir.absoluteFilePath(fileName);

        ImageOperation* operation = new ImageOperation();

        if (opParser.read(operation, filePath, false)) {
            mAvailOps.append(operation);
        }
    }

    // Sort operations alphabetically by name

    std::sort(mAvailOps.begin(), mAvailOps.end(), [] (const ImageOperation* op1, const ImageOperation* op2) {
        return (op1->name() < op2->name());
    });
}



void Factory::clear()
{
    emit cleared();

    qDeleteAll(mOperations);
    mOperations.clear();

    qDeleteAll(mSeeds);
    mSeeds.clear();
}



void Factory::setMidiEnabled(bool enabled)
{
    mMidiEnabled = enabled;
}
