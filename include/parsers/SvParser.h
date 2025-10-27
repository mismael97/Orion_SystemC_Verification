/**
 * @file SvParser.h
 * @brief SystemVerilog parser for extracting module information and port definitions
 * 
 * This module provides functionality to parse SystemVerilog (.sv) files and extract
 * module information including module names, input/output ports, and port widths.
 * It supports complex port declarations with ranges, directions, and data types.
 * 
 * Key Features:
 * - Parse SystemVerilog module declarations
 * - Extract input and output port information
 * - Handle port width specifications (e.g., [7:0], [31:0])
 * - Support for complex expressions in port ranges
 * - Backward compatibility with legacy parsing methods
 * 
 * Supported Port Formats:
 * - input logic clk
 * - output logic [7:0] data
 * - input logic [8-1:0] address
 * - output reg [31:0] result
 * 
 * @author SCV Project Team
 * @version 1.0
 * @date 2024
 */

#ifndef SVPARSER_H
#define SVPARSER_H

#include <QString>
#include <QList>

/**
 * @struct Port
 * @brief Represents a SystemVerilog port with direction, name, and width
 * 
 * This structure encapsulates all the information about a SystemVerilog port
 * including its direction (input/output), name, and width specification.
 */
struct Port {
    /**
     * @enum Direction
     * @brief Port direction enumeration
     */
    enum Direction { 
        Input,  ///< Input port
        Output  ///< Output port
    };
    
    Direction direction;  ///< Port direction (input or output)
    QString name;         ///< Port name
    QString width;        ///< Port width specification (e.g., "[7:0]", "[2:0]", or empty for 1-bit)
};

/**
 * @struct ModuleInfo
 * @brief Complete module information extracted from SystemVerilog files
 * 
 * This structure contains all the parsed information about a SystemVerilog module
 * including its name and lists of input and output ports.
 */
struct ModuleInfo {
    QString name;              ///< Module name
    QList<Port> inputs;        ///< List of input ports
    QList<Port> outputs;       ///< List of output ports
};

/**
 * @class SvParser
 * @brief Static parser class for SystemVerilog files
 * 
 * This class provides static methods to parse SystemVerilog files and extract
 * module information. It handles complex port declarations and supports various
 * SystemVerilog syntax patterns.
 */
class SvParser {
public:
    /**
     * @brief Parse the top module (backward compatibility method)
     * @return ModuleInfo structure containing parsed module information
     * 
     * Parses the "top.sv" file and extracts the first module found.
     * This method is provided for backward compatibility.
     */
    static ModuleInfo parseTopModule();
    
    /**
     * @brief Parse a specific module from a SystemVerilog file
     * @param filePath Path to the SystemVerilog file
     * @param moduleName Name of the module to parse (empty string to parse first module)
     * @return ModuleInfo structure containing parsed module information
     * 
     * Parses the specified SystemVerilog file and extracts information about
     * the specified module. If moduleName is empty, parses the first module
     * found in the file.
     * 
     * Supported features:
     * - Complex port declarations with ranges
     * - Multiple data types (logic, reg, wire, etc.)
     * - Port ranges with expressions (e.g., [8-1:0])
     * - Comments in port lists
     * - Nested parentheses in port declarations
     */
    static ModuleInfo parseModule(const QString& filePath, const QString& moduleName);
};

#endif // SVPARSER_H

