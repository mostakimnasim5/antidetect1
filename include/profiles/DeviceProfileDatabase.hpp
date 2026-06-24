#pragma once

/**
 * DeviceProfileDatabase - Realistic Device Profile Database
 * 
 * Contains 50+ real device profiles from Samsung, Google, Xiaomi, etc.
 */

#include "../VirtualPhonePro.hpp"
#include <initializer_list>

namespace VirtualPhonePro {

class DeviceProfileDatabase {
public:
    DeviceProfileDatabase();
    ~DeviceProfileDatabase();
    
    // Access
    std::vector<DeviceProfile> getAll() const;
    std::optional<DeviceProfile> getById(const std::string& id) const;
    std::vector<DeviceProfile> getByManufacturer(const std::string& manufacturer) const;
    std::vector<DeviceProfile> getBySeries(const std::string& series) const;
    std::vector<DeviceProfile> getByRegion(const std::string& region) const;
    std::optional<DeviceProfile> getRandom() const;
    
    // Search
    std::vector<DeviceProfile> search(const std::string& query) const;
    
    // Statistics
    size_t count() const;
    std::vector<std::string> getManufacturers() const;
    std::vector<std::string> getSeries() const;

private:
    void initializeDatabase();
    void addSamsungDevices();
    void addGoogleDevices();
    void addXiaomiDevices();
    void addOnePlusDevices();
    void addOppoDevices();
    void addVivoDevices();
    void addRealmeDevices();
    void addSamsungGalaxySeries();
    void addXiaomiMiSeries();
    void addXiaomiRedmiSeries();
    void addPocoDevices();
    void addSonyXperiaDevices();
    void addLGDevices();
    void addMotorolaDevices();
    void addAsusDevices();
    void addNokiaDevices();
    void addHuaweiDevices();
    void addHonorDevices();
    void addNothingDevices();
    
    std::vector<DeviceProfile> m_profiles;
};

class CarrierDatabase {
public:
    CarrierDatabase();
    ~CarrierDatabase();
    
    std::vector<CarrierProfile> getAll() const;
    std::optional<CarrierProfile> getById(const std::string& id) const;
    std::vector<CarrierProfile> getByCountry(const std::string& country) const;
    std::optional<CarrierProfile> getRandom() const;
    std::optional<CarrierProfile> getByMCCMNC(const std::string& mcc, const std::string& mnc) const;
    
private:
    void initializeDatabase();
    void addUSCarriers();
    void addEuropeCarriers();
    void addAsiaCarriers();
    void addBangladeshCarriers();
    void addIndiaCarriers();
    void addMiddleEastCarriers();
    
    std::vector<CarrierProfile> m_carriers;
};

class RegionDatabase {
public:
    RegionDatabase();
    ~RegionDatabase();
    
    std::vector<RegionProfile> getAll() const;
    std::optional<RegionProfile> getById(const std::string& id) const;
    std::vector<RegionProfile> getByCountry(const std::string& countryCode) const;
    std::optional<RegionProfile> getByTimezone(const std::string& timezone) const;
    std::optional<RegionProfile> getRandom() const;
    
private:
    void initializeDatabase();
    void addAsiaPacific();
    void addEurope();
    void addAmericas();
    void addMiddleEast();
    void addAfrica();
    
    std::vector<RegionProfile> m_regions;
};

} // namespace VirtualPhonePro
