#include "io/FileManager.h"
#include "io/RadFileReader.h"
#include "io/RadFileWriter.h"
#include "core/Model.h"
#include "utils/Logger.h"

FileManager::FileManager(Model* model) 
    : m_Model(model) {
}

bool FileManager::LoadRadFile(const std::string& filepath) {
    RadFileReader reader(m_Model);
    
    if (reader.Read(filepath)) {
        m_CurrentFile = filepath;
        m_Model->CalculateBounds();
        return true;
    }
    
    return false;
}

bool FileManager::SaveRadFile(const std::string& filepath) {
    RadFileWriter writer(m_Model);
    
    if (writer.Write(filepath)) {
        m_CurrentFile = filepath;
        return true;
    }
    
    return false;
}

bool FileManager::ExportToSTL(const std::string& filepath) {
    // Implementation for STL export
    LOG_INFO("Exporting to STL: {}", filepath);
    // TODO: Implement STL export
    return false;
}

bool FileManager::ExportToVTK(const std::string& filepath) {
    // Implementation for VTK export
    LOG_INFO("Exporting to VTK: {}", filepath);
    // TODO: Implement VTK export
    return false;
}
