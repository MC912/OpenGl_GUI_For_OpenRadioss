#pragma once
#include <string>
#include <memory>

class Model;

class FileManager {
public:
    FileManager(Model* model);
    ~FileManager() = default;
    
    // RAD file operations
    bool LoadRadFile(const std::string& filepath);
    bool SaveRadFile(const std::string& filepath);
    
    // Export functions
    bool ExportToSTL(const std::string& filepath);
    bool ExportToVTK(const std::string& filepath);
    
    // Import functions
    bool ImportFromNastran(const std::string& filepath);
    bool ImportFromAbaqus(const std::string& filepath);
    
    std::string GetCurrentFile() const { return m_CurrentFile; }
    
private:
    Model* m_Model;
    std::string m_CurrentFile;
};
