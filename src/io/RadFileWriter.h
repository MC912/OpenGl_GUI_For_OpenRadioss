#pragma once
#include <string>
#include <fstream>

class Model;

class RadFileWriter {
public:
    RadFileWriter(Model* model);
    ~RadFileWriter() = default;
    
    bool Write(const std::string& filepath);
    
private:
    void WriteHeader();
    void WriteNodes();
    void WriteElements();
    void WriteMaterials();
    void WriteProperties();
    void WriteBoundaryConditions();
    void WriteLoads();
    
    void WriteComment(const std::string& comment);
    
private:
    Model* m_Model;
    std::ofstream m_File;
};
