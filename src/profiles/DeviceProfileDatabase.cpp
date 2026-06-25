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
    addOnePlusDevices();
}

void DeviceProfileDatabase::addSamsungDevices() {
    // Galaxy S Series
    m_profiles.push_back({"samsung-s21-1", "Samsung", "SM-G991B", "o1s", "Galaxy S21 5G",
        "11", "14", "Samsung/o1s/forees:13/RP1A.200720.012/G991BXXU9FVA5:user/release-keys",
        "G991BXXU9FVA5", "exynos2100", "o1s", 8, 128, 1080, 2400, 421,
        {"US", "EU", "BD", "IN"}, {"AT&T", "T-Mobile"}});
    
    m_profiles.push_back({"samsung-s21ultra-1", "Samsung", "SM-G998B", "o2", "Galaxy S21 Ultra 5G",
        "11", "14", "Samsung/o2/o2s:13/RP1A.200720.012/G998BXXU9FVA5:user/release-keys",
        "G998BXXU9FVA5", "exynos2100", "o2", 12, 256, 1440, 3200, 515,
        {"US", "EU", "BD"}, {"Verizon", "AT&T"}});
    
    m_profiles.push_back({"samsung-s22-1", "Samsung", "SM-S906B", "tmoq", "Galaxy S22+",
        "12", "14", "Samsung/tmoq/t02q:13/RP1A.200720.012/S906BXXS2CVA5:user/release-keys",
        "S906BXXS2CVA5", "exynos2200", "t02q", 8, 256, 1080, 2340, 401,
        {"US", "EU"}, {"T-Mobile"}});
    
    m_profiles.push_back({"samsung-s22ultra-1", "Samsung", "SM-S908B", "r0q", "Galaxy S22 Ultra",
        "12", "14", "Samsung/r0q/r0qs:13/RP1A.200720.012/S908BXXS2CVA5:user/release-keys",
        "S908BXXS2CVA5", "exynos2200", "r0q", 12, 512, 1440, 3080, 500,
        {"US", "EU", "BD"}, {"Verizon"}});
    
    m_profiles.push_back({"samsung-s23-1", "Samsung", "SM-S911B", "zuma", "Galaxy S23",
        "13", "14", "Samsung/zuma/zuma:13/TP1A.200720.012/S911BXXU2BWA3:user/release-keys",
        "S911BXXU2BWA3", "snapdragon8gen2", "zuma", 8, 256, 1080, 2340, 422,
        {"US", "EU", "BD"}, {"AT&T", "T-Mobile"}});
    
    m_profiles.push_back({"samsung-s23ultra-1", "Samsung", "SM-S918B", "zumaq", "Galaxy S23 Ultra",
        "13", "14", "Samsung/zumaq/zumaq:13/TP1A.200720.012/S918BXXU2BWA3:user/release-keys",
        "S918BXXU2BWA3", "snapdragon8gen2", "zumaq", 12, 512, 1440, 3080, 501,
        {"US", "EU", "BD"}, {"Verizon"}});
    
    m_profiles.push_back({"samsung-s24ultra-1", "Samsung", "SM-S928B", "樱花", "Galaxy S24 Ultra",
        "14", "14", "Samsung/淡coconut/淡coconut:14/UP1A.231005.007/S928BXXU1BXAG:user/release-keys",
        "S928BXXU1BXAG", "snapdragon8gen3", "淡coconut", 12, 512, 1440, 3120, 505,
        {"US", "EU", "BD", "KR"}, {"SK Telecom"}});
    
    // Galaxy A Series
    m_profiles.push_back({"samsung-a54-1", "Samsung", "SM-A546E", "a54x", "Galaxy A54 5G",
        "13", "14", "Samsung/a54x/a54x:13/TP1A.200720.012/A546EXXS4CWA2:user/release-keys",
        "A546EXXS4CWA2", "exynos1380", "a54x", 8, 128, 1080, 2340, 403,
        {"BD", "IN", "EU"}, {"Grameenphone", "Robi"}});
    
    m_profiles.push_back({"samsung-a34-1", "Samsung", "SM-A346E", "a34x", "Galaxy A34 5G",
        "13", "14", "Samsung/a34x/a34x:13/TP1A.200720.012/A346EXXS4BWA2:user/release-keys",
        "A346EXXS4BWA2", "mediTek1380", "a34x", 8, 128, 1080, 2400, 411,
        {"BD", "IN"}, {"Banglalion", "AirTel"}});
    
    // Galaxy Z Fold/Flip
    m_profiles.push_back({"samsung-zfold5-1", "Samsung", "SM-F946B", "q5q", "Galaxy Z Fold5",
        "13", "14", "Samsung/q5q/q5q:13/TP1A.200720.012/F946BXXS2CXAF:user/release-keys",
        "F946BXXS2CXAF", "snapdragon8gen2", "q5q", 12, 512, 904, 2176, 373,
        {"US", "EU", "BD"}, {"AT&T"}});
}

void DeviceProfileDatabase::addGoogleDevices() {
    m_profiles.push_back({"pixel-7-1", "Google", "Pixel 7", "panther", "Pixel 7",
        "12", "14", "Google/panther/panther:14/TP1A.220624.014/AD1A.230626.001:user/release-keys",
        "oriole", "tensor g2", "panther", 8, 128, 1080, 2400, 411,
        {"US", "EU", "BD"}, {"Google Fi", "AT&T"}});
    
    m_profiles.push_back({"pixel-7pro-1", "Google", "Pixel 7 Pro", "cheetah", "Pixel 7 Pro",
        "12", "14", "Google/cheetah/cheetah:14/TP1A.220624.014/AD1A.230626.001:user/release-keys",
        "cheetah", "tensor g2", "cheetah", 12, 256, 1440, 3120, 512,
        {"US", "EU", "BD"}, {"T-Mobile"}});
    
    m_profiles.push_back({"pixel-8-1", "Google", "Pixel 8", "shiba", "Pixel 8",
        "13", "14", "Google/shiba/shiba:14/UP1A.231005.007/TP1A.230620.001:user/release-keys",
        "shiba", "tensor g3", "shiba", 8, 256, 1080, 2400, 424,
        {"US", "EU", "BD"}, {"Google Fi"}});
    
    m_profiles.push_back({"pixel-8pro-1", "Google", "Pixel 8 Pro", "husky", "Pixel 8 Pro",
        "13", "14", "Google/husky/husky:14/UP1A.231005.007/TP1A.230620.001:user/release-keys",
        "husky", "tensor g3", "husky", 12, 512, 1344, 2992, 489,
        {"US", "EU", "BD"}, {"Verizon"}});
    
    m_profiles.push_back({"pixel-8a-1", "Google", "Pixel 8a", "akita", "Pixel 8a",
        "13", "14", "Google/akita/akita:14/UP1A.231005.007/TP1A.230620.001:user/release-keys",
        "akita", "tensor g3", "akita", 8, 256, 1080, 2400, 430,
        {"US", "EU"}, {"Google Fi"}});
    
    m_profiles.push_back({"pixel-6a-1", "Google", "Pixel 6a", "bluejay", "Pixel 6a",
        "12", "13", "Google/bluejay/bluejay:13/TP1A.220624.014/SD1A.210817.037:user/release-keys",
        "bluejay", "tensor", "bluejay", 6, 128, 1080, 2400, 429,
        {"US", "EU", "BD"}, {"T-Mobile"}});
}

void DeviceProfileDatabase::addXiaomiDevices() {
    m_profiles.push_back({"xiaomi-14-1", "Xiaomi", "23127PN0CC", "houji", "Xiaomi 14",
        "14", "14", "Xiaomi/houji/houji:14/V816.20241219.0/S0000.231103.0:user/release-keys",
        "V816.20241219.0", "snapdragon8gen3", "houji", 12, 512, 1200, 2670, 460,
        {"CN", "EU", "BD"}, {"Mi Home"}});
    
    m_profiles.push_back({"xiaomi-14pro-1", "Xiaomi", "23116PN5BC", "shennong", "Xiaomi 14 Pro",
        "14", "14", "Xiaomi/shennong/shennong:14/V816.20241219.0/S0000.231103.0:user/release-keys",
        "V816.20241219.0", "snapdragon8gen3", "shennong", 16, 1024, 1440, 3200, 522,
        {"CN", "EU"}, {"Mi Store"}});
    
    m_profiles.push_back({"xiaomi-13-1", "Xiaomi", "2211133C", "fuxi", "Xiaomi 13",
        "13", "14", "Xiaomi/fuxi/fuxi:13/V816.20231219.0/S0000.221103.0:user/release-keys",
        "V816.20231219.0", "snapdragon8gen2", "fuxi", 12, 256, 1080, 2400, 414,
        {"CN", "EU", "BD"}, {"Xiaomi"}});
    
    m_profiles.push_back({"xiaomi-13ultra-1", "Xiaomi", "2304FPN6DC", "ishi", "Xiaomi 13 Ultra",
        "13", "14", "Xiaomi/ishi/ishi:13/V816.20231219.0/S0000.230301.0:user/release-keys",
        "V816.20231219.0", "snapdragon8gen2", "ishi", 16, 512, 1440, 3200, 522,
        {"CN", "EU"}, {"Mi"}});
    
    m_profiles.push_back({"redmi-note13pro-1", "Xiaomi", "23129RAAEG", "garnet", "Redmi Note 13 Pro+ 5G",
        "13", "14", "Xiaomi/garnet/garnet:13/V816.20231201.0/S0000.231003.0:user/release-keys",
        "V816.20231201.0", "mediTek7200Ultra", "garnet", 12, 512, 1220, 2712, 446,
        {"CN", "IN", "BD"}, {"Xiaomi"}});
    
    m_profiles.push_back({"redmi-note12-1", "Xiaomi", "22111317AI", "mojito", "Redmi Note 12",
        "12", "13", "Xiaomi/mojito/mojito:12/V816.20230501.0/S0000.230301.0:user/release-keys",
        "V816.20230501.0", "snapdragon685", "mojito", 8, 128, 1080, 2400, 395,
        {"CN", "IN", "BD"}, {"Xiaomi"}});
    
    m_profiles.push_back({"poco-f5-1", "Xiaomi", "23049PCD8G", "marble", "POCO F5",
        "13", "14", "Xiaomi/marble/marble:13/V816.20230801.0/S0000.230617.0:user/release-keys",
        "V816.20230801.0", "snapdragon7plusgen2", "marble", 12, 256, 1080, 2400, 394,
        {"IN", "EU", "BD"}, {"POCO"}});
}

void DeviceProfileDatabase::addOnePlusDevices() {
    m_profiles.push_back({"oneplus-12-1", "OnePlus", "CPH2573", "porsche", "OnePlus 12",
        "14", "14", "OnePlus/porsche/porsche:14/TP1A.200720.012/OP575L1S0EU1:user/release-keys",
        "OP575L1S0EU1", "snapdragon8gen3", "porsche", 16, 512, 1440, 3168, 510,
        {"EU", "US", "BD"}, {"OnePlus"}});
    
    m_profiles.push_back({"oneplus-11-1", "OnePlus", "CPH2449", "samsung", "OnePlus 11",
        "13", "14", "OnePlus/samsung/samsung:13/TP1A.200720.012/OP569L1S0EU1:user/release-keys",
        "OP569L1S0EU1", "snapdragon8gen2", "samsung", 16, 256, 1440, 3216, 525,
        {"EU", "US"}, {"OnePlus"}});
    
    m_profiles.push_back({"oneplus-nord3-1", "OnePlus", "CPH2493", "dimitra", "OnePlus Nord 3",
        "13", "14", "OnePlus/dimitra/dimitra:13/TP1A.200720.012/OP569L3S0EU1:user/release-keys",
        "OP569L3S0EU1", "mediTek9000", "dimitra", 8, 256, 1240, 2772, 451,
        {"EU", "IN", "BD"}, {"OnePlus"}});
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
    return {"Samsung", "Google", "Xiaomi", "OnePlus"};
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
