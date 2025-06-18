#include <iostream>
#include <Standard_Version.hxx>
#include <vtkVersion.h>

int main()
{
    std::cout << "OpenCASCADE Version: "
              << OCC_VERSION_MAJOR << "."
              << OCC_VERSION_MINOR << "."
              << OCC_VERSION_MAINTENANCE
              << std::endl;
    std::cout << "VTK Version: "
              << vtkVersion::GetVTKMajorVersion() << "."
              << vtkVersion::GetVTKMinorVersion() << "."
              << vtkVersion::GetVTKBuildVersion()
              << std::endl;
    return 0;
}
