// SvParser.cpp
#include "parsers/SvParser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QStack>

// Helper: evaluate simple expressions like "8-1"
int evaluateSimpleExpr(const QString& expr) {
    // Only handle "N-1" pattern
    QRegularExpression re(R"(^(\d+)\s*-\s*1$)");
    QRegularExpressionMatch match = re.match(expr.trimmed());
    if (match.hasMatch()) {
        return match.captured(1).toInt() - 1;
    }
    // Otherwise, assume it's a number
    bool ok;
    int val = expr.trimmed().toInt(&ok);
    return ok ? val : 0;
}

// Helper function to parse module from content
static ModuleInfo parseModuleFromContent(const QString& content, const QString& targetModuleName) {
    ModuleInfo mod;

    // Find the specific module in the content
    QString modulePattern = QString(R"(^\s*module\s+%1\s*[\(;])").arg(targetModuleName);
    QRegularExpression reModule(modulePattern, QRegularExpression::MultilineOption);
    QRegularExpressionMatch moduleMatch = reModule.match(content);
    
    if (!moduleMatch.hasMatch()) {
        return mod;  // Module not found
    }
    
    mod.name = targetModuleName;
    
    // Find the position of the module declaration
    int modulePos = moduleMatch.capturedStart();
    
    // Extract the part of content starting from this module
    QString moduleContent = content.mid(modulePos);

    // Extract port list between parentheses (from module declaration)
    QRegularExpression rePorts(R"(\(([\s\S]*?)\))", QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = rePorts.match(moduleContent);
    if (!match.hasMatch()) {
        return mod;
    }

    QString portsStr = match.captured(1).trimmed();

    // Split by commas, respecting nested brackets
    QList<QString> ports;
    int level = 0;
    QString current;
    for (int i = 0; i < portsStr.length(); ++i) {
        QChar c = portsStr[i];
        if (c == '(') level++;
        else if (c == ')') level--;
        else if (c == ',' && level == 0) {
            ports.append(current.trimmed());
            current.clear();
            continue;
        }
        current += c;
    }
    if (!current.trimmed().isEmpty()) {
        ports.append(current.trimmed());
    }

    // Parse each port
    for (const QString& port : ports) {
        if (port.isEmpty() || port.startsWith("//")) continue;

        QString cleanPort = port;
        // Remove comments
        int commentPos = cleanPort.indexOf("//");
        if (commentPos >= 0) {
            cleanPort = cleanPort.left(commentPos).trimmed();
        }

        bool isInput = cleanPort.contains(QRegularExpression(R"(\binput\b)"));
        bool isOutput = cleanPort.contains(QRegularExpression(R"(\boutput\b)"));

        if (!isInput && !isOutput) continue;

        // Remove direction keywords
        QString rest = cleanPort;
        rest.remove(QRegularExpression(R"(\binput\b)"));
        rest.remove(QRegularExpression(R"(\boutput\b)"));
        rest = rest.trimmed();

        // Handle reg/logic/signed/etc.
        rest.remove(QRegularExpression(R"(\breg\b|\blogic\b|\bsigned\b|\bwire\b)"));
        rest = rest.trimmed();

        // Now extract name and optional range
        QRegularExpression rePort(R"(^\s*(?:\[([^\]]+)\]\s*)?(\w+)\s*$)");
        QRegularExpressionMatch portMatch = rePort.match(rest);

        if (portMatch.hasMatch()) {
            QString range = portMatch.captured(1);
            QString name = portMatch.captured(2);

            QString widthStr = "";
            if (!range.isEmpty()) {
                // Parse range like "8-1:0" or "7:0"
                QStringList parts = range.split(':');
                if (parts.size() == 2) {
                    int msb = evaluateSimpleExpr(parts[0]);
                    int lsb = evaluateSimpleExpr(parts[1]);
                    widthStr = "[" + QString::number(msb) + ":" + QString::number(lsb) + "]";
                }
            }

            Port p;
            p.direction = isInput ? Port::Input : Port::Output;
            p.name = name;
            p.width = widthStr;

            if (isInput) {
                mod.inputs.append(p);
            } else {
                mod.outputs.append(p);
            }
        }
    }

    return mod;
}

ModuleInfo SvParser::parseTopModule() {
    return parseModule("top.sv", "");
}

ModuleInfo SvParser::parseModule(const QString& filePath, const QString& moduleName) {
    ModuleInfo mod;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return mod;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // If no module name specified, get the first module in the file
    if (moduleName.isEmpty()) {
        QRegularExpression reModule(R"(^\s*module\s+(\w+))", QRegularExpression::MultilineOption);
        QRegularExpressionMatch match = reModule.match(content);
        if (match.hasMatch()) {
            QString foundModuleName = match.captured(1);
            return parseModuleFromContent(content, foundModuleName);
        }
        return mod;
    }

    // Parse the specific module
    return parseModuleFromContent(content, moduleName);
}

