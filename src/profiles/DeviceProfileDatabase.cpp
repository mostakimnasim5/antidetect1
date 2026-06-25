#include "profiles/DeviceProfileDatabase.hpp"
#include "core/CryptoUtils.hpp"

namespace VirtualPhonePro {

DeviceProfileDatabase::DeviceProfileDatabase() {
    initializeDatabase();
}

DeviceProfileDatabase::~DeviceProfileDatabase() = default;

void DeviceProfileDatabase::initializeDatabase() {
    addSamsungDevices();
    addGoogleDevices();
    addXiaomiDevices();
}

void DeviceProfileDatabase::addSamsungDevices() {
    m_profiles.push_back({
        "samsung-s21-1", "Samsung", "SM-G991B", "o1s", "Galaxy S21",
        "10", "14", "Samsung/o1s/forees:12/SP1A.201217P/ABC123:user/release-keys",
        "G991BXXU", "exynos2100", "o1s", 8, 128, 1080, 2400, 421,
        {"US", "EU", "BD", "IN"}, {"Carrier1", "Carrier2"}
    });
    m_profiles.push_back({
        "samsung-s22-1", "Samsung", "SM-S906B", "tmoq", "Galaxy S22",
        "10", "14", "Samsung/tmoq/t02q:12/SP2A.201217P/DEF456:user/release-keys",
        "S906BXXU", "exynos2200", "t02q", 8, 256, 1080, 2340, 401,
        {"US", "EU"}, {"Carrier3"}
    });
}

void DeviceProfileDatabase::addGoogleDevices() {
    m_profiles.push_back({
        "pixel-7-1", "Google", "Pixel 7", "panther", "Pixel 7",
        "10", "14", "Google/panther/panther:13/TQ2A.230305.001/G0123:user/release-keys",
        "oriole", "qcom", "panther", 8, 128, 1080, 2400, 411,
        {"US", "EU", "BD"}, {"Google Fi"}
    });
}

void DeviceProfileDatabase::addXiaomiDevices() {
    m_profiles.push_back({
        "xiaomi-12-1", "Xiaomi", "2201123G", "psyche", "Xiaomi 12",
        "10", "14", "Xiaomi/psyche/psyche:12/SKQ1.211103.001/X123:user/release-keys",
        "V14.0.2", "qcom", "psyche", 8, 256, 1080, 2400, 419,
        {"CN", "IN", "BD"}, {"China Mobile"}
    });
}

std::vector<DeviceProfile> DeviceProfileDatabase::getAll() const { return m_profiles; }

std::optional<DeviceProfile> DeviceProfileDatabase::getById(const std::string& id) const {
    for (const auto& p : m_profiles) if (p.id == id) return p;
    return std::nullopt;
}

std::vector<DeviceProfile> DeviceProfileDatabase::getByManufacturer(const std::string& m) const {
    std::vector<DeviceProfile> r;
    for (const auto& p : m_profiles) if (p.manufacturer == m) r.push_back(p);
    return r;
}

std::vector<DeviceProfile> DeviceProfileDatabase::getBySeries(const std::string&) const { return {}; }

std::vector<DeviceProfile> DeviceProfileDatabase::getByRegion(const std::string&) const { return {}; }

std::optional<DeviceProfile> DeviceProfileDatabase::getRandom() const {
    if (m_profiles.empty()) return std::nullopt;
    return m_profiles[Crypto::SecureRandomGenerator().generateUint32() % m_profiles.size()];
}

std::vector<DeviceProfile> DeviceProfileDatabase::search(const std::string&) const { return m_profiles; }

size_t DeviceProfileDatabase::count() const { return m_profiles.size(); }

std::vector<std::string> DeviceProfileDatabase::getManufacturers() const {
    return {"Samsung", "Google", "Xiaomi"};
}

std::vector<std::string> DeviceProfileDatabase::getSeries() const { return {}; }

CarrierDatabase::CarrierDatabase() { initializeDatabase(); }
CarrierDatabase::~CarrierDatabase() = default;
void CarrierDatabase::initializeDatabase() {}
std::vector<CarrierProfile> CarrierDatabase::getAll() const { return {}; }
std::optional<CarrierProfile> CarrierDatabase::getById(const std::string&) const { return std::nullopt; }
std::vector<CarrierProfile> CarrierDatabase::getByCountry(const std::string&) const { return {}; }
std::optional<CarrierProfile> CarrierDatabase::getRandom() const { return std::nullopt; }
std::optional<CarrierProfile> CarrierDatabase::getByMCCMNC(const std::string&, const std::string&) const { return std::nullopt; }

RegionDatabase::RegionDatabase() { initializeDatabase(); }
RegionDatabase::~RegionDatabase() = default;
void RegionDatabase::initializeDatabase() {}
std::vector<RegionProfile> RegionDatabase::getAll() const { return {}; }
std::optional<RegionProfile> RegionDatabase::getById(const std::string&) const { return std::nullopt; }
std::vector<RegionProfile> RegionDatabase::getByCountry(const std::string&) const { return {}; }
std::optional<RegionProfile> RegionDatabase::getByTimezone(const std::string&) const { return std::nullopt; }
std::optional<RegionProfile> RegionDatabase::getRandom() const { return std::nullopt; }

} // namespace VirtualPhonePro
