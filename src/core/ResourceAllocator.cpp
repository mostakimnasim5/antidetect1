#include "core/CoreEngine.hpp"

namespace VirtualPhonePro {

struct ResourceAllocator::Impl {
    std::map<std::string, int> allocatedCPU;
    std::map<std::string, int> allocatedMemory;
};

ResourceAllocator::ResourceAllocator() : m_impl(std::make_unique<Impl>()) {}
ResourceAllocator::~ResourceAllocator() = default;

bool ResourceAllocator::allocate(const std::string& vmId, int cpuCores, int memoryMB) {
    m_impl->allocatedCPU[vmId] = cpuCores;
    m_impl->allocatedMemory[vmId] = memoryMB;
    return true;
}

void ResourceAllocator::release(const std::string& vmId) {
    m_impl->allocatedCPU.erase(vmId);
    m_impl->allocatedMemory.erase(vmId);
}

int ResourceAllocator::getTotalCPU() const {
    return std::thread::hardware_concurrency();
}

int ResourceAllocator::getTotalMemoryMB() const {
    return 8192; // Placeholder
}

} // namespace VirtualPhonePro
