// PortEditorDialog.cpp
#include "ui/widgets/PortEditorDialog.h"
#include "graphics/ModuleGraphicsItem.h"
#include <QApplication>
#include <QDebug>

PortEditorDialog::PortEditorDialog(const ModuleInfo& moduleInfo, QWidget* parent)
    : QDialog(parent)
    , m_originalInfo(moduleInfo)
    , m_currentInfo(moduleInfo)
    , m_inputCountSpinBox(nullptr)
    , m_outputCountSpinBox(nullptr)
    , m_previewView(nullptr)
    , m_previewScene(nullptr)
    , m_applyButton(nullptr)
    , m_cancelButton(nullptr)
{
    setWindowTitle("Edit Ports - " + moduleInfo.name);
    setMinimumSize(500, 400);
    setModal(true);
    
    setupUI();
    updatePreview();
}

void PortEditorDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel* titleLabel = new QLabel("Customize Module Ports");
    titleLabel->setStyleSheet("font-size: 16pt; font-weight: bold; margin: 10px; font-family: 'Tajawal';");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Port configuration group
    QGroupBox* configGroup = new QGroupBox("Port Configuration");
    configGroup->setStyleSheet("QGroupBox { font-family: 'Tajawal'; font-weight: bold; }");
    QHBoxLayout* configLayout = new QHBoxLayout(configGroup);
    
    // Input ports
    QVBoxLayout* inputLayout = new QVBoxLayout();
    QLabel* inputLabel = new QLabel("Input Ports:");
    inputLabel->setStyleSheet("font-family: 'Tajawal'; font-weight: bold;");
    inputLayout->addWidget(inputLabel);
    
    m_inputCountSpinBox = new QSpinBox();
    m_inputCountSpinBox->setRange(MIN_PORTS, MAX_PORTS);
    m_inputCountSpinBox->setValue(m_currentInfo.inputs.size());
    m_inputCountSpinBox->setStyleSheet("font-family: 'Tajawal'; font-size: 12pt;");
    connect(m_inputCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PortEditorDialog::onInputCountChanged);
    inputLayout->addWidget(m_inputCountSpinBox);
    
    configLayout->addLayout(inputLayout);
    
    // Output ports
    QVBoxLayout* outputLayout = new QVBoxLayout();
    QLabel* outputLabel = new QLabel("Output Ports:");
    outputLabel->setStyleSheet("font-family: 'Tajawal'; font-weight: bold;");
    outputLayout->addWidget(outputLabel);
    
    m_outputCountSpinBox = new QSpinBox();
    m_outputCountSpinBox->setRange(MIN_PORTS, MAX_PORTS);
    m_outputCountSpinBox->setValue(m_currentInfo.outputs.size());
    m_outputCountSpinBox->setStyleSheet("font-family: 'Tajawal'; font-size: 12pt;");
    connect(m_outputCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PortEditorDialog::onOutputCountChanged);
    outputLayout->addWidget(m_outputCountSpinBox);
    
    configLayout->addLayout(outputLayout);
    
    mainLayout->addWidget(configGroup);
    
    // Preview group
    QGroupBox* previewGroup = new QGroupBox("Preview");
    previewGroup->setStyleSheet("QGroupBox { font-family: 'Tajawal'; font-weight: bold; }");
    QVBoxLayout* previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewScene = new QGraphicsScene(this);
    m_previewView = new QGraphicsView(m_previewScene);
    m_previewView->setMinimumHeight(200);
    m_previewView->setStyleSheet("border: 1px solid #ccc; background-color: #f9f9f9;");
    previewLayout->addWidget(m_previewView);
    
    mainLayout->addWidget(previewGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setStyleSheet("font-family: 'Tajawal'; font-size: 10pt; padding: 8px 16px;");
    connect(m_cancelButton, &QPushButton::clicked, this, &PortEditorDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelButton);
    
    m_applyButton = new QPushButton("Apply");
    m_applyButton->setStyleSheet("font-family: 'Tajawal'; font-size: 10pt; padding: 8px 16px; background-color: #637AB9; color: white; font-weight: bold;");
    connect(m_applyButton, &QPushButton::clicked, this, &PortEditorDialog::onApplyClicked);
    buttonLayout->addWidget(m_applyButton);
    
    mainLayout->addLayout(buttonLayout);
}

void PortEditorDialog::onInputCountChanged(int count)
{
    m_currentInfo.inputs = generatePorts(count, "in");
    updatePreview();
}

void PortEditorDialog::onOutputCountChanged(int count)
{
    m_currentInfo.outputs = generatePorts(count, "out");
    updatePreview();
}

void PortEditorDialog::onApplyClicked()
{
    accept();
}

void PortEditorDialog::onCancelClicked()
{
    reject();
}

void PortEditorDialog::updatePreview()
{
    // Clear the scene
    m_previewScene->clear();
    
    // Create a temporary module graphics item for preview
    ModuleGraphicsItem* previewModule = new ModuleGraphicsItem(m_currentInfo);
    previewModule->setRTLView(false); // Show detailed view for preview
    
    // Add to scene
    m_previewScene->addItem(previewModule);
    
    // Center the module in the view
    m_previewScene->setSceneRect(previewModule->boundingRect());
    m_previewView->fitInView(previewModule->boundingRect(), Qt::KeepAspectRatio);
    
    // Update the module info display
    QString infoText = QString("Module: %1\nInputs: %2\nOutputs: %3")
                      .arg(m_currentInfo.name)
                      .arg(m_currentInfo.inputs.size())
                      .arg(m_currentInfo.outputs.size());
    
    qDebug() << "Preview updated:" << infoText;
}

QList<Port> PortEditorDialog::generatePorts(int count, const QString& prefix) const
{
    QList<Port> ports;
    
    for (int i = 0; i < count; ++i) {
        Port port;
        port.name = QString("%1_%2").arg(prefix).arg(i);
        port.width = QString("[%1:0]").arg(DEFAULT_WIDTH - 1);
        port.direction = (prefix == "in") ? Port::Input : Port::Output;
        ports.append(port);
    }
    
    return ports;
}

ModuleInfo PortEditorDialog::getUpdatedModuleInfo() const
{
    return m_currentInfo;
}
