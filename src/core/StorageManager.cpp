#include "core/CoreEngine.hpp"

namespace VirtualPhonePro {

struct StorageManager::Impl {
    std::map<std::string, std::string> configs;
};

StorageManager::StorageManager() : m_impl(std::make_unique<Impl>()) {}
StorageManager::~StorageManager() = default;

bool StorageManager::createVMStorage(const std::string& vmId, int sizeMB) {
    return true;
}

bool StorageManager::deleteVMStorage(const std::string& vmId) {
    return true;
}

void StorageManager::setConfig(const std::string& key, const std::string& value) {
    m_impl->configs[key] = value;
}

std::string StorageManager::getConfig(const std::string& key) {
    auto it = m_impl->configs.find(key);
    return it != m_impl->configs.end() ? it->second : "";
}

} // namespace VirtualPhonePro
