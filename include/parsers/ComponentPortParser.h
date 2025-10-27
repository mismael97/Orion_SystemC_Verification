// ComponentPortParser.h
#ifndef COMPONENTPORTPARSER_H
#define COMPONENTPORTPARSER_H

#include <QString>
#include <QList>
#include "parsers/SvParser.h"  // Reuse Port structure

/**
 * @brief Parser for extracting port definitions from SystemC component files
 * 
 * Parses .cpp files to extract input and output port definitions.
 * Supports a structured format for easy user modification.
 */
class ComponentPortParser
{
public:
    /**
     * @brief Parse a component .cpp file to extract port information
     * @param filePath Path to the component .cpp file
     * @return ModuleInfo containing parsed inputs and outputs
     */
    static ModuleInfo parseComponentFile(const QString& filePath);
    
    /**
     * @brief Parse port definitions from component code content
     * @param content The text content of the .cpp file
     * @param componentId The component ID (module name)
     * @return ModuleInfo containing parsed inputs and outputs
     */
    static ModuleInfo parseComponentContent(const QString& content, const QString& componentId);
    
private:
    /**
     * @brief Parse a single port declaration line
     * @param line The port declaration line
     * @param port Output Port structure to fill
     * @return true if successfully parsed, false otherwise
     */
    static bool parsePortLine(const QString& line, Port& port);
    
    /**
     * @brief Extract port type and width from sc_in/sc_out declaration
     * @param typeStr The type string (e.g., "sc_uint<8>", "bool")
     * @param port Output Port structure to update with width info
     */
    static void extractPortTypeAndWidth(const QString& typeStr, Port& port);
};

#endif // COMPONENTPORTPARSER_H
