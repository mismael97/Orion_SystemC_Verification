// ComponentPortParser.cpp
#include "parsers/ComponentPortParser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

ModuleInfo ComponentPortParser::parseComponentFile(const QString& filePath)
{
    ModuleInfo info;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open component file for parsing:" << filePath;
        return info;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();
    
    // Extract component ID from the file
    QRegularExpression componentIdRegex(R"(SC_MODULE\s*\(\s*(\w+)\s*\))");
    QRegularExpressionMatch match = componentIdRegex.match(content);
    QString componentId;
    if (match.hasMatch()) {
        componentId = match.captured(1);
    } else {
        qWarning() << "Could not find SC_MODULE declaration in" << filePath;
        return info;
    }
    
    return parseComponentContent(content, componentId);
}

ModuleInfo ComponentPortParser::parseComponentContent(const QString& content, const QString& componentId)
{
    ModuleInfo info;
    info.name = componentId;
    
    // Find the SC_MODULE block
    QRegularExpression moduleRegex(R"(SC_MODULE\s*\(\s*\w+\s*\)\s*\{([\s\S]*?)\};)", 
                                   QRegularExpression::MultilineOption);
    QRegularExpressionMatch moduleMatch = moduleRegex.match(content);
    
    if (!moduleMatch.hasMatch()) {
        qWarning() << "Could not find SC_MODULE block";
        return info;
    }
    
    QString moduleBody = moduleMatch.captured(1);
    
    // Split into lines and parse each line
    QStringList lines = moduleBody.split('\n');
    bool inInputSection = false;
    bool inOutputSection = false;
    
    for (const QString& rawLine : lines) {
        QString line = rawLine.trimmed();
        
        // Skip empty lines and pure comment lines
        if (line.isEmpty() || line.startsWith("//") || line.startsWith("/*")) {
            continue;
        }
        
        // Check for section markers
        if (line.contains("// Input ports") || line.contains("// Ports")) {
            inInputSection = true;
            inOutputSection = false;
            continue;
        } else if (line.contains("// Output ports")) {
            inInputSection = false;
            inOutputSection = true;
            continue;
        } else if (line.contains("SC_CTOR") || line.contains("void ") || line.contains("{")) {
            // End of port declarations
            break;
        }
        
        // Parse port declarations
        // Format: sc_in<type> name; or sc_out<type> name;
        QRegularExpression portRegex(R"((sc_in|sc_out)\s*<\s*([^>]+)\s*>\s+(\w+)\s*;)");
        QRegularExpressionMatch portMatch = portRegex.match(line);
        
        if (portMatch.hasMatch()) {
            QString direction = portMatch.captured(1);
            QString typeStr = portMatch.captured(2).trimmed();
            QString name = portMatch.captured(3);
            
            Port port;
            port.name = name;
            extractPortTypeAndWidth(typeStr, port);
            
            if (direction == "sc_in") {
                port.direction = Port::Input;
                info.inputs.append(port);
            } else if (direction == "sc_out") {
                port.direction = Port::Output;
                info.outputs.append(port);
            }
            
            qDebug() << "Parsed port:" << name << "type:" << typeStr << "direction:" << direction;
        }
    }
    
    qDebug() << "ComponentPortParser: Found" << info.inputs.size() << "inputs and" 
             << info.outputs.size() << "outputs for" << componentId;
    
    return info;
}

bool ComponentPortParser::parsePortLine(const QString& line, Port& port)
{
    // Match: sc_in<type> name; or sc_out<type> name;
    QRegularExpression regex(R"((sc_in|sc_out)\s*<\s*([^>]+)\s*>\s+(\w+)\s*;)");
    QRegularExpressionMatch match = regex.match(line);
    
    if (!match.hasMatch()) {
        return false;
    }
    
    QString direction = match.captured(1);
    QString typeStr = match.captured(2).trimmed();
    QString name = match.captured(3);
    
    port.name = name;
    port.direction = (direction == "sc_in") ? Port::Input : Port::Output;
    
    extractPortTypeAndWidth(typeStr, port);
    
    return true;
}

void ComponentPortParser::extractPortTypeAndWidth(const QString& typeStr, Port& port)
{
    // Handle different SystemC types
    if (typeStr == "bool") {
        port.width = "";  // Single bit
    } else if (typeStr.contains("sc_uint") || typeStr.contains("sc_int")) {
        // Extract width from sc_uint<N> or sc_int<N>
        QRegularExpression widthRegex(R"(sc_u?int\s*<\s*(\d+)\s*>)");
        QRegularExpressionMatch match = widthRegex.match(typeStr);
        if (match.hasMatch()) {
            int width = match.captured(1).toInt();
            port.width = QString("[%1:0]").arg(width - 1);
        }
    } else if (typeStr.contains("sc_biguint") || typeStr.contains("sc_bigint")) {
        // Extract width from sc_biguint<N> or sc_bigint<N>
        QRegularExpression widthRegex(R"(sc_bigu?int\s*<\s*(\d+)\s*>)");
        QRegularExpressionMatch match = widthRegex.match(typeStr);
        if (match.hasMatch()) {
            int width = match.captured(1).toInt();
            port.width = QString("[%1:0]").arg(width - 1);
        }
    } else {
        // Unknown type, leave width empty
        port.width = "";
    }
}
