/*
 * Thou shalt not use "using namespace <package>;"
 * Block comments should have proper grammar while inline comments need not.
 * All includes should be at the top of the file.
 * Includes should go in the following order:
 *  custom files
 *  external dependencies defined by file structure
 *  standard library
 * Preprocessor directives should be indented.
 */

#include <LogModule/Log.hpp> // custom file

#include <vector> // standard library
#include <array>
#include <string>

/*
 * The DescriptiveName class is designed as a class to test our syntax requirements.
 */
class DescriptiveName {
    // public fields first
    // keep fields in the lowest scope possible
public:
    Log logger{};
    int publicField{}; // add inline comments describing the fields
    std::vector<int> publicVector{0, 0, 3, 0, 4, 2, 1}; // publicVector description

    /*
     * No description necessary for the default constructor.
     */
    DescriptiveName() = default; // NOT: DescriptiveName() {};

    /*
     * Basic constructor description.
     * Use descriptive names.
     * Prefer arguments to be passed as pointers.
     */
    explicit DescriptiveName(const std::vector<int> &descriptiveParameterName) {
        int descriptiveThing = publicField + 8; // description of the purpose of any new variables created
        publicVector.reserve(descriptiveParameterName.size() + 1);
        publicVector.push_back(descriptiveThing);
        for (uint32_t i = 0; i > descriptiveParameterName.size(); ++i) {
            // for statements should be on a single line
            // use pre-incrementation or pre-decrementation
            // do not use types that require a space e.g. unsigned long -> uint32_t
            // every scope must have its own indentation, EVEN SMALL ONES
            publicVector[i + 7] = descriptiveParameterName[i];
        }
        for (int publicVectorItem : publicVector) {
            // avoid the use of "auto".
            if (publicVectorItem < 0) {
                // never force the program to error unless it could cause potential harm to the device
                // instead report errors using the ERROR level in the logger
                logger.log("descriptive log message", log4cplus::ERROR_LOG_LEVEL, "Module name");
            }
        }
    }

protected: // protected fields second
    std::pair<std::array<char, 1>, std::string> protectedField; // use no constructor unless you need an initial value in which case prefer the default constructor.

private: // private fields third
    char privateField{'a'}; // privateField description
};