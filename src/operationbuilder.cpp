


#include "operationbuilder.h"
#include "imageoperation.h"
#include "operationparser.h"

#include <QToolBar>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QTextCursor>



OperationBuilder::OperationBuilder(ImageOperation *operation, QWidget *parent) :
    QWidget { parent },
    mOperation { operation }
{
    // OpenGL context

    mContext = new QOpenGLContext();
    mContext->setFormat(mOperation->context()->format());
    mContext->setShareContext(mOperation->context());
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(mOperation->context()->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);
    initializeOpenGLFunctions();
    mContext->doneCurrent();

    mProgram = new QOpenGLShaderProgram();

    populateParamContainers();

    // Toolbar

    QToolBar* toolBar = new QToolBar;
    toolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    toolBar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load operation", this, &OperationBuilder::loadOperation);

    toolBar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save operation", this, &OperationBuilder::saveOperation);

    toolBar->addSeparator();

    toolBar->addAction(QIcon(QPixmap(":/icons/letter-v.png")), "Load vertex shader", this, &OperationBuilder::loadVertexShader);

    toolBar->addAction(QIcon(QPixmap(":/icons/letter-f.png")), "Load fragment shader", this, &OperationBuilder::loadFragmentShader);

    toolBar->addAction(QIcon(QPixmap(":/icons/run-build.png")), "Parse shaders", this, &OperationBuilder::parseShaders);

    toolBar->addSeparator();

    setupOpAction = new QAction(QIcon(QPixmap(":/icons/dialog-ok.png")), "Setup operation", this);
    setupOpAction->setEnabled(false);
    connect(setupOpAction, &QAction::triggered, this, &OperationBuilder::setupOperation);

    toolBar->addAction(setupOpAction);

    // Tabs

    QFont fixed = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixed.setStyleHint(QFont::Monospace);
    fixed.setPointSize(9);

    vertexEditor = new QPlainTextEdit;
    vertexEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    vertexEditor->setUndoRedoEnabled(true);
    vertexEditor->setFont(fixed);
    vertexEditor->setPlainText(mOperation->vertexShader());

    connect(vertexEditor, &QPlainTextEdit::textChanged, this, [=, this](){
        setupOpAction->setEnabled(false);
    });

    connect(vertexEditor, &QPlainTextEdit::cursorPositionChanged, this, &OperationBuilder::updateCursorPosLabel);

    fragmentEditor = new QPlainTextEdit;
    fragmentEditor->setLineWrapMode(QPlainTextEdit::NoWrap);
    fragmentEditor->setUndoRedoEnabled(true);
    fragmentEditor->setFont(fixed);
    fragmentEditor->setPlainText(mOperation->fragmentShader());

    connect(fragmentEditor, &QPlainTextEdit::textChanged, this, [=, this](){
        setupOpAction->setEnabled(false);
    });

    connect(fragmentEditor, &QPlainTextEdit::cursorPositionChanged, this, &OperationBuilder::updateCursorPosLabel);

    shadersTabWidget = new QTabWidget;
    shadersTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    shadersTabWidget->addTab(vertexEditor, "Vertex shader");
    shadersTabWidget->addTab(fragmentEditor, "Fragment shader");

    connect(shadersTabWidget, &QTabWidget::currentChanged, this, &OperationBuilder::updateCursorPosLabel);

    // Status bar

    statusBar = new QStatusBar;
    statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    statusBar->setSizeGripEnabled(false);

    QToolBar* viewToolBar = new QToolBar;
    viewToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    viewToolBar->addAction(QIcon(QPixmap(":/icons/zoom-in.png")), "Zoom in", this, [=, this]() {
        int index = shadersTabWidget->currentIndex();
        if (index == 0) {
            vertexEditor->zoomIn(1);
        }
        else if (index == 1) {
            fragmentEditor->zoomIn(1);
        }
    });

    viewToolBar->addAction(QIcon(QPixmap(":/icons/zoom-out.png")), "Zoom out", this, [=, this]() {
        int index = shadersTabWidget->currentIndex();
        if (index == 0) {
            vertexEditor->zoomOut(1);
        }
        else if (index == 1) {
            fragmentEditor->zoomOut(1);
        }
    });

    viewToolBar->addAction(QIcon(QPixmap(":/icons/edit-undo-2.png")), "Undo", this, [=, this]() {
        int index = shadersTabWidget->currentIndex();
        if (index == 0) {
            vertexEditor->undo();
        }
        else if (index == 1) {
            fragmentEditor->undo();
        }
    });

    viewToolBar->addAction(QIcon(QPixmap(":/icons/edit-redo-2.png")), "Redo", this, [=, this]() {
        int index = shadersTabWidget->currentIndex();
        if (index == 0) {
            vertexEditor->redo();
        }
        else if (index == 1) {
            fragmentEditor->redo();
        }
    });

    cursorPosLabel = new QLabel("Row: 0, Col: 0");

    statusBar->insertWidget(0, cursorPosLabel, 1);
    statusBar->insertWidget(1, viewToolBar, 0);

    // Layouts

    QVBoxLayout* shadersLayout = new QVBoxLayout;
    shadersLayout->addWidget(toolBar, 0);
    shadersLayout->addWidget(shadersTabWidget, 1);
    shadersLayout->addWidget(statusBar, 0);

    setLayout(shadersLayout);

    setWindowTitle("Operation Builder");
    resize(500, 500);
}



OperationBuilder::~OperationBuilder()
{
    delete mProgram;
    delete mContext;
    delete mSurface;
}



void OperationBuilder::populateParamContainers()
{
    fParamMap.clear();
    iParamMap.clear();
    uiParamMap.clear();
    mat4ParamMap.clear();
    paramList.clear();

    foreach (auto parameter, mOperation->uniformParameters<float>())
    {
        fParamMap.insert(parameter->uniformName(), parameter);
        paramList.append(parameter->uniformName());
    }

    foreach (auto parameter, mOperation->uniformParameters<int>())
    {
        iParamMap.insert(parameter->uniformName(), parameter);
        paramList.append(parameter->uniformName());
    }

    foreach (auto parameter, mOperation->uniformParameters<unsigned int>())
    {
        uiParamMap.insert(parameter->uniformName(), parameter);
        paramList.append(parameter->uniformName());
    }

    foreach (auto parameter, mOperation->mat4UniformParameters())
    {
        mat4ParamMap.insert(parameter->uniformName(), parameter);
        paramList.append(parameter->uniformName());
    }
}



void OperationBuilder::setOperation(ImageOperation* operation)
{
    mOperation = operation;

    mContext->setFormat(mOperation->context()->format());
    mContext->setShareContext(mOperation->context());

    mSurface->setFormat(mOperation->context()->format());

    vertexEditor->setPlainText(mOperation->vertexShader());
    fragmentEditor->setPlainText(mOperation->fragmentShader());

    populateParamContainers();
}


void OperationBuilder::addMat4UniformParameter(UniformMat4Parameter* parameter)
{
    mat4ParamMap.insert(parameter->uniformName(), parameter);
    mOperation->addMat4UniformParameter(parameter);
}



void OperationBuilder::removeMat4UniformParameter(UniformMat4Parameter* parameter)
{
    if (mat4ParamMap.contains(parameter->uniformName()))
    {
        mat4ParamMap.remove(parameter->uniformName());
        mOperation->removeMat4UniformParameter(parameter);
    }
}



void OperationBuilder::addUniformFloatParameter(UniformParameter<float>* parameter)
{
    fParamMap.insert(parameter->uniformName(), parameter);
    mOperation->addUniformParameter<float>(parameter);
}



void OperationBuilder::removeUniformFloatParameter(UniformParameter<float>* parameter)
{
    if (fParamMap.contains(parameter->uniformName()))
    {
        fParamMap.remove(parameter->uniformName());
        mOperation->removeUniformParameter<float>(parameter);
    }
}



void OperationBuilder::loadOperation()
{
    QString path = QFileDialog::getOpenFileName(this, "Load operation", QDir::currentPath() + "/operations", "Operations (*.op)");

    if (!path.isEmpty())
    {
        // Clear data

        vertexEditor->clear();
        fragmentEditor->clear();

        setupOpAction->setEnabled(false);

        // Parse operation

        OperationParser opParser;
        if (!opParser.read(mOperation, path, false))
        {
            emit operationSetUp();
            return;
        }

        mOperation->linkShaders();
        mOperation->enableUpdate(true);
        mOperation->setAllParameters();

        // Shaders

        vertexShader = mOperation->vertexShader();
        fragmentShader = mOperation->fragmentShader();

        vertexEditor->setPlainText(vertexShader);
        fragmentEditor->setPlainText(fragmentShader);

        // Uniforms

        populateParamContainers();

        setupOpAction->setEnabled(true);

        emit operationSetUp();
    }
}



void OperationBuilder::saveOperation()
{
    QString path = QFileDialog::getSaveFileName(this, "Save operation", QDir::currentPath() + "/operations", "Operations (*.op)");

    if (!path.isEmpty())
    {
        OperationParser opParser;
        opParser.write(mOperation, path, false);
    }
}



void OperationBuilder::loadVertexShader()
{
    QString path = QFileDialog::getOpenFileName(this, "Load vertex shader", QDir::currentPath() + "/shaders", "Vertex shaders (*.vert)");

    if (!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly))
            QMessageBox::information(this, "Error opening file", file.errorString());

        QTextStream in(&file);
        vertexShader = in.readAll();

        vertexEditor->setPlainText(vertexShader);

        file.close();

        shadersTabWidget->setCurrentIndex(0);
    }
}



void OperationBuilder::loadFragmentShader()
{
    QString path = QFileDialog::getOpenFileName(this, "Load fragment shader", QDir::currentPath() + "/shaders", "Fragment shaders (*.frag)");

    if (!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly))
            QMessageBox::information(this, "Error opening file", file.errorString());

        QTextStream in(&file);
        fragmentShader = in.readAll();

        fragmentEditor->setPlainText(fragmentShader);

        file.close();

        shadersTabWidget->setCurrentIndex(1);
    }
}



void OperationBuilder::parseShaders()
{
    mOperation->enableUpdate(false);

    mContext->makeCurrent(mSurface);

    if (linkProgram())
    {
        parseUniforms();

        bool success = true;

        if (!parseSamplers()) {
            QString message = "You may specify a sampler2D and/or a sampler2DArray in the fragment shader, corresponding to the input texture and/or imput array texture.";
            QMessageBox::information(this, "Samplers error", message);
            success = false;
        }

        if (!parseInputAttributes()) {
            QString message = "Exactly two active vec2 input attributes must be specified in the vertex shader, corresponding to the 2D vertex position (location 0) and texture coordinates (location 1).";
            QMessageBox::information(this, "Input attributes error", message);
            success = false;
        }

        setupOpAction->setEnabled(success);
    }

    mContext->doneCurrent();
}



bool OperationBuilder::linkProgram()
{
    mProgram->removeAllShaders();

    if (!mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexEditor->toPlainText()))
    {
        QMessageBox::information(this, "Vertex shader error", mProgram->log());
        return false;
    }
    if (!mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentEditor->toPlainText()))
    {
        QMessageBox::information(this, "Fragment shader error", mProgram->log());
        return false;
    }
    if (!mProgram->link())
    {
        QMessageBox::information(this, "Shader link error", mProgram->log());
        return false;
    }

    return true;
}



bool OperationBuilder::parseInputAttributes()
{
    int numVec2InAttribs = 0;

    mProgram->bind();

    GLint numAttributes;
    glGetProgramInterfaceiv(mProgram->programId(), GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttributes);

    for (GLint index = 0; index < numAttributes; index++)
    {
        QList<GLenum> properties = { GL_TYPE, GL_ARRAY_SIZE };
        QList<GLint> values(properties.size());

        glGetProgramResourceiv(mProgram->programId(), GL_PROGRAM_INPUT, index, properties.size(), properties.data(), values.size(), NULL, values.data());

        int attributeType = values.at(0);
        int numItems = values.at(1);

        if (attributeType == GL_FLOAT_VEC2 && numItems == 1) {
            numVec2InAttribs++;
        }
    }

    mProgram->release();

    return (numVec2InAttribs == 2);
}



bool OperationBuilder::parseSamplers()
{
    int numSampler2D = 0;
    int numSampler2DArray = 0;

    QString sampler2DName;
    QString sampler2DArrayName;

    mProgram->bind();

    GLint numUniforms;
    glGetProgramInterfaceiv(mProgram->programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    for (GLint index = 0; index < numUniforms; index++)
    {
        QList<GLenum> properties = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE };
        QList<GLint> values(properties.size());

        glGetProgramResourceiv(mProgram->programId(), GL_UNIFORM, index, properties.size(), properties.data(), values.size(), NULL, values.data());

        QList<GLchar> name(values.at(0));
        glGetProgramResourceName(mProgram->programId(), GL_UNIFORM, index, name.size(), nullptr, name.data());

        QString uniformName(name.constData());
        int uniformType = values.at(1);
        int numItems = values.at(2);

        if (uniformType == GL_SAMPLER_2D && numItems == 1)
        {
            sampler2DName = uniformName;
            numSampler2D++;
        }
        else if (uniformType == GL_SAMPLER_2D_ARRAY && numItems == 1)
        {
            sampler2DArrayName = uniformName;
            numSampler2DArray++;
        }
    }

    mProgram->release();

    bool success = ((numSampler2D == 1 && numSampler2DArray == 0) || (numSampler2D == 0 && numSampler2DArray == 1) || (numSampler2D == 1 && numSampler2DArray == 1));

    if (success)
    {
        if (numSampler2D == 1) {
            mOperation->setSampler2DName(sampler2DName);
        }
        else {
            mOperation->setSampler2DName("");
        }
        mOperation->setSampler2DAvail(numSampler2D == 1);

        if (numSampler2DArray == 1) {
            mOperation->setSampler2DArrayName(sampler2DArrayName);
        }
        else {
            mOperation->setSampler2DArrayName("");
        }
        mOperation->setSampler2DArrayAvail(numSampler2DArray == 1);
    }
    else
    {
        mOperation->setSampler2DName("");
        mOperation->setSampler2DAvail(false);

        mOperation->setSampler2DArrayName("");
        mOperation->setSampler2DArrayAvail(false);
    }

    return success;
}



void OperationBuilder::parseUniforms()
{
    newParamList.clear();

    mProgram->bind();

    GLint numUniforms;
    glGetProgramInterfaceiv(mProgram->programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    for (GLint index = 0; index < numUniforms; index++)
    {
        QList<GLenum> properties = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE };
        QList<GLint> values(properties.size());

        glGetProgramResourceiv(mProgram->programId(), GL_UNIFORM, index, properties.size(), properties.data(), values.size(), NULL, values.data());

        QList<GLchar> name(values.at(0));
        glGetProgramResourceName(mProgram->programId(), GL_UNIFORM, index, name.size(), nullptr, name.data());

        QString uniformName(name.constData());
        int uniformType = values.at(1);
        int numItems = values.at(2);

        if (!paramList.contains(uniformName))
        {
            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (fParamMap.contains(uniformName) && (fParamMap[uniformName]->uniformType() != uniformType || fParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<float>(fParamMap[uniformName]);
            fParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (iParamMap.contains(uniformName) && (iParamMap[uniformName]->uniformType() != uniformType || iParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<int>(iParamMap[uniformName]);
            iParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (uiParamMap.contains(uniformName) && (uiParamMap[uniformName]->uniformType() != uniformType || uiParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<unsigned int>(uiParamMap[uniformName]);
            uiParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else
        {
            newParamList.append(uniformName);
        }
    }

    mProgram->release();

    foreach (QString name, paramList)
    {
        if (!newParamList.contains(name))
        {
            if (fParamMap.contains(name))
            {
                mOperation->removeUniformParameter<float>(fParamMap[name]);
                fParamMap.remove(name);
            }
            else if (iParamMap.contains(name))
            {
                mOperation->removeUniformParameter<int>(iParamMap[name]);
                iParamMap.remove(name);
            }
            else if (uiParamMap.contains(name))
            {
                mOperation->removeUniformParameter<unsigned int>(uiParamMap[name]);
                uiParamMap.remove(name);
            }
            else if (mat4ParamMap.contains(name))
            {
                mOperation->removeMat4UniformParameter(mat4ParamMap[name]);
                mat4ParamMap.remove(name);
            }
        }
    }

    paramList = newParamList;
}



void OperationBuilder::addUniformParameter(QString uniformName, int uniformType, int numItems)
{
    Parameter* parameter = nullptr;

    if (uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC2 || uniformType == GL_FLOAT_VEC3 || uniformType == GL_FLOAT_VEC4 || uniformType == GL_FLOAT_MAT2 || uniformType == GL_FLOAT_MAT3 || uniformType == GL_FLOAT_MAT4)
    {
        UniformParameter<float>* fParameter = new UniformParameter<float>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        fParamMap.insert(uniformName, fParameter);
        mOperation->addUniformParameter<float>(fParameter);
        parameter = fParameter;
    }
    else if (uniformType == GL_INT || uniformType == GL_INT_VEC2 || uniformType == GL_INT_VEC3 || uniformType == GL_INT_VEC4)
    {
        UniformParameter<int>* iParameter = new UniformParameter<int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        iParamMap.insert(uniformName, iParameter);
        mOperation->addUniformParameter<int>(iParameter);
        parameter = iParameter;
    }
    else if (uniformType == GL_UNSIGNED_INT || uniformType == GL_UNSIGNED_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC3 || uniformType == GL_UNSIGNED_INT_VEC4)
    {
        UniformParameter<unsigned int>* uiParameter = new UniformParameter<unsigned int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        uiParamMap.insert(uniformName, uiParameter);
        mOperation->addUniformParameter<unsigned int>(uiParameter);
        parameter = uiParameter;
    }

    if (parameter)
        newParamList.append(uniformName);
}



void OperationBuilder::setupOperation()
{
    mOperation->setVertexShader(vertexEditor->toPlainText());
    mOperation->setFragmentShader(fragmentEditor->toPlainText());

    if (mOperation->linkShaders())
    {
        emit operationSetUp();
        mOperation->enableUpdate(true);
    }
    else
    {
        QMessageBox::information(this, "GLSL Shaders error", "Could not set up operation due to GLSL shaders error.");
    }
}



void OperationBuilder::updateCursorPosLabel()
{
    int index = shadersTabWidget->currentIndex();

    if (index == 0)
    {
        QTextCursor cursor = vertexEditor->textCursor();
        cursorPosLabel->setText("Row: " + QString::number(cursor.blockNumber()) + ", Col: " + QString::number(cursor.positionInBlock()));
    }
    else if (index == 1)
    {
        QTextCursor cursor = fragmentEditor->textCursor();
        cursorPosLabel->setText("Row: " + QString::number(cursor.blockNumber()) + ", Col: " + QString::number(cursor.positionInBlock()));
    }
}
